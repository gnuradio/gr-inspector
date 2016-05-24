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
        }

        /*
         * Our virtual destructor.
         */
        signal_separator_c_impl::~signal_separator_c_impl() {
        }

        void
        signal_separator_c_impl::set_samp_rate(double samp_rate) {
            d_samp_rate = samp_rate;
        }

        void
        signal_separator_c_impl::set_window(int window) {
            d_window = static_cast<filter::firdes::win_type>(window);
        }

        void
        signal_separator_c_impl::build_taps() {
            d_taps.clear();
            d_taps = filter::firdes::low_pass(1, d_samp_rate,
                1000, 100, d_window, 6.76);
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
            //<+OTYPE + > *out = (<+OTYPE + > *) output_items[0];

            // Do <+signal processing+>
            // Tell runtime system how many input items we consumed on
            // each input stream.
            consume_each(noutput_items);

            // Tell runtime system how many output items we produced.
            return 0;
        }

    } /* namespace inspector */
} /* namespace gr */

