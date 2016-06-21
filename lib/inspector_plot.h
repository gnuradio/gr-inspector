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

#ifndef GR_INSPECTOR_INSPECTOR_SINK_H
#define GR_INSPECTOR_INSPECTOR_SINK_H

#include <gnuradio/thread/thread.h>

#include <complex>

#include <QApplication>
#include <QTimer>
#include <QWidget>
#include <QtGui/QtGui>

#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_painter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>

#include <signal_marker.h>

namespace gr {
  namespace inspector {

    class inspector_plot : public QWidget
    {
    Q_OBJECT

    public:
      inspector_plot(int fft_len, std::vector<double> *buffer, std::vector<std::vector<float> >* rf_map,
                     bool* manual, QWidget* parent = NULL);
      ~inspector_plot();

    private:
      int d_interval, d_fft_len, d_marker_count;
      bool *d_manual;
      std::vector<float> d_axis_x, d_axis_y;
      std::vector<double> *d_buffer;
      float d_max, d_min, d_cfreq;
      double* d_freq;
      std::vector<std::vector<float> >* d_rf_map;
      QwtPlotMarker* d_clicked_marker;

      QwtSymbol *d_symbol;
      QwtPlotZoomer* d_zoomer;
      QwtPlot *d_plot;
      QwtScaleWidget *d_scale;
      QwtPlotCurve *d_curve;
      QTimer *d_timer;
      QwtPlotGrid* d_grid;
      QGridLayout *d_layout;
      QList<signal_marker*> d_markers;
      QCheckBox* d_manual_cb;

      gr::thread::mutex d_mutex;

    protected:
      void resizeEvent(QResizeEvent * event);

    public slots:
      void refresh();

      void set_axis_x(float start, float stop);
      void msg_received();
      void set_cfreq(float freq);
      void detach_markers();
      void mousePressEvent (QMouseEvent * eventPress);
      void mouseReleaseEvent(QMouseEvent *eventRelease);
      void manual_cb_clicked(int state);
      void spawn_signal_selector();


      void drawOverlay();

    };


  }
}

#endif //GR_INSPECTOR_INSPECTOR_SINK_H