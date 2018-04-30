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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "vis3d_vf_impl.h"
#include <chrono>
#include <gnuradio/io_signature.h>
#include <gnuradio/prefs.h>
#include <QFile>
#include <iostream>
#include <QApplication>

using namespace std::chrono;

namespace gr
{
    namespace inspector
    {

        vis3d_vf::sptr
        vis3d_vf::make(int vlen,int width, int height,int gwidth,int gheight,double maxz,int fps,char*xaxis, char*yaxis, char *zaxis, QWidget *parent)
        {
            return gnuradio::get_initial_sptr
                   (new vis3d_vf_impl(vlen,width,height,gwidth,gheight,maxz,fps,xaxis,yaxis,zaxis,parent));

        }

        vis3d_vf_impl::vis3d_vf_impl(int vlen,int width, int height,int gwidth,int gheight,double maxz, int fps,char *xaxis,char* yaxis, char *zaxis,QWidget *parent)
            : gr::sync_block("vis3d_vf",
                             gr::io_signature::make(1,1, sizeof(float)*vlen),
                             gr::io_signature::make(0, 0, 0))
        {
            this->width = width;
            this->height = height;
            this->maxz = maxz;
            this->fps = fps;
    
            if(qApp != NULL)
            {
                d_qApplication = qApp;
            }
            else
            {

#if QT_VERSION >= 0x040500
                std::string style = prefs::singleton()->get_string("qtgui", "style", "raster");
                QApplication::setGraphicsSystem(QString(style.c_str()));
#endif

                char *d_argv;

                int d_argc = 1;
                d_argv = new char;
                d_argv[0] = '\0';

                if (d_qApplication == NULL)
                    d_qApplication = new QApplication(d_argc, &d_argv);

            }

            d_main_gui = new vis3d_vf_form(parent,width,height,gwidth,gheight,xaxis,yaxis,zaxis);
            d_main_gui->show();

        }

#ifdef ENABLE_PYTHON
        PyObject*
        vis3d_vf_impl::pyqwidget()
        {
            PyObject *w = PyLong_FromVoidPtr((void*)d_main_gui);
            PyObject *retarg = Py_BuildValue("N", w);
            return retarg;
        }
#else
        void *
        vis3d_vf_impl::pyqwidget()
        {
            return NULL;
        }
#endif

        vis3d_vf_impl::~vis3d_vf_impl()
        {
        }

        int
        vis3d_vf_impl::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
                          gr_vector_void_star &output_items)
        {
            const float *in = (const float *) input_items[0];

            unsigned int rows = height; 
            unsigned int columns = width;

            double** dat = new double*[rows];
            for(int i = 0; i < rows; ++i)
                dat[i] = new double[columns];

            int z = 0.0;
            for(int y=0; y<columns; y++)
            {
                for(int x=0; x<rows; x++)
                {
                    dat[x][y] = in[z];
                    z++;
                }
            }

            milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

            if (ms > oldms+std::chrono::milliseconds(1000)){
                fcount = 0;
                oldms = ms;
            }

            if (fcount < fps){
                QCoreApplication::postEvent(d_main_gui,new UpdateEvent(dat, this->maxz));
            }

            fcount++;

            return noutput_items;
        }

    } /* namespace inspector */
} /* namespace gr */

