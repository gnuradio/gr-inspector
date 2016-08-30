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


#ifndef INCLUDED_INSPECTOR_VIS3D_VF_H
#define INCLUDED_INSPECTOR_VIS3D_VF_H


#ifdef ENABLE_PYTHON
  #include <Python.h>
#endif


#include <inspector/api.h>
#include <gnuradio/sync_block.h>
#include <qapplication.h>
#include <QtGui>

namespace gr {
  namespace inspector {
    /*!
     * \brief 3D visualisation for the inspector toolkit
     * \ingroup inspector
     *
     * \details
     * This GUI uses QwtPlot3D to visualise data in 3D, for instance data
     * from the gr-specest FAM block.
     * 
     * Controls: Hold left mouse button and move cursor to rotate.
     * Dragging using two fingers, zooms in or out
     *
     */
    class INSPECTOR_API vis3d_vf : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<vis3d_vf> sptr;

      /*!
       * \brief Build a 3D visualisation 
       *
       * \param vlen The size of the input vector containing the data to be displayed
       * \param width The 1D vector will be reshaped with this width
       * \param height The 1D vector will be reshaped with this height
       * \param gwidth The graph will be stretched to this width
       * \param gheight The graph will be stretched to this height
       * \param maxz You can set the graph to have a maximum Z value, so that the graph boundaries remain fixed, or if you set to 0, the Z axis of the graph, will grow according to the data.
       * \param fps You can set the framerate of the graph, so that if a higher number vectors are received than the frame rate, they can be discarded.
       * \param xaxis Label for X axis
       * \param yaxis Label for Y axis
       * \param zaxis Label for Z axis
       * \param parent QWidget parent
       */
      static sptr make(int vlen,int width, int height,int gwidth,int gheight,double maxz, int fps,char *xaxis,char* yaxis, char *zaxis,QWidget *parent=NULL);

#ifdef ENABLE_PYTHON
      virtual PyObject* pyqwidget() = 0;
#else
      virtual void* pyqwidget() = 0;
#endif
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_VIS3D_VF_H */

