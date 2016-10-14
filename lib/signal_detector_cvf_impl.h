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

#ifndef INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CVF_IMPL_H
#define INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CVF_IMPL_H

#include <inspector/signal_detector_cvf.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/fft.h>
#include <gnuradio/filter/single_pole_iir.h>
#include <fstream>

namespace gr {
  namespace inspector {

    class signal_detector_cvf_impl : public signal_detector_cvf {
    private:
      bool d_auto_threshold;
      int d_fft_len;
      unsigned int d_tmpbuflen;
      float d_threshold, d_sensitivity, d_average, d_quantization, d_min_bw;
      float *d_pxx, *d_tmp_pxx, *d_pxx_out, *d_tmpbuf;
      double d_samp_rate;
      std::ofstream logfile;

      std::vector<filter::single_pole_iir<float,float,double> > d_avg_filter;
      filter::firdes::win_type d_window_type;
      std::vector<float> d_window;
      std::vector<std::vector<float> > d_signal_edges;
      fft::fft_complex *d_fft;
      std::vector<float> d_freq;
      const char* d_filename;

      void write_logfile_header();
      void write_logfile_entry();

    public:
      signal_detector_cvf_impl(double samp_rate, int fft_len,
                              int window_type, float threshold,
                              float sensitivity, bool auto_threshold,
                              float average, float quantization,
                              float min_bw, const char *filename);

      ~signal_detector_cvf_impl();

      // set window coefficients
      void build_window();
      // create frequency vector
      std::vector<float> build_freq();
      // auto threshold calculation
      void build_threshold();
      // signal grouping logic
      std::vector<std::vector<unsigned int> > find_signal_edges();

      pmt::pmt_t pack_message();
      // check if signals have changed
      bool compare_signal_edges(std::vector<std::vector<float> >* edges);
      // PSD estimation
      void periodogram(float *pxx, const gr_complex *signal);

      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);

      //<editor-fold desc="Setter">

      void set_samp_rate(double d_samp_rate) {
        signal_detector_cvf_impl::d_samp_rate = d_samp_rate;
      }

      void set_fft_len(int fft_len);
      void set_window_type(int d_window);

      void set_threshold(float d_threshold) {
        signal_detector_cvf_impl::d_threshold = d_threshold;
      }

      void set_sensitivity(float d_sensitivity) {
        signal_detector_cvf_impl::d_sensitivity = d_sensitivity;
      }

      void set_auto_threshold(bool d_auto_threshold) {
        signal_detector_cvf_impl::d_auto_threshold = d_auto_threshold;
      }

      void set_average(float d_average) {
        signal_detector_cvf_impl::d_average = d_average;
        for(unsigned int i = 0; i < d_fft_len; i++) {
          d_avg_filter[i].set_taps(d_average);
        }
      }

      void set_quantization(float d_quantization){
        signal_detector_cvf_impl::d_quantization = d_quantization;
      }

      //</editor-fold>

    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CVF_IMPL_H */
