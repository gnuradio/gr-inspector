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

namespace gr {
  namespace inspector {

    inspector_plot::inspector_plot(int fft_len, std::vector<double> *buffer, std::vector<float> axis_x, QWidget* parent) : QWidget(parent)
    {
      d_vlen = fft_len;
      // Setup GUI
      resize(QSize(600,600));

      d_buffer = buffer;
      d_autoscale_z = true;
      d_samp_rate = -axis_x[0]*2;

      d_layout = new QGridLayout(this);
      d_plot = new QwtPlot(this); // make main plot

      d_curve = new QwtPlotCurve(); // make spectrogram
      d_layout->addWidget(d_plot, 0, 0);
      setLayout(d_layout);
      d_curve->attach(d_plot); // attach spectrogram to plot
      d_grid = new QwtPlotGrid;
      d_freq = new double[fft_len];
      d_curve->setPen(Qt::white, 1);

      d_symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(QColor(Qt::white)),
                                        QPen(QColor(Qt::white)), QSize(7,7));
      //d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
      d_curve->setSymbol(d_symbol);
      // Plot axis and title
      std::string label_title = "Inspector GUI";

      d_plot->setTitle(QwtText(label_title.c_str()));
      d_plot->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
      d_plot->setAxisTitle(QwtPlot::yLeft, "dB");
      //d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[1]);
      //d_plot->setAxisScale(QwtPlot::yLeft, d_axis_y[0], d_axis_y[1]);
      d_plot->setCanvasBackground(Qt::blue);
      d_grid->setPen(QPen(QColor(119,136,153),0.5, Qt::DashLine));
      d_grid->attach(d_plot);
      this->show();
      d_plot->show();
      d_curve->show();
      // Do replot
      d_plot->replot();
      //refresh();

    }

    inspector_plot::~inspector_plot(){
    }

    void
    inspector_plot::resizeEvent( QResizeEvent * event ){
      d_plot->setGeometry(0,0,this->width(),this->height());
    }

    void
    inspector_plot::set_axis_x(float start, float step) {
      d_axis_x.clear();
      d_axis_x.push_back(start);
      d_axis_x.push_back(start+d_vlen*step);
      d_axis_x.push_back(step);

      d_plot->setAxisScale(d_axis_x[0], d_axis_x[1], d_axis_x[2]);
      refresh();
    }

    void
    inspector_plot::refresh(){
      // Fetch new data and push to matrix
      d_plot_data.clear();
      d_plot_data.resize(d_buffer->size());

      float maximum, minimum; // get maximum and minimum
      if(d_buffer->size()!=0){
        maximum = (*d_buffer)[0];
        minimum = (*d_buffer)[0];
      }
      else throw std::runtime_error("data buffer has size zero");

      for(int k=0; k<d_buffer->size(); k++){
        d_plot_data[k] = (*d_buffer)[k];
        if(d_plot_data[k]<minimum) minimum = d_plot_data[k];
        if(d_plot_data[k]>maximum) maximum = d_plot_data[k];
      }
      if(std::isnan(minimum)||std::isnan(maximum)) throw std::runtime_error("minimum or maximum for z axis is NaN");

      if(d_autoscale_z){
        d_plot->setAxisAutoScale(0, true);
        d_plot->setAxisAutoScale(1, true);
      }
      else{
        d_plot->setAxisAutoScale(0, false);
        d_plot->setAxisAutoScale(1, false);
      }

      for(int i = 0; i < d_vlen; i++) {
        d_freq[i] = -d_axis_x[0] + i*d_samp_rate/d_vlen;
      }
      d_curve->setRawSamples(d_freq, &d_buffer->at(0), d_vlen);
      d_curve->attach(d_plot);

      // Do replot
      d_plot->replot();
    }

  }
}
