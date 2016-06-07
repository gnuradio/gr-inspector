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

#ifndef INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CVF_IMPL_H
#define INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CVF_IMPL_H

#include <inspector/signal_detector_cvf.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/fft.h>
#include <boost/circular_buffer.hpp>
#include <gnuradio/filter/single_pole_iir.h>

namespace gr {
  namespace inspector {

    class signal_detector_cvf_impl : public signal_detector_cvf {
    private:


    public:
      signal_detector_cvf_impl(double samp_rate, int fft_len,
                              int window_type, float threshold,
                              float sensitivity, bool auto_threshold,
                              float average);

      ~signal_detector_cvf_impl();

      void build_window();
      std::vector<float> build_freq();
      void build_threshold();
      std::vector<std::vector<unsigned int> > find_signal_edges();

      std::vector<filter::single_pole_iir<float,float,double> > d_avg_filter;
      double d_samp_rate;
      int d_fft_len;
      filter::firdes::win_type d_window_type;
      std::vector<float> d_window;
      float d_threshold;
      float d_sensitivity;
      bool d_auto_threshold;
      std::vector<std::vector<float> > d_signal_edges;
      fft::fft_complex *d_fft;
      float* d_tmpbuf;
      unsigned int d_tmpbuflen;
      float* d_tmp_pxx;
      float* d_pxx;
      float* d_pxx_out;
      float d_average;
      std::vector<float> d_freq;
      pmt::pmt_t pack_message();
      bool compare_signal_edges(std::vector<std::vector<float> >* edges);

      //<editor-fold desc="Getter and Setter">

      double samp_rate() const {
        return d_samp_rate;
      }

      void set_samp_rate(double d_samp_rate) {
        signal_detector_cvf_impl::d_samp_rate = d_samp_rate;
      }

      int fft_len() const {
        return d_fft_len;

      }

      void set_fft_len(int fft_len);

      int window_type() const {
        return static_cast<int>(d_window_type);
      }

      void set_window_type(int d_window);

      float threshold() const {
        return d_threshold;
      }

      void set_threshold(float d_threshold) {
        signal_detector_cvf_impl::d_threshold = d_threshold;
      }

      float sensitivity() const {
        return d_sensitivity;
      }

      void set_sensitivity(float d_sensitivity) {
        signal_detector_cvf_impl::d_sensitivity = d_sensitivity;
      }

      bool auto_threshold() const {
        return d_auto_threshold;
      }

      void set_auto_threshold(bool d_auto_threshold) {
        signal_detector_cvf_impl::d_auto_threshold = d_auto_threshold;
      }

      float average() const {
        return d_average;
      }

      void set_average(float d_average) {
        signal_detector_cvf_impl::d_average = d_average;
      }

      //</editor-fold>

// Where all the action really happens

      void periodogram(float *pxx, const gr_complex *signal);

      int work(int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CVF_IMPL_H */

