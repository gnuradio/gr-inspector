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

#include "vis3d_vf_form.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <qwt_transform.h>
#include <pmt/pmt.h>
#include <QApplication>
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
        hull.maxVertex.z = maxz;

        setHull(hull);
    }

    Plot::Plot()
    {
        for (unsigned i=0; i!=coordinates()->axes.size(); ++i)
        {
            coordinates()->axes[i].setMajors(0);
            coordinates()->axes[i].setMinors(0);
        }
        
        setCoordinateStyle(FRAME); 
 
        setRotation(15,0,15);
        setScale(1.0,1.0,1.0);
        setZoom(1.0);

        updateData();
        updateGL();
    }
}


namespace gr
{
    namespace inspector
    {

        vis3d_vf_form::vis3d_vf_form(QWidget *parent, int width, int height,int gwidth,int gheight,char *xaxis, char *yaxis, char *zaxis) : QMainWindow(parent) // QWidget(parent)
        {
            plot = new Qwt3D::Plot() ;
            this->width = width;
            this->height = height;
            this->gwidth =gwidth;
            this->gheight = gheight;

            this->xaxis = xaxis;
            this->yaxis = yaxis;    
            this->zaxis = zaxis;
            
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

        void vis3d_vf_form::btn3d()
        {
            QCoreApplication::postEvent(this,new RotEvent(true));
        }

        void vis3d_vf_form::btn2d()
        {
            QCoreApplication::postEvent(this,new RotEvent(false));
        }

        vis3d_vf_form::~vis3d_vf_form()
        {
        }

        void vis3d_vf_form::update( double **d,double maxz)
        {
            unsigned int rows =  height;
            unsigned int columns = width;
            double minx=0.0,maxx=gwidth,miny=0,maxy=gheight;

            plot->minx = minx;
            plot->maxx = maxx;

            plot->miny = miny;
            plot->maxy = maxy;


            if (maxz == 0) {
                for (int y = 0; y < rows ; y++) {
                    for (int x = 0; x < columns ; x++) {
                        if (d[y][x] > maxz) {
                            maxz = d[y][x];
                        } 
        
                    }
                }
            }

            plot->maxz = maxz;
            plot->loadFromData 	(   d,
                                    rows,
                                    columns,
                                    minx,
                                    maxx,
                                    miny,
                                    maxy
                                );

            plot->coordinates()->axes[Qwt3D::X1].setLabel(true);
            plot->coordinates()->axes[Qwt3D::X1].setLabelString(QString(xaxis));

            plot->coordinates()->axes[Qwt3D::Y1].setLabel(true);
            plot->coordinates()->axes[Qwt3D::Y1].setLabelString(QString(yaxis));

            plot->coordinates()->axes[Qwt3D::Z1].setLabel(true);
            plot->coordinates()->axes[Qwt3D::Z1].setLabelString(QString(zaxis));

            plot->updateData();
            plot->updateGL();

            plot->repaint();
            repaint();
        }


        void vis3d_vf_form::customEvent(QEvent * event)
        {
            // When we get here, we've crossed the thread boundary and are now
            // executing in the Qt object's thread

            if(event->type() == UPDATE_EVENT) {
                handleUpdateEvent(static_cast<UpdateEvent *>(event));
            } else if(event->type() == ROT_EVENT) {
                handleRotEvent(static_cast<RotEvent *>(event));
            }

        }

        void vis3d_vf_form::handleUpdateEvent(const UpdateEvent *event)
        {
            update(event->getData(),event->getZaxis());
        }

        void vis3d_vf_form::handleRotEvent(const RotEvent *event)
        {
            if (event->getRotation() == true) {
                plot->setRotation(15,0,15);
                plot->setScale(1.0,1.0,1.0);
                plot->setZoom(1.0);
            } else {
                plot->setRotation(85,0,0);
                plot->setScale(1.0,1.0,1.0);
                plot->setZoom(2.0);
            }
        }

    }

}
