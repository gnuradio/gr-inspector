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
#include <volk/volk.h>

namespace gr {
  namespace inspector {

    ofdm_zkf_c::sptr
    ofdm_zkf_c::make(double samp_rate, const std::vector<int> &typ_len,
                     const std::vector<int> &typ_cp)
    {
      return gnuradio::get_initial_sptr
        (new ofdm_zkf_c_impl(samp_rate, typ_len, typ_cp));
    }

    /*
     * The private constructor
     */
    ofdm_zkf_c_impl::ofdm_zkf_c_impl(double samp_rate,
                                     const std::vector<int> &typ_len,
                                     const std::vector<int> &typ_cp)
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
      if(len == 0) {
        return akf;
      }
      gr_complex Rxx[len];
      gr_complex akf_temp;
      for(unsigned int k = 0; k < d_typ_len.back(); k++) {
        akf_temp = 0;
        volk_32fc_x2_multiply_conjugate_32fc(Rxx, in, &in[k], len-k);
        for(unsigned int i = 0; i < len-k; i++) {
          akf_temp += Rxx[i];
        }
        akf.push_back(std::abs(akf_temp/gr_complex((len-k),0)));
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
      if(noutput_items < d_typ_len.back()) {
        // too few items to recognize desired fft lengths
        return 0;
      }

      // Do <+signal processing+>
      //gr_complex test[3] = {gr_complex(1,0), gr_complex(2,0), gr_complex(3,0)};
      std::vector<float> akf = autocorr(in, noutput_items);
      int a = std::distance(akf.begin(), std::max_element(akf.begin()+d_typ_len.front(), akf.end()));
      std::cout << a << std::endl;

      memcpy(out, &akf[0], sizeof(float)*d_typ_len.back());

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

