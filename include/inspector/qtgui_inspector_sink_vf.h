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


#ifndef INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_H
#define INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_H

#ifdef ENABLE_PYTHON
  #include <Python.h>
#endif

#include <inspector/api.h>
#include <gnuradio/sync_block.h>
#include <qapplication.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief GUI for the inspector toolkit
     * \ingroup inspector
     *
     * \details
     * This GUI uses QT and QWT to create a graphical interface to control the
     * inspector and get feedback about the detected and analyzed signals.
     *
     * The PSD estimation of the Signal Detector block gets displayed here along
     * with signal markers and informations. Manual signal selection can be done
     * within the GUI created by this block.
     *
     * Analysis blocks can pass information to the GUI block by providing
     * messages in the tuple format
     *
     * <code>((pmt::intern("signal"), pmt::from_uint64(signal_no)), (pmt::intern("description1"), pmt::from_float(value1)), ...)</code>
     *
     * Controls: Use CTRL+LeftMouse to zoom, RightMouse to zoom out.
     * This prevents confusion when in manual selection mode, where the
     * signals are selected with LeftMouse.
     *
     * Message ports:
     * - map in: takes RF map from Signal Detector block and visualizes the included
     * information in the plot
     * - analysis_in: takes messages of analysis blocks and displays their content
     * in the GUI plot.
     * - map out: passes either the input message when manual selection is deactivated
     * or passes RF map according to the user's manual settings.
     */
    class INSPECTOR_API qtgui_inspector_sink_vf : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<qtgui_inspector_sink_vf> sptr;

      /*!
       * \brief Build a GUI sink instance.
       *
       * \param samp_rate Sample rate of the input signal.
       * \param fft_len Vector input size and number of PSD sample points
       * \param cfreq Center frequency of source. Set to 0 if only relative frequencies are desired downstream.
       * \param rf_unit Determine which unit the GUI uses to display frequencies
       * \param msgports Number of message ports for analysis feedback
       * \param manual Enable manual signal selection
       * \param parent QWidget parent
       */
      static sptr make(double samp_rate, int fft_len, float cfreq = 0,
                       int rf_unit = 1000000, int msgports = 1,
                       bool manual = false, QWidget *parent = NULL);

      virtual void set_rf_unit(int unit) = 0;
      virtual void set_samp_rate(double d_samp_rate) = 0;
      virtual void set_cfreq(float cfreq) = 0;
#ifdef ENABLE_PYTHON
      virtual PyObject* pyqwidget() = 0;
#else
      virtual void* pyqwidget() = 0;
#endif
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_H */
