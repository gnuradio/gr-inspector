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

#ifndef GR_INSPECTOR_INSPECTOR_FAMFORM_H
#define GR_INSPECTOR_INSPECTOR_FAMFORM_H


#include <QTimer>
#include <QWidget>
#include <QtGui/QtGui>
#include <gnuradio/msg_queue.h>
#include <qwt3d_surfaceplot.h>
#include <qwt3d_function.h>
#include <gnuradio/io_signature.h>
#include "signal_marker.h"

namespace Qwt3D
{
    class Plot : public SurfacePlot
    {
    public:
        void adjustScales();
        void calculateHull();
        Plot();
        int minx;
        int miny;
        int maxx;
        int maxy;
        int maxz;
    };

}

namespace gr
{

    namespace inspector
    {

    
        const QEvent::Type UPDATE_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);

        const QEvent::Type ROT_EVENT = static_cast<QEvent::Type>(QEvent::User + 2);


        class UpdateEvent : public QEvent
        {
        public:
            UpdateEvent( double **data, const double zaxis):
                QEvent(UPDATE_EVENT),
                m_data(data),
                m_zaxis(zaxis)
            {
            }

            double ** getData() const
            {
                return m_data;
            }


            double getZaxis() const
            {
                return m_zaxis;
            }

        private:
            double ** m_data;
            double m_zaxis;
        };

        class RotEvent : public QEvent
        {
        public:
            RotEvent(bool rotation):
                QEvent(ROT_EVENT),
                m_rotation(rotation)
            {
            }

            bool getRotation() const
            {
                return m_rotation;
            }

       private:
            bool m_rotation;
        };

        class vis3d_vf_form : public QMainWindow
        {
            Q_OBJECT

        public:
            Qwt3D::Plot * plot;
            vis3d_vf_form(QWidget *parent,int,int,int,int,char*,char*,char*);
            ~vis3d_vf_form();
        
        private:
            // Width of data
            int width;  
            // Height of data
            int height;
            // Graph width
            int gwidth;  
            // Graph height
            int gheight;
            // X axis 
            char *xaxis;
            // Y axis
            char *yaxis;
            // Z axis
            char *zaxis;             
        public slots:
            // 3D button press 
            void btn3d();
            // 2D button press
            void btn2d();
            // Update the plot
            void update(double * *d,double maxz);
        protected:
            // Handle custom QT events
            void customEvent(QEvent *event); 
        private:
        
            // Handle updating of plot
            void handleUpdateEvent(const UpdateEvent *event);

            // Handle rotation of plot
            void handleRotEvent(const RotEvent *event);

        };

    }
}

#endif
