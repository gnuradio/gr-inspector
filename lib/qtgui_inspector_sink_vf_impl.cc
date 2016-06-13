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
#include <gnuradio/prefs.h>
#include <QFile>

namespace gr {
  namespace inspector {

    qtgui_inspector_sink_vf::sptr
    qtgui_inspector_sink_vf::make(double samp_rate, int fft_len, QWidget *parent)
    {
      return gnuradio::get_initial_sptr
        (new qtgui_inspector_sink_vf_impl(samp_rate, fft_len, parent));
    }

    /*
     * The private constructor
     */
    qtgui_inspector_sink_vf_impl::qtgui_inspector_sink_vf_impl(double samp_rate, int fft_len,
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
      d_samp_rate = samp_rate;
      d_initialized = false;
      build_axis_x();

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
#if QT_VERSION >= 0x040500
        std::string style = prefs::singleton()->get_string("qtgui", "style", "raster");
        QApplication::setGraphicsSystem(QString(style.c_str()));
#endif
        d_qApplication = new QApplication(d_argc, &d_argv);
      }

      std::string qssfile = prefs::singleton()->get_string("qtgui","qss","");
      if(qssfile.size() > 0) {
        QString sstext = get_qt_style_sheet(QString(qssfile.c_str()));
        d_qApplication->setStyleSheet(sstext);
      }

      d_main_gui = new inspector_plot(d_fft_len, &d_buffer, d_axis_x, d_parent);

      d_main_gui->set_axis_x(-d_samp_rate/2, d_samp_rate/d_fft_len);
    }

    void
    qtgui_inspector_sink_vf_impl::handle_msg(pmt::pmt_t msg) {
      return;
    }

    void
    qtgui_inspector_sink_vf_impl::build_axis_x() {
      d_axis_x.clear();
      d_axis_x.push_back(-d_samp_rate/d_fft_len);
      d_axis_x.push_back(d_samp_rate/d_fft_len - 1);
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

    QString
    qtgui_inspector_sink_vf_impl::get_qt_style_sheet(QString filename)
    {
      QString sstext;
      QFile ss(filename);
      if(!ss.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return sstext;
      }

      QTextStream sstream(&ss);
      while(!sstream.atEnd()) {
        sstext += sstream.readLine();
      }
      ss.close();

      return sstext;
    }

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
      memcpy(&d_buffer[0], in, noutput_items*sizeof(double)*d_fft_len);
      if(!d_initialized) {
        initialize();
        d_initialized = true;
      }
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

