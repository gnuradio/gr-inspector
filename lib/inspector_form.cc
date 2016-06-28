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

#include "inspector_form.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <qwt_transform.h>
#include <pmt/pmt.h>


namespace gr {
  namespace inspector {

    inspector_form::inspector_form(int fft_len, std::vector<double> *buffer,
                                   std::vector<std::vector<float> >* rf_map,
                                   bool* manual, gr::msg_queue* msg_queue, QWidget *parent) : QWidget(parent)
    {
      d_fft_len = fft_len;
      // Setup GUI
      //resize(QSize(600,600));
      d_buffer = buffer;
      d_interval = 250;
      d_rf_map = rf_map;
      d_manual = manual;
      d_marker_count = 30;
      d_msg_queue = msg_queue;

      setMouseTracking(true);

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
      d_zoomer = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, d_plot->canvas());
      d_grid->setPen(QPen(QColor(60,60,60),0.5, Qt::DashLine));
      d_grid->attach(d_plot);
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
      d_clicked_marker = NONE;

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

    inspector_form::~inspector_form(){
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
    inspector_form::spawn_signal_selector() {
      detach_markers();
      d_markers[0]->set_marker(0, d_cfreq, d_axis_x[1]/4*d_fft_len*1000000);
      add_msg_queue(d_cfreq, d_axis_x[1]/4*d_fft_len*1000000);
    }

    void
    inspector_form::manual_cb_clicked(int state) {
      *d_manual = static_cast<bool>(state);
      if(*d_manual) {
        spawn_signal_selector();
      }
      else {
        detach_markers();
      }
    }

    void
    inspector_form::detach_markers() {
      d_plot->detachItems( QwtPlotItem::Rtti_PlotMarker, false);
      d_plot->detachItems( QwtPlotItem::Rtti_PlotZone, false);
    }


    void
    inspector_form::resizeEvent( QResizeEvent * event ){
      d_plot->setGeometry(0,0,this->width(),this->height());
    }

    void
    inspector_form::set_axis_x(float start, float stop) {
      d_axis_x.clear();
      d_axis_x.push_back((d_cfreq + start)/1000000);
      d_axis_x.push_back((stop-start)/d_fft_len/1000000);
      d_axis_x.push_back((d_cfreq + stop)/1000000);

      d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[2]);

      for(int i = 0; i < d_fft_len; i++) {
        d_freq[i] = d_axis_x[0] + i*d_axis_x[1];
      }
      refresh();
      QRectF zbase = d_zoomer->zoomBase();
      d_zoomer->zoom(zbase);
      d_zoomer->setZoomBase(zbase);
      d_zoomer->setZoomBase(true);
      d_zoomer->zoom(0);
    }

    void
    inspector_form::set_cfreq(float freq) {
      d_cfreq = freq;
    }

    void
    inspector_form::mousePressEvent(QMouseEvent *eventPress) {
      if(*d_manual && eventPress->button() == Qt::LeftButton && eventPress->modifiers() != Qt::ControlModifier) {
        std::cout << d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().minValue())+60 << ", " <<
          eventPress->x() << std::endl;
        if(d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().minValue())+67 <= eventPress->x() &&
                d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().maxValue())+67 >= eventPress->x()) {
          d_zoomer->setEnabled(false);
          d_clicked_marker = CENTER;
        }
        else if(std::abs(d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().minValue()) - eventPress->x() + 67) < 3) {
          d_zoomer->setEnabled(false);
          d_clicked_marker = LEFT;
        }
        else if(std::abs(d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().maxValue()) - eventPress->x() + 67) < 3) {
          d_zoomer->setEnabled(false);
          d_clicked_marker = RIGHT;
        }
        else {
          d_clicked_marker = NONE;
        }
      }
    }

    void
    inspector_form::mouseMoveEvent(QMouseEvent *eventMove) {
      if (d_clicked_marker != NONE &&
          eventMove->buttons() == Qt::LeftButton) {
        double xVal = d_plot->invTransform(QwtPlot::xBottom,
                                           eventMove->x() - 67);
        float cfreq = d_markers[0]->d_freq;
        float bandwidth = d_markers[0]->d_bw;
        detach_markers();
        if (d_clicked_marker == CENTER) {
          d_markers[0]->set_marker(0, xVal * 1000000, bandwidth);
        }
        else if (d_clicked_marker == LEFT or
                 d_clicked_marker == RIGHT) {
          d_markers[0]->set_marker(0, cfreq, 2 * std::abs(
                  cfreq - xVal * 1000000));
        }
        if(d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().minValue())+67 <= eventMove->x() &&
           d_plot->transform(QwtPlot::xBottom, d_markers[0]->d_zone->interval().maxValue())+67 >= eventMove->x() && *d_manual) {
          //d_plot->canvas()->setCursor(Qt::SizeHorCursor);
        }
        else {
          d_plot->canvas()->setCursor(Qt::CrossCursor);
        }
      }
    }

    void
    inspector_form::mouseReleaseEvent(QMouseEvent *eventRelease) {
      if(d_clicked_marker != NONE && eventRelease->button() == Qt::LeftButton) {
        double xVal = d_plot->invTransform(QwtPlot::xBottom, eventRelease->x()-67);
        float cfreq = d_markers[0]->d_freq;
        float bandwidth = d_markers[0]->d_bw;
        detach_markers();
        if(d_clicked_marker == CENTER) {
          d_markers[0]->set_marker(0, xVal*1000000, bandwidth);
        }
        else if (d_clicked_marker == LEFT or d_clicked_marker == RIGHT) {
          d_markers[0]->set_marker(0, cfreq, 2*std::abs(cfreq-xVal*1000000));
        }
        d_clicked_marker = NONE;
        d_zoomer->setEnabled(true);
        add_msg_queue(cfreq, bandwidth);
      }
    }

    void
    inspector_form::add_msg_queue(float cfreq, float bandwidth) {
      gr::message::sptr msg = gr::message::make(0, cfreq, bandwidth, 1);
      d_msg_queue->flush();
      d_msg_queue->insert_tail(msg);
    }

    void
    inspector_form::msg_received() {
      if(!*d_manual) {
        drawOverlay();
      }
    }

    void
    inspector_form::drawOverlay() {
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
    inspector_form::refresh(){
      gr::thread::scoped_lock guard(d_mutex);

      if(d_buffer->size() < d_fft_len) {
        return;
      }

      d_curve->setRawSamples(d_freq, &d_buffer->at(0), d_fft_len);

      // Do replot
      d_plot->replot();
      d_plot->repaint();
    }

  }
}