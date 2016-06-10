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


#ifndef INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_H
#define INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_H

#include <inspector/api.h>
#include <gnuradio/sync_block.h>
#include <qapplication.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief <+description of block+>
     * \ingroup inspector
     *
     */
    class INSPECTOR_API qtgui_inspector_sink_vf : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<qtgui_inspector_sink_vf> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::qtgui_inspector_sink_vf.
       *
       * To avoid accidental use of raw pointers, inspector::qtgui_inspector_sink_vf's
       * constructor is in a private implementation
       * class. inspector::qtgui_inspector_sink_vf::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fft_len, QWidget *parent = NULL);

#ifdef ENABLE_PYTHON
      virtual PyObject* pyqwidget() = 0;
#else
      virtual void* pyqwidget() = 0;
#endif
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_QTGUI_INSPECTOR_SINK_VF_H */

