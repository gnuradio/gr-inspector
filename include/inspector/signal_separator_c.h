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


#ifndef INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_H
#define INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_H

#include <inspector/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief <+description of block+>
     * \ingroup inspector
     *
     */
    class INSPECTOR_API signal_separator_c : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<signal_separator_c> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::signal_separator_c.
       *
       * To avoid accidental use of raw pointers, inspector::signal_separator_c's
       * constructor is in a private implementation
       * class. inspector::signal_separator_c::make is the public interface for
       * creating new instances.
       */
      static sptr make(double samp_rate, int window, float trans_width, int oversampling);

      virtual void set_samp_rate(double d_samp_rate) = 0;
      virtual void set_window(int d_window) = 0;
      virtual void set_trans_width(float d_trans_width) = 0;
      virtual void set_oversampling(int d_oversampling) = 0;
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_H */

