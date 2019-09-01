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

#ifndef INCLUDED_INSPECTOR_OFDM_BOUZEGZI_C_IMPL_H
#define INCLUDED_INSPECTOR_OFDM_BOUZEGZI_C_IMPL_H

#include <inspector/ofdm_bouzegzi_c.h>
#include <gnuradio/fft/fft.h>

namespace gr {
  namespace inspector {

    class ofdm_bouzegzi_c_impl : public ofdm_bouzegzi_c
    {
     private:
      int d_Nb, d_len;
      double d_samp_rate;
      float *d_x1, *d_y1, *d_x2, *d_y2, *d_tmp1, *d_tmp2, *d_real_pre, *d_imag_pre, *d_osc_vec;
      gr_complex *d_sig_shift, *d_res;
      std::vector<int> d_alpha, d_beta;
      fft::fft_complex *d_fft;

     public:

      ofdm_bouzegzi_c_impl(double samp_rate, int Nb,
                           const std::vector<int> &alpha,
                           const std::vector<int> &beta);

      ~ofdm_bouzegzi_c_impl();

      void rescale_fft(bool forward);
      void do_fft(const gr_complex *in, gr_complex *out);

      float autocorr(const gr_complex *sig, int a, int b, int p);
      gr_complex autocorr_orig(const gr_complex *sig, int a, int b, int p);
      float cost_func(const gr_complex *sig, int a, int b);

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_OFDM_BOUZEGZI_C_IMPL_H */

