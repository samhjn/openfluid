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
 \file ICLayerPoint.cpp
 \brief Implements ...

 \author Damien CHABBERT <dams.vivien@gmail.com>
 */

#include "ICLayerPoint.hpp"

ICLayerPoint::ICLayerPoint()
{

}

// =====================================================================
// =====================================================================

void ICLayerPoint::drawPoint(Cairo::RefPtr<Cairo::Context> cr,
    OGRGeometry* ObjectGeo, double scale, bool select)
{

  OGRPoint* Point = static_cast<OGRPoint*> (ObjectGeo);
  cr->move_to(Point->getX() + (2 / scale), Point->getY());
  cr->line_to(Point->getX() + (2 / scale), Point->getY() + (2 / scale));
  cr->line_to(Point->getX() - (2 / scale), Point->getY() + (2 / scale));
  cr->line_to(Point->getX() - (2 / scale), Point->getY() - (2 / scale));
  cr->line_to(Point->getX() + (2 / scale), Point->getY() - (2 / scale));
  cr->close_path();
  if (select)
    cr->fill();
  else
    cr->stroke();
}

// =====================================================================
// =====================================================================

void ICLayerPoint::draw(Cairo::RefPtr<Cairo::Context> cr, double scale,
    std::set<int> select, bool DisplayID, double Alpha)
{
  std::map<int, ICLayerObject*>::iterator it;

  for (it = m_ICLayerObject.begin(); it != m_ICLayerObject.end(); it++)
  {
    if ((*it).second->selfIdExisting())
    {
      bool isSelect = false;
      if (!select.empty())
      {
        std::set<int>::iterator it2;
        it2 = select.find((*it).first);
        if (it2 != select.end() && (*it2) == (*it).first)
        {
          drawPoint(cr, (*it).second->getOGRGeometryObject(), scale, true);
          isSelect = true;
        } else
          drawPoint(cr, (*it).second->getOGRGeometryObject(), scale, false);
      } else
        drawPoint(cr, (*it).second->getOGRGeometryObject(), scale, false);
      if (DisplayID)
      {
        Cairo::TextExtents extents;

        std::stringstream str;
        str << (*it).first;
        std::string text = str.str();

        cr->select_font_face("Bitstream Vera Sans, Arial",
            Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
        cr->set_font_size(12 / scale);

        Cairo::FontOptions font_options;

        font_options.set_hint_style(Cairo::HINT_STYLE_NONE);
        font_options.set_hint_metrics(Cairo::HINT_METRICS_OFF);
        font_options.set_antialias(Cairo::ANTIALIAS_GRAY);

        cr->set_font_options(font_options);
        cr->save();

        cr->get_text_extents(text, extents);
        cr->move_to(static_cast<OGRPoint*>((*it).second->getOGRGeometryObject())->getX(),
            static_cast<OGRPoint*>((*it).second->getOGRGeometryObject())->getY());
        cr->scale(1, -1);
        if (isSelect)
          cr->set_source_rgba(0, 0, 0, Alpha);

        cr->show_text(text);
        cr->stroke();
        cr->restore();
      }
    }
  }
}

// =====================================================================
// =====================================================================
// =====================================================================
// =====================================================================

std::pair<std::pair<double, double>, std::pair<double, double> > ICLayerPoint::getMinMax()
{
  std::pair<std::pair<double, double>, std::pair<double, double> > MinMaxTemp;

  double x;
  double y;

  bool first = true;
  std::map<int, ICLayerObject*>::iterator it;
  for (it = m_ICLayerObject.begin(); it != m_ICLayerObject.end(); it++)
  {
    if ((*it).second->selfIdExisting())
    {
      x
          = (static_cast<OGRPoint*> ((*it).second->getOGRGeometryObject()))->getX();
      y
          = (static_cast<OGRPoint*> ((*it).second->getOGRGeometryObject()))->getY();

      if (first)
      {
        (MinMaxTemp.second).first = x;
        (MinMaxTemp.second).second = y;
        (MinMaxTemp.first).first = x;
        (MinMaxTemp.first).second = y;
        first = false;
      } else
      {
        (MinMaxTemp.second).first = std::max((MinMaxTemp.second).first, x);
        (MinMaxTemp.second).second = std::max((MinMaxTemp.second).second, y);
        (MinMaxTemp.first).first = std::min((MinMaxTemp.first).first, x);
        (MinMaxTemp.first).second = std::min((MinMaxTemp.first).second, y);
      }
    }
  }
  return MinMaxTemp;
}

// =====================================================================
// =====================================================================

std::pair<std::pair<double, double>, std::pair<double, double> > ICLayerPoint::getMinMax(
    std::set<int> TempSet)
{
  std::pair<std::pair<double, double>, std::pair<double, double> > MinMaxTemp;

  double x;
  double y;

  bool first = true;
  std::map<int, ICLayerObject*>::iterator it;
  for (it = m_ICLayerObject.begin(); it != m_ICLayerObject.end(); it++)
  {
    std::set<int>::iterator ite;
    ite = TempSet.find((*it).first);
    if ((*it).second->selfIdExisting() && ite != TempSet.end())
    {
      x
          = (static_cast<OGRPoint*> ((*it).second->getOGRGeometryObject()))->getX();
      y
          = (static_cast<OGRPoint*> ((*it).second->getOGRGeometryObject()))->getY();

      if (first)
      {
        (MinMaxTemp.second).first = x;
        (MinMaxTemp.second).second = y;
        (MinMaxTemp.first).first = x;
        (MinMaxTemp.first).second = y;
        first = false;
      } else
      {
        (MinMaxTemp.second).first = std::max((MinMaxTemp.second).first, x);
        (MinMaxTemp.second).second = std::max((MinMaxTemp.second).second, y);
        (MinMaxTemp.first).first = std::min((MinMaxTemp.first).first, x);
        (MinMaxTemp.first).second = std::min((MinMaxTemp.first).second, y);
      }
    }
  }
  return MinMaxTemp;
}

// =====================================================================
// =====================================================================
// =====================================================================
// =====================================================================


int ICLayerPoint::isSelected(double x, double y, double scale)
{
  std::map<int, ICLayerObject*>::iterator it;
  for (it = m_ICLayerObject.begin(); it != m_ICLayerObject.end(); it++)
  {
    double X =
        static_cast<OGRPoint*> ((*it).second->getOGRGeometryObject())->getX();
    double Y =
        static_cast<OGRPoint*> ((*it).second->getOGRGeometryObject())->getY();
    if ((X - (2 / scale)) <= x && (X + (2 / scale)) >= x && (Y - (2 / scale))
        <= y && (Y + (2 / scale)) >= y)
    {
      //std::cout << " -> " <<  (*it).first << std::endl;
      return (*it).first;
    }
  }
  return -1;

}

