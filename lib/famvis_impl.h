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

#ifndef INCLUDED_INSPECTOR_FAMVIS_IMPL_H
#define INCLUDED_INSPECTOR_FAMVIS_IMPL_H

#include <inspector/famvis.h>
#include "fam_form.h"

namespace gr
{
    namespace inspector
    {

        /*!
         * \brief 3D visualisation block
         * \ingroup inspector
         *
         */
        class famvis_impl : public famvis
        {
        private:
            // Nothing to declare in this block.
            QApplication *d_qApplication;
            int width, height;

            /**
             * Z axis height
             */
            double maxz;

        public:
            /**
             * Implement the block
             */
            famvis_impl(int,int,int,int,int,double maxz,QWidget*);
            ~famvis_impl();
            /**
             * Pointer to our GUI
             */
            fam_form *d_main_gui;
            /**
             * Work function, to handle processing of data
             */
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

#endif /* INCLUDED_INSPECTOR_FAMVIS_IMPL_H */

