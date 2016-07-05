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
      d_ready = false;

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

      //d_samples.clear();
      if(d_signal+1 > pmt::length(msg)) {
        GR_LOG_WARN(d_logger, "Specified signal does not exist.");
        return;
      }
      else {
        //std::cout << "Msg len = " << pmt::length(msg) << std::endl;
        pmt::pmt_t pmt_samples = pmt::vector_ref(msg, d_signal);
        d_length = pmt::length(pmt_samples);
        for(int i = 0; i < d_length; i++) {
          d_samples.push_back(pmt::c32vector_ref(pmt_samples, i));
        }
        d_msg_buffer = &d_samples[0];
        d_ready = true;
        //std::cout << "Buffered samples... " << d_length << std::endl;
      }
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
        //std::cout << "Work work work work work" << std::endl;
        d_ready = false;
        if(noutput_items >= d_length) {
          memcpy(out, d_msg_buffer, d_length*sizeof(gr_complex));
          //std::cout << d_length << std::endl;
          for(int i = 0; i < d_length; i++) {
            d_samples.erase(d_samples.begin()+i);
          }
          d_msg_buffer = &d_samples[0];
          return d_length;

        }
        else {
          memcpy(out, d_msg_buffer, noutput_items*sizeof(gr_complex));
          //std::cout << noutput_items << std::endl;
          for(int i = 0; i < noutput_items; i++) {
            d_samples.erase(d_samples.begin()+i);
          }
          d_msg_buffer = &d_samples[0];
          return noutput_items;

        }
      }
      else {
        return 0;
      }
    }

  } /* namespace inspector */
} /* namespace gr */

