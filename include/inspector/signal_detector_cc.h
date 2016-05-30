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


#ifndef INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CC_H
#define INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CC_H

#include <inspector/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief <+description of block+>
     * \ingroup inspector
     *
     */
    class INSPECTOR_API signal_detector_cc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<signal_detector_cc> sptr;



      /*!
       * \brief Return a shared_ptr to a new instance of inspector::signal_detector_cc.
       *
       * To avoid accidental use of raw pointers, inspector::signal_detector_cc's
       * constructor is in a private implementation
       * class. inspector::signal_detector_cc::make is the public interface for
       * creating new instances.
       */
      static sptr make(double samp_rate, int fft_len = 1024, int window_type = 0, float threshold = 0.7, float sensitivity = 0.2, bool auto_threshold = true);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_DETECTOR_CC_H */

