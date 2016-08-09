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

namespace Qwt3D
{

    class Rosenbrock : public Function
    {
    public:

        Rosenbrock(SurfacePlot* pw) :Function(pw)
        {
        }

        double operator()(double x, double y)
        {
            return log((1-x)*(1-x) + 100 * (y - x*x)*(y - x*x)) / 8;
        }
    };


    class Plot : public SurfacePlot
    {
    public:
        Plot();
    };

    Plot::Plot()
    {
        setTitle("A Simple SurfacePlot Demonstration");

        Rosenbrock rosenbrock(this);

        rosenbrock.setMesh(41,31);
        rosenbrock.setDomain(-1.73,1.5,-1.5,1.5);
        rosenbrock.setMinZ(-10);

        rosenbrock.create();

        setRotation(30,0,15);
        setScale(1,1,1);
        setShift(0.15,0,0);
        setZoom(0.9);

        for (unsigned i=0; i!=coordinates()->axes.size(); ++i)
        {
            coordinates()->axes[i].setMajors(7);
            coordinates()->axes[i].setMinors(4);
        }


        coordinates()->axes[X1].setLabelString("x-axis");
        coordinates()->axes[Y1].setLabelString("y-axis");
        coordinates()->axes[Z1].setLabelString(QChar (0x38f)); // Omega - see http://www.unicode.org/charts/


        setCoordinateStyle(BOX);

        updateData();
        updateGL();
    }



}

namespace gr
{
    namespace inspector
    {
        fam_form::fam_form(QWidget *parent) : QWidget(parent)
        {
            d_marker_count = 30;
            Qwt3D::Plot * plot = new Qwt3D::Plot() ;
            QHBoxLayout layout;
            layout.addWidget(plot);
            plot->resize(800,600);
            setLayout(&layout);
        }

        fam_form::~fam_form()
        {
            delete d_timer;
            delete[] d_freq;
            delete d_symbol;
            delete d_manual_cb;
        }

        void
        fam_form::spawn_signal_selector()
        {
        }


        void
        fam_form::resizeEvent( QResizeEvent * event )
        {
        }

    }

}
