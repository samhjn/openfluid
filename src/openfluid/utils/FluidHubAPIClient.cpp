/*

  This file is part of OpenFLUID software
  Copyright(c) 2007, INRA - Montpellier SupAgro


 == GNU General Public License Usage ==

  OpenFLUID is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenFLUID is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenFLUID. If not, see <http://www.gnu.org/licenses/>.


 == Other Usage ==

  Other Usage means a use of OpenFLUID that is inconsistent with the GPL
  license, and requires a written agreement between You and INRA.
  Licensees for Other Usage of OpenFLUID may use this file in accordance
  with the terms contained in the written agreement between You and INRA.

*/


/**
  @file FluidHubAPIClient.cpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@inra.fr>
  @author Armel THONI <armel.thoni@inrae.fr>
*/


#include <openfluid/thirdparty/JSON.hpp>
#include <openfluid/utils/FluidHubAPIClient.hpp>


namespace openfluid { namespace utils {


template<class T>
void JSONArrayToStringSet(const openfluid::thirdparty::json& Obj, std::set<T>& Set)
{
  Set.clear();

  if (Obj.is_array() && !Obj.empty())
  {
    for (unsigned int i=0;i<Obj.size();i++)
    {
      if (Obj[i].is_string())
      {
        Set.insert(T(Obj[i].get<std::string>().c_str()));
      }
    }
  }
}


// =====================================================================
// =====================================================================


void JSONArrayToStringVector(const openfluid::thirdparty::json& Obj, std::vector<std::string>& Vector)
{
  Vector.clear();

  if (Obj.is_array() && !Obj.empty())
  {
    for (unsigned int i=0;i<Obj.size();i++)
    {
      if (Obj[i].is_string())
      {
        Vector.push_back(Obj[i].get<std::string>());
      }
    }
  }
}


// =====================================================================
// =====================================================================


void JSONObjectToDetailedWares(const openfluid::thirdparty::json& Obj,
                               FluidHubAPIClient::WaresDetailsByID_t& WMap, bool IsV0ofAPI=true)
{
  WMap.clear();

  if (Obj.is_object())
  {
    std::string DASH, DescriptionKey;
    if (IsV0ofAPI)
    {
      DASH = "-";
      DescriptionKey = "shortdesc";
    }
    else
    {
      DASH = "_";
      DescriptionKey = "description";
    }
    
    std::string GitUrlKey = "git"+DASH+"url";
    std::string GitBranchesKey = "git"+DASH+"branches";
    std::string IssuesKey = "issues"+DASH+"counts";
    std::string UsersROKey = "users"+DASH+"ro";
    std::string UsersRWKey = "users"+DASH+"rw";

    for (const auto& [WareID,WareInfo] : Obj.items())
    {
      if (WareInfo.is_object())
      {
        WMap[WareID] = FluidHubAPIClient::WareDetailedDescription();

        if (WareInfo.contains(DescriptionKey) && WareInfo[DescriptionKey].is_string())
        {
          WMap[WareID].ShortDescription = WareInfo[DescriptionKey];
        }

        if (WareInfo.contains(GitUrlKey) && WareInfo[GitUrlKey].is_string())
        {
          WMap[WareID].GitUrl = WareInfo[GitUrlKey];
        }

        if (WareInfo.contains(GitBranchesKey) && WareInfo[GitBranchesKey].is_array())
        {
          JSONArrayToStringVector(WareInfo[GitBranchesKey],WMap[WareID].GitBranches);
        }

        if (WareInfo.contains(IssuesKey) && WareInfo[IssuesKey].is_object())
        {
          for (const auto& [k,v] : WareInfo[IssuesKey].items())
          {
            if (v.is_number_integer())
            {
              WMap[WareID].IssuesCounters[k] = v.get<int>();
            }
          }
        }

        if (WareInfo.contains(UsersROKey) && WareInfo[UsersROKey].is_array())
        {
          JSONArrayToStringSet(WareInfo[UsersROKey],WMap[WareID].ROUsers);
        }

        if (WareInfo.contains(UsersRWKey) && WareInfo[UsersRWKey].is_array())
        {
          JSONArrayToStringSet(WareInfo[UsersRWKey],WMap[WareID].RWUsers);
        }
      }
    }
  }
}


// =====================================================================
// =====================================================================


void FluidHubAPIClient::reset()
{
  m_RESTClient.setBaseURL("");
  m_RESTClient.setSSLConfiguration(RESTClient::SSLConfiguration());
  logout();
  m_RESTClient.resetRawHeaders();
  m_RESTClient.addRawHeader("Content-Type", "application/json");
  m_RESTClient.addRawHeader("Accept", "application/x.openfluid.FLUIDhub+json;version=1.0");
  m_HubAPIVersion.clear();
  m_HubCapabilities.clear();
  m_HubName.clear();
  m_HubStatus.clear();
}


// =====================================================================
// =====================================================================


void FluidHubAPIClient::logout()
{
  m_RESTClient.removeRawHeader("Authorization");
}


// =====================================================================
// =====================================================================


bool FluidHubAPIClient::isCapable(const QString& Capacity) const
{
  return (m_HubCapabilities.find(Capacity) != m_HubCapabilities.end());
}


// =====================================================================
// =====================================================================


QString FluidHubAPIClient::wareTypeToString(openfluid::ware::WareType Type)
{
  if (Type == openfluid::ware::WareType::SIMULATOR)
  {
    return "simulators";
  }
  else if (Type == openfluid::ware::WareType::OBSERVER)
  {
    return "observers";
  }
  else if (Type == openfluid::ware::WareType::BUILDEREXT)
  {
    return "builderexts";
  }

  return "";
}


// =====================================================================
// =====================================================================


bool FluidHubAPIClient::connect(const QString& URL,const RESTClient::SSLConfiguration& SSLConfig)
{
  RESTClient::Reply Reply;
  bool ValidNature = false;

  disconnect();
  m_RESTClient.setBaseURL(URL);
  m_RESTClient.setSSLConfiguration(SSLConfig);

  Reply = m_RESTClient.getResource("/");

  if (Reply.isOK())
  {
    openfluid::thirdparty::json JSONDoc;

    try
    {
      JSONDoc = openfluid::thirdparty::json::parse(Reply.getContent().toStdString());
    }
    catch (openfluid::thirdparty::json::parse_error&)
    {
      return false;
    }

    if (JSONDoc.is_object())
    {
      // deduce API system from API base call content
      for (const auto& [Key, Val] : JSONDoc.items())
      {
        if ((Key == "api-version" || Key == "api_version") && Val.is_string())
        {
          m_HubAPIVersion = QString::fromStdString(Val.get<std::string>());
          m_IsV0ofAPI = m_HubAPIVersion.startsWith("0."); 
        }
      }

      if (m_IsV0ofAPI)
      {
        m_WareCapabilityName = "wareshub";
      }
      else
      {
        m_WareCapabilityName = "wares";
      }

      for (const auto& [Key, Val] : JSONDoc.items())
      {
        if (Key == "nature" && Val.is_string())
        {
          ValidNature = (m_IsV0ofAPI && Val.get<std::string>() == "OpenFLUID FluidHub") || 
                        (Val.get<std::string>() == "OpenFLUID"); // URL is really a FluidHub
        }
        else if (Key == "name" && Val.is_string())
        {
          m_HubName = QString::fromStdString(Val.get<std::string>());
        }
        else if (Key == "status" && Val.is_string())
        {
          m_HubStatus = QString::fromStdString(Val.get<std::string>());
        }
        else if (Key == "capabilities" && Val.is_array())
        {
          JSONArrayToStringSet(Val,m_HubCapabilities);
        }
      }
    }
  }

  if (!ValidNature)
  {
    disconnect();
  }

  return isConnected();
}


// =====================================================================
// =====================================================================


void FluidHubAPIClient::disconnect()
{
  reset();
}


// =====================================================================
// =====================================================================


FluidHubAPIClient::WaresListByType_t FluidHubAPIClient::getAllAvailableWares() const
{
  WaresListByType_t WaresDesc;

  WaresDesc[openfluid::ware::WareType::SIMULATOR] = std::set<openfluid::ware::WareID_t>();
  WaresDesc[openfluid::ware::WareType::OBSERVER] = std::set<openfluid::ware::WareID_t>();
  WaresDesc[openfluid::ware::WareType::BUILDEREXT] = std::set<openfluid::ware::WareID_t>();

  if (isConnected() && isCapable(m_WareCapabilityName))
  {
    RESTClient::Reply Reply = m_RESTClient.getResource("/wares");

    if (Reply.isOK())
    {
      openfluid::thirdparty::json JSONDoc;

      try
      {
        JSONDoc = openfluid::thirdparty::json::parse(Reply.getContent().toStdString());
      }
      catch (openfluid::thirdparty::json::parse_error&)
      {
         return WaresDesc;
      }
     
      if (JSONDoc.is_object())
      {
        for (const auto& [Key,Info] : JSONDoc.items())
        {
          if (Key == "simulators")
          {
            JSONArrayToStringSet(Info,WaresDesc[openfluid::ware::WareType::SIMULATOR]);
          }
          else if (Key == "observers")
          {
            JSONArrayToStringSet(Info,WaresDesc[openfluid::ware::WareType::OBSERVER]);
          }
          else if (Key == "builderexts")
          {
            JSONArrayToStringSet(Info,WaresDesc[openfluid::ware::WareType::BUILDEREXT]);
          }
        }
      }
    }
  }

  return WaresDesc;
}


// =====================================================================
// =====================================================================


std::string fetchFieldFromEndpoint(const RESTClient& Client, const std::string Method, const std::string Url, 
                                   const std::string& WantedKey, const std::string& RequestData="")
{
  RESTClient::Reply Reply;
  if (Method == "GET")
  {
    Reply = Client.getResource(QString::fromStdString(Url));
  }
  else if (Method == "POST")
  {
    Reply = Client.postResource(QString::fromStdString(Url), QString::fromStdString(RequestData));
  }

  if (Reply.isOK())
  {
    openfluid::thirdparty::json JSONDoc;

    try
    {
      JSONDoc = openfluid::thirdparty::json::parse(Reply.getContent().toStdString());
    }
    catch (openfluid::thirdparty::json::parse_error&)
    {
      return "";
    }

    if (JSONDoc.is_object())
    {
      for (const auto& [Key,Val] : JSONDoc.items())
      {

        if (Key == WantedKey)
        {
          if (Val.is_string())
          {
            return Val.get<std::string>();
          }
        }
      }
    }
  }

  return "";
}


// =====================================================================
// =====================================================================


std::string FluidHubAPIClient::getUserUnixname(const std::string& Email, const std::string& Password)
{
  if (!m_IsV0ofAPI && isConnected())
  {
    if (!m_RESTClient.hasRawHeader("Authorization"))
    {
      std::string AccessBody = "{\"email\":\""+Email+"\",\"password\":\""+Password+"\"}";
      std::string AccessToken = fetchFieldFromEndpoint(m_RESTClient, "POST", "/auth/token", "access", AccessBody);
      if (AccessToken != "")
      {
        QString HeaderData = "Bearer " + QString::fromStdString(AccessToken);
        m_RESTClient.addRawHeader("Authorization", HeaderData.toLocal8Bit()); 
      }
    }
    std::string unixname = fetchFieldFromEndpoint(m_RESTClient, "GET", "/account", "unixname");
    return unixname;
  }
  return "";
}
  

// =====================================================================
// =====================================================================


FluidHubAPIClient::WaresDetailsByID_t FluidHubAPIClient::getAvailableWaresWithDetails(openfluid::ware::WareType Type, 
                                                                                      const QString& Username) const
{
  WaresDetailsByID_t WaresDesc = std::map<openfluid::ware::WareID_t,WareDetailedDescription>();

  QString Path = wareTypeToString(Type);

  if (isConnected() && isCapable(m_WareCapabilityName) && !(Path.isEmpty()))
  {
    Path = "/wares/"+Path;

    if (!Username.isEmpty())
    {
      Path += "?username="+Username;
    }

    RESTClient::Reply Reply = m_RESTClient.getResource(Path);

    if (Reply.isOK())
    {
      openfluid::thirdparty::json JSONDoc;

      try
      {
        JSONDoc = openfluid::thirdparty::json::parse(Reply.getContent().toStdString());
      }
      catch (openfluid::thirdparty::json::parse_error&)
      {
        return WaresDesc;
      }

      if (JSONDoc.is_object())
      {
        JSONObjectToDetailedWares(JSONDoc, WaresDesc, m_IsV0ofAPI);
      }
    }
  }

  return WaresDesc;
}


// =====================================================================
// =====================================================================


QString FluidHubAPIClient::getNews(const QString& Lang) const
{
  QString Content;

  if (isConnected() && isCapable("news"))
  {
    QString Path = "/news";

    if (!Lang.isEmpty())
    {
      Path += "?lang="+Lang;
    }

    RESTClient::Reply Reply = m_RESTClient.getResource(Path);

    if (Reply.isOK())
    {
      return Reply.getContent();
    }
  }

  return "";
}


} }  // namespaces
