/*
 This file is part of OpenFLUID software
 Copyright (c) 2007-2010 INRA-Montpellier SupAgro


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
 along with OpenFLUID.  If not, see <http://www.gnu.org/licenses/>.

 In addition, as a special exception, INRA gives You the additional right
 to dynamically link the code of OpenFLUID with code not covered
 under the GNU General Public License ("Non-GPL Code") and to distribute
 linked combinations including the two, subject to the limitations in this
 paragraph. Non-GPL Code permitted under this exception must only link to
 the code of OpenFLUID dynamically through the OpenFLUID libraries
 interfaces, and only for building OpenFLUID plugins. The files of
 Non-GPL Code may be link to the OpenFLUID libraries without causing the
 resulting work to be covered by the GNU General Public License. You must
 obey the GNU General Public License in all respects for all of the
 OpenFLUID code and other code used in conjunction with OpenFLUID
 except the Non-GPL Code covered by this exception. If you modify
 this OpenFLUID, you may extend this exception to your version of the file,
 but you are not obligated to do so. If you do not wish to provide this
 exception without modification, you must delete this exception statement
 from your version and license this OpenFLUID solely under the GPL without
 exception.


 == Other Usage ==

 Other Usage means a use of OpenFLUID that is inconsistent with the GPL
 license, and requires a written agreement between You and INRA.
 Licensees for Other Usage of OpenFLUID may use this file in accordance
 with the terms contained in the written agreement between You and INRA.
 */

/**
 \file BuilderAppModule.cpp
 \brief Implements ...

 \author Aline LIBRES <libres@supagro.inra.fr>
 */

#include "BuilderAppModule.hpp"

#include <boost/filesystem/operations.hpp>

#include <openfluid/guicommon/PreferencesManager.hpp>
#include <openfluid/base/RuntimeEnv.hpp>

#include "builderconfig.hpp"
#include "BuilderAppCoordinator.hpp"
#include "BuilderAppWindow.hpp"
#include "BuilderAppActions.hpp"
#include "BuilderWorkdirCreationDialog.hpp"
#include "FunctionSignatureRegistry.hpp"
#include "BuilderExtensionsManager.hpp"

// =====================================================================
// =====================================================================


BuilderAppModule::BuilderAppModule() :
  m_MainWindow(*new BuilderAppWindow()), m_Actions(*new BuilderAppActions())
{
  mp_Coordinator = new BuilderAppCoordinator(m_MainWindow, m_Actions);
}

// =====================================================================
// =====================================================================


bool BuilderAppModule::initialize()
{
  openfluid::guicommon::PreferencesManager* PrefMgr =
      openfluid::guicommon::PreferencesManager::getInstance();

  openfluid::base::RuntimeEnvironment* RunEnv =
      openfluid::base::RuntimeEnvironment::getInstance();

  BuilderExtensionsManager* ExtMgr = BuilderExtensionsManager::getInstance();


  // Checking working directory

  std::string WorkDirFromPref = PrefMgr->getWorkdir();
  if (!boost::filesystem::exists(WorkDirFromPref))
  {
    BuilderWorkdirCreationDialog Dialog;
    if (!Dialog.show())
      return false;
  }


  // Checking extra plugin paths (for pluggable functions and extensions)

  std::vector<Glib::ustring> PrefXPaths = PrefMgr->getExtraPlugPaths();

  for (int i = PrefXPaths.size() - 1; i > -1; i--)
  {
    RunEnv->addExtraPluginsPaths(PrefXPaths[i]);
    ExtMgr->prependExtensionSearchPath(PrefXPaths[i]);
  }

  // Setting pluggable functions

  FunctionSignatureRegistry::getInstance()->updatePluggableSignatures();


  // Setting extensions

  //TODO add HomeLaucher
  ExtMgr->prependExtensionSearchPath(Glib::ustring::compose("%1/%2",
      RunEnv->getInstallPrefix(), BUILDEREXT_INSTALL_PATH));
  ExtMgr->prependExtensionSearchPath(RunEnv->getUserDataPath(
      BUILDER_EXTSDIR));
  ExtMgr->registerExtensions();

  mp_Coordinator->configExtensionsMenus();



  mp_Coordinator->setHomeModule();

  return true;

}

// =====================================================================
// =====================================================================


Gtk::Window& BuilderAppModule::composeAndGetAsWindow()
{
  compose();
  return m_MainWindow;
}

// =====================================================================
// =====================================================================


void BuilderAppModule::compose()
{
  m_MainWindow.setMenuBarWidget(*m_Actions.getMenuBarWidget());
  m_MainWindow.setToolBarWidget(*m_Actions.getToolBarWidget());
  m_MainWindow.addAccelGroup(m_Actions.getAccelGroup());
}

// =====================================================================
// =====================================================================


Gtk::Widget* BuilderAppModule::asWidget()
{
  return (Gtk::Widget*) 0;
}
