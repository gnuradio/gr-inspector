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

    inspector_plot::inspector_plot(int fft_len, std::vector<double> *buffer, std::vector<float> axis_x, bool* ready, QWidget* parent) : QWidget(parent)
    {
      d_vlen = fft_len;
      // Setup GUI
      resize(QSize(600,600));
      d_ready = ready;
      d_buffer = buffer;
      d_autoscale_z = true;
      d_samp_rate = -axis_x[0]*2;
      d_interval = 1000;

      d_layout = new QGridLayout(this);
      d_plot = new QwtPlot(this); // make main plot

      d_curve = new QwtPlotCurve(); // make spectrogram
      // cannot add null widget?
      //d_layout->addWidget(d_plot, 0, 0);
      setLayout(d_layout);
      d_curve->attach(d_plot); // attach spectrogram to plot
      d_grid = new QwtPlotGrid;
      d_freq = new double[fft_len];
      d_curve->setPen(Qt::blue, 1);

      d_symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(QColor(Qt::blue)),
                                        QPen(QColor(Qt::blue)), QSize(7,7));
      //d_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
      d_curve->setSymbol(d_symbol);
      // Plot axis and title
      std::string label_title = "Inspector GUI";

      d_plot->setTitle(QwtText(label_title.c_str()));
      d_plot->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
      d_plot->setAxisTitle(QwtPlot::yLeft, "dB");
      //d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[1]);
      d_plot->setAxisScale(QwtPlot::yLeft, -120, 20);
      d_plot->setCanvasBackground(Qt::white);
      d_grid->setPen(QPen(QColor(119,136,153),0.5, Qt::DashLine));
      //d_grid->attach(d_plot);
      d_plot->show();
      // Do replot
      d_plot->replot();
      //refresh();

      // Setup timer and connect refreshing plot
      d_timer = new QTimer(this);
      connect(d_timer, SIGNAL(timeout()), this, SLOT(refresh()));
      d_timer->start(d_interval);

    }

    inspector_plot::~inspector_plot(){
      delete d_timer;
      delete d_layout;
      delete d_plot;
      delete d_curve;
      delete d_grid;
      delete[] d_freq;
      delete d_symbol;
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

      d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[1]);

      for(int i = 0; i < d_vlen; i++) {
        d_freq[i] = -d_axis_x[0] + i*d_axis_x[2];
      }

      refresh();
    }

    void
    inspector_plot::refresh(){
      if(!*d_ready) {
        return;
      }
      std::cout << "Refreshing..." << std::endl;
      // Fetch new data and push to matrix
      d_plot_data.clear();
      d_plot_data.resize(d_buffer->size());


      d_curve->detach();
      d_curve->setRawSamples(d_freq, &d_buffer->at(0), d_vlen);
      d_curve->attach(d_plot);

      // Do replot
      d_plot->replot();
    }

  }
}
