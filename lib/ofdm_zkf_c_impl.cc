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
#include "ofdm_zkf_c_impl.h"
#include <complex>

namespace gr {
  namespace inspector {

    ofdm_zkf_c::sptr
    ofdm_zkf_c::make(double samp_rate, const std::vector<int> &typ_len, const std::vector<int> &typ_cp)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_zkf_c_impl(samp_rate, typ_len, typ_cp));
    }

    /*
     * The private constructor
     */
    ofdm_zkf_c_impl::ofdm_zkf_c_impl(double samp_rate, const std::vector<int> &typ_len, const std::vector<int> &typ_cp)
      : gr::sync_block("ofdm_zkf_c",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float)))
    {
      d_samp_rate = samp_rate;
      d_typ_len = typ_len;
      d_typ_cp = typ_cp;
    }

    /*
     * Our virtual destructor.
     */
    ofdm_zkf_c_impl::~ofdm_zkf_c_impl()
    {
    }

    std::vector<float>
    ofdm_zkf_c_impl::autocorr(const gr_complex *in, int len) {
      std::vector<float> akf;
      float Rxx = 0;
      for(unsigned int k = 0; k < len; k++) {
        for(unsigned int n = 0; n < len-k; n++) {
          Rxx += std::abs(in[n+k]*std::conj(in[n]));
        }
        akf.push_back(Rxx/(len-k-1)); // TODO: div by zero
        Rxx = 0;
      }
      return akf;
    }

    int
    ofdm_zkf_c_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      // Do <+signal processing+>
      std::vector<float> akf = autocorr(in, noutput_items);

      memcpy(out, &akf[0], sizeof(float)*noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

