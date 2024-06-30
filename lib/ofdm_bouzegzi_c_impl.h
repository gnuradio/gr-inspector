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

#include <gnuradio/fft/fft.h>
#include <gnuradio/inspector/ofdm_bouzegzi_c.h>
#include <volk/volk_alloc.hh>
#include <memory>

namespace gr {
namespace inspector {

class ofdm_bouzegzi_c_impl : public ofdm_bouzegzi_c
{
public:
    static constexpr unsigned int DEFAULT_FFT_LEN = 1024;
    static constexpr unsigned int MIN_NOUTPUT_ITEMS = 7000;
    static constexpr unsigned int DEFAULT_LEN = 10000;

private:
    int d_Nb;
    unsigned int d_len = DEFAULT_LEN;
    volk::vector<float> d_x1;
    volk::vector<float> d_y1;
    volk::vector<float> d_x2;
    volk::vector<float> d_y2;
    volk::vector<float> d_tmp1;
    volk::vector<float> d_tmp2;
    volk::vector<float> d_real_pre;
    volk::vector<float> d_imag_pre;
    volk::vector<float> d_osc_vec;
    volk::vector<gr_complex> d_sig_shift, d_res;
    std::vector<int> d_alpha, d_beta;
    std::unique_ptr<fft::fft_complex_fwd> d_fft;

    void rescale_fft(unsigned int length);

    float autocorr(const gr_complex* sig, int a, int b, int p, unsigned int length);
    gr_complex autocorr_orig(const gr_complex* sig, int a, int b, int p);
    float cost_func(const gr_complex* sig, int a, int b, unsigned int length);

public:
    ofdm_bouzegzi_c_impl(double samp_rate,
                         int Nb,
                         const std::vector<int>& alpha,
                         const std::vector<int>& beta);

    ~ofdm_bouzegzi_c_impl() override;


    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_OFDM_BOUZEGZI_C_IMPL_H */
