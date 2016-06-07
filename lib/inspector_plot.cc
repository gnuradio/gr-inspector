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

    inspector_plot::inspector_plot(int fft_len, QWidget* parent) : QWidget(parent)
    {
      d_vlen = fft_len;
      // Setup GUI
      resize(QSize(600,600));

      d_plot = new QwtPlot(this); // make main plot
      d_curve = new QwtPlotCurve(); // make spectrogram
      d_curve->attach(d_plot); // attach spectrogram to plot

      // Plot axis and title
      std::string label_title = "Inspector GUI";

      d_plot->setTitle(QwtText(label_title.c_str()));
      d_plot->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
      d_plot->setAxisTitle(QwtPlot::yLeft, "dB");

      // Do replot
      d_plot->replot();
    }

    inspector_plot::~inspector_plot(){
    }

    void
    inspector_plot::resizeEvent( QResizeEvent * event ){
      d_plot->setGeometry(0,0,this->width(),this->height());
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

      // Do replot
      d_plot->replot();
    }

  }
}
