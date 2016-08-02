/* -*- c++ -*- */
/* 
 * Copyright 2016 Sebastian Müller.
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
#include "fm_demod_cf_impl.h"

namespace gr {
  namespace inspector {

    fm_demod_cf::sptr
    fm_demod_cf::make(int signal)
    {
      return gnuradio::get_initial_sptr
        (new fm_demod_cf_impl(signal));
    }

    /*
     * The private constructor
     */
    fm_demod_cf_impl::fm_demod_cf_impl(int signal)
      : gr::sync_decimator("fm_demod_cf",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float)), 10)
    {
      d_signal = signal;
      message_port_register_in(pmt::intern("map_in"));
      set_msg_handler(pmt::intern("map_in"), boost::bind(
              &fm_demod_cf_impl::handle_msg, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    fm_demod_cf_impl::~fm_demod_cf_impl()
    {
    }

    void
    fm_demod_cf_impl::handle_msg(pmt::pmt_t msg) {
      pmt::pmt_t tuple = pmt::vector_ref(msg, d_signal);
      set_decimation(pmt::to_float(pmt::vector_ref(tuple, 1))/48000.0);
    }

    int
    fm_demod_cf_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      int j = 0;
      for(int i = 0; i < noutput_items; i++) {
        out[i] = std::abs(in[j]);
        j += decimation();
      }

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

