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

namespace gr {
  namespace inspector {

    inspector_plot::inspector_plot(int fft_len, std::vector<double> *buffer,
                                   std::vector<std::vector<float> >* rf_map,
                                   bool* ready, QWidget* parent) : QWidget(parent)
    {
      d_fft_len = fft_len;
      // Setup GUI
      //resize(QSize(600,600));
      d_ready = ready;
      d_buffer = buffer;
      d_interval = 250;
      d_rf_map = rf_map;

      // spawn all QT stuff
      d_plot = new QwtPlot(this); // make main plot
      d_painter = new QPainter(); // painter for text and markers
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

      // Plot axis and title
      std::string label_title = "Inspector GUI";
      d_plot->setTitle(QwtText(label_title.c_str()));
      d_plot->setAxisTitle(QwtPlot::xBottom, "Frequency [Hz]");
      d_plot->setAxisTitle(QwtPlot::yLeft, "dB");
      d_plot->setAxisScale(QwtPlot::yLeft, -120, 30);
      d_plot->setCanvasBackground(QColor(30,30,30));
      //d_plot->show();
      // Do replot
      d_plot->replot();

      // frequency vector for plot
      d_freq = new double[fft_len];

      // Setup timer and connect refreshing plot
      d_timer = new QTimer(this);
      connect(d_timer, SIGNAL(timeout()), this, SLOT(refresh()));
      d_timer->start(d_interval);

    }

    inspector_plot::~inspector_plot(){
      delete d_timer;
      delete d_zoomer;
      delete d_plot;
      delete d_curve;
      delete[] d_freq;
      delete d_symbol;
      delete d_grid;
      delete_markers();
    }

    void
    inspector_plot::delete_markers() {
      for(int i = 0; i < d_labels.size(); i++) {
        delete d_labels[i];
        delete d_left_lines[i];
        delete d_right_lines[i];
      }
    }

    void
    inspector_plot::resizeEvent( QResizeEvent * event ){
      d_plot->setGeometry(0,0,this->width(),this->height());
    }

    void
    inspector_plot::set_axis_x(float start, float stop) {
      d_axis_x.clear();
      d_axis_x.push_back(start);
      d_axis_x.push_back((stop-start)/d_fft_len);
      d_axis_x.push_back(stop);

      d_plot->setAxisScale(QwtPlot::xBottom, d_axis_x[0], d_axis_x[2]);

      for(int i = 0; i < d_fft_len; i++) {
        d_freq[i] = d_axis_x[0] + i*d_axis_x[1];
      }

      refresh();
    }

    void
    inspector_plot::plot_markers() {
      for(int i = 0; i < d_labels.size(); i++) {
        d_labels[i]->detach();
        d_left_lines[i]->detach();
        d_right_lines[i]->detach();
      }
      delete_markers();
      d_labels.clear();
      d_left_lines.clear();
      d_right_lines.clear();
      for(int i = 0; i < d_rf_map->size(); i++) {
        QwtPlotMarker* label = new QwtPlotMarker();
        QwtText text;
        QString qstring;
        qstring.push_back("Signal "+QString::number(i+1));
        qstring.append("\n");
        qstring.append("f = "+QString::number(d_rf_map->at(i)[0]));
        qstring.append("\n");
        qstring.append("B = "+QString::number(d_rf_map->at(i)[1]));
        text.setText(qstring);
        text.setColor(Qt::red);
        label->setLabelAlignment(Qt::AlignLeft);
        label->setLabel(text);
        label->setXValue(d_rf_map->at(i)[0]-300);
        label->setYValue(13);
        label->attach(d_plot);
        d_labels.push_back(label);

        QwtPlotMarker* left_line = new QwtPlotMarker();
        left_line->setLinePen(Qt::red, 0.5);
        left_line->setLineStyle(QwtPlotMarker::VLine);
        left_line->setXValue(d_rf_map->at(i)[0]-d_rf_map->at(i)[1]/2);
        left_line->attach(d_plot);
        d_left_lines.push_back(left_line);

        QwtPlotMarker* right_line = new QwtPlotMarker();
        right_line->setLinePen(Qt::red, 0.5);
        right_line->setLineStyle(QwtPlotMarker::VLine);
        right_line->setXValue(d_rf_map->at(i)[0]+d_rf_map->at(i)[1]/2);
        right_line->attach(d_plot);
        d_right_lines.push_back(right_line);
      }


    }

    void
    inspector_plot::refresh(){
      // write process, dont touch array!
      if(!*d_ready) {
        return;
      }
      // Fetch new data and push to matrix

      d_curve->detach();
      d_curve->setRawSamples(d_freq, &d_buffer->at(0), d_fft_len);
      d_curve->attach(d_plot);

      // Do replot
      d_plot->replot();
    }

  }
}
