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
    signal_separator_c::make(double samp_rate, int window, float trans_width) {
      return gnuradio::get_initial_sptr
              (new signal_separator_c_impl(samp_rate, window, trans_width));
    }

    //<editor-fold desc="Initalization">

    /*
     * The private constructor
     */
    signal_separator_c_impl::signal_separator_c_impl(
            double samp_rate, int window, float trans_width)
            : gr::block("signal_separator_c",
                        gr::io_signature::make(1, 1,
                                               sizeof(gr_complex)),
                        gr::io_signature::make(0, 0, 0)) {
      // fill properties
      set_window(window);
      set_samp_rate(samp_rate);
      d_trans_width = trans_width;

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
      // delete all filters
      for(std::vector<filter::kernel::fir_filter_ccf*>::iterator it = d_filterbank.begin();
              it != d_filterbank.end(); ++it) {
        delete(*it);
      }
    }

    //</editor-fold>

    //<editor-fold desc="Helpers">

    std::vector<float>
    signal_separator_c_impl::build_taps(double cutoff) {
      return filter::firdes::low_pass(1, d_samp_rate, cutoff,
                                      d_trans_width*cutoff, d_window, 6.76);
    }

    filter::kernel::fir_filter_ccf*
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
      // TODO: remove this and do proper oversampling
      bandwidth *= 100;
      int decim = static_cast<int>(d_samp_rate / bandwidth);

      d_taps= build_taps(bandwidth / 2);

      // build filter here
      filter::kernel::fir_filter_ccf*
              filter = new filter::kernel::fir_filter_ccf(decim, d_taps);
      d_decimations[signal] = decim;
      std::cout << "Decim " << signal << " = " << decim << std::endl;
      return filter;
    }

    void
    signal_separator_c_impl::add_filter(
            filter::kernel::fir_filter_ccf* filter) {
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
      d_decimations.clear();
      std::vector<std::vector<float> > result;
      std::vector<float> temp;
      for (unsigned int i = 0; i < pmt::length(msg); i++) {
        pmt::pmt_t row = pmt::vector_ref(msg, i);
        temp.clear();
        temp.push_back(pmt::f32vector_ref(row, 0));
        temp.push_back(pmt::f32vector_ref(row, 1));
        result.push_back(temp);
      }
      d_rf_map = result;

      std::cout << "RF Map: " << d_rf_map.size() << "\n";

      // calculate filters
      // TODO: make this more efficient
      std::vector<filter::kernel::fir_filter_ccf*> temp_filter;
      d_decimations.resize(d_rf_map.size());
      for (unsigned int i = 0; i < d_rf_map.size(); i++) {
        temp_filter.push_back(build_filter(i));
      }

      d_filterbank = temp_filter;
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
      }
      return msg;
    }

    void
    signal_separator_c_impl::forecast(int noutput_items,
                                      gr_vector_int &ninput_items_required) {

      ninput_items_required[0] = noutput_items;
    }

    int
    signal_separator_c_impl::general_work(int noutput_items,
                                          gr_vector_int &ninput_items,
                                          gr_vector_const_void_star &input_items,
                                          gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];

      d_result_vector.clear();
      std::cout << "Filters active: " << d_filterbank.size() <<
      "\n";
      // apply all filters on input signal
      for (unsigned int i = 0; i < d_filterbank.size(); i++) {
        d_temp_buffer = (gr_complex*)volk_malloc(
                ninput_items[0]*sizeof(gr_complex),
                volk_get_alignment());
        std::cout << "ping " << ninput_items[0]*sizeof(gr_complex)/d_decimations[i] << std::endl;

        d_filterbank.at(i)->filterNdec(d_temp_buffer, in, ninput_items[0]/d_decimations[i], d_decimations[i]);
        std::cout << "ping" << std::endl;
        std::vector<gr_complex> temp_results(d_temp_buffer, d_temp_buffer+ninput_items[0]/d_decimations[i]);
        d_result_vector.push_back(temp_results);
        std::cout << "Pushed results " << d_result_vector.size() << std::endl;
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

