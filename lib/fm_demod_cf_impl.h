/* -*- c++ -*- */
/* 
 * Copyright 2016 Sebastian Müller.
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

#ifndef INCLUDED_INSPECTOR_FM_DEMOD_CF_IMPL_H
#define INCLUDED_INSPECTOR_FM_DEMOD_CF_IMPL_H

#include <inspector/fm_demod_cf.h>
#include <gnuradio/filter/fir_filter.h>
#include <gnuradio/filter/pfb_arb_resampler.h>

namespace gr {
  namespace inspector {

    class fm_demod_cf_impl : public fm_demod_cf
    {
     private:
      int d_signal, d_audio_rate;
      float d_gain, d_ratio;
      filter::kernel::fir_filter_fff *d_audio_filter;
      std::vector<float> d_taps, d_fir_buffer;
      filter::kernel::pfb_arb_resampler_fff *d_resampler;

     public:
      fm_demod_cf_impl(int signal, int audio_rate);
      ~fm_demod_cf_impl();

      void handle_msg(pmt::pmt_t msg);

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_FM_DEMOD_CF_IMPL_H */

