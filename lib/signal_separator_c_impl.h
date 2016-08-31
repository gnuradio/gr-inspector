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

#ifndef INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_IMPL_H
#define INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_IMPL_H

#include <inspector/signal_separator_c.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter.h>
#include <gnuradio/blocks/rotator.h>
#include <gnuradio/thread/thread.h>
#include <gnuradio/messages/msg_queue.h>

namespace gr {
  namespace inspector {

    class signal_separator_c_impl : public signal_separator_c {
    private:
      bool d_use_file;
      int d_buffer_len;
      unsigned int d_buffer_stage;
      float d_trans_width, d_oversampling;
      double d_samp_rate;
      gr_complex* d_temp_buffer;

      filter::firdes::win_type d_window;

      std::vector<filter::kernel::fir_filter_ccf*> d_filterbank;
      std::vector<std::vector<float> > d_rf_map;

      std::vector<float> build_taps(double cutoff);
      std::vector<int> d_decimations;
      std::vector<int> d_ntaps;
      std::vector<blocks::rotator> d_rotators;

      std::vector<gr_complex*> d_history_buffer;
      std::vector<std::vector<gr_complex> > d_result_vector;
      gr::thread::mutex d_mutex;

      std::map<float, std::vector<float> > d_precalc;
      std::vector<float> d_taps;


    public:
      signal_separator_c_impl(double samp_rate, int window, float trans_width,
                              float oversampling, bool taps_file,
                              std::map<float, std::vector<float> > &file_path);

      ~signal_separator_c_impl();

      // free allocated buffer arrays and FIR filters
      void free_allocation();
      // build new filter for signal
      void build_filter(unsigned int signal);
      // build new filters for all signals
      void rebuild_all_filters();
      // apply filter no i to history buffer i
      void apply_filter(int i);
      // trigger rebuild of filters and unpack message
      void handle_msg(pmt::pmt_t msg);
      // create pmt from filtered samples
      pmt::pmt_t pack_message();
      // create RF map from received message
      void unpack_message(pmt::pmt_t msg);

      // Where all the action really happens
      void forecast(int noutput_items,
                    gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);

      //<editor-fold desc="Getter and Setter">

      double samp_rate() const {
        return d_samp_rate;
      }

      void set_samp_rate(double d_samp_rate) {
        signal_separator_c_impl::d_samp_rate = d_samp_rate;
        rebuild_all_filters();
      }

      filter::firdes::win_type window() const {
        return d_window;
      }

      void set_window(int d_window) {
        signal_separator_c_impl::d_window =
                static_cast<filter::firdes::win_type >(d_window);
        rebuild_all_filters();
      }

      float trans_width() const {
        return d_trans_width;
      }

      void set_trans_width(float d_trans_width) {
        signal_separator_c_impl::d_trans_width = d_trans_width;

      }

      int oversampling() const {
        return d_oversampling;
      }

      void set_oversampling(float d_oversampling) {
        signal_separator_c_impl::d_oversampling = d_oversampling;
        rebuild_all_filters();
      }

      //</editor-fold>

    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_IMPL_H */
