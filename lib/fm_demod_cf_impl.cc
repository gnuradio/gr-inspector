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
#include <gnuradio/math.h>
#include <volk/volk.h>
#include <gnuradio/filter/firdes.h>

namespace gr {
  namespace inspector {

    fm_demod_cf::sptr
    fm_demod_cf::make(int signal, int audio_rate)
    {
      return gnuradio::get_initial_sptr
        (new fm_demod_cf_impl(signal, audio_rate));
    }

    /*
     * The private constructor
     */
    fm_demod_cf_impl::fm_demod_cf_impl(int signal, int audio_rate)
      : gr::sync_decimator("fm_demod_cf",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float)), 1)
    {
      std::vector<float> taps{1};
      d_audio_filter = new filter::kernel::fir_filter_fff(1, taps);
      //d_resampler = new filter::kernel::pfb_arb_resampler_fff()
      d_audio_rate = audio_rate;
      d_signal = signal;
      d_gain = 1;
      message_port_register_in(pmt::intern("map_in"));
      set_msg_handler(pmt::intern("map_in"), boost::bind(
              &fm_demod_cf_impl::handle_msg, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    fm_demod_cf_impl::~fm_demod_cf_impl()
    {
      delete d_audio_filter;
      delete d_resampler;
    }

    void
    fm_demod_cf_impl::handle_msg(pmt::pmt_t msg) {
      // no signal information in message for specified signal
      if(pmt::length(msg) < d_signal+1) {
        GR_LOG_WARN(d_logger, "Specified signal does not exist.");
        return;
      }
      // get bandwidth out of message
      pmt::pmt_t tuple = pmt::vector_ref(msg, d_signal);
      float bw = pmt::f32vector_ref(tuple, 1);
      // demod gain
      d_gain = bw/(2*M_PI*75000.0);
      d_ratio = d_audio_rate/bw;
      // decimation case
      if(bw > d_audio_rate) {
        int decim = 1/d_ratio;
        if(decim < 1) {
          decim = 1;
        }
        d_taps = filter::firdes::low_pass(1.0,
                                          bw,
                                          d_audio_rate/2-1000.0,
                                          1000.0, filter::firdes::WIN_HAMMING);
        d_audio_filter->set_taps(d_taps);
      }
      else {
        float osf = d_audio_rate/bw;
      }


    }

    int
    fm_demod_cf_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      d_fir_buffer.resize(noutput_items+d_audio_filter->ntaps());

      // copied from quadrature_demod_cf
      std::vector<gr_complex> tmp(noutput_items);
      float demod[noutput_items];
      volk_32fc_x2_multiply_conjugate_32fc(&tmp[0], &in[1], &in[0], noutput_items);
      for(int i = 0; i < noutput_items; i++) {
        demod[i] = d_gain * gr::fast_atan2f(imag(tmp[i]), real(tmp[i]));
      }

      memcpy(&d_fir_buffer[d_audio_filter->ntaps()], demod, noutput_items*sizeof(float));

      // audio filtering
      int output_size = noutput_items*d_ratio;
      int j = 0;
      for(int i = 0; i < output_size; i++) {
        out[i] = d_audio_filter->filter(&d_fir_buffer[j]);
        j += 1.0/d_ratio;
      }
      // Do <+signal processing+>
      memcpy(&d_fir_buffer[0], &d_fir_buffer[noutput_items], d_audio_filter->ntaps());
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

