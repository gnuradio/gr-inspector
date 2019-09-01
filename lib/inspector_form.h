/* -*- c++ -*- */
/*
 * Copyright 2019 Free Software Foundation, Inc.
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

#include <QTimer>
#include <QtWidgets>
#include <QtGui>
#include <gnuradio/msg_queue.h>

#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>

#include "signal_marker.h"

namespace gr {
  namespace inspector {

    class Zoomer: public QwtPlotZoomer
    {
    public:

      Zoomer(int xAxis, int yAxis, QWidget *canvas):
              QwtPlotZoomer(xAxis, yAxis, canvas)
      {
        //setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
        setTrackerMode(QwtPicker::AlwaysOn);
        setRubberBand(QwtPicker::RectRubberBand);
        QPen pen = QPen(QColor(100, 100, 100), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
        setTrackerPen(pen);
        setRubberBandPen(pen);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

#if QT_VERSION < 0x040000
        setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlButton);
#else
        setMousePattern(QwtEventPattern::MouseSelect2,
                        Qt::RightButton, Qt::ControlModifier);
#endif
        setMousePattern(QwtEventPattern::MouseSelect3,
                        Qt::RightButton);
        setMousePattern(QwtEventPattern::MouseSelect1,
                        Qt::LeftButton, Qt::ControlModifier);

      }
    };

    class inspector_form : public QWidget
    {
    Q_OBJECT

    public:
      void set_axis_x(float start, float stop);
      void msg_received();
      void set_cfreq(float freq);
      void detach_markers();
      void mousePressEvent (QMouseEvent * eventPress);
      void mouseReleaseEvent(QMouseEvent *eventRelease);
      void mouseMoveEvent(QMouseEvent *eventMove);
      void spawn_signal_selector();
      void add_msg_queue(float cfreq, float bandwidth);
      float freq_to_x(float freq);
      float x_to_freq(float x);
      void add_analysis_text(int signal, std::string text);

      void drawOverlay();
      void draw_analysis_text();
      inspector_form(int fft_len, std::vector<double> *buffer, std::vector<std::vector<float> >* rf_map,
                     bool* manual, gr::msg_queue* msg_queue, int *rf_unit, QWidget *parent);
      ~inspector_form();

    private:

      enum markerType {
        NONE,
        LEFT,
        CENTER,
        RIGHT
      };

      bool change_flag;
      int d_interval, d_fft_len, d_marker_count;
      int *d_rf_unit;
      bool *d_manual;
      std::vector<float> d_axis_x, d_axis_y;
      std::vector<double> *d_buffer;
      float d_max, d_min, d_cfreq, d_mouse_offset;
      double* d_freq;
      std::map<int, std::string> d_analysis_texts;
      std::vector<std::vector<float> >* d_rf_map;
      markerType d_clicked_marker;
      QList<signal_marker*> d_markers;
      QwtSymbol *d_symbol;
      Zoomer* d_zoomer;
      QwtPlot *d_plot;
      QwtScaleWidget *d_scale;
      QwtPlotCurve *d_curve;
      QTimer *d_timer;
      QwtPlotGrid* d_grid;
      QGridLayout *d_layout;

      QCheckBox* d_manual_cb;
      gr::msg_queue* d_msg_queue;

      gr::thread::mutex d_mutex;

    protected:
      void resizeEvent(QResizeEvent * event);

    public slots:
      void refresh();
      void manual_cb_clicked(int state);

    };


  }
}

#endif //GR_INSPECTOR_INSPECTOR_SINK_H
