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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "ofdm_synchronizer_cc_impl.h"
#include <volk/volk.h>
#include <complex>

namespace gr {
  namespace inspector {

    ofdm_synchronizer_cc::sptr
    ofdm_synchronizer_cc::make(int min_items)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_synchronizer_cc_impl(min_items));
    }

    /*
     * The private constructor
     */
    ofdm_synchronizer_cc_impl::ofdm_synchronizer_cc_impl(int min_items)
      : gr::sync_block("ofdm_synchronizer_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      d_fft_len = 0;
      d_cp_len = 0;
      d_msg_received = false;
      d_min_items = min_items;
      // message port for parameter estimations
      message_port_register_in(pmt::intern("ofdm_in"));
      set_msg_handler(pmt::intern("ofdm_in"), boost::bind(
              &ofdm_synchronizer_cc_impl::handle_msg, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    ofdm_synchronizer_cc_impl::~ofdm_synchronizer_cc_impl()
    {
    }

    void
    ofdm_synchronizer_cc_impl::handle_msg(pmt::pmt_t msg) {
      // get FFT and CP length out of parameter estmation msg
      gr::thread::scoped_lock guard(d_mutex);
      int fftlen = (int)pmt::to_float(pmt::tuple_ref(pmt::tuple_ref(msg, 4), 1));
      int cplen = (int)pmt::to_float(pmt::tuple_ref(pmt::tuple_ref(msg, 5), 1));
      if(fftlen < 10000 && fftlen > 0 && cplen < 1000 && cplen > 0) {
        d_fft_len = fftlen;
        d_cp_len = cplen;
        d_msg_received = true;
        d_tag_pos = -1;
      }
    }

    std::vector<gr_complex>
    ofdm_synchronizer_cc_impl::autocorr(const gr_complex *in, int len) {
      std::vector<gr_complex> result;
      gr_complex *temp = (gr_complex*)volk_malloc((len-d_fft_len)*
                            sizeof(gr_complex), volk_get_alignment());
      gr_complex Rxx;

      // shift equal to fft length
      volk_32fc_x2_multiply_conjugate_32fc(temp, &in[d_fft_len], in, len-d_fft_len);
      // sum up over cyclic prefix length
      for(int i = 0; i < len-d_fft_len-d_cp_len; i++) {
        Rxx = gr_complex(0,0);
        for(int k = 0; k < d_cp_len; k++) {
          Rxx += temp[i+k];
        }
        result.push_back(Rxx);
      }
      volk_free(temp);
      return result;
    }

    int
    ofdm_synchronizer_cc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      if(!d_msg_received) {
        return noutput_items;
      }

      // skip work function if too freq items provided
      if(noutput_items < d_min_items) {
        return 0;
      }

      std::vector<gr_complex> r = autocorr(in, noutput_items);
      __GR_VLA(float, r_mag, noutput_items - d_fft_len - d_cp_len);
      volk_32fc_magnitude_32f(r_mag, &r[0], noutput_items-d_fft_len-d_cp_len);
      std::vector<float> r_vec(r_mag, r_mag+noutput_items-d_fft_len-d_cp_len);
      // calculate argmax
      int k = std::distance(r_vec.begin(),
                            std::max_element(r_vec.begin(), r_vec.begin()+d_fft_len+d_cp_len));
      float n = std::arg(r[k]); // phase at argmax
      // set frequency offset compensation
      d_rotator.set_phase_incr(std::exp(gr_complex(0,-n/d_fft_len)));

      // correct frequency
      for(int i = 0; i < noutput_items; i++) {
        out[i] = d_rotator.rotate(in[i]);
      }
      if(d_tag_pos == -1) {
        d_tag_pos = k;
      }
      // add stream tags
      while(d_tag_pos < nitems_written(0)+noutput_items) {
        add_item_tag(0, d_tag_pos, pmt::intern("symbol"), pmt::from_long(d_fft_len+d_cp_len));
        d_tag_pos += d_fft_len+d_cp_len;
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */
