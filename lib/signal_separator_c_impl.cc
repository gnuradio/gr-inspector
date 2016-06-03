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
#include "signal_separator_c_impl.h"

namespace gr {
  namespace inspector {

    signal_separator_c::sptr
    signal_separator_c::make(double samp_rate, int window, float trans_width, int oversampling) {
      return gnuradio::get_initial_sptr
              (new signal_separator_c_impl(samp_rate, window, trans_width, oversampling));
    }

    //<editor-fold desc="Initalization">

    /*
     * The private constructor
     */
    signal_separator_c_impl::signal_separator_c_impl(
            double samp_rate, int window, float trans_width, int oversampling)
            : gr::block("signal_separator_c",
                        gr::io_signature::make(1, 1,
                                               sizeof(gr_complex)),
                        gr::io_signature::make(0, 0, 0)) {
      // fill properties

      //TODO: write setter and getter
      set_window(window);
      set_samp_rate(samp_rate);
      set_trans_width(trans_width);
      set_oversampling(oversampling);

      // message port
      message_port_register_out(pmt::intern("msg_out"));
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
      for(std::vector<filter::kernel::fir_filter_ccc*>::iterator it = d_filterbank.begin();
          it != d_filterbank.end(); ++it) {
        delete(*it);
      }
      for(std::vector<blocks::rotator*>::iterator it = d_rotators.begin();
          it != d_rotators.end(); ++it) {
        delete(*it);
      }
    }

    // use firdes to generate lowpass taps
    std::vector<float>
    signal_separator_c_impl::build_taps(double cutoff) {
      return filter::firdes::low_pass(1, d_samp_rate, cutoff,
                                      d_trans_width*cutoff, d_window, 6.76);
    }

    // build filter and pass pointer and other calculations in vectors
    void
    signal_separator_c_impl::build_filter(unsigned int signal) {
      // calculate signal parameters
      double freq_center = ((d_rf_map.at(signal)).at(1) +
                            (d_rf_map.at(signal)).at(0)) / 2;
      double bandwidth = (d_rf_map.at(signal)).at(1) -
                         (d_rf_map.at(signal)).at(0);
      // if only one bin detected, we still need a bandwidth > 0
      if (bandwidth == 0) {
        bandwidth = 1;
      }

      // do oversampling
      bandwidth *= d_oversampling;
      int decim = static_cast<int>(d_samp_rate / bandwidth);

      // save decimation for later
      d_decimations[signal] = decim;

      d_taps= build_taps(bandwidth / 2);
      // copied from xlating fir filter
      std::vector<gr_complex> ctaps(d_taps.size());
      float fwT0 = 2 * M_PI * freq_center / d_samp_rate;
      for(unsigned int i = 0; i < d_taps.size(); i++) {
        ctaps[i] = d_taps[i] * exp(gr_complex(0, i * fwT0));
      }
      // create rotator for current signal
      blocks::rotator* rotator = new blocks::rotator();
      rotator->set_phase_incr(exp(gr_complex(0, -fwT0 * decim)));
      d_rotators[signal] = rotator;

      // build filter here
      filter::kernel::fir_filter_ccc*
              filter = new filter::kernel::fir_filter_ccc(decim, ctaps);

      d_filterbank[signal] = filter;
    }

    void
    signal_separator_c_impl::add_filter(
            filter::kernel::fir_filter_ccc* filter) {
      d_filterbank.push_back(filter);
    }

    void
    signal_separator_c_impl::remove_filter(
            unsigned int signal) {
      d_filterbank.erase(d_filterbank.begin() - 1 + signal);
    }

    //</editor-fold>

    //<editor-fold desc="GR Stuff">

    void
    signal_separator_c_impl::handle_msg(pmt::pmt_t msg) {
      // extract rf map out of message
      d_filterbank.clear();
      d_decimations.clear();
      d_rotators.clear();

      // free allocated space
      free_allocation();

      unpack_message(msg);

      // calculate filters
      // TODO: make this more efficient
      d_decimations.resize(d_rf_map.size());
      d_rotators.resize(d_rf_map.size());
      d_filterbank.resize(d_rf_map.size());
      for (unsigned int i = 0; i < d_rf_map.size(); i++) {
        build_filter(i);
      }
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
        pmt::vector_set(msg, i, curr_signal);
      }
      return msg;
    }

    void
    signal_separator_c_impl::forecast(int noutput_items,
                                      gr_vector_int &ninput_items_required) {
      //ninput_items_required[0] = noutput_items;
      ninput_items_required[0] = 0;
      for(int i = 0; i < d_decimations.size(); i++) {
        ninput_items_required[0] += noutput_items*d_decimations[i];
      }
    }

    int
    signal_separator_c_impl::general_work(int noutput_items,
                                          gr_vector_int &ninput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];

      d_result_vector.clear();

      // apply all filters on input signal
      // iterate over each filter
      for (unsigned int i = 0; i < d_filterbank.size(); i++) {
        // size of filter output
        int size = ninput_items[0]/d_decimations[i];
        // allocate enough space for result
        d_temp_buffer = (gr_complex*)volk_malloc(size*sizeof(gr_complex),
                volk_get_alignment());
        // copied from xlating fir filter
        unsigned j = 0;
        for (int k = 0; k < size; k++){
          d_temp_buffer[k] = d_rotators[i]->rotate(d_filterbank[i]->filter(&in[j]));
          j += d_decimations[i];
        }

        // convert buffer to vector
        std::vector<gr_complex> temp_results(d_temp_buffer, d_temp_buffer+ninput_items[0]/d_decimations[i]);
        // save results for current filter
        d_result_vector.push_back(temp_results);
        volk_free(d_temp_buffer);
      }
      // pack message
      pmt::pmt_t msg = pack_message();

      message_port_pub(pmt::intern("msg_out"), msg);
      // Tell runtime system how many output items we produced.
      consume_each(ninput_items[0]);
      return noutput_items;
    }

    //</editor-fold>

  } /* namespace inspector */
} /* namespace gr */

