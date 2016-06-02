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
#include <gnuradio/qtgui/vectordisplayform.h>

namespace gr {
  namespace inspector {

    class qtgui_inspector_sink_vf_impl : public qtgui_inspector_sink_vf
    {
     private:
      // Nothing to declare in this block.

     public:
      qtgui_inspector_sink_vf_impl(int fft_len);
      ~qtgui_inspector_sink_vf_impl();

      int d_argc;
      char *d_argv;
      QWidget *d_parent;
      int d_fft_len;
      VectorDisplayForm *d_main_gui;

      void handle_msg(pmt::pmt_t msg);

      void initialize();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_IMPL_H */

