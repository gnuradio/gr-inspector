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

#ifndef INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H
#define INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H

#include <inspector/qtgui_inspector_sink_vf.h>
#include "inspector_plot.h"
#include <QString>

namespace gr {
  namespace inspector {

    class qtgui_inspector_sink_vf_impl : public qtgui_inspector_sink_vf
    {
     private:
      // Nothing to declare in this block.

     public:
      qtgui_inspector_sink_vf_impl(double samp_rate, int fft_len, QWidget *parent);
      ~qtgui_inspector_sink_vf_impl();

#ifdef ENABLE_PYTHON
      PyObject* pyqwidget();
#else
      void* pyqwidget();
#endif

      int d_argc;
      bool d_initialized;
      char *d_argv;
      QWidget *d_parent;
      int d_fft_len;
      double d_samp_rate;
      inspector_plot *d_main_gui;
      QApplication *d_qApplication;
      std::vector<double> d_buffer;
      std::vector<float> d_axis_x;
      void handle_msg(pmt::pmt_t msg);
      void build_axis_x();
      QString get_qt_style_sheet(QString filename);

      void initialize();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H */

