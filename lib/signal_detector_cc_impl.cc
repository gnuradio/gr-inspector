/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
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
#include "signal_detector_cc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace inspector {

    signal_detector_cc::sptr
    signal_detector_cc::make(double samp_rate, int fft_len,
                             int window_type, float threshold,
                             float sensitivity, bool auto_threshold) {
      return gnuradio::get_initial_sptr
              (new signal_detector_cc_impl(samp_rate, fft_len, window_type,
                                           threshold, sensitivity,
                                           auto_threshold));
    }

    /*
     * The private constructor
     */
    signal_detector_cc_impl::signal_detector_cc_impl(double samp_rate,
                                                     int fft_len,
                                                     int window_type,
                                                     float threshold,
                                                     float sensitivity,
                                                     bool auto_threshold)
            : gr::block("signal_detector_cc",
                        gr::io_signature::make(1, 1,
                                               sizeof(gr_complex)),
                        gr::io_signature::make(1, 1, sizeof(float))) {
      set_samp_rate(samp_rate);
      set_fft_len(fft_len);
      set_window_type(window_type);
      set_threshold(threshold);
      set_sensitivity(sensitivity);
      set_auto_threshold(auto_threshold);
      message_port_register_out(pmt::intern("map_out"));

      //fill properties
      build_window();
      d_fft = new fft::fft_complex(fft_len, true);
    }

    /*
     * Our virtual destructor.
     */
    signal_detector_cc_impl::~signal_detector_cc_impl() {
      delete d_fft;
    }

    void
    signal_detector_cc_impl::periodogram(float *pxx, const gr_complex *signal) {
      // window signal
      if(d_window.size()) {
        volk_32fc_32f_multiply_32fc(d_fft->get_inbuf(), signal, &d_window.front(), d_fft_len);
      }
      else {
        memcpy(d_fft->get_inbuf(), signal, sizeof(gr_complex)*d_fft_len);
      }
      d_fft->execute();
      volk_32fc_magnitude_squared_32f(pxx, d_fft->get_outbuf(), d_fft_len);
      volk_32f_s32f_normalize(pxx, d_fft_len, d_fft_len);
      // calculate in dB
      volk_32f_log2_32f(pxx, pxx, d_fft_len);
      volk_32f_s32f_normalize(pxx, 10/log2(10),d_fft_len);
    }

    std::vector<float>
    signal_detector_cc_impl::build_freq() {
      std::vector<float> freq(d_fft_len);
      float point = -d_samp_rate/2;
      for(std::vector<float>::iterator it=freq.begin(); it!= freq.end();
              ++it) {
        *it = point;
        point += d_samp_rate/d_fft_len;
      }
    }

    void
    signal_detector_cc_impl::build_window() {
      d_window.clear();
      if(d_window_type != filter::firdes::WIN_NONE) {
        d_window = filter::firdes::window(d_window_type, d_fft_len, 6.76);
      }
    }

    void
    signal_detector_cc_impl::build_threshold(std::vector<float> pxx) {
      std::sort(pxx.begin(), pxx.end());
      float maximum = *pxx.end();
      unsigned int pos = 0;
      for(std::vector<float>::iterator it = pxx.begin(); it != pxx.end();
              ++it) {
        if(*(it+1) - *it > maximum-d_sensitivity) {
          d_threshold = pxx.at(it - pxx.begin());
          break;
        }
      }
    }

    std::vector<std::vector<float> >
    signal_detector_cc_impl::find_signal_edges(
            std::vector<float> *pxx) {
      std::vector<std::vector<float>::iterator > pos;

      //find values above threshold
      for(std::vector<float>::iterator it = pxx->begin(); it != pxx->end();
          ++it) {
        if(*it > d_threshold) {
          pos.push_back(it);
        }
      }

      //find adjacent bins

    }

    void
    signal_detector_cc_impl::forecast(int noutput_items,
                                      gr_vector_int &ninput_items_required) {
      //TODO: is this correct?
      ninput_items_required[0] = noutput_items;
    }

    int
    signal_detector_cc_impl::general_work(int noutput_items,
                                          gr_vector_int &ninput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      // Do <+signal processing+>
      std::vector<float> pxx;
      std::vector<float> d_fbuffreq = build_freq();
      periodogram(&(*pxx.begin()), in);

      if(d_auto_threshold) {
        build_threshold(pxx);
      }

      // Tell runtime system how many input items we consumed on
      // each input stream.
      //TODO: is this correct?
      consume_each(ninput_items[0]);
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace inspector */
} /* namespace gr */

