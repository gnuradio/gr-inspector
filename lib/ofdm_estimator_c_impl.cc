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
#include <volk/volk.h>

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
      d_fft = new fft::fft_complex(1024, true);
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
      float m_vec[M];
      for(unsigned int i = 0; i < M; i++){
        m_vec[i] = i;
      }

      float *osc_vec = (float*)volk_malloc(sizeof(float)*M, volk_get_alignment());
      // create oscillation argument
      volk_32f_s32f_multiply_32f(osc_vec, m_vec, -2*M_PI*p/(a/b+a), M);

      // generate vectors
      float *x1 = (float*)volk_malloc(sizeof(float)*M, volk_get_alignment()); //real part of sig
      float *y1 = (float*)volk_malloc(sizeof(float)*M, volk_get_alignment()); //imag part of sig
      float *x2 = (float*)volk_malloc(sizeof(float)*M, volk_get_alignment()); //real part of oscillation
      float *y2 = (float*)volk_malloc(sizeof(float)*M, volk_get_alignment()); //imag part of oscillation

      volk_32fc_deinterleave_real_32f(x1, sig, M);
      volk_32fc_deinterleave_imag_32f(y1, sig, M);
      volk_32f_cos_32f(x2, osc_vec, M); // create cosine vector
      volk_32f_sin_32f(y2, osc_vec, M); // create sine vector

      float *tmp1 = (float*) volk_malloc(sizeof(float)*M, volk_get_alignment());
      float *tmp2 = (float*) volk_malloc(sizeof(float)*M, volk_get_alignment());
      float *real_pre = (float*) volk_malloc(sizeof(float)*M, volk_get_alignment());
      float *imag_pre = (float*) volk_malloc(sizeof(float)*M, volk_get_alignment());

      volk_32f_x2_multiply_32f(tmp1, x1, x2, M);
      volk_32f_x2_multiply_32f(tmp2, y1, y2, M);
      volk_32f_x2_add_32f(real_pre, tmp1, tmp2, M); // final real part

      volk_32f_x2_multiply_32f(tmp1, x1, y2, M);
      volk_32f_x2_multiply_32f(tmp2, x2, y1, M);
      volk_32f_x2_subtract_32f(imag_pre, tmp1, tmp2, M); // final imag part

      volk_free(x2);
      volk_free(y2);

      float *sig_shift_real = (float*)volk_malloc(sizeof(float)*(M-a), volk_get_alignment());
      float *sig_shift_imag = (float*)volk_malloc(sizeof(float)*(M-a), volk_get_alignment());

      memcpy(sig_shift_real, &x1[a], M-a);
      memcpy(sig_shift_imag, &y1[a], M-a);

      float *real = (float*) volk_malloc(sizeof(float)*(M-a), volk_get_alignment());
      float *imag = (float*) volk_malloc(sizeof(float)*(M-a), volk_get_alignment());

      volk_32f_x2_multiply_32f(tmp1, sig_shift_real, real_pre, M-a);
      volk_32f_x2_multiply_32f(tmp2, sig_shift_imag, imag_pre, M-a);
      volk_32f_x2_subtract_32f(real, tmp1, tmp2, M-a);

      volk_32f_x2_multiply_32f(tmp1, sig_shift_real, imag_pre, M-a);
      volk_32f_x2_multiply_32f(tmp2, sig_shift_imag, real_pre, M-a);
      volk_32f_x2_add_32f(imag, tmp1, tmp2, M-a);

      for(unsigned int i = 0; i < M-a; i++) {
        R += real[i]+gr_complex(0,1)*imag[i];
      }

      /*
      for(int m = 0; m < M-a; m++) {
        R += sig[m+a] * std::conj(sig[m]) *
                std::exp(gr_complex(0,-1)*gr_complex(2*M_PI*p*m/(a/b+a),0));
      }
      */

      volk_free(x1);
      volk_free(y1);
      volk_free(tmp1);
      volk_free(tmp2);
      volk_free(real_pre);
      volk_free(imag_pre);
      volk_free(sig_shift_real);
      volk_free(sig_shift_imag);
      volk_free(real);
      volk_free(imag);


      R = R/gr_complex(M,0); // normalize
      return R;
    }

    void
    ofdm_estimator_c_impl::rescale_fft(bool forward) {
      delete d_fft;
      d_fft = new fft::fft_complex(d_len, forward);
      d_fft->set_nthreads(4);
    }

    void
    ofdm_estimator_c_impl::do_fft(const gr_complex *in, gr_complex *out) {
      memcpy(d_fft->get_inbuf(), in, d_len*sizeof(gr_complex));
      d_fft->execute();
      memcpy(out, d_fft->get_outbuf(), d_len*sizeof(gr_complex));
    }

    float
    ofdm_estimator_c_impl::cost_func(const gr_complex *sig, int a,
                                     int b) {
      float J = 0;
      for(int p = -d_Nb; p <= d_Nb; p++) {
        J += std::pow(std::abs(autocorr(sig, a, b, p)), 2.0);
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

