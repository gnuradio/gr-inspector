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
#include "signal_detector_cf_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace inspector {

    signal_detector_cf::sptr
    signal_detector_cf::make(double samp_rate, int fft_len,
                             int window_type, float threshold,
                             float sensitivity, bool auto_threshold) {
      return gnuradio::get_initial_sptr
              (new signal_detector_cf_impl(samp_rate, fft_len, window_type,
                                           threshold, sensitivity,
                                           auto_threshold));
    }

    //<editor-fold desc="Initalization">

    /*
     * The private constructor
     */
    signal_detector_cf_impl::signal_detector_cf_impl(double samp_rate,
                                                     int fft_len,
                                                     int window_type,
                                                     float threshold,
                                                     float sensitivity,
                                                     bool auto_threshold)
            : gr::block("signal_detector_cf",
                        gr::io_signature::make(1, 1,
                                               sizeof(gr_complex)),
                        gr::io_signature::make(1, 2, sizeof(float)*fft_len)) {
      set_samp_rate(samp_rate);
      set_fft_len(fft_len);
      set_window_type(window_type);
      set_threshold(threshold);
      set_sensitivity(sensitivity);
      set_auto_threshold(auto_threshold);
      message_port_register_out(pmt::intern("map_out"));

      //fill properties
      build_window();
      set_fft(new fft::fft_complex(fft_len, true));
    }

    /*
     * Our virtual destructor.
     */
    signal_detector_cf_impl::~signal_detector_cf_impl() {
      delete d_fft;
      //TODO: free allocations here
    }

    //</editor-fold>

    //<editor-fold desc="Helpers">

    void
    signal_detector_cf_impl::periodogram(float *pxx, const gr_complex *signal) {
      // window signal
      if(d_window.size()) {
        //std::cout << "Applying window..." << "\n";
        volk_32fc_32f_multiply_32fc(d_fft->get_inbuf(), signal, &d_window.front(), d_fft_len);
      }
      else {
        //std::cout << "No Window specified..." << "\n";
        memcpy(d_fft->get_inbuf(), signal, sizeof(gr_complex)*d_fft_len);
      }
      //std::cout << "Calculating FFT" << "\n";
      d_fft->execute();
      //std::cout << "Calculating mag square..." << "\n";
      volk_32fc_magnitude_squared_32f(pxx, d_fft->get_outbuf(), d_fft_len);
      //std::cout << "Normizing PSD..." << "\n";
      volk_32f_s32f_normalize(pxx, d_fft_len, d_fft_len);
      // calculate in dB
      //std::cout << "Applying log..." << "\n";
      volk_32f_log2_32f(pxx, pxx, d_fft_len);
      //std::cout << "Normizing log PSD..." << "\n";
      volk_32f_s32f_normalize(pxx, 10/log2(10),d_fft_len);

      //std::cout << "Doing FFTShift..." << "\n";
      //fftshift

      //TODO: rename variables/make global
      float* d_tmpbuf = static_cast<float*>(volk_malloc(sizeof(float)*d_fft_len, volk_get_alignment()));

      unsigned int d_tmpbuflen = (unsigned int)(floor(d_fft_len/2.0));
      memcpy(d_tmpbuf, &pxx[0], sizeof(float)*(d_tmpbuflen + 1));
      memcpy(&pxx[0], &pxx[d_fft_len - d_tmpbuflen], sizeof(float)*(d_tmpbuflen));
      memcpy(&pxx[d_tmpbuflen], d_tmpbuf, sizeof(float)*(d_tmpbuflen + 1));
    }

    // builds the frequency vector for periodogram
    std::vector<float>
    signal_detector_cf_impl::build_freq() {
      std::vector<float> freq(d_fft_len);
      double point = -d_samp_rate/2;
      for(unsigned int i = 0; i < d_fft_len; i++) {
        freq[i] = point;
        point += d_samp_rate/d_fft_len;
      }
      return freq;
    }

    // use firdes to get window coefficients
    void
    signal_detector_cf_impl::build_window() {
      d_window.clear();
      if(d_window_type != filter::firdes::WIN_NONE) {
        d_window = filter::firdes::window(d_window_type, d_fft_len, 6.76);
      }
    }

    // set auto threshold by searching for jumps between bins
    void
    signal_detector_cf_impl::build_threshold(float* pxx) {
      float* temp = (float*)volk_malloc(sizeof(float)*d_fft_len, volk_get_alignment());
      memcpy(temp, pxx, sizeof(float)*d_fft_len);
      //std::cout << "Pxx length = " << d_fft_len << "\n";
      //std::cout << "Ordering bins..." << "\n";
      std::sort(temp, temp + d_fft_len);
      //std::cout << "Finding maximum..." << "\n";
      float maximum = temp[d_fft_len-1];
      //std::cout << "Maximum = " << maximum << "\n";
      //std::cout << "Preparing loop..." << "\n";
      for(unsigned int i = 0; i < d_fft_len; i++) {
        //std::cout << "Comparing bins..." << i << "\n";
        //std::cout << "Jump " << temp[i+1] - temp[i] << "\n";
        if((temp[i + 1] - temp[i])/maximum > 1-d_sensitivity) {
          //std::cout << "Found magnitude jump... " << temp[i] << " " << temp[i+1] << "\n";
          d_threshold = temp[i];
          std::cout << "Setting threshold to " << d_threshold << "\n";

          break;
        }
      }
      volk_free(temp);
    }

    // find bins above threshold and adjacent bins for each signal
    std::vector<std::vector<unsigned int> >
    signal_detector_cf_impl::find_signal_edges(float* pxx) {

      std::vector<unsigned int> pos;
      //std::cout << "Finding bins above threshold..." << "\n";
      //find values above threshold
      for(unsigned int i = 0; i < d_fft_len; i++) {
        if(pxx[i] > d_threshold) {
          pos.push_back(i);
        }
      }
      //std::cout << "Bins above threshold = " << pos.size() << "\n";
      //for(std::vector<unsigned int>::iterator it = pos.begin(); it != pos.end(); ++it) {
        //std::cout << *it << ", ";
      //}
      //std::cout << "\nFinding adjacent bins..." << "\n";
      std::vector<unsigned int> curr_edges;
      std::vector<std::vector<unsigned int> > flanks;
      curr_edges.push_back(pos[0]); //first bin is signal begin
      // check some special cases
      if(pos.size() == 0) {
        return flanks;
      }
      else if(pos.size() == 1) {
        curr_edges.push_back(pos[0]); // use same value as both flanks
        flanks.push_back(curr_edges);
      }
      else if(pos.size() == 2) {
        if(pos[0] + 1 == pos[1]) { // one signal with two bins
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
        //find adjacent bins dynimacially
        bool new_signal = false;
        for(unsigned int i = 1; i < pos.size(); i++) {
          if(new_signal) {
            //std::cout << "Saving bin as new signal..." << pos[i] << "\n";
            curr_edges.push_back(pos[i]);
            new_signal = false;
            if(i == pos.size()-1) {
              //std::cout << "Saving last bin as signal end..." << pos[i] << "\n";
              curr_edges.push_back(pos[i]);
              flanks.push_back(curr_edges);
            }
          }
          else {
            if(i == pos.size()-1) {
              //std::cout << "Saving last bin as signal end..." << pos[i] << "\n";
              curr_edges.push_back(pos[i]);
              flanks.push_back(curr_edges);
            }
            else {
              //std::cout << "Checking next bin..." << i << "\n";
              if(pos[i+1] != pos[i]+1) {
                //std::cout << "Found signal gap. Saving bin..." << pos[i] << "\n";
                curr_edges.push_back(pos[i]);
                flanks.push_back(curr_edges);
                curr_edges.clear();
                new_signal = true;
              }
            }
          }
        }
      }
      std::cout << "Signals detected = " << flanks.size() << "\n";
      return flanks;
    }

    //</editor-fold>

    //<editor-fold desc="GR Stuff">

    void
    signal_detector_cf_impl::forecast(int noutput_items,
                                      gr_vector_int &ninput_items_required) {
      //TODO: is this correct?
      ninput_items_required[0] = d_fft_len*noutput_items;
      //std::cout << "Is it gonna rain?... Output items = " << noutput_items << "\n";
      GR_LOG_DEBUG(d_debug_logger, "Checking forecast...");
    }

    int
    signal_detector_cf_impl::general_work(int noutput_items,
                                          gr_vector_int &ninput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      std::cout << "Work work work work work!" << "\n";
      //std::cout << "Input length = " << ninput_items[0] << "\n";
      if(ninput_items[0] < d_fft_len) {
        return 0;
      }

      // Do <+signal processing+>
      float* pxx = static_cast<float*>(volk_malloc(sizeof(float)*d_fft_len, volk_get_alignment()));

      std::vector<float> freq = build_freq();
      //std::cout << "Building periodogram..." << "\n";
      periodogram(pxx, in);

      if(d_auto_threshold) {
        std::cout << "Setting auto threshold..." << "\n";
        build_threshold(pxx);
      }

      //std::cout << "Finding signal edges" << "\n";
      std::vector<std::vector<unsigned int> > flanks = find_signal_edges(pxx);
      std::vector<std::vector<float> > rf_map;
      std::cout << "Calculating frequencies..." << "\n";
      for(unsigned int i = 0; i < flanks.size(); i++) {
        std::vector<float> temp;
        //std::cout << "Processing signal " << i << "\n";
        //std::cout << "Start freq at pos " << flanks[i][0] << "\n";
        temp.push_back(freq[flanks[i][0]]);
        //std::cout << "Stop freq at pos " << flanks[i][1] << "\n";
        temp.push_back(freq[flanks[i][1]]);
        rf_map.push_back(temp);
        std::cout << rf_map[i][0] << " - " << rf_map[i][1] << "\n";
      }

      std::cout << "Output periodogram... Requested " << noutput_items << "\n";
      std::cout << "Input items consumed = " << ninput_items[0] << "\n";
      memcpy(out, pxx, d_fft_len*sizeof(float));

      float marker[d_fft_len];
      for (int a = 0; a < d_fft_len; a++) marker[a] = 0;
      for(int i = 0; i < flanks.size(); i++) {
        for(int j = flanks[i][0]; j <= flanks[i][1]; j++) {
          marker[j] = 10;
        }
      }

      memcpy(output_items[1], marker, d_fft_len*sizeof(float));
      // Tell runtime system how many input items we consumed on
      // each input stream.
      //TODO: is this correct?
      consume_each(d_fft_len);
      // Tell runtime system how many output items we produced.
      return 1;
    }

    //</editor-fold>

  } /* namespace inspector */
} /* namespace gr */

