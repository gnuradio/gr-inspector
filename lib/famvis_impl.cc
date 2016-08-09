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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "qtgui_inspector_sink_vf_impl.h"
#include <gnuradio/prefs.h>
#include <QFile>
#include <iostream>
#include <QApplication>
#include <qwtplot3d/qwt3d_surfaceplot.h>
#include <qwtplot3d/qwt3d_function.h>
#include <gnuradio/io_signature.h>
#include "famvis_impl.h"

namespace gr
{
    namespace inspector
    {

        famvis::sptr
        famvis::make(int vlen,QWidget *parent)
        {
            return gnuradio::get_initial_sptr
                   (new famvis_impl(vlen,parent));
        }

        /*
         * The private constructor
         */
        famvis_impl::famvis_impl(int vlen,QWidget *parent)
            : gr::sync_block("famvis",
                             gr::io_signature::make(1,1, sizeof(float)*vlen),
                             gr::io_signature::make(0, 0, 0))
        {

#if QT_VERSION >= 0x040500
            std::string style = prefs::singleton()->get_string("qtgui", "style", "raster");
            QApplication::setGraphicsSystem(QString(style.c_str()));
#endif

            d_main_gui = new fam_form(parent);
            d_main_gui->show();
        }

#ifdef ENABLE_PYTHON
        PyObject*
        famvis_impl::pyqwidget()
        {
            PyObject *w = PyLong_FromVoidPtr((void*)d_main_gui);
            PyObject *retarg = Py_BuildValue("N", w);
            return retarg;
        }
#else
        void *
        famvis_impl::pyqwidget()
        {
            return NULL;
        }
#endif

        /*
         * Our virtual destructor.
         */
        famvis_impl::~famvis_impl()
        {
        }

        int
        famvis_impl::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
                          gr_vector_void_star &output_items)
        {
            const float *in = (const float *) input_items[0];
            printf("got sig\n");
            fflush(stdout);
            return noutput_items;
        }

    } /* namespace inspector */
} /* namespace gr */

