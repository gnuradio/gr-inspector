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

#ifndef INCLUDED_INSPECTOR_FIXED_RESAMPLER_FF_IMPL_H
#define INCLUDED_INSPECTOR_FIXED_RESAMPLER_FF_IMPL_H

#include <inspector/fixed_resampler_ff.h>

namespace gr {
  namespace inspector {

    class fixed_resampler_ff_impl : public fixed_resampler_ff
    {
     private:
      int d_samp_rate, d_signal;
      float d_bw;
      boost::shared_ptr<filter::fractional_resampler_cc> d_mother;

     public:
      fixed_resampler_ff_impl(int signal, float out_samp_rate);
      ~fixed_resampler_ff_impl();

      void handle_msg(pmt::pmt_t msg);

    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_FIXED_RESAMPLER_FF_IMPL_H */

