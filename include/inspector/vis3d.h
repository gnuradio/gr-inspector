/* -*- c++ -*- */
/* 
 * Copyright 2016 Christopher Richardson
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


#ifndef INCLUDED_INSPECTOR_VIS3D_H
#define INCLUDED_INSPECTOR_VIS3D_H


#ifdef ENABLE_PYTHON
  #include <Python.h>
#endif


#include <inspector/api.h>
#include <gnuradio/sync_block.h>
#include <QtGui>
namespace gr {
  namespace inspector {
    class INSPECTOR_API vis3d : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<vis3d> sptr;

      static sptr make(int,int,int,int,int,double,int,char*,char*,char*,QWidget*parent=NULL);
#ifdef ENABLE_PYTHON
      virtual PyObject* pyqwidget() = 0;
#else
      virtual void* pyqwidget() = 0;
#endif
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_VIS3D_H */

