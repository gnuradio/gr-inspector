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
#include "ofdm_zkf_c_impl.h"
#include <complex>
#include <volk/volk.h>

namespace gr {
  namespace inspector {

    ofdm_zkf_c::sptr
    ofdm_zkf_c::make(double samp_rate, int signal, int min_items,
                     const std::vector<int> &typ_len,
                     const std::vector<int> &typ_cp)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_zkf_c_impl(samp_rate, signal, min_items, typ_len,
                             typ_cp));
    }

    /*
     * The private constructor
     */
    ofdm_zkf_c_impl::ofdm_zkf_c_impl(double samp_rate, int signal,
                                     int min_items,
                                     const std::vector<int> &typ_len,
                                     const std::vector<int> &typ_cp)
      : gr::sync_block("ofdm_zkf_c",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(0, 0, 0))
    {
      d_samp_rate = samp_rate;
      d_typ_len = typ_len;
      d_typ_cp = typ_cp;
      d_signal = signal;
      d_min_items = min_items;
      d_fft = new fft::fft_complex(1024, true);
      message_port_register_out(pmt::intern("ofdm_out"));
    }

    /*
     * Our virtual destructor.
     */
    ofdm_zkf_c_impl::~ofdm_zkf_c_impl()
    {
      delete d_fft;
    }

    // calculate autocorrelation function of given vector
    std::vector<float>
    ofdm_zkf_c_impl::autocorr(const gr_complex *in, int len) {
      std::vector<float> akf;
      if(len == 0) {
        return akf;
      }
      __GR_VLA(gr_complex, Rxx, len);
      gr_complex akf_temp;
      for(unsigned int k = 0; k < d_typ_len.back(); k++) {
        akf_temp = 0;
        volk_32fc_x2_multiply_conjugate_32fc(Rxx, in, &in[k], len-k);
        // sum
        for(unsigned int i = 0; i < len-k; i++) {
          akf_temp += Rxx[i];
        }
        akf.push_back(std::abs(akf_temp/gr_complex((len-k),0)));
      }
      return akf;
    }

    void ofdm_zkf_c_impl::set_samp_rate(double d_samp_rate) {
      ofdm_zkf_c_impl::d_samp_rate = d_samp_rate;
    }

    // calculate time variant autocorrelation for fixed shift
    gr_complex*
    ofdm_zkf_c_impl::tv_autocorr(const gr_complex *in, int len,
                                 int shift) {
      __GR_VLA(gr_complex, corr_temp, len);
      gr_complex *Rxx = (gr_complex*)volk_malloc(len*sizeof(gr_complex), volk_get_alignment());
      gr_complex R = gr_complex(0,0);
      volk_32fc_x2_multiply_conjugate_32fc(corr_temp, in, &in[shift], len);
      int k = 0;
      // begin at back and summarize up to front
      for(int i = len-1; i >= 0; i--) {
        R *= k;
        R += corr_temp[i];
        R *= 1.0/(k+1.0);
        Rxx[k] = R;
        k++;
      }
      return Rxx;
    }

    // round value to nearest list entry
    int
    ofdm_zkf_c_impl::round_to_list(int val, std::vector<int> *list) {
      int result = -1;
      int diff = 99999; // "high value"
      for(unsigned int i = 0; i < list->size(); i++) {
        if(std::abs(list->at(i) - val) < diff) {
          diff = std::abs(list->at(i) - val);
          result = list->at(i);
        }
      }
      return result;
    }

    void
    ofdm_zkf_c_impl::resize_fft(int size) {
      delete d_fft;
      d_fft = new fft::fft_complex(size, true);
    }

    // GUI message
    pmt::pmt_t
    ofdm_zkf_c_impl::pack_message(float subc, float time, int fft,
                                  int cp) {
      pmt::pmt_t identifier = pmt::make_tuple(pmt::string_to_symbol("Signal"), pmt::from_uint64(d_signal));
      pmt::pmt_t info = pmt::make_tuple(pmt::string_to_symbol("OFDM"), pmt::from_float(1));
      pmt::pmt_t subcarr = pmt::make_tuple(pmt::string_to_symbol("Subc. space"), pmt::from_float(subc));
      pmt::pmt_t symtime = pmt::make_tuple(pmt::string_to_symbol("Sym time"), pmt::from_float(time));
      pmt::pmt_t fftsize = pmt::make_tuple(pmt::string_to_symbol("Subcarriers"), pmt::from_float(fft));
      pmt::pmt_t cyclpre = pmt::make_tuple(pmt::string_to_symbol("CP len"), pmt::from_float(cp));
      pmt::pmt_t msg = pmt::make_tuple(identifier, info, subcarr, symtime, fftsize, cyclpre);
      return msg;
    }

    int
    ofdm_zkf_c_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      if(noutput_items <= d_min_items) {
        // too few items to make good estimations
        return 0;
      }

      // calculate autocorrelation and estimate FFT length
      std::vector<float> akf = autocorr(in, noutput_items);
      // argmax
      int a = std::distance(akf.begin(),
          std::max_element(akf.begin()+d_typ_len.front(), akf.end()));
      a = round_to_list(a, &d_typ_len); // round to possible values

      // calculate time variant autocorr and cyclic correlation function
      // and estimate CP length
      int fft_len = noutput_items-a; // length of all following vectors because of shift a
      resize_fft(fft_len); // resize FFT

      gr_complex *Rxx;
      Rxx = tv_autocorr(in, fft_len, a); // calc time varaint autocorr

      // FFT to get CCF (cyclic correlation function)
      memcpy(d_fft->get_inbuf(), Rxx, sizeof(gr_complex)*fft_len);
      d_fft->execute();
      volk_free(Rxx);
      __GR_VLA(float, result, fft_len); // magnitude of CCF
      volk_32fc_magnitude_32f(result, d_fft->get_outbuf(), fft_len);

      // fftshift
      d_tmpbuflen = static_cast<unsigned int>(std::floor((fft_len) / 2.0));
      __GR_VLA(float, d_tmpbuf, fft_len / 2);
      memcpy(d_tmpbuf, &result[0], sizeof(float) * (d_tmpbuflen + 1));
      memcpy(&result[0], &result[fft_len - d_tmpbuflen],
             sizeof(float) * (d_tmpbuflen));
      memcpy(&result[d_tmpbuflen], d_tmpbuf,
             sizeof(float) * (d_tmpbuflen + 1));

      // only use positive frequencies
      std::vector<float> Cxx(result+(fft_len)/2, result+fft_len);
      // search for peak in possible area
      long b = std::distance(Cxx.begin(), std::max_element(
              Cxx.begin()+(int)(fft_len/(a+d_typ_cp.back())),
              Cxx.begin()+(int)(fft_len/(a+d_typ_cp.front()))));
      b = fft_len/b; // convert peak to length value
      b = b-a; // calculate CP len from total len
      b = round_to_list(b, &d_typ_cp); // round to possible value

      // calculate subcarr spacing and symbol time and pub message
      float subspc = d_samp_rate/a;
      float symtime = 1/subspc;
      pmt::pmt_t msg = pack_message(subspc, symtime, a, b);
      message_port_pub(pmt::intern("ofdm_out"), msg);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */
