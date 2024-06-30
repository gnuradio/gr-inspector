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
#include <config.h>
#endif

#include "ofdm_bouzegzi_c_impl.h"
#include <gnuradio/gr_complex.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/logger.h>
#include <spdlog/fmt/ranges.h>
#include <volk/volk.h>
#include <volk/volk_alloc.hh>
#include <complex>
#include <memory>

namespace gr {
namespace inspector {

ofdm_bouzegzi_c::sptr ofdm_bouzegzi_c::make(double samp_rate,
                                            int Nb,
                                            const std::vector<int>& alpha,
                                            const std::vector<int>& beta)
{
    return gnuradio::get_initial_sptr(
        new ofdm_bouzegzi_c_impl(samp_rate, Nb, alpha, beta));
}

/*
 * The private constructor
 */
ofdm_bouzegzi_c_impl::ofdm_bouzegzi_c_impl(double samp_rate,
                                           int Nb,
                                           const std::vector<int>& alpha,
                                           const std::vector<int>& beta)
    : gr::sync_block("ofdm_bouzegzi_c",
                     gr::io_signature::make(1, 1, sizeof(gr_complex)),
                     gr::io_signature::make(0, 0, 0)),
      d_Nb(Nb),
      /* TODO: 2024-06-30b: Almost certain most of these buffers are redundant, and
       * DEFAULT_LEN makes no sense, whatsoever. Fix. See Fixme comment in work.
       */
      d_x1(DEFAULT_LEN),
      d_y1(DEFAULT_LEN),
      d_x2(DEFAULT_LEN),
      d_y2(DEFAULT_LEN),
      d_tmp1(DEFAULT_LEN),
      d_tmp2(DEFAULT_LEN),
      d_real_pre(DEFAULT_LEN),
      d_imag_pre(DEFAULT_LEN),
      d_osc_vec(DEFAULT_LEN),
      d_sig_shift(DEFAULT_LEN),
      d_res(DEFAULT_LEN),
      d_alpha(alpha),
      d_beta(beta),
      d_fft(std::make_unique<fft::fft_complex_fwd>(DEFAULT_FFT_LEN))
{
    set_min_noutput_items(MIN_NOUTPUT_ITEMS);
    message_port_register_out(pmt::intern("ofdm_out"));
}

/*
 * Our virtual destructor.
 */
ofdm_bouzegzi_c_impl::~ofdm_bouzegzi_c_impl() {}

float ofdm_bouzegzi_c_impl::autocorr(
    const gr_complex* sig, int a, int b, int p, unsigned int length)
{
    /* TODO: 2024-06-30a: DEFINITELY make this cooler.
     * We have a lot of unnecessary vector members, and here we make (allocate and
     * deallocate two new ones every single iteration, "m_vec" and "pre".
     */
    volk::vector<float> m_vec(length);
    for (unsigned int i = 0; i < length; i++) {
        m_vec[i] = i;
    }
    // create oscillation argument
    /* TODO: The integer division done here seems wrong, but I don't know the original
     * intent. Figure out whether p/(a/b+a) is correct here.
     */
    volk_32f_s32f_multiply_32f(
        d_osc_vec.data(), m_vec.data(), -2 * M_PI * p / (a / b + a), length);

    volk_32fc_deinterleave_real_32f(d_x1.data(), sig, length);
    volk_32fc_deinterleave_imag_32f(d_y1.data(), sig, length);
    volk_32f_cos_32f(d_x2.data(), d_osc_vec.data(), length); // create cosine vector
    volk_32f_sin_32f(d_y2.data(), d_osc_vec.data(), length); // create sine vector


    volk_32f_x2_multiply_32f(d_tmp1.data(), d_x1.data(), d_x2.data(), length);
    volk_32f_x2_multiply_32f(d_tmp2.data(), d_y1.data(), d_y2.data(), length);
    volk_32f_x2_add_32f(
        d_real_pre.data(), d_tmp1.data(), d_tmp2.data(), length); // final real part

    volk_32f_x2_multiply_32f(d_tmp1.data(), d_x1.data(), d_y2.data(), length);
    volk_32f_x2_multiply_32f(d_tmp2.data(), d_x2.data(), d_y1.data(), length);
    volk_32f_x2_subtract_32f(
        d_imag_pre.data(), d_tmp1.data(), d_tmp2.data(), length); // final imag part


    volk::vector<gr_complex> pre(length);
    for (unsigned int i = 0; i < length; i++) {
        pre[i] = d_real_pre[i] + gr_complex(0, 1) * d_imag_pre[i];
    }

    memcpy(d_sig_shift.data(), &sig[a], (length - a) * sizeof(gr_complex));
    volk_32fc_x2_multiply_32fc(d_res.data(), d_sig_shift.data(), pre.data(), length - a);

    gr_complex R{ 0, 0 };
    for (unsigned int i = 0; i < length - a; i++) {
        R += d_res[i];
    }

    /*
    for(int m = 0; m < M-a; m++) {
      R += sig[m+a] * std::conj(sig[m]) *
              std::exp(gr_complex(0,-1)*gr_complex(2*M_PI*p*m/(a/b+a),0));
    }
    */
    R = R / static_cast<float>(length - a); // normalize
    return std::abs(R);
}

void ofdm_bouzegzi_c_impl::rescale_fft(unsigned int length)
{
    d_fft = std::make_unique<fft::fft_complex_fwd>(length, 4 /*threads*/);
}


float ofdm_bouzegzi_c_impl::cost_func(const gr_complex* sig,
                                      int a,
                                      int b,
                                      unsigned int length)
{
    float J = 0;
    volk::vector<float> power(2 * d_Nb + 1);
    volk::vector<float> R(2 * d_Nb + 1);
    for (int p = -d_Nb; p <= d_Nb; p++) {
        R[p + d_Nb] = autocorr(sig, a, b, p, length);
    }
    volk_32f_s32f_power_32f(power.data(), R.data(), 2.0, 2 * d_Nb + 1);
    for (int i = 0; i < 2 * d_Nb + 1; i++) {
        J += power[i];
    }
    J = J / (2 * d_Nb + 1); // normalize
    return J;
}

int ofdm_bouzegzi_c_impl::work(int noutput_items,
                               gr_vector_const_void_star& input_items,
                               gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    /*
     * FIXME: 2024-06-30 This a terrible idea and will lead to inconsistent results
     * instead of choosing a different FFT size every time, the author should have
     * chose an input multiple and worked with the same length.
     *
     * However, mission right now is only to make this compile, and if you're inclined to
     * use this block, please fix this.
     */
    rescale_fft(noutput_items);

    // TODO: after fixing the above, remove the next lines
    d_logger->warn("Calculating based on FFT length {}. If this number changes, your "
                   "results are inconsistent. There is a fix for that, which you should "
                   "contribute upstream -- see {}'s FIXME 2024-06-30!",
                   noutput_items,
                   __FILE__);
    // we need a max number of items for analysis
    if (noutput_items < static_cast<int>(MIN_NOUTPUT_ITEMS)) {
        return 0;
    }
#warning This block has consistency issues. See the FIXME 2024-06-30 comment above. DO NOT USE IN PRODUCTION.

    // Do <+signal processing+>
    float J = 0.0;
    float J_new;
    int a_res = 0;
    int b_res = 0;

    for (std::vector<int>::iterator a = d_alpha.begin(); a != d_alpha.end(); ++a) {
        for (std::vector<int>::iterator b = d_beta.begin(); b != d_beta.end(); ++b) {
            J_new = cost_func(in, *a, *b, noutput_items);
            if (J_new > J) {
                J = J_new;
                a_res = *a;
                b_res = *b;
            }
        }
    }
    d_logger->trace("-------- Result -------\n"
                    "FFT len = {}\n"
                    "CP len = {}\n",
                    a_res,
                    1.0 / b_res * a_res);

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

} /* namespace inspector */
} // namespace gr
/* namespace gr */
