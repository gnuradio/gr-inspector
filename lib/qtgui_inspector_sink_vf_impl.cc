/* -*- c++ -*- */
/*
 * Copyright 2019 Free Software Foundation, Inc..
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "qtgui_inspector_sink_vf_impl.h"
#include <gnuradio/prefs.h>
#include <QFile>
#include <iostream>

namespace gr {
  namespace inspector {

    qtgui_inspector_sink_vf::sptr
    qtgui_inspector_sink_vf::make(double samp_rate, int fft_len,
                                  float cfreq, int rf_unit, int msgports,
                                  bool manual, QWidget *parent)
    {
      return gnuradio::get_initial_sptr
              (new qtgui_inspector_sink_vf_impl(samp_rate, fft_len,
                                                cfreq, rf_unit, msgports,
                                                manual, parent));
    }


    /*
     * The private constructor
     */
    qtgui_inspector_sink_vf_impl::qtgui_inspector_sink_vf_impl(double samp_rate,
                                                               int fft_len,
                                                               float cfreq,
                                                               int rf_unit,
                                                               int msgports,
                                                               bool manual,
                                                               QWidget *parent)
            : gr::sync_block("qtgui_inspector_sink_vf",
                             gr::io_signature::make(1, 1, sizeof(float)*fft_len),
                             gr::io_signature::make(0, 0, 0))
    {

      message_port_register_out(pmt::intern("map_out"));
      message_port_register_in(pmt::intern("map_in"));
      set_msg_handler(pmt::intern("map_in"), boost::bind(
              &qtgui_inspector_sink_vf_impl::handle_msg, this, _1));

      // analysis port logic
      if(msgports == 1) {
        message_port_register_in(pmt::intern("analysis_in"));
        set_msg_handler(pmt::intern("analysis_in"), boost::bind(
                &qtgui_inspector_sink_vf_impl::handle_analysis, this, _1));
      }
      else {
        for(int i = 0; i < msgports; i++) {
          message_port_register_in(pmt::intern("analysis_in"+std::to_string(i)));
          set_msg_handler(pmt::intern("analysis_in"+std::to_string(i)), boost::bind(
                  &qtgui_inspector_sink_vf_impl::handle_analysis, this, _1));
        }
      }

      d_argc = 1;
      d_argv = new char;
      d_argv[0] = '\0';
      d_main_gui = NULL;
      d_parent = parent;
      d_fft_len = fft_len;
      d_samp_rate = samp_rate;
      d_manual = false;
      d_cfreq = cfreq;
      d_rf_unit = rf_unit;
      d_msg_queue = new gr::msg_queue(1);
      d_manual = manual;
      initialize();
    }

    /*
     * Our virtual destructor.
     */
    qtgui_inspector_sink_vf_impl::~qtgui_inspector_sink_vf_impl()
    {
      d_main_gui->close();
      delete d_argv;
      delete d_main_gui;
      delete d_msg_queue;
    }

    void
    qtgui_inspector_sink_vf_impl::set_rf_unit(int unit) {
      d_rf_unit = unit;
      d_main_gui->set_axis_x(-d_samp_rate/2, d_samp_rate/2-1);
    }

    void
    qtgui_inspector_sink_vf_impl::set_samp_rate(double d_samp_rate) {
      qtgui_inspector_sink_vf_impl::d_samp_rate = d_samp_rate;
      d_main_gui->set_axis_x(-d_samp_rate/2, d_samp_rate/2-1);
    }


    void
    qtgui_inspector_sink_vf_impl::initialize() {
      if(qApp != NULL) {
        d_qApplication = qApp;
      }
      else {
#if QT_VERSION >= 0x040500 && QT_VERSION < 0x050000
        std::string style = prefs::singleton()->get_string("qtgui", "style", "raster");
        QApplication::setGraphicsSystem(QString(style.c_str()));
#endif
        d_qApplication = new QApplication(d_argc, &d_argv);
      }
      d_main_gui = new inspector_form(d_fft_len, &d_buffer, &d_rf_map,
                                      &d_manual, d_msg_queue, &d_rf_unit, d_parent);
      d_main_gui->show();
      d_main_gui->set_cfreq(d_cfreq);
      d_main_gui->set_axis_x(-d_samp_rate/2, d_samp_rate/2-1);
    }

    void
    qtgui_inspector_sink_vf_impl::handle_msg(pmt::pmt_t msg) {
      unpack_message(msg);
      d_main_gui->msg_received();

      // bypass block if no manual signal selection
      if(!d_manual){
        message_port_pub(pmt::intern("map_out"), msg);
      }
      d_tmp_msg = msg;
    }

    void
    qtgui_inspector_sink_vf_impl::handle_analysis(pmt::pmt_t msg) {
      int len = pmt::length(msg);
      int signal = pmt::to_uint64(pmt::tuple_ref(pmt::tuple_ref(msg, 0), 1));
      float val;
      std::string text = "";
      for(int i = 1; i < len; i++) {
        text += pmt::symbol_to_string(pmt::tuple_ref(pmt::tuple_ref(msg, i), 0));
        text += ": ";
        val = pmt::to_float(pmt::tuple_ref(pmt::tuple_ref(msg, i), 1));
        if(floor(val) == val) {
          text += std::to_string((int)val);
        }
        else {
          text += std::to_string(val);
        }
        text += "\n";
      }
      d_main_gui->add_analysis_text(signal, text);
    }

    void
    qtgui_inspector_sink_vf_impl::send_manual_message(float center, float bw) {
      pmt::pmt_t msg = pmt::make_vector(1, pmt::PMT_NIL);
      pmt::pmt_t curr_edge = pmt::make_f32vector(2, 0.0);
      pmt::f32vector_set(curr_edge, 0, center);
      pmt::f32vector_set(curr_edge, 1, bw);
      pmt::vector_set(msg, 0, curr_edge);
      message_port_pub(pmt::intern("map_out"), msg);
    }


    void
    qtgui_inspector_sink_vf_impl::unpack_message(pmt::pmt_t msg) {
      d_rf_map.clear();
      std::vector<float> temp;
      for (unsigned int i = 0; i < pmt::length(msg); i++) {
        pmt::pmt_t row = pmt::vector_ref(msg, i);
        temp.clear();
        temp.push_back(pmt::f32vector_ref(row, 0));
        temp.push_back(pmt::f32vector_ref(row, 1));
        d_rf_map.push_back(temp);
      }
    }

    // copied from gr-qtgui
#ifdef ENABLE_PYTHON
    PyObject*
    qtgui_inspector_sink_vf_impl::pyqwidget()
    {
      PyObject *w = PyLong_FromVoidPtr((void*)d_main_gui);
      return w;
    }
#else
    void *
    qtgui_inspector_sink_vf_impl::pyqwidget()
    {
      return NULL;
    }
#endif

    int
    qtgui_inspector_sink_vf_impl::work(int noutput_items,
                                       gr_vector_const_void_star &input_items,
                                       gr_vector_void_star &output_items)
    {
      const float *in = (const float*) input_items[0];
      // push samples into buffer for QWidget
      d_buffer.clear();
      for(int i = 0; i < d_fft_len; i++) {
        d_buffer.push_back(in[i]);
      }
      // check msg_queue from QWidget for manual message info
      if(d_manual && !d_msg_queue->empty_p()) {
        gr::message::sptr msg = d_msg_queue->delete_head();
        float center = msg->arg1();
        float bw = msg->arg2();
        send_manual_message(center, bw);
        d_auto_sent = false;
      }
      // resend last auto detection when manual just disabled
      if(!d_manual && !d_auto_sent) {
        message_port_pub(pmt::intern("map_out"), d_tmp_msg);
        d_auto_sent = true;
      }
      return 1;
    }

  } /* namespace inspector */
} /* namespace gr */

