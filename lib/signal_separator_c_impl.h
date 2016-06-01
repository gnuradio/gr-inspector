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
#include <gnuradio/filter/fir_filter.h>
#include <gnuradio/blocks/rotator.h>

namespace gr {
  namespace inspector {

    class signal_separator_c_impl : public signal_separator_c {
    private:
      double d_samp_rate;
      filter::firdes::win_type d_window;
      std::vector<filter::kernel::fir_filter_ccf*> d_filterbank;
      blocks::rotator d_rotator;
      std::vector<std::vector<float> > d_rf_map;
      float d_trans_width;

      std::vector<float> d_taps;
      std::vector<float> build_taps(double cutoff);
      std::vector<int> d_decimations;
      gr_complex* d_temp_buffer;
      std::vector<std::vector<gr_complex> > d_result_vector;


    public:
      signal_separator_c_impl(double samp_rate, int window, float trans_width);

      ~signal_separator_c_impl();


      //<editor-fold desc="Getter and Setter">

      double samp_rate() const {
        return d_samp_rate;
      }

      void set_samp_rate(double d_samp_rate) {
        signal_separator_c_impl::d_samp_rate = d_samp_rate;
      }

      filter::firdes::win_type window() const {
        return d_window;
      }

      void set_window(int d_window) {
        signal_separator_c_impl::d_window =
                static_cast<filter::firdes::win_type >(d_window);
      }

      //</editor-fold>


      filter::kernel::fir_filter_ccf* build_filter(unsigned int signal);

      void add_filter(filter::kernel::fir_filter_ccf* filter);

      void remove_filter(unsigned int signal);

      void handle_msg(pmt::pmt_t msg);

      pmt::pmt_t pack_message();

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

