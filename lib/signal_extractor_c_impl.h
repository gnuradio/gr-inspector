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

#ifndef INCLUDED_INSPECTOR_SIGNAL_EXTRACTOR_C_IMPL_H
#define INCLUDED_INSPECTOR_SIGNAL_EXTRACTOR_C_IMPL_H

#include <inspector/signal_extractor_c.h>
#include <gnuradio/filter/pfb_arb_resampler.h>

namespace gr {
  namespace inspector {

    class signal_extractor_c_impl : public signal_extractor_c
    {
     private:
      int d_signal, d_length;
      bool d_ready, d_resample;
      float d_out_rate, d_oversampling, d_rate;
      std::vector<gr_complex> d_samples;
      gr_complex* d_msg_buffer;
      filter::kernel::pfb_arb_resampler_ccf *d_resampler;

     public:
      signal_extractor_c_impl(int signal, bool resample, float rate,
      float osf);
      ~signal_extractor_c_impl();

      // write samples from message in buffer
      void handle_msg(pmt::pmt_t msg);

      // Where all the action really happens
      int work(int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);

      // callback
      void set_signal(int d_signal) {
        signal_extractor_c_impl::d_signal = d_signal;
      }
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_EXTRACTOR_C_IMPL_H */
