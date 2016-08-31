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

#ifndef INCLUDED_INSPECTOR_VIS3D_VF_IMPL_H 
#define INCLUDED_INSPECTOR_VIS3D_VF_IMPL_H 

#include <inspector/vis3d_vf.h>
#include "vis3d_vf_form.h"
#include <chrono>

using namespace std::chrono;


namespace gr
{
    namespace inspector
    {

        class vis3d_vf_impl : public vis3d_vf
        {
        private:
            QApplication *d_qApplication;
            int width, height;

            double maxz;

            int fcount;

            int fps;

            milliseconds oldms; 
        public:
            vis3d_vf_impl(int vlen,int width, int height,int gwidth,int gheight,double maxz, int fps,char *xaxis,char* yaxis, char *zaxis,QWidget *parent);
            ~vis3d_vf_impl();
            vis3d_vf_form *d_main_gui;
            int work(int noutput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items);
#ifdef ENABLE_PYTHON
            PyObject* pyqwidget();
#else
            void* pyqwidget();
#endif
        };

    } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_VIS3D_VF_IMPL_H */

