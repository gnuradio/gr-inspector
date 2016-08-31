/* -*- c++ -*- */
/*
 * Copyright 2016 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
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


#ifndef INCLUDED_INSPECTOR_SIGNAL_EXTRACTOR_C_H
#define INCLUDED_INSPECTOR_SIGNAL_EXTRACTOR_C_H

#include <inspector/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief Extractor block to extract signal samples out of bundled message passed from Signal Separator block.
     * \ingroup inspector
     *
     * \details
     * Signal Extractor block is able to extract signal samples for specified
     * signal out of message passed by Signal Separator block. These messages
     * contain the samples of all detected signals. This block only passes the
     * ones beloging to the specified signal number as a complex stream. This
     * way, custom signal chains can be appended after the inspector blocks.
     *
     * If the resample option is used, the block will try to resample the
     * input samples based on the message input from the map port to the
     * desired output sample rate. An oversampling factor can be taken
     * into account, if the input samples are oversampled by the
     * signal separator.
     */
    class INSPECTOR_API signal_extractor_c : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<signal_extractor_c> sptr;

      /*!
       * \brief Return a new Extractor block instance
       *
       * \param signal Output Signal number (beginning with 0) for desired sample
       * \param resample Enable resampling of samples
       * \param rate Desired output sample rate if resample is enabled
       * \param osf Oversampling factor of the input signal (relative to map input)
       */
      static sptr make(int signal, bool resample = false,
                       float rate = 1.0, float osf = 1.0);
      virtual void set_signal(int signal) = 0;
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_EXTRACTOR_C_H */
