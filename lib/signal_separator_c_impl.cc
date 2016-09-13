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
#include "signal_separator_c_impl.h"

namespace gr {
  namespace inspector {

    signal_separator_c::sptr
    signal_separator_c::make(double samp_rate, int window, float trans_width,
                             float oversampling, bool taps_file,
                             std::map<float, std::vector<float> > &file_path) {
      return gnuradio::get_initial_sptr
              (new signal_separator_c_impl(samp_rate, window, trans_width,
                                           oversampling, taps_file, file_path));
    }

    //<editor-fold desc="Initalization">

    /*
     * The private constructor
     */
    signal_separator_c_impl::signal_separator_c_impl(
            double samp_rate, int window, float trans_width, float oversampling,
            bool taps_file, std::map<float, std::vector<float> > &file_path)
            : gr::block("signal_separator_c",
                        gr::io_signature::make(1, 1,
                                               sizeof(gr_complex)),
                        gr::io_signature::make(0, 0, 0)) {
      // fill properties
      d_window = (filter::firdes::win_type )window;
      d_samp_rate = samp_rate;
      d_trans_width = trans_width;
      d_oversampling = oversampling;
      d_buffer_stage = 0;
      d_use_file = taps_file;
      d_precalc = file_path;
      d_buffer_len = 6400; // good empirical value

      // message port
      message_port_register_out(pmt::intern("sig_out"));
      message_port_register_in(pmt::intern("map_in"));
      set_msg_handler(pmt::intern("map_in"), boost::bind(
              &signal_separator_c_impl::handle_msg, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    signal_separator_c_impl::~signal_separator_c_impl() {
      free_allocation();
    }

    //</editor-fold>

    //<editor-fold desc="Helpers">

    void
    signal_separator_c_impl::free_allocation() {
      // delete all filters
      for(std::vector<filter::kernel::fir_filter_ccf*>::iterator it =
              d_filterbank.begin(); it != d_filterbank.end(); ++it) {
        delete *it;
      }
      // delete histroy buffers
      while(!d_history_buffer.empty()) {
        volk_free(d_history_buffer.back());
        d_history_buffer.pop_back();
      }

    }

    // use firdes to generate lowpass taps or find suitable taps
    // in specified JSON file
    std::vector<float>
    signal_separator_c_impl::build_taps(double cutoff) {
      std::vector<float> taps;
      if(!d_use_file) {
        if (0 < cutoff && cutoff <= d_samp_rate / 2) {
          taps = filter::firdes::low_pass(1, d_samp_rate, cutoff,
                                          d_trans_width * cutoff,
                                          d_window, 6.76);
        }
        else {
          taps = std::vector<float>(2, 0.0);
          GR_LOG_WARN(d_logger,
                      "Firdes check failed: 0 < cutoff <= samp_rate/2");
        }
      }
      else {
        cutoff = cutoff/d_samp_rate; // normize cutoff
        if(d_precalc.upper_bound(cutoff) == d_precalc.end()) {
          taps = d_precalc.rbegin()->second;
        }
        else {
          taps = d_precalc.upper_bound(cutoff)->second;
        }
      }

      return taps;
    }

    // build filter and pass pointer and other calculations in vectors
    void
    signal_separator_c_impl::build_filter(unsigned int signal) {
      // calculate signal parameters
      double freq_center = d_rf_map.at(signal).at(0);
      double bandwidth = d_rf_map.at(signal).at(1);

      // ERROR HANDLING: if only one bin detected, we still need a bandwidth > 0
      if (bandwidth == 0) {
        bandwidth = 1;
      }
      // do oversampling
      bandwidth *= d_oversampling;
      // ERROR HANDLING: maximum signal bw must be samp rate
      if (bandwidth > d_samp_rate) {
        bandwidth = d_samp_rate;
      }
      int decim = static_cast<int>(d_samp_rate / bandwidth);
      // save decimation for later
      d_decimations[signal] = decim;

      // let stopband begin at nyquist border
      d_taps = build_taps((1-d_trans_width)*bandwidth/2);
      d_ntaps[signal] = d_taps.size();

      // copied from xlating fir filter
      float fwT0 = 2 * M_PI * freq_center / d_samp_rate;
      // create rotator for current signal
      blocks::rotator rotator;
      rotator.set_phase_incr(exp(gr_complex(0, -fwT0)));
      d_rotators[signal] = rotator;

      // build filter here
      filter::kernel::fir_filter_ccf*
              filter = new filter::kernel::fir_filter_ccf(decim, d_taps);

      d_filterbank[signal] = filter;
    }

    //</editor-fold>

    //<editor-fold desc="GR Stuff">

    void
    signal_separator_c_impl::handle_msg(pmt::pmt_t msg) {
      gr::thread::scoped_lock guard(d_mutex);
      //free allocated space
      free_allocation();
      unpack_message(msg);
      // calculate filters
      rebuild_all_filters();
    }

    void
    signal_separator_c_impl::rebuild_all_filters() {
      d_filterbank.clear();
      d_decimations.clear();
      d_ntaps.clear();
      d_rotators.clear();
      d_decimations.resize(d_rf_map.size());
      d_ntaps.resize(d_rf_map.size());
      d_rotators.resize(d_rf_map.size());
      d_filterbank.resize(d_rf_map.size());
      d_history_buffer.resize(d_rf_map.size());
      for (unsigned int i = 0; i < d_rf_map.size(); i++) {
        build_filter(i);
      }
      // tell work method to reallocate history buffers with eventually
      // other tap count
      d_buffer_stage = 1;
    }

    void
    signal_separator_c_impl::unpack_message(pmt::pmt_t msg) {
      d_rf_map.clear();
      std::vector<float> temp;
      for (unsigned int i = 0; i < pmt::length(msg); i++) {
        pmt::pmt_t row = pmt::vector_ref(msg, i);
        temp.clear();
        temp.push_back(pmt::f32vector_ref(row, 0));
        temp.push_back(pmt::f32vector_ref(row, 1));
        d_rf_map.push_back(temp);
      }
    }

    // pack vector in array to send with message
    pmt::pmt_t
    signal_separator_c_impl::pack_message() {
      std::complex<float> zero;
      zero.real(0);
      zero.imag(0);
      unsigned signal_count = d_result_vector.size();
      pmt::pmt_t msg = pmt::make_vector(signal_count, pmt::PMT_NIL);
      for (unsigned i = 0; i < signal_count; i++) {
        pmt::pmt_t curr_signal = pmt::make_c32vector(d_result_vector[i].size(), zero);
        for(unsigned j = 0; j < d_result_vector[i].size(); j++) {
          pmt::c32vector_set(curr_signal, j, d_result_vector[i][j]);
        }
        pmt::pmt_t tupel = pmt::make_tuple(pmt::from_uint64(i), pmt::from_float(d_rf_map[i][0]),
                                           pmt::from_float(d_rf_map[i][1]), curr_signal);

        pmt::vector_set(msg, i, tupel);
      }
      return msg;
    }

    void
    signal_separator_c_impl::apply_filter(int i) {
      // size of filter output
      int size = (int)((float)(d_buffer_len)/(float)d_decimations[i]);
      // allocate enough space for result
      d_temp_buffer = (gr_complex*)volk_malloc(size*sizeof(gr_complex),
              volk_get_alignment());

      // copied from xlating fir
      unsigned j = 0;
      for (int k = 0; k < size; k++) {
        d_temp_buffer[k] = d_filterbank[i]->filter(&d_history_buffer[i][j]);
        j += d_decimations[i];
      }

      // convert buffer to vector
      std::vector<gr_complex> temp_results(d_temp_buffer, d_temp_buffer+size);
      // save results for current filter
      d_result_vector.push_back(temp_results);
      volk_free(d_temp_buffer);
    }

    void
    signal_separator_c_impl::forecast(int noutput_items,
                                      gr_vector_int &ninput_items_required) {
      // lets pretend we're a sync block (in fact we're not - hehe)
      ninput_items_required[0] = noutput_items;
    }

    int
    signal_separator_c_impl::general_work(int noutput_items,
                                          gr_vector_int &ninput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
      gr::thread::scoped_lock guard(d_mutex);
      const gr_complex *in = (const gr_complex *) input_items[0];

      // no message received -> nothing to do
      if(d_buffer_stage == 0) {
        // throw away all items, we can't use them now
        consume_each(ninput_items[0]);
        return 0;
      }
      // message received, so let's allocate all the needed memory
      else if(d_buffer_stage == 1) {
        // allocate history buffer for each signal with length d_ntaps + d_buffer_len
        for(int i = 0; i < d_history_buffer.size(); i++) {
          d_history_buffer[i] = (gr_complex*)volk_malloc((d_ntaps[i]+d_buffer_len-1)*sizeof(gr_complex),
                                                         volk_get_alignment());
          // write zeros in buffers
          for(int j = 0; j < d_ntaps[i]+d_buffer_len-1; j++) {
            d_history_buffer[i][j] = 0.0;
          }
        }
        d_buffer_stage = 2; // everything set up, no need to repeat any stuff in this else if
      }

      // if too few items, wait for more
      if(ninput_items[0] < d_buffer_len) {
        return 0;
      }

      // free previous result vector
      d_result_vector.clear();

      int copy_len = d_buffer_len;
      if(d_buffer_len > ninput_items[0])
        copy_len = ninput_items[0];

      // rotate and buffer input samples
      for(int i = 0; i < d_history_buffer.size(); i++){
        for(int j = 0; j < copy_len; j++) {
          d_history_buffer[i][j+d_ntaps[i]-2] = d_rotators[i].rotate(in[j]);
        }
      }

      // apply all filters on input signal
      // iterate over each filter
      for (unsigned int i = 0; i < d_history_buffer.size(); i++) {
        apply_filter(i);
      }


      // put current items in history
      for(int i = 0; i < d_history_buffer.size(); i++) {
        memcpy(d_history_buffer[i], &d_history_buffer[i][d_buffer_len],
               (d_ntaps[i]-1)*sizeof(gr_complex));
      }

      // pack message
      pmt::pmt_t msg = pack_message();

      message_port_pub(pmt::intern("sig_out"), msg);
      // Tell runtime system how many output items we produced.
      consume_each(d_buffer_len);
      return noutput_items;
    }

    //</editor-fold>

  } /* namespace inspector */
} /* namespace gr */
