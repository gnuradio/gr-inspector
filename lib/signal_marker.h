/* -*- c++ -*- */
/*
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
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

#ifndef GR_INSPECTOR_SIGNAL_MARKER_H
#define GR_INSPECTOR_SIGNAL_MARKER_H

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoneitem.h>

namespace gr {
  namespace inspector {

    class signal_marker
    {
    public:
      signal_marker(int i, QwtPlot *plot);

      ~signal_marker();

      void set_marker(int i, float center, float bw);

      QwtPlotMarker* d_center;
      QwtPlotMarker* d_label;
      QwtPlotZoneItem* d_zone;
      QwtPlot* d_plot;
      float d_freq, d_bw;
      int d_number;
    };

  }
}

#endif //GR_INSPECTOR_SIGNAL_MARKER_H
