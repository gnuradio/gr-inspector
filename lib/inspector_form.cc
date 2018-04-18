/* -*- c++ -*- */
/*
 * Copyright 2016 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
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
#include <qwt_plot_canvas.h>


namespace gr {
  namespace inspector {

    inspector_form::inspector_form(int fft_len,
                                   std::vector<double> *buffer,
                                   std::vector<std::vector<float> >* rf_map,
                                   bool* manual,
                                   gr::msg_queue* msg_queue,
                                   int* rf_unit,
                                   QWidget *parent) : QWidget(parent)
    {
      change_flag = false;
      d_fft_len = fft_len;
      // Setup GUI
      d_buffer = buffer;
      d_interval = 250; // update interval
      d_rf_map = rf_map;
      d_manual = manual;
      d_marker_count = 30; // max number of signal markers
      d_msg_queue = msg_queue;
      d_rf_unit = rf_unit;

      setMouseTracking(true);

      // spawn all QT stuff
      d_layout = new QGridLayout(this);

      d_plot = new QwtPlot(this); // make main plot
      d_plot->setMouseTracking(true); // needed for drag & drop
      d_curve = new QwtPlotCurve(); // make curve plot

      d_curve->attach(d_plot); // attach curve to plot
      d_curve->setPen(Qt::cyan, 1); // curve color
      d_symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(QColor(Qt::blue)),
                               QPen(QColor(Qt::blue)), QSize(7,7));
      d_curve->setSymbol(d_symbol);

      d_grid = new QwtPlotGrid();
      d_grid->setPen(QPen(QColor(60,60,60),0.5, Qt::DashLine));
      d_grid->attach(d_plot); // add grid to plot

      d_layout->addWidget(d_plot, 0, 0);
      d_layout->setColumnStretch(0, 1);
      setLayout(d_layout);
      d_zoomer = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, d_plot->canvas());

      // Plot axis and title
      std::string label_title = "Inspector GUI";
      d_plot->setTitle(QwtText(label_title.c_str()));

      d_plot->setAxisTitle(QwtPlot::yLeft, "dB");
      d_plot->setAxisScale(QwtPlot::yLeft, -120, 30); // should work for all signals
      d_plot->setCanvasBackground(QColor(30,30,30));
      // Set this to avoid segfaults due to synchronous Qt plotting
      ((QwtPlotCanvas*)d_plot->canvas())->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);

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

      d_manual_cb = new QCheckBox("Manual", d_plot); // checkbox for manual signal selection
      d_manual_cb->setGeometry(QRect(10,10,85,20)),
      connect(d_manual_cb, SIGNAL(stateChanged(int)), this, SLOT(manual_cb_clicked(int)));
      d_manual_cb->setChecked(*d_manual);
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
      // initial marker when manual is selected (covers 1/2 bw)
      detach_markers();
      d_markers[0]->set_marker(0,
                  *d_rf_unit*(d_zoomer->zoomRect().x()+ d_zoomer->zoomRect().width()/2), // center of current zoom
                  *d_rf_unit*d_zoomer->zoomRect().width()/2, *d_rf_unit); // half bandwidth of current zoom
      add_msg_queue(d_markers[0]->d_freq-d_cfreq, d_markers[0]->d_bw);
    }

    void
    inspector_form::manual_cb_clicked(int state) {
      // slot to enable manual signal selection
      gr::thread::scoped_lock guard(d_mutex);
      *d_manual = static_cast<bool>(state);
      if(*d_manual) {
        spawn_signal_selector();
      }
      else {
        detach_markers();
        drawOverlay();
      }
    }

    void
    inspector_form::detach_markers() {
      // let markers disappear
      d_plot->detachItems( QwtPlotItem::Rtti_PlotMarker, false);
      d_plot->detachItems( QwtPlotItem::Rtti_PlotZone, false);
    }

    void
    inspector_form::resizeEvent( QResizeEvent * event ){
      d_plot->setGeometry(0,0,this->width(),this->height());
    }

    void
    inspector_form::set_axis_x(float start, float stop) {
      // calc axis stuff
      d_axis_x.clear();
      d_axis_x.push_back((d_cfreq + start)/ *d_rf_unit); // start
      d_axis_x.push_back((stop-start)/d_fft_len/ *d_rf_unit); // step
      d_axis_x.push_back((d_cfreq + stop)/ *d_rf_unit); // stop

      d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[2]);

      // build frequency vector
      for(int i = 0; i < d_fft_len; i++) {
        d_freq[i] = d_axis_x[0] + i*d_axis_x[1];
      }
      refresh();

      // set zoom base to new axis width
      QRectF zbase = d_zoomer->zoomBase();
      d_zoomer->zoom(zbase);
      d_zoomer->setZoomBase(zbase);
      d_zoomer->setZoomBase(true);
      d_zoomer->zoom(0);

      QString unit = "Hz";
      switch(*d_rf_unit) {
        case 1:
          unit = "Hz";
          break;
        case 1000:
          unit = "kHz";
          break;
        case 1000000:
          unit = "MHz";
          break;
        case 1000000000:
          unit = "GHz";
          break;
        default:
          unit = "Hz";
      }

      d_plot->setAxisTitle(QwtPlot::xBottom, "Frequency ["+unit+"]");

      if(*d_manual) {
        spawn_signal_selector();
      }
    }

    void
    inspector_form::set_cfreq(float freq) {
      d_cfreq = freq;
    }

    void
    inspector_form::mousePressEvent(QMouseEvent *eventPress) {
      // move or resize manual signal marker
      // check if mouse clicked in manual mode without CTRL key
      if(*d_manual && eventPress->button() == Qt::LeftButton &&
              eventPress->modifiers() != Qt::ControlModifier) {
        // check left boundry
        if(std::abs(freq_to_x(d_markers[0]->d_zone->interval().minValue())
                    - eventPress->x()) < 3) {
          d_zoomer->setEnabled(false);
          d_clicked_marker = LEFT;
          d_mouse_offset = 0;
        }
        // check right boundry
        else if(std::abs(freq_to_x(d_markers[0]->d_zone->interval().maxValue())
                         - eventPress->x()) < 3) {
          d_zoomer->setEnabled(false);
          d_clicked_marker = RIGHT;
          d_mouse_offset = 0;
        }
        // check center
        else if(d_markers[0]->d_zone->boundingRect().contains(
                x_to_freq(eventPress->x()), 0)) {
          d_zoomer->setEnabled(false);
          d_clicked_marker = CENTER;
          d_mouse_offset = eventPress->x() - freq_to_x(d_markers[0]->d_freq/ *d_rf_unit);
        }
        else {
          d_clicked_marker = NONE;
        }
      }
    }

    void
    inspector_form::mouseMoveEvent(QMouseEvent *eventMove) {
      // implements drag and drop and mouse over cursors
      // check if marker was selected with left button
      if (d_clicked_marker != NONE &&
          eventMove->buttons() == Qt::LeftButton) {
        double xVal = x_to_freq(eventMove->x()-d_mouse_offset);
        float cfreq = d_markers[0]->d_freq;
        float bandwidth = d_markers[0]->d_bw;
        detach_markers();
        if (d_clicked_marker == CENTER) {
          d_markers[0]->set_marker(0, xVal * *d_rf_unit, bandwidth, *d_rf_unit);
        }
        else if (d_clicked_marker == LEFT ||
                 d_clicked_marker == RIGHT) {
          d_markers[0]->set_marker(0, cfreq, 2 * std::abs(
                  cfreq - xVal * *d_rf_unit), *d_rf_unit);
        }
      }
      // mouse over
      if(d_markers[0]->d_zone->interval().minValue() <= x_to_freq(eventMove->x()) &&
              d_markers[0]->d_zone->interval().maxValue() >= x_to_freq(eventMove->x())) {
        d_plot->canvas()->setCursor(Qt::SizeHorCursor);
      }
      else {
        d_plot->canvas()->setCursor(Qt::CrossCursor);
      }
    }

    void
    inspector_form::mouseReleaseEvent(QMouseEvent *eventRelease) {
      // after marker moving, set values
      if(d_clicked_marker != NONE && eventRelease->button() == Qt::LeftButton) {
        double xVal = x_to_freq(eventRelease->x()-d_mouse_offset);
        float cfreq = d_markers[0]->d_freq;
        float bandwidth = d_markers[0]->d_bw;
        detach_markers();
        if(d_clicked_marker == CENTER) {
          d_markers[0]->set_marker(0, xVal**d_rf_unit, bandwidth, *d_rf_unit);
        }
        else if (d_clicked_marker == LEFT || d_clicked_marker == RIGHT) {
          d_markers[0]->set_marker(0, cfreq, 2*std::abs(cfreq-xVal* *d_rf_unit), *d_rf_unit);
        }
        d_clicked_marker = NONE;
        d_mouse_offset = 0.0;
        d_zoomer->setEnabled(true);
        add_msg_queue(cfreq-d_cfreq, bandwidth);
      }
    }

    float
    inspector_form::freq_to_x(float freq) {
      // convert frequency value to pixel value
      float offset = this->width()-d_plot->canvas()->width();
      return d_plot->transform(QwtPlot::xBottom, freq)+offset;
    }

    float
    inspector_form::x_to_freq(float x) {
      // convert pixel value to frequency value
      float offset = this->width()-d_plot->canvas()->width();
      return d_plot->invTransform(QwtPlot::xBottom, x-offset);
    }

    void
    inspector_form::add_msg_queue(float cfreq, float bandwidth) {
      gr::message::sptr msg = gr::message::make(0, cfreq, bandwidth, 1);
      d_msg_queue->flush();
      d_msg_queue->insert_tail(msg);
    }

    void
    inspector_form::msg_received() {
      change_flag = true;
    }

    void
    inspector_form::add_analysis_text(int signal, std::string text) {
      d_analysis_texts[signal] = text;
    }

    void
    inspector_form::draw_analysis_text() {
      if(d_rf_map->size() <= d_marker_count) {
        for (int i = 0; i < d_rf_map->size(); i++) {
          if(!d_analysis_texts[i].empty()) {
            d_markers[i]->add_text(d_analysis_texts[i]);
          }
        }
      }
      else {
        for (int i = 0; i < d_marker_count; i++) {
          if(!d_analysis_texts[i].empty()) {
            d_markers[i]->add_text(d_analysis_texts[i]);
          }
        }
      }
    }

    void
    inspector_form::drawOverlay() {
      detach_markers();
    
      if(d_rf_map->size() <= d_marker_count) {
        for (int i = 0; i < d_rf_map->size(); i++) {
          d_markers[i]->set_marker(i, d_cfreq + d_rf_map->at(i)[0],
                                   d_rf_map->at(i)[1], *d_rf_unit);
        }
      }
      else {
        for (int i = 0; i < d_marker_count; i++) {
          d_markers[i]->set_marker(i, d_cfreq + d_rf_map->at(i)[0],
                                   d_rf_map->at(i)[1], *d_rf_unit);
        }
      }
    }

    void
    inspector_form::refresh(){
      gr::thread::scoped_lock guard(d_mutex);
      if (change_flag) {
         if (!*d_manual) {
             drawOverlay();
         }
         draw_analysis_text();
         change_flag = false;
      }
      // adjust text position to zoom rect
      for(int i = 0; i < d_markers.size(); i++) {
        d_markers[i]->set_label_y(
                d_zoomer->zoomRect().y()+d_zoomer->zoomRect().height()*0.85);
      }

      if(d_buffer->size() < d_fft_len) {
        return;
      }

      d_curve->setRawSamples(d_freq, &d_buffer->at(0), d_fft_len);

      // Do replot
      d_plot->replot();
    }

  }
}
