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
#include "signal_detector_cc_impl.h"
#include <gnuradio/filter/firdes.h>

namespace gr {
  namespace inspector {

    signal_detector_cc::sptr
    signal_detector_cc::make(double samp_rate, int fft_len, int window, float threshold, float sensitivity, bool auto)
    {
      return gnuradio::get_initial_sptr
        (new signal_detector_cc_impl(samp_rate, fft_len, window, threshold, sensitivity, auto));
    }

    /*
     * The private constructor
     */
    signal_detector_cc_impl::signal_detector_cc_impl(double samp_rate, int fft_len, int window, float threshold, float sensitivity, bool auto)
      : gr::block("signal_detector_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float)))
    {
        message_port_register_out(pmt::intern("map_out"));
    }

    /*
     * Our virtual destructor.
     */
    signal_detector_cc_impl::~signal_detector_cc_impl()
    {
    }

    void
    signal_detector_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    signal_detector_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

