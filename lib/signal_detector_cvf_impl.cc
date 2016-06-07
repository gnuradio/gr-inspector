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
#include "signal_detector_cvf_impl.h"
#include <volk/volk.h>
#include <cmath>

namespace gr {
  namespace inspector {

    signal_detector_cvf::sptr
    signal_detector_cvf::make(double samp_rate, int fft_len,
                             int window_type, float threshold,
                             float sensitivity, bool auto_threshold,
                             float average) {
      return gnuradio::get_initial_sptr
              (new signal_detector_cvf_impl(samp_rate, fft_len,
                                           window_type,
                                           threshold, sensitivity,
                                           auto_threshold, average));
    }

    //<editor-fold desc="Initalization">

    /*
     * The private constructor
     */
    signal_detector_cvf_impl::signal_detector_cvf_impl(double samp_rate,
                                                     int fft_len,
                                                     int window_type,
                                                     float threshold,
                                                     float sensitivity,
                                                     bool auto_threshold,
                                                     float average)
            : sync_decimator("signal_detector_cvf",
                        gr::io_signature::make(1, 1,
                                               sizeof(gr_complex)),
                        gr::io_signature::make(1, 2, sizeof(float) *
                                                     fft_len),
                        fft_len) {

      // set properties
      d_samp_rate = samp_rate;
      d_fft_len = fft_len;
      d_window_type = (filter::firdes::win_type)window_type;
      d_threshold = threshold;
      d_sensitivity = sensitivity;
      d_auto_threshold = auto_threshold;
      d_average = average;

      d_tmpbuf = static_cast<float *>(volk_malloc(
              sizeof(float) * d_fft_len, volk_get_alignment()));
      d_tmp_pxx = static_cast<float *>(volk_malloc(
              sizeof(float) * d_fft_len, volk_get_alignment()));
      d_pxx = static_cast<float *>(volk_malloc(
              sizeof(float) * d_fft_len, volk_get_alignment()));
      d_pxx_out = (float*)volk_malloc(sizeof(float)*d_fft_len,
                                      volk_get_alignment());
      d_fft = new fft::fft_complex(fft_len, true);

      d_avg_filter.resize(d_fft_len);
      build_window();
      for(unsigned int i = 0; i < d_fft_len; i++) {
        d_avg_filter[i].set_taps(d_average);
      }
      message_port_register_out(pmt::intern("map_out"));

    }

    /*
     * Our virtual destructor.
     */
    signal_detector_cvf_impl::~signal_detector_cvf_impl() {
      delete d_fft;
      volk_free(d_tmpbuf);
      volk_free(d_tmp_pxx);
      volk_free(d_pxx);
      volk_free(d_pxx_out);
    }

    void
    signal_detector_cvf_impl::set_fft_len(int fft_len)  {
      signal_detector_cvf_impl::d_fft_len = fft_len;
      delete d_fft;
      volk_free(d_tmpbuf);
      volk_free(d_tmp_pxx);
      volk_free(d_pxx);
      volk_free(d_pxx_out);
      d_fft = new fft::fft_complex(fft_len, true);
      d_tmpbuf = static_cast<float *>(volk_malloc(
              sizeof(float) * d_fft_len, volk_get_alignment()));
      d_tmp_pxx = static_cast<float *>(volk_malloc(
              sizeof(float) * d_fft_len, volk_get_alignment()));
      d_pxx = static_cast<float *>(volk_malloc(
              sizeof(float) * d_fft_len, volk_get_alignment()));
      d_pxx_out = (float*)volk_malloc(sizeof(float)*d_fft_len,
                                      volk_get_alignment());
      d_avg_filter.resize(d_fft_len);
      build_window();
      for(unsigned int i = 0; i < d_fft_len; i++) {
        d_avg_filter[i].set_taps(d_average);
      }
      set_decimation(fft_len);
    }

    void
    signal_detector_cvf_impl::set_window_type(int window)  {
      signal_detector_cvf_impl::d_window_type = static_cast<filter::firdes::win_type>(window);
      build_window();
    }

    //</editor-fold>

    //<editor-fold desc="Helpers">

    // calculate periodogram and save in specified array
    void
    signal_detector_cvf_impl::periodogram(float *pxx,
                                         const gr_complex *signal) {
      if (d_window.size()) {
        // window signal
        volk_32fc_32f_multiply_32fc(d_fft->get_inbuf(), signal,
                                    &d_window.front(), d_fft_len);
      }
      else {
        // don't window signal
        memcpy(d_fft->get_inbuf(), signal,
               sizeof(gr_complex) * d_fft_len);
      }

      d_fft->execute(); // fft

      // calc fft to periodogram
      volk_32fc_s32f_x2_power_spectral_density_32f(pxx, d_fft->get_outbuf(),
      d_fft_len, 1.0, d_fft_len);
      //volk_32fc_magnitude_squared_32f(pxx, d_fft->get_outbuf(),
                                      //d_fft_len);
      //volk_32f_s32f_normalize(pxx, d_fft_len, d_fft_len);

      // calculate in dB
      //volk_32f_log2_32f(pxx, pxx, d_fft_len);
      //volk_32f_s32f_normalize(pxx, log2(10)/10, d_fft_len);

      // do fftshift

      d_tmpbuflen = static_cast<unsigned int>(floor(d_fft_len / 2.0));
      memcpy(d_tmpbuf, &pxx[0], sizeof(float) * (d_tmpbuflen + 1));
      memcpy(&pxx[0], &pxx[d_fft_len - d_tmpbuflen],
             sizeof(float) * (d_tmpbuflen));
      memcpy(&pxx[d_tmpbuflen], d_tmpbuf,
             sizeof(float) * (d_tmpbuflen + 1));
    }

    // builds the frequency vector for periodogram
    std::vector<float>
    signal_detector_cvf_impl::build_freq() {
      std::vector<float> freq(d_fft_len);
      double point = -d_samp_rate / 2;
      for (unsigned int i = 0; i < d_fft_len; i++) {
        freq[i] = point;
        point += d_samp_rate / d_fft_len;
      }
      return freq;
    }

    // use firdes to get window coefficients
    void
    signal_detector_cvf_impl::build_window() {
      d_window.clear();
      if (d_window_type != filter::firdes::WIN_NONE) {
        d_window = filter::firdes::window(d_window_type, d_fft_len,
                                          6.76);
      }
    }

    // set auto threshold by searching for jumps between bins
    void
    signal_detector_cvf_impl::build_threshold() {
      // copy array to work with
      memcpy(d_tmp_pxx, d_pxx_out, sizeof(float) * d_fft_len);
      // sort bins
      d_threshold = 500;
      std::sort(d_tmp_pxx, d_tmp_pxx + d_fft_len);
      float maximum = d_tmp_pxx[d_fft_len - 1];
      // search specified normized jump
      for (unsigned int i = 0; i < d_fft_len; i++) {
        if ((d_tmp_pxx[i + 1] - d_tmp_pxx[i]) / maximum > 1 - d_sensitivity) {
          d_threshold = d_tmp_pxx[i];
          break;
        }
      }
    }

    // find bins above threshold and adjacent bins for each signal
    std::vector<std::vector<unsigned int> >
    signal_detector_cvf_impl::find_signal_edges() {

      std::vector<unsigned int> pos;
      //find values above threshold
      for (unsigned int i = 0; i < d_fft_len; i++) {
        if (d_pxx_out[i] > d_threshold) {
          pos.push_back(i);
        }
      }
      std::vector<std::vector<unsigned int> > flanks;
      if(pos.size() == 0) {
        return flanks;
      }
      // check for adjacent bins to group
      std::vector<unsigned int> curr_edges;

      curr_edges.push_back(pos[0]); //first position is signal begin
      // check some special cases
      if (pos.size() == 0) {
        return flanks; // empty result
      }
      else if (pos.size() == 1) {
        curr_edges.push_back(pos[0]); // use same value for both flanks
        flanks.push_back(curr_edges);
      }
      else if (pos.size() == 2) {
        if (pos[0] + 1 == pos[1]) { // one signal with two bins
          curr_edges.push_back(pos[1]);
          flanks.push_back(curr_edges);
        }
        else { // two signals with one bin each
          curr_edges.push_back(pos[0]);
          flanks.push_back(curr_edges);
          curr_edges.clear();
          curr_edges.push_back(pos[1]);
          curr_edges.push_back(pos[1]);
          flanks.push_back(curr_edges);
        }
      }
      else {
        bool new_signal = false;
        for (unsigned int i = 1; i < pos.size(); i++) {
          if (new_signal) {
            curr_edges.push_back(pos[i]);
            new_signal = false;
            if (i == pos.size() - 1) {
              curr_edges.push_back(pos[i]);
              flanks.push_back(curr_edges);
            }
          }
          else {
            if (i == pos.size() - 1) {
              curr_edges.push_back(pos[i]);
              flanks.push_back(curr_edges);
            }
            else {
              if (pos[i + 1] != pos[i] + 1) {
                curr_edges.push_back(pos[i]);
                flanks.push_back(curr_edges);
                curr_edges.clear();
                new_signal = true;
              }
            }
          }
        }
      }
      return flanks;
    }

    // pack vector in array to send with message
    pmt::pmt_t
    signal_detector_cvf_impl::pack_message() {
      unsigned signal_count = d_signal_edges.size();
      pmt::pmt_t msg = pmt::make_vector(signal_count, pmt::PMT_NIL);
      for (unsigned i = 0; i < signal_count; i++) {
        pmt::pmt_t curr_edge = pmt::make_f32vector(2, 0.0);
        pmt::f32vector_set(curr_edge, 0, d_signal_edges.at(i).at(0));
        pmt::f32vector_set(curr_edge, 1, d_signal_edges.at(i).at(0));
        pmt::vector_set(msg, i, curr_edge);
      }
      return msg;
    }

    // check if RF Map has changed since last detection
    bool
    signal_detector_cvf_impl::compare_signal_edges(
            std::vector<std::vector<float> > *edges) {
      bool change = false;
      if (edges->size() == d_signal_edges.size()) {
        for (unsigned i = 0; i < edges->size(); i++) {
          // check for each signal if border changed within 1%
          if (std::abs(edges->at(i).at(0) - d_signal_edges.at(i).at(0)) >
              0.01 * d_signal_edges.at(i).at(0)) {
            change = true;
          }
          if (std::abs(edges->at(i).at(1) - d_signal_edges.at(i).at(1)) >
              0.01 * d_signal_edges.at(i).at(1)) {
            change = true;
          }
        }
      }
      else {
        change = true;
      }
      return change;
    }

    //</editor-fold>

    //<editor-fold desc="GR Stuff">

    int
    signal_detector_cvf_impl::work(int noutput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      d_freq = build_freq();
      periodogram(d_pxx, in);

      // averaging
      for(int i = 0; i < d_fft_len; i++) {
        d_pxx_out[i] = d_avg_filter[i].filter(d_pxx[i]);
      }

      if (d_auto_threshold) {
        build_threshold();
      }

      std::vector<std::vector<unsigned int> > flanks = find_signal_edges();

      std::vector<std::vector<float> > rf_map;
      for (unsigned int i = 0; i < flanks.size(); i++) {
        std::vector<float> temp;
        temp.push_back(d_freq[flanks[i][0]]);
        temp.push_back(d_freq[flanks[i][1]]);
        rf_map.push_back(temp);
      }

      memcpy(out, d_pxx_out, d_fft_len * sizeof(float));

      //TODO: Remove this (just debug output)
      float marker[d_fft_len];
      for (int a = 0; a < d_fft_len; a++) marker[a] =
               *std::min_element(d_pxx_out, d_pxx_out+d_fft_len);
      for (int i = 0; i < flanks.size(); i++) {
        for (int j = flanks[i][0]; j <= flanks[i][1]; j++) {
          marker[j] = *std::max_element(d_pxx_out, d_pxx_out+d_fft_len);
        }
      }
      memcpy(output_items[1], marker, d_fft_len * sizeof(float));

      // spread the message
      if (compare_signal_edges(&rf_map)) {
        d_signal_edges = rf_map;
        message_port_pub(pmt::intern("map_out"),
                         pack_message());
      }

      return 1;
    }

    //</editor-fold>

  } /* namespace inspector */
} /* namespace gr */
