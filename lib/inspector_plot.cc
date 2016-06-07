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

    inspector_plot::inspector_plot(int interval, int vlen, std::vector<float> *buffer, std::string label_x, std::string label_y, std::string label, std::vector<float> axis_x, std::vector<float> axis_y, std::vector<float> axis_z, bool autoscale_z,
                                       QWidget* parent) : QWidget(parent)
    {
      d_interval = interval;
      d_vlen = vlen;
      d_buffer = buffer;
      d_axis_x = axis_x;
      d_axis_y = axis_y;
      d_axis_z = axis_z;
      d_autoscale_z = autoscale_z;

      // Setup GUI
      resize(QSize(600,600));

      d_plot = new QwtPlot(this); // make main plot
      d_spectrogram = new QwtPlotSpectrogram(); // make spectrogram
      d_spectrogram->attach(d_plot); // attach spectrogram to plot

      d_data = new QwtMatrixRasterData(); // make data structure

      // Setup colormap
      d_colormap = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
      d_colormap->addColorStop(0.25, Qt::cyan);
      d_colormap->addColorStop(0.5, Qt::green);
      d_colormap->addColorStop(0.75, Qt::yellow);

      d_spectrogram->setColorMap(d_colormap);

      // Plot axis and title
      std::string label_title = "Spectrogram Plot: ";
      label_title.append(label_x);
      label_title.append("/");
      label_title.append(label_y);
      if(label!=""){
        label_title.append(" (");
        label_title.append(label);
        label_title.append(")");
      }
      d_plot->setTitle(QwtText(label_title.c_str()));
      d_plot->setAxisTitle(QwtPlot::xBottom, label_x.c_str());
      d_plot->setAxisTitle(QwtPlot::yLeft, label_y.c_str());

      // Do replot
      d_plot->replot();

      // Setup timer and connect refreshing plot
      d_timer = new QTimer(this);
      connect(d_timer, SIGNAL(timeout()), this, SLOT(refresh()));
      d_timer->start(d_interval);
    }

    inspector_plot::~spectrogram_plot(){
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

      // Get rows and columns
      int columns, rows;
      columns = d_vlen;
      rows = d_buffer->size()/d_vlen;

      // Fill data in spectrogram
      d_data->setValueMatrix(d_plot_data, columns);
      d_data->setInterval(Qt::XAxis,QwtInterval(d_axis_x[0], d_axis_x[1]));
      d_data->setInterval(Qt::YAxis,QwtInterval(d_axis_y[0], d_axis_y[1]));
      if(d_autoscale_z){
        d_data->setInterval(Qt::ZAxis,QwtInterval(minimum, maximum));
      }
      else{
        d_data->setInterval(Qt::ZAxis,QwtInterval(d_axis_z[0], d_axis_z[1]));
      }

      d_spectrogram->setData(d_data);

      // Set colorbar
      d_scale = d_plot->axisWidget(QwtPlot::yRight);
      d_scale->setColorBarEnabled(true);
      d_scale->setColorBarWidth(20);
      if(d_autoscale_z){
        d_scale->setColorMap(QwtInterval(minimum, maximum), d_colormap);
        d_plot->setAxisScale(QwtPlot::yRight,minimum,maximum);
      }
      else{
        d_scale->setColorMap(QwtInterval(d_axis_z[0], d_axis_z[1]), d_colormap);
        d_plot->setAxisScale(QwtPlot::yRight,d_axis_z[0],d_axis_z[1]);
      }
      d_plot->enableAxis(QwtPlot::yRight);

      // Do replot
      d_plot->replot();
    }

  }
}
