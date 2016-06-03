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
#include "signal_extractor_c_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace inspector {

    signal_extractor_c::sptr
    signal_extractor_c::make(int signal)
    {
      return gnuradio::get_initial_sptr
        (new signal_extractor_c_impl(signal));
    }

    /*
     * The private constructor
     */
    signal_extractor_c_impl::signal_extractor_c_impl(int signal)
      : gr::sync_block("signal_extractor_c",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      message_port_register_in(pmt::intern("sig_in"));
      set_msg_handler(pmt::intern("sig_in"), boost::bind(
              &signal_extractor_c_impl::handle_msg, this, _1));

      d_signal = signal;

    }

    /*
     * Our virtual destructor.
     */
    signal_extractor_c_impl::~signal_extractor_c_impl()
    {
    }

    void
    signal_extractor_c_impl::handle_msg(pmt::pmt_t msg) {
      // error handling
      d_samples.clear();
      if(d_signal < pmt::length(msg)-1) {
        GR_LOG_ERROR(d_logger, "Specified signal does not exist.");
        std::cout << "Signal " << d_signal << " not in message!" << std::endl;
        return;
      }

      pmt::pmt_t pmt_samples = pmt::vector_ref(msg, d_signal);
      d_length = pmt::length(pmt_samples);
      for(int i = 0; i < d_length; i++) {
        d_samples.push_back(pmt::c32vector_ref(pmt_samples, i));
      }
      d_msg_buffer = &d_samples[0];
      d_ready = true;
    }


    int
    signal_extractor_c_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      //const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      //std::cout << d_ready << "\n";
      // Do <+signal processing+>
      if(d_ready) {
        memcpy(out, d_msg_buffer, d_length*sizeof(gr_complex));
        return d_length;
        d_ready = false;
      }
      else {
        return 0;
      }
    }

  } /* namespace inspector */
} /* namespace gr */

