/* -*- c++ -*- */
/*
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

#include "fam_form.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <qwt_transform.h>
#include <pmt/pmt.h>
#include <QApplication>
#include <qwtplot3d/qwt3d_surfaceplot.h>
#include <qwtplot3d/qwt3d_function.h>
#include <gnuradio/io_signature.h>
#include <qwt_plot_canvas.h>


namespace Qwt3D
{

    void Plot::calculateHull()
    {
        if (actualData_p->empty())
            return;

        ParallelEpiped hull(Triple(DBL_MAX,DBL_MAX,DBL_MAX),Triple(-DBL_MAX,-DBL_MAX,-DBL_MAX));

        hull.minVertex.x = minx;
        hull.maxVertex.x = maxx;

        hull.minVertex.y = miny;
        hull.maxVertex.y = maxy;

        hull.minVertex.z = 0;
        hull.maxVertex.z = 10 ; //maxz;

        setHull(hull);
    }

    Plot::Plot()
    {
        setTitle("A Simple SurfacePlot Demonstration");
        for (unsigned i=0; i!=coordinates()->axes.size(); ++i)
        {
            coordinates()->axes[i].setMajors(7);
            coordinates()->axes[i].setMinors(4);
        }
        setRotation(15,0,15);
        setScale(1.0,1.0,1.0);
        setZoom(0.5);


        updateData();
        updateGL();
    }



}


namespace gr
{
    namespace inspector
    {
        fam_form::fam_form(QWidget *parent) : QMainWindow(parent) // QWidget(parent)
        {
            plot = new Qwt3D::Plot() ;


            setCentralWidget(plot);

        }

        fam_form::~fam_form()
        {
            delete d_timer;
            delete[] d_freq;
            delete d_symbol;
            delete d_manual_cb;
        }

        void fam_form::update( double * *d)
        {
            int Np = 64  ;// 2xNp is the number of columns
            int P = 256  ;// number of new items needed to calculate estimate
            int L = 2   ;

            unsigned int rows = 2 * Np;
            unsigned int columns = 2*P*L;
            double minx=0.0,maxx=10,miny=0,maxy=10;

            plot->minx = minx;
            plot->maxx = maxx;

            plot->miny = miny;
            plot->maxy = maxy;

            plot->maxz = 20;
            plot->loadFromData 	(   d,
                                    rows,
                                    columns,
                                    minx,
                                    maxx,
                                    miny,
                                    maxy
                                );

            plot-> updateData();
            plot->updateGL();

            plot->repaint();
            repaint();
        }

        void
        fam_form::spawn_signal_selector()
        {
        }


        void
        fam_form::resizeEvent( QResizeEvent * event )
        {
        }

        void fam_form::customEvent(QEvent * event)
        {
            // When we get here, we've crossed the thread boundary and are now
            // executing in the Qt object's thread

            if(event->type() == MY_CUSTOM_EVENT)
            {
                handleMyCustomEvent(static_cast<MyCustomEvent *>(event));
            }
        }


        void fam_form::handleMyCustomEvent(const MyCustomEvent *event)
        {
            // Now you can safely do something with your Qt objects.
            // Access your custom data using event->getCustomData1() etc.
            update(event->getCustomData1());
        }

    }

}
