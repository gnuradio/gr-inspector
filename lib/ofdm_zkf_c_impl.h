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

#ifndef INCLUDED_INSPECTOR_OFDM_ZKF_C_IMPL_H
#define INCLUDED_INSPECTOR_OFDM_ZKF_C_IMPL_H

#include <inspector/ofdm_zkf_c.h>
#include <gnuradio/fft/fft.h>

namespace gr {
  namespace inspector {

    class ofdm_zkf_c_impl : public ofdm_zkf_c
    {
     private:
      double d_samp_rate;
      unsigned int d_signal;
      std::vector<int> d_typ_len, d_typ_cp;
      gr_complex *d_Rxx;
      fft::fft_complex *d_fft;
      unsigned int d_tmpbuflen;
      int d_min_items;

     public:
      ofdm_zkf_c_impl(double samp_rate, int signal, int min_items,
                      const std::vector<int> &typ_len,
                      const std::vector<int> &typ_cp);
      ~ofdm_zkf_c_impl();
      std::vector<float> autocorr(const gr_complex *in, int len);
      gr_complex* tv_autocorr(const gr_complex *in, int len, int shift);
      int round_to_list(int val, std::vector<int> *list);
      void resize_fft(int size);

      void set_samp_rate(double d_samp_rate);

      pmt::pmt_t pack_message(float subc, float time, int fft, int cp);
      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_OFDM_ZKF_C_IMPL_H */
