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


#ifndef INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_H
#define INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_H

#include <inspector/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief Signal Separator takes input signal and RF map as input and passes samples of each filtered and decimated signal as output
     * \ingroup inspector
     *
     * \details
     * Signal Separator block takes input signal and RF map with signal center
     * frequencies and bandwidths and filters the input spectrum for each signal.
     *
     * Internally, FIR filters get calculated according to the RF map's specs.
     * The steepness of the filters can be set with the relative transition width parameter.
     * The spectrum gets shifted by the center frequency of each signal and filtered
     * with the corresponding filter calculated from the RF map information. A floored integer
     * decimation will be applied according to the set oversampling factor
     * (decim = floor(samp_rate/(bw*OSF))).
     *
     * To save CPU load, generic taps can be precalculated with the tool in the apps/
     * folder. This tool outputs a JSON file, which path can be specified in the block
     * parameter <code>file_path</code>.
     *
     * Output of this block is a pmt vector of tuples in the format
     *
     * <code>#[#(SigNo, cfreq, bandwidth, #[vector of samples]), #(...)]</code>
     */
    class INSPECTOR_API signal_separator_c : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<signal_separator_c> sptr;

      /*!
       * \brief Return new instance of Signal Separator block
       *
       * \param samp_rate Sample rate of input signal
       * \param window Firdes window type to scale input samples with
       * \param trans_width Transition width for each signal relative to center frequency (w = trans_width*center_freq)
       * \param oversampling Oversampling factor (OSF) to ensure the whole signal gets passed
       * \param taps_file Determine wether to use a file of precalculated taps or caluculate them during runtime
       * \param file_path File path to taps file (leave empty if not enabled)
       */
      static sptr make(double samp_rate, int window, float trans_width,
                       float oversampling, bool taps_file, std::map<float, std::vector<float> > &file_path);

      virtual void set_samp_rate(double d_samp_rate) = 0;

      /*!
       *  Takes integers and does internal cast to firdes::win_type
       */
      virtual void set_window(int d_window) = 0;

      virtual void set_trans_width(float d_trans_width) = 0;
      virtual void set_oversampling(float d_oversampling) = 0;
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_SIGNAL_SEPARATOR_C_H */
