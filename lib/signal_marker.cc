/* -*- c++ -*- */
/*
 * Copyright 2019 Free Software Foundation, Inc..
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "signal_marker.h"

namespace gr {
  namespace inspector {

    signal_marker::signal_marker(int i, QwtPlot *plot) {
      d_plot = plot;
      d_number = i;
      d_center = new QwtPlotMarker();
      d_label = new QwtPlotMarker();
      d_zone = new QwtPlotZoneItem();
    }

    signal_marker::~signal_marker() {
      delete d_center;
      delete d_label;
      delete d_zone;
    }

    void
    signal_marker::set_marker(int i, float center, float bw, int unit) {
      d_number = i;
      d_freq = center;
      d_bw = bw;
      d_unit = unit;
      d_center->setLineStyle(QwtPlotMarker::VLine);
      QColor c = Qt::white;
      c.setAlpha(70);
      d_center->setLinePen(c);
      d_center->setXValue(center/unit);

      QString unittxt = "Hz";
      switch(unit) {
        case 1:
          d_unittxt = "Hz";
          break;
        case 1000:
          d_unittxt = "kHz";
          break;
        case 1000000:
          d_unittxt = "MHz";
          break;
        case 1000000000:
          d_unittxt = "GHz";
          break;
        default:
          d_unittxt = "Hz";
      }

      set_label_text(basic_text());

      d_zone->setOrientation(Qt::Vertical);
      c = Qt::red;
      c.setAlpha(100);
      d_zone->setPen(c);
      c.setAlpha(20);
      d_zone->setBrush(c);
      d_zone->setInterval((center-bw/2)/unit, (center+bw/2)/unit);
      d_zone->setXAxis(QwtPlot::xBottom);

      d_label->attach(d_plot);
      d_zone->attach(d_plot);
      d_center->attach(d_plot);
    }

    void
    signal_marker::set_label_y(qreal yval) {
      d_label->setYValue(yval);
    }

    QString
    signal_marker::basic_text() {
      QString qstring;
      qstring.push_back("Signal "+QString::number(d_number+1));
      qstring.append("\n");
      qstring.append("f = "+QString::number(d_freq/d_unit)+" "+d_unittxt);
      qstring.append("\n");
      qstring.append("B = "+QString::number(d_bw/d_unit)+" "+d_unittxt);
      return qstring;
    }

    void
    signal_marker::set_label_text(QString qstring) {
      QwtText qwttext;
      qwttext.setText(qstring);
      qwttext.setColor(Qt::red);
      d_label->label();
      d_label->setLabelAlignment(Qt::AlignLeft);
      d_label->setLabel(qwttext);
      d_label->setXValue((d_freq-d_bw/2-300)/d_unit);
      d_label->setYValue(13);
    }

    void
    signal_marker::add_text(std::string text) {
      QString qstring = basic_text();
      qstring.append("\n");
      qstring.append(text.c_str());
      set_label_text(qstring);
    }
  } /* namespace inspector */
} /* namespace gr */
