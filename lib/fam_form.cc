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
        fam_form::fam_form(QWidget *parent, int width, int height,int gwidth,int gheight) : QMainWindow(parent) // QWidget(parent)
        {
            plot = new Qwt3D::Plot() ;
            this->width = width;
            this->height = height;
            this->gwidth =gwidth;
            this->gheight = gheight;

            QPushButton *button_3d = new QPushButton("3D", this);
            QPushButton *button_2d = new QPushButton("2D", this);

            connect(button_3d, SIGNAL (released()), this, SLOT (btn3d()));
            connect(button_2d, SIGNAL (released()), this, SLOT (btn2d()));

            QGridLayout *gridLayout = new QGridLayout;
            gridLayout->addWidget(plot,0,0,2,2);
            gridLayout->addWidget(button_2d,1,0,1,1);
            gridLayout->addWidget(button_3d,1,1,1,1);


            QWidget *window = new QWidget();
            window->setLayout(gridLayout);

            setCentralWidget(window);
        }

        void fam_form::btn3d()
        {
            QCoreApplication::postEvent(this,new RotEvent(true));
        }

        void fam_form::btn2d()
        {
            QCoreApplication::postEvent(this,new RotEvent(false));
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
            unsigned int rows =  height;
            unsigned int columns = width;
            double minx=0.0,maxx=gwidth,miny=0,maxy=gheight;

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
            } else if(event->type() == ROT_EVENT)
            {
                handleRotEvent(static_cast<RotEvent *>(event));
            }


            
        }


        void fam_form::handleMyCustomEvent(const MyCustomEvent *event)
        {
            // Now you can safely do something with your Qt objects.
            // Access your custom data using event->getCustomData1() etc.
            update(event->getCustomData1());
        }

        void fam_form::handleRotEvent(const RotEvent *event)
        {
            // Now you can safely do something with your Qt objects.
            // Access your custom data using event->getCustomData1() etc.
            if (event->getCustomData1() == true) {
                plot->setRotation(15,0,15);
                plot->setScale(1.0,1.0,1.0);
                plot->setZoom(0.5);
                
                /*plot->enableLighting(true);

                for (int i=0;i<7;i++)
                    plot->illuminate(i);*/
                /*plot->setLightComponent(GL_LIGHT0,1.0,1);

                plot->setLightShift(0.5,0.5,0.5);
                plot->setLightRotation(90,0,0);*/


            } else {
                plot->setRotation(85,0,0);
                plot->setScale(1.0,1.0,1.0);
                plot->setZoom(2.0);
                //plot->setMeshColor(Qwt3D::RGBA(255,255,255,122));
                /*plot->setLightRotation(70,0,0);
                plot->setLightShift(0.4,0.4,0.4);
                plot->setShininess(100.0);*/

            }
        }

    }

}
