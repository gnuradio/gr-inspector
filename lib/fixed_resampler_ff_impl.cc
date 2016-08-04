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
#include "fixed_resampler_ff_impl.h"

namespace gr {
  namespace inspector {

    fixed_resampler_ff::sptr
    fixed_resampler_ff::make(int signal, float out_samp_rate)
    {
      return gnuradio::get_initial_sptr
        (new fixed_resampler_ff_impl(signal, out_samp_rate));
    }

    /*
     * The private constructor
     */
    fixed_resampler_ff_impl::fixed_resampler_ff_impl(int signal, float out_samp_rate)
      : filter::fractional_resampler_cc()
    {
      d_mother = filter::fractional_resampler_cc::make(0.0, 1.0);
      d_signal = signal;
      d_samp_rate = out_samp_rate;
      set_mu(0);
      set_resamp_ratio(1.0); // initial value
      message_port_register_in(pmt::intern("map_in"));
      set_msg_handler(pmt::intern("map_in"), boost::bind(
              &fixed_resampler_ff_impl::handle_msg, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    fixed_resampler_ff_impl::~fixed_resampler_ff_impl()
    {
    }

    void
    fixed_resampler_ff_impl::handle_msg(pmt::pmt_t msg) {
      int len = pmt::length(msg);
      if(len < d_signal+1) {
        GR_LOG_WARN(d_logger, "Specified signal does not exist");
        return;
      }
      d_bw = pmt::f32vector_ref(pmt::vector_ref(msg, d_signal), 1);
      set_resamp_ratio(d_bw/d_samp_rate);
    }

  } /* namespace inspector */
} /* namespace gr */

