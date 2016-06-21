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
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_zoneitem.h>

namespace gr {
  namespace inspector {

    class signalMarker
    {
    public:
      signalMarker(int i, float center, float bw, QwtPlot* plot) {
        d_plot = plot;
        d_number = i;
        d_freq = center;
        d_bw = bw;
        d_center = new QwtPlotMarker();
        d_center->setLineStyle(QwtPlotMarker::VLine);
        QColor c = Qt::white;
        c.setAlpha(70);
        d_center->setLinePen(c);
        d_center->setXValue(center/1000000);

        d_label = new QwtPlotMarker();
        QwtText text;
        QString qstring;
        qstring.push_back("Signal "+QString::number(i+1));
        qstring.append("\n");
        qstring.append("f = "+QString::number(center/1000000)+" M");
        qstring.append("\n");
        qstring.append("B = "+QString::number(bw/1000)+" k");
        text.setText(qstring);
        text.setColor(Qt::red);
        d_label->setLabelAlignment(Qt::AlignLeft);
        d_label->setLabel(text);
        d_label->setXValue((center-bw/2-300)/1000000);
        d_label->setYValue(13);

        d_zone = new QwtPlotZoneItem();
        d_zone->setOrientation(Qt::Vertical);
        c = Qt::red;
        c.setAlpha(100);
        d_zone->setPen(c);
        c.setAlpha(20);
        d_zone->setBrush(c);
        d_zone->setInterval((center-bw/2)/1000000, (center+bw/2)/1000000);
        d_zone->setXAxis(QwtPlot::xBottom);

        d_label->attach(plot);
        d_zone->attach(plot);
        d_center->attach(plot);
      }

      ~signalMarker() {
      }

      QwtPlotMarker* d_center;
      QwtPlotMarker* d_label;
      QwtPlotZoneItem* d_zone;
      QwtPlot* d_plot;
      float d_freq, d_bw;
      int d_number;
    };

    class inspector_plot : public QWidget
    {
    Q_OBJECT

    public:
      inspector_plot(int fft_len, std::vector<double> *buffer, std::vector<std::vector<float> >* rf_map,
                     bool* ready, bool* manual, QWidget* parent = NULL);
      ~inspector_plot();

    private:
      int d_interval, d_fft_len;
      bool* d_ready, *d_manual;
      bool d_marker_ready;
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
      QList<signalMarker*> d_markers;

      gr::thread::mutex d_mutex;

    protected:
      void resizeEvent(QResizeEvent * event);

    public slots:
      void refresh();

      void set_axis_x(float start, float stop);
      void msg_received();
      void set_cfreq(float freq);
      void delete_markers();
      void mousePressEvent (QMouseEvent * eventPress);
      void mouseReleaseEvent(QMouseEvent *eventRelease);


      void drawOverlay();

    };


  }
}

#endif //GR_INSPECTOR_INSPECTOR_SINK_H