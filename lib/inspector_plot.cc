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

#include "inspector_plot.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <qwt_transform.h>


namespace gr {
  namespace inspector {

    inspector_plot::inspector_plot(int fft_len, std::vector<double> *buffer,
                                   std::vector<std::vector<float> >* rf_map,
                                   bool* manual, QWidget* parent) : QWidget(parent)
    {
      d_fft_len = fft_len;
      // Setup GUI
      //resize(QSize(600,600));
      d_buffer = buffer;
      d_interval = 250;
      d_rf_map = rf_map;
      d_manual = manual;
      d_marker_count = 30;

      // spawn all QT stuff
      d_layout = new QGridLayout(this);
      d_plot = new QwtPlot(this); // make main plot
      d_curve = new QwtPlotCurve(); // make curve plot
      d_curve->attach(d_plot); // attach curve to plot
      d_curve->setPen(Qt::cyan, 1); // curve color
      d_symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(QColor(Qt::blue)),
                               QPen(QColor(Qt::blue)), QSize(7,7));
      d_curve->setSymbol(d_symbol);
      d_grid = new QwtPlotGrid();
      d_grid->setPen(QPen(QColor(60,60,60),0.5, Qt::DashLine));
      d_grid->attach(d_plot);
      //d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
      //d_zoomer = new QwtPlotZoomer(d_plot);
      d_layout->addWidget(d_plot, 0, 0);
      d_layout->setColumnStretch(0, 1);
      setLayout(d_layout);
      // Plot axis and title
      std::string label_title = "Inspector GUI";
      d_plot->setTitle(QwtText(label_title.c_str()));
      d_plot->setAxisTitle(QwtPlot::xBottom, "Frequency [MHz]");
      d_plot->setAxisTitle(QwtPlot::yLeft, "dB");
      d_plot->setAxisScale(QwtPlot::yLeft, -120, 30);
      d_plot->setCanvasBackground(QColor(30,30,30));
      //d_plot->show();
      // Do replot
      d_plot->replot();

      // frequency vector for plot
      d_freq = new double[fft_len];
      d_clicked_marker = NULL;

      // Setup timer and connect refreshing plot
      d_timer = new QTimer(this);
      connect(d_timer, SIGNAL(timeout()), this, SLOT(refresh()));
      d_timer->start(d_interval);
      qRegisterMetaType<QList<QwtLegendData> >("QList<QwtLegendData>");

      for(int i = 0; i < d_marker_count; i++)
      {
        signal_marker* marker = new signal_marker(i, d_plot);
        d_markers.push_back(marker);
      }


      d_manual_cb = new QCheckBox("Manual", d_plot);
      d_manual_cb->setGeometry(QRect(10,10,85,20)),
      connect(d_manual_cb, SIGNAL(stateChanged(int)), this, SLOT(manual_cb_clicked(int)));
    }

    inspector_plot::~inspector_plot(){
      detach_markers();
      delete d_timer;
      delete d_zoomer;
      delete d_plot;
      delete d_curve;
      delete[] d_freq;
      delete d_symbol;
      delete d_grid;
      delete d_manual_cb;
      while(!d_markers.empty()) {
        delete d_markers.back();
        d_markers.pop_back();
      }
    }

    void
    inspector_plot::spawn_signal_selector() {
      detach_markers();
      d_markers[0]->set_marker(0, d_cfreq, -d_axis_x[0]/2*1000000);
    }

    void
    inspector_plot::manual_cb_clicked(int state) {
      *d_manual = static_cast<bool>(state);
      if(*d_manual) {
        spawn_signal_selector();
      }
    }

    void
    inspector_plot::detach_markers() {
      d_plot->detachItems( QwtPlotItem::Rtti_PlotMarker, false);
      d_plot->detachItems( QwtPlotItem::Rtti_PlotZone, false);
    }


    void
    inspector_plot::resizeEvent( QResizeEvent * event ){
      d_plot->setGeometry(0,0,this->width(),this->height());
    }

    void
    inspector_plot::set_axis_x(float start, float stop) {
      d_axis_x.clear();
      d_axis_x.push_back((d_cfreq + start)/1000000);
      d_axis_x.push_back((stop-start)/d_fft_len/1000000);
      d_axis_x.push_back((d_cfreq + stop)/1000000);

      d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[2]);

      for(int i = 0; i < d_fft_len; i++) {
        d_freq[i] = d_axis_x[0] + i*d_axis_x[1];
      }

      refresh();
    }

    void
    inspector_plot::set_cfreq(float freq) {
      d_cfreq = freq;
    }

    // TODO: compare mouse with marker position and select clicked
    // marker
    void
    inspector_plot::mousePressEvent(QMouseEvent *eventPress) {
      if(eventPress->button() == Qt::LeftButton) {

      }
    }


    // TODO: mouse release event to set new marker position
    void
    inspector_plot::mouseReleaseEvent(QMouseEvent *eventRelease) {
      if(eventRelease->button() == Qt::LeftButton) {
        if(d_clicked_marker != NULL) {
          double xVal = d_plot->invTransform(QwtPlot::xBottom, eventRelease->x());
          d_clicked_marker->setXValue(eventRelease->globalX());

          d_clicked_marker = NULL;
        }
      }
    }

    void
    inspector_plot::msg_received() {
      if(!*d_manual) {
        drawOverlay();
      }
    }

    void
    inspector_plot::drawOverlay() {
      gr::thread::scoped_lock guard(d_mutex);

      detach_markers();
      if(d_rf_map->size() <= d_marker_count) {
        for (int i = 0; i < d_rf_map->size(); i++) {
          d_markers[i]->set_marker(i, d_cfreq + d_rf_map->at(i)[0],
                                   d_rf_map->at(i)[1]);
        }
      }
      else {
        for (int i = 0; i < d_marker_count; i++) {
          d_markers[i]->set_marker(i, d_cfreq + d_rf_map->at(i)[0],
                                   d_rf_map->at(i)[1]);
        }
      }
    }

    void
    inspector_plot::refresh(){
      gr::thread::scoped_lock guard(d_mutex);

      if(d_buffer->size() < d_fft_len) {
        return;
      }

      d_curve->setRawSamples(d_freq, &d_buffer->at(0), d_fft_len);

      // Do replot
      d_plot->replot();
    }

  }
}