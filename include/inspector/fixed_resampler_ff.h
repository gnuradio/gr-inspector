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


#ifndef INCLUDED_INSPECTOR_FIXED_RESAMPLER_FF_H
#define INCLUDED_INSPECTOR_FIXED_RESAMPLER_FF_H

#include <inspector/api.h>
#include <gnuradio/filter/fractional_resampler_cc.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief <+description of block+>
     * \ingroup inspector
     *
     */
    class INSPECTOR_API fixed_resampler_ff : virtual public filter::fractional_resampler_cc
    {
     public:
      typedef boost::shared_ptr<fixed_resampler_ff> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::fixed_resampler_ff.
       *
       * To avoid accidental use of raw pointers, inspector::fixed_resampler_ff's
       * constructor is in a private implementation
       * class. inspector::fixed_resampler_ff::make is the public interface for
       * creating new instances.
       */
      static sptr make(int signal, float out_samp_rate);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_FIXED_RESAMPLER_FF_H */

