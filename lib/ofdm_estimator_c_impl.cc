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
#include <complex>

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
    ofdm_estimator_c_impl::autocorr(const gr_complex *sig, int a, int b,
                                    int p) {
      int M = d_len;
      gr_complex R = gr_complex(0,0);

      gr_complex base = std::exp(gr_complex(0,-1)*gr_complex(2*M_PI,0)*
                                 gr_complex(p,0)/
                                 gr_complex(a*(1+b),0));

      for(int m = 0; m < M-a; m++) {
        R += sig[m+a] * std::conj(sig[m]) * std::pow(base, m);
      }

      R = R/gr_complex(M,0); // normalize
      return R;
    }

    float
    ofdm_estimator_c_impl::cost_func(const gr_complex *sig, int a,
                                     int b) {
      float J = 0;
      for(int p = -d_Nb; p <= d_Nb; p++) {
        J += std::abs(autocorr(sig, a, b, p));
      }
      J = J/(2*d_Nb+1); // normalize
      return J;
    }

    int
    ofdm_estimator_c_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      d_len = noutput_items;
      //if(d_len < 512*20) {
      //  return 0;
      //}

      // Do <+signal processing+>
      float J = 0.0;
      float J_new;
      int a_res = 0;
      int b_res = 0;

      for(std::vector<int>::iterator a = d_alpha.begin();
              a != d_alpha.end(); ++a) {
        for(std::vector<int>::iterator b = d_beta.begin();
                b != d_beta.end(); ++b){
          J_new = cost_func(in, *a, 1/ *b);
          if(J_new > J) {
            J = J_new;
            a_res = *a;
            b_res = *b;
          }
        }
      }
      std::cout << "-------- Result -------" << std::endl;
      std::cout << "FFT len = " << a_res << std::endl;
      std::cout << "CP len = " << 1.0/b_res*a_res << std::endl;

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

