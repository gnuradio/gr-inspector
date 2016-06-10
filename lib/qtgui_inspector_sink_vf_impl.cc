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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "qtgui_inspector_sink_vf_impl.h"

namespace gr {
  namespace inspector {

    qtgui_inspector_sink_vf::sptr
    qtgui_inspector_sink_vf::make(int fft_len, QWidget *parent)
    {
      return gnuradio::get_initial_sptr
        (new qtgui_inspector_sink_vf_impl(fft_len, parent));
    }

    /*
     * The private constructor
     */
    qtgui_inspector_sink_vf_impl::qtgui_inspector_sink_vf_impl(int fft_len,
                                                               QWidget *parent)
      : gr::sync_block("qtgui_inspector_sink_vf",
              gr::io_signature::make(1, 1, sizeof(float)*fft_len),
              gr::io_signature::make(0, 0, 0))
    {
      d_fft_len = fft_len;
      message_port_register_out(pmt::intern("map_out"));
      message_port_register_in(pmt::intern("map_in"));
      set_msg_handler(pmt::intern("map_in"), boost::bind(
              &qtgui_inspector_sink_vf_impl::handle_msg, this, _1));

      d_argc = 1;
      d_argv = new char;
      d_argv[0] = '\0';
      d_main_gui = NULL;
      d_parent = parent;

      initialize();
    }

    /*
     * Our virtual destructor.
     */
    qtgui_inspector_sink_vf_impl::~qtgui_inspector_sink_vf_impl()
    {
      d_main_gui->close();
      delete d_argv;
    }

    void
    qtgui_inspector_sink_vf_impl::initialize() {
      if(qApp != NULL) {
        d_qApplication = qApp;
      }
      else {
        d_qApplication = new QApplication(d_argc, &d_argv);
      }
      d_main_gui = new inspector_plot(d_fft_len, &d_buffer, d_parent);
      d_main_gui->show();
    }

    void
    qtgui_inspector_sink_vf_impl::handle_msg(pmt::pmt_t msg) {
      return;
    }

#ifdef ENABLE_PYTHON
    PyObject*
    qtgui_inspector_sink_vf_impl::pyqwidget()
    {
      PyObject *w = PyLong_FromVoidPtr((void*)d_main_gui);
      PyObject *retarg = Py_BuildValue("N", w);
      return retarg;
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
      if(d_buffer.size()!=noutput_items*d_fft_len){ // resize buffer if needed
        d_buffer.resize(noutput_items*d_fft_len);
      }
      // Do <+signal processing+>
      memcpy(&d_buffer[0], in, noutput_items*sizeof(float)*d_fft_len);
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

