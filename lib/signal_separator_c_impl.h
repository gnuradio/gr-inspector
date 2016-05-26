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

#ifndef INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_IMPL_H
#define INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_IMPL_H

#include <inspector/signal_separator_c.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccc.h>
#include <pmt/pmt.h>

namespace gr {
    namespace inspector {

        class signal_separator_c_impl : public signal_separator_c {
        private:
            // Nothing to declare in this block.

        public:
            signal_separator_c_impl(double samp_rate, int window);

            ~signal_separator_c_impl();

            // properties
            double d_samp_rate;
            filter::firdes::win_type d_window;
            std::vector<boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> > d_filterbank;
            std::vector<std::vector<float> > d_rf_map;
            // setter
            void set_samp_rate(double samp_rate);
            void set_window(int window);
            void set_filterbank(std::vector<boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> > filterbank);
            void set_rf_map(std::vector<std::vector<float> > map);

            // getter
            double samp_rate();
            int window();
            std::vector<boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> > filterbank();
            std::vector<std::vector<float> > rf_map();

            std::vector<float> build_taps(double cutoff, double trans);
            boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> build_filter(unsigned int signal);
            void add_filter(boost::shared_ptr<filter::freq_xlating_fir_filter_ccc> filter);
            void remove_filter(unsigned int signal);
            void handle_msg(pmt::pmt_t msg);

            // Where all the action really happens
            void forecast(int noutput_items,
                          gr_vector_int &ninput_items_required);

            int general_work(int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items);
        };

    } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_IMPL_H */

