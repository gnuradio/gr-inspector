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
#include "signal_extractor_c_impl.h"
#include <volk/volk.h>
#include <gnuradio/filter/firdes.h>

namespace gr {
  namespace inspector {

    signal_extractor_c::sptr
    signal_extractor_c::make(int signal, bool resample, float rate, float osf)
    {
      return gnuradio::get_initial_sptr
        (new signal_extractor_c_impl(signal, resample, rate, osf));
    }


    /*
     * The private constructor
     */
    signal_extractor_c_impl::signal_extractor_c_impl(int signal,
                                                     bool resample,
                                                     float rate,
                                                     float osf)
      : gr::sync_block("signal_extractor_c",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      message_port_register_in(pmt::intern("sig_in"));
      set_msg_handler(pmt::intern("sig_in"), boost::bind(
              &signal_extractor_c_impl::handle_msg, this, _1));

      d_signal = signal;
      d_oversampling = osf;
      d_ready = false; // tell work to not emit anything until a message arrives
      d_out_rate = rate;
      d_rate = 1;
      d_resample = resample;
      std::vector<float> taps = filter::firdes::low_pass(64, 1000, 1000.0/64.0, 100/64);
      d_resampler = new filter::kernel::pfb_arb_resampler_ccf(d_rate, taps, 64);
    }

    /*
     * Our virtual destructor.
     */
    signal_extractor_c_impl::~signal_extractor_c_impl()
    {
      delete d_resampler;
    }

    void
    signal_extractor_c_impl::handle_msg(pmt::pmt_t msg) {
      if(d_signal+1 > pmt::length(msg)) {
        GR_LOG_WARN(d_logger, "Specified signal does not exist.");
        return;
      }
      else {
        pmt::pmt_t pmt_samples = pmt::tuple_ref(
                pmt::vector_ref(msg, d_signal), 3);
        d_length = pmt::length(pmt_samples);
        for (int i = 0; i < d_length; i++) {
          d_samples.push_back(pmt::c32vector_ref(pmt_samples, i));
        }
        d_msg_buffer = &d_samples[0];
        d_ready = true; // buffer filled, ready to emit samples
        if (d_resample) {
          float bw = pmt::to_float(pmt::tuple_ref(pmt::vector_ref(msg, d_signal), 2));
          d_rate = d_out_rate/(d_oversampling*bw);
          d_resampler->set_rate(d_rate);
          //GR_LOG_INFO(d_logger, "Actual output sample rate: "+std::to_string(
                  //bw*d_oversampling*d_resampler->interpolation_rate()/d_resampler->decimation_rate()));
        }
      }
    }

    int
    signal_extractor_c_impl::work(int noutput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items)
    {
      gr_complex *out = (gr_complex*)output_items[0];
      // if no samples received, skip work
       if(d_samples.size() > 0) {
         int nout;
         int item_count = noutput_items;
         // resampling
         if(d_resample) {
           item_count *= 1/d_rate;
           if(item_count > d_samples.size())
             item_count = d_samples.size();
           d_resampler->filter(out, &d_samples[0], item_count, nout);
           d_samples.clear();
           return d_rate*nout;
         }
         // copy raw samples into output buffer
         else {
           if(item_count > d_samples.size())
             item_count = d_samples.size();
           memcpy(out, &d_samples[0], sizeof(gr_complex)*item_count);
           d_samples.clear();
           return item_count;
         }
      }
      else {
        return 0;
      }
    }

  } /* namespace inspector */
} /* namespace gr */

