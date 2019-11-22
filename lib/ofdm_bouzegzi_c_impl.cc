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
#include "ofdm_bouzegzi_c_impl.h"
#include <complex>
#include <volk/volk.h>

namespace gr {
  namespace inspector {

    ofdm_bouzegzi_c::sptr
    ofdm_bouzegzi_c::make(double samp_rate, int Nb,
                          const std::vector<int> &alpha,
                          const std::vector<int> &beta)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_bouzegzi_c_impl(samp_rate, Nb, alpha, beta));
    }

    /*
     * The private constructor
     */
    ofdm_bouzegzi_c_impl::ofdm_bouzegzi_c_impl(double samp_rate, int Nb,
                                                 const std::vector<int> &alpha,
                                                 const std::vector<int> &beta)
      : gr::sync_block("ofdm_bouzegzi_c",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0))
    {
      d_samp_rate = samp_rate;
      d_Nb = Nb;
      d_alpha = alpha;
      d_beta = beta;
      d_fft = new fft::fft_complex(1024, true);
      message_port_register_out(pmt::intern("ofdm_out"));
      d_len = 10000;

      // generate vectors
      d_x1 = (float*)volk_malloc(sizeof(float)*d_len, volk_get_alignment()); //real part of sig
      d_y1 = (float*)volk_malloc(sizeof(float)*d_len, volk_get_alignment()); //imag part of sig
      d_x2 = (float*)volk_malloc(sizeof(float)*d_len, volk_get_alignment()); //real part of oscillation
      d_y2 = (float*)volk_malloc(sizeof(float)*d_len, volk_get_alignment()); //imag part of oscillation
      d_tmp1 = (float*) volk_malloc(sizeof(float)*d_len, volk_get_alignment());
      d_tmp2 = (float*) volk_malloc(sizeof(float)*d_len, volk_get_alignment());
      d_real_pre = (float*) volk_malloc(sizeof(float)*d_len, volk_get_alignment());
      d_imag_pre = (float*) volk_malloc(sizeof(float)*d_len, volk_get_alignment());
      d_sig_shift = (gr_complex*)volk_malloc(sizeof(gr_complex)*(d_len), volk_get_alignment());
      d_res = (gr_complex*) volk_malloc(sizeof(gr_complex)*(d_len), volk_get_alignment());
      d_osc_vec = (float*)volk_malloc(sizeof(float)*d_len, volk_get_alignment());
    }

    /*
     * Our virtual destructor.
     */
    ofdm_bouzegzi_c_impl::~ofdm_bouzegzi_c_impl()
    {
      volk_free(d_x1);
      volk_free(d_y1);
      volk_free(d_tmp1);
      volk_free(d_tmp2);
      volk_free(d_real_pre);
      volk_free(d_imag_pre);
      volk_free(d_sig_shift);
      volk_free(d_res);
      volk_free(d_osc_vec);
      volk_free(d_x2);
      volk_free(d_y2);
    }

    gr_complex
    ofdm_bouzegzi_c_impl::autocorr_orig(const gr_complex *sig, int a, int b,
                                         int p) {
      std::cout << "----------- ORIG ------------" << std::endl;
      int M = d_len;
      gr_complex R = gr_complex(0,0);

      for(int m = 0; m < 100; m++) {
        R += sig[m+a] * std::conj(sig[m]) *
             std::exp(gr_complex(0,-1)*gr_complex(2*M_PI*p*m/(a/b+a),0));
        std::cout << sig[m+a] * std::conj(sig[m]) *
                     std::exp(gr_complex(0,-1)*gr_complex(2*M_PI*p*m/(a/b+a),0)) << " ";
      }
      std::cout << std::endl;

      R = R/gr_complex(M-a,0); // normalize
      //std::cout << "R = " << R << std::endl;
      return R;

    }
    float
    ofdm_bouzegzi_c_impl::autocorr(const gr_complex *sig, int a, int b,
                                    int p) {
      //std::cout << "----------- VOLK ------------" << std::endl;
      int M = d_len;
      /*
      gr_complex f[M];
      for(int i = 0; i < M; i++) {
        f[i] = sig[i] * std::exp(gr_complex(0,1)*gr_complex(2*M_PI*i*p/(a/b+a),0));
      }


      gr_complex f_fft[M];
      gr_complex g_fft[M];
      gr_complex R_vec[M];

      // fast convolution
      do_fft(f, f_fft);
      do_fft(sig, g_fft);
      for(int i = 0; i < d_len; i++) {
        R_vec[i] = f_fft[i]*g_fft[i]; // convolution
      }
      gr_complex result[M];
      rescale_fft(false);
      do_fft(R_vec, result);
      gr_complex R = result[a];
       */

      gr_complex R = gr_complex(0,0);

      // TODO: make this cooler
      __GR_VLA(float, m_vec, M);
      for(unsigned int i = 0; i < M; i++){
        m_vec[i] = i;
      }
      // create oscillation argument
      volk_32f_s32f_multiply_32f(d_osc_vec, m_vec, -2*M_PI*p/(a/b+a), M);

      volk_32fc_deinterleave_real_32f(d_x1, sig, M);
      volk_32fc_deinterleave_imag_32f(d_y1, sig, M);
      volk_32f_cos_32f(d_x2, d_osc_vec, M); // create cosine vector
      volk_32f_sin_32f(d_y2, d_osc_vec, M); // create sine vector


      volk_32f_x2_multiply_32f(d_tmp1, d_x1, d_x2, M);
      volk_32f_x2_multiply_32f(d_tmp2, d_y1, d_y2, M);
      volk_32f_x2_add_32f(d_real_pre, d_tmp1, d_tmp2, M); // final real part

      volk_32f_x2_multiply_32f(d_tmp1, d_x1, d_y2, M);
      volk_32f_x2_multiply_32f(d_tmp2, d_x2, d_y1, M);
      volk_32f_x2_subtract_32f(d_imag_pre, d_tmp1, d_tmp2, M); // final imag part


      __GR_VLA(gr_complex, pre, M);
      for(unsigned int i = 0; i < M; i++) {
        pre[i] = d_real_pre[i]+gr_complex(0,1)*d_imag_pre[i];
      }

      memcpy(d_sig_shift, &sig[a], (M-a)*sizeof(gr_complex));
      volk_32fc_x2_multiply_32fc(d_res, d_sig_shift, pre, M-a);

      for(unsigned int i = 0; i< M-a; i++) {
        R += d_res[i];
        //std::cout << d_res[i] << " ";
      }
      //std::cout << std::endl;

      /*
      for(int m = 0; m < M-a; m++) {
        R += sig[m+a] * std::conj(sig[m]) *
                std::exp(gr_complex(0,-1)*gr_complex(2*M_PI*p*m/(a/b+a),0));
      }
      */
      R = R/gr_complex(M-a,0); // normalize
      //std::cout << "R = " << R << std::endl;
      return std::abs(R);
    }

    void
    ofdm_bouzegzi_c_impl::rescale_fft(bool forward) {
      delete d_fft;
      d_fft = new fft::fft_complex(d_len, forward);
      d_fft->set_nthreads(4);
    }

    void
    ofdm_bouzegzi_c_impl::do_fft(const gr_complex *in, gr_complex *out) {
      memcpy(d_fft->get_inbuf(), in, d_len*sizeof(gr_complex));
      d_fft->execute();
      memcpy(out, d_fft->get_outbuf(), d_len*sizeof(gr_complex));
    }

    float
    ofdm_bouzegzi_c_impl::cost_func(const gr_complex *sig, int a,
                                     int b) {
      float J = 0;
      __GR_VLA(float, power, 2 * d_Nb + 1);
      __GR_VLA(float, R, 2 * d_Nb + 1);
      for(int p = -d_Nb; p <= d_Nb; p++) {
        R[p + d_Nb] = autocorr(sig, a, b, p);
      }
      volk_32f_s32f_power_32f(power, R, 2.0, 2*d_Nb+1);
      for(int i = 0; i < 2*d_Nb+1; i++) {
        J += power[i];
      }
      J = J/(2*d_Nb+1); // normalize
      return J;
    }

    int
    ofdm_bouzegzi_c_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      d_len = noutput_items;
      rescale_fft(true);
      //std::cout << "len = " << d_len << std::endl;

      // we need a max number of items for analysis
      if(d_len < 7000) {
        return 0;
      }

      // Do <+signal processing+>
      float J = 0.0;
      float J_new;
      int a_res = 0;
      int b_res = 0;

      for(std::vector<int>::iterator a = d_alpha.begin();
              a != d_alpha.end(); ++a) {
        for(std::vector<int>::iterator b = d_beta.begin();
                b != d_beta.end(); ++b){
          J_new = cost_func(in, *a, *b);
          //std::cout << "a = " << *a << ", b = " << *b << ", J = " << J_new << std::endl;
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
