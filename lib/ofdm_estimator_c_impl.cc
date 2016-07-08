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
#include "ofdm_estimator_c_impl.h"

namespace gr {
  namespace inspector {

    ofdm_estimator_c::sptr
    ofdm_estimator_c::make(double samp_rate, int Nb,
                           const std::vector<int> &alpha,
                           const std::vector<int> &beta)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_estimator_c_impl(samp_rate, Nb, alpha, beta));
    }

    /*
     * The private constructor
     */
    ofdm_estimator_c_impl::ofdm_estimator_c_impl(double samp_rate, int Nb,
                                                 const std::vector<int> &alpha,
                                                 const std::vector<int> &beta)
      : gr::sync_block("ofdm_estimator_c",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0))
    {
      d_samp_rate = samp_rate;
      d_Nb = Nb;
      d_alpha = alpha;
      d_beta = beta;
      message_port_register_out(pmt::intern("ofdm_out"));
    }

    /*
     * Our virtual destructor.
     */
    ofdm_estimator_c_impl::~ofdm_estimator_c_impl()
    {
    }

    gr_complex
    ofdm_estimator_c_impl::autocorr(gr_complex *sig, int a, int b,
                                    int p) {
      int M = d_len;
      gr_complex R = gr_complex(0,0);
      for(int m = 0; m < M-a; m++) {
        R += sig[m+a];
      }

      R = R/M;
      return R;
    }

    int
    ofdm_estimator_c_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

