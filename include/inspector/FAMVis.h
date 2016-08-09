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


#ifndef INCLUDED_INSPECTOR_FAMVIS_H
#define INCLUDED_INSPECTOR_FAMVIS_H


#ifdef ENABLE_PYTHON
  #include <Python.h>
#endif


#include <inspector/api.h>
#include <gnuradio/sync_block.h>
#include <QtGui>
namespace gr {
  namespace inspector {

    /*!
     * \brief <+description of block+>
     * \ingroup inspector
     *
     */
    class INSPECTOR_API FAMVis : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<FAMVis> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::FAMVis.
       *
       * To avoid accidental use of raw pointers, inspector::FAMVis's
       * constructor is in a private implementation
       * class. inspector::FAMVis::make is the public interface for
       * creating new instances.
       */
      static sptr make(int,QWidget*parent=NULL);
#ifdef ENABLE_PYTHON
      virtual PyObject* pyqwidget() = 0;
#else
      virtual void* pyqwidget() = 0;
#endif
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_FAMVIS_H */

