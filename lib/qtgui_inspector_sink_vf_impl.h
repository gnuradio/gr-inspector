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

#ifndef INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H
#define INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H

#include <inspector/qtgui_inspector_sink_vf.h>
#include "inspector_form.h"

namespace gr {
  namespace inspector {

    class qtgui_inspector_sink_vf_impl : public qtgui_inspector_sink_vf
    {
     private:
      int d_argc, d_rf_unit, d_fft_len;
      bool d_manual, d_auto_sent;
      float d_cfreq;
      double d_samp_rate;
      char *d_argv;
      QWidget *d_parent;
      pmt::pmt_t d_tmp_msg;
      inspector_form *d_main_gui;
      QApplication *d_qApplication;
      std::vector<double> d_buffer;
      std::vector<std::vector<float> > d_rf_map;
      gr::msg_queue* d_msg_queue;

     public:
      qtgui_inspector_sink_vf_impl(double samp_rate, int fft_len,
                                   float cfreq, int rf_unit,
                                   int msgports, bool manual,
                                   QWidget *parent);
      ~qtgui_inspector_sink_vf_impl();

#ifdef ENABLE_PYTHON
      PyObject* pyqwidget();
#else
      void* pyqwidget();
#endif


      void handle_msg(pmt::pmt_t msg);
      void handle_analysis(pmt::pmt_t msg);
      void unpack_message(pmt::pmt_t msg);
      void send_manual_message(float center, float bw);
      void set_rf_unit(int unit);
      void set_samp_rate(double d_samp_rate);

      void initialize();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);


      void set_cfreq(float cfreq) {
        d_cfreq = cfreq;
        d_main_gui->set_cfreq(cfreq);
        d_main_gui->set_axis_x(-d_samp_rate/2, d_samp_rate/2-1);
      }

    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H */
