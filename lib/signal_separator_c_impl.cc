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
        signal_separator_c::make(double samp_rate, int window) {
            return gnuradio::get_initial_sptr
                    (new signal_separator_c_impl(samp_rate, window));
        }

        //<editor-fold desc="Initalization">

        /*
         * The private constructor
         */
        signal_separator_c_impl::signal_separator_c_impl(
                double samp_rate, int window)
                : gr::block("signal_separator_c",
                            gr::io_signature::make(1, 1,
                                                   sizeof(gr_complex)),
                            gr::io_signature::make(0, 0, 0)) {
            // fill properties
            set_window(window);
            set_samp_rate(samp_rate);

            // message port
            message_port_register_out(pmt::intern("msg_out"));
            message_port_register_in(pmt::intern("map_in"));
            boost::bind(&signal_separator_c_impl::handle_msg, this, _1);
        }

        /*
         * Our virtual destructor.
         */
        signal_separator_c_impl::~signal_separator_c_impl() {
        }

        //</editor-fold>

        //<editor-fold desc="Setter">

        void
        signal_separator_c_impl::set_samp_rate(double samp_rate) {
            d_samp_rate = samp_rate;
        }

        void
        signal_separator_c_impl::set_window(int window) {
            d_window = static_cast<filter::firdes::win_type>(window);
        }

        void
        signal_separator_c_impl::set_filterbank(
                std::vector<boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> > filterbank) {
            d_filterbank = filterbank;
        }

        void
        signal_separator_c_impl::set_rf_map(
                std::vector<std::vector<float> > map) {
            if(map != d_rf_map) {
                d_rf_map = map;
            }
        }

        //</editor-fold>

        //<editor-fold desc="Getter">

        double
        signal_separator_c_impl::samp_rate() {
            return d_samp_rate;
        }

        int
        signal_separator_c_impl::window() {
            return d_window;
        }

        std::vector<boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> >
        signal_separator_c_impl::filterbank() {
            return d_filterbank;
        }

        //</editor-fold>

        std::vector<float>
        signal_separator_c_impl::build_taps(double cutoff, double trans) {
            return filter::firdes::low_pass(1, d_samp_rate,
                cutoff, trans, d_window, 6.76);
        }

        boost::shared_ptr<filter::freq_xlating_fir_filter_ccc>
        signal_separator_c_impl::build_filter(unsigned int signal) {
            // calculate signal parameters
            double freq_center = ((d_rf_map.at(signal)).at(1) + (d_rf_map.at(signal)).at(0))/2;
            double bandwidth = (d_rf_map.at(signal)).at(1) - (d_rf_map.at(signal)).at(0);
            int decim = (int)(d_samp_rate/bandwidth);

            //TODO: is there a better way to cast?
            std::vector<float> taps_float = build_taps(bandwidth/2, 0.05*bandwidth/2);
            std::vector<gr_complex> taps(taps_float.begin(), taps_float.end());

            // build filter here
            boost::shared_ptr<filter::freq_xlating_fir_filter_ccc>
            filter = filter::freq_xlating_fir_filter_ccc::make(
                    decim, taps, freq_center, d_samp_rate
            );

            return filter;
        }

        void
        signal_separator_c_impl::add_filter(
                boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> filter) {
            d_filterbank.push_back(filter);
        }

        void
        signal_separator_c_impl::remove_filter(
                unsigned int signal) {
            d_filterbank.erase(d_filterbank.begin()-1+signal);
        }

        //<editor-fold desc="GR Stuff">

        void
        signal_separator_c_impl::handle_msg(pmt::pmt_t msg) {
            // extract rf map out of message
            std::vector<std::vector<float> > result;
            for(unsigned int i = 0; i < pmt::length(msg); i++) {
                pmt::pmt_t row = pmt::vector_ref(msg, i);
                std::vector<float> temp;
                temp.push_back(pmt::f32vector_ref(row, 0));
                temp.push_back(pmt::f32vector_ref(row, 1));
                result.push_back(temp);
            }
            set_rf_map(result);

            // calculate filters
            // TODO: make this more efficient
            std::vector<boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> > temp;
            for(unsigned int i = 0; i < d_rf_map.size(); i++) {
                temp.push_back(build_filter(i));
            }
        }

        void
        signal_separator_c_impl::forecast(int noutput_items,
                                          gr_vector_int &ninput_items_required) {
            ninput_items_required[0] = noutput_items; // is this correct? :/
        }

        int
        signal_separator_c_impl::general_work(int noutput_items,
                                              gr_vector_int &ninput_items,
                                              gr_vector_const_void_star &input_items,
                                              gr_vector_void_star &output_items) {
            const float *in = (const float *) input_items[0];

            std::vector<gr_vector_void_star> result_vector;

            // apply all filters on input signal
            for(unsigned int i = 0; i < d_filterbank.size(); i++) {
                gr_vector_void_star temp_results;
                d_filterbank.at(i)->general_work(
                        (int)(noutput_items/d_filterbank.size()), ninput_items, input_items, temp_results
                );
                result_vector.push_back(temp_results);
            }
            consume_each(noutput_items);

            // Tell runtime system how many output items we produced.
            return 0;
        }

        //</editor-fold>

    } /* namespace inspector */
} /* namespace gr */

