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


void Plot::adjustScales(){
    ParallelEpiped hull = this->hull();

    int d = (hull.maxVertex - hull.minVertex).length();
        
    double xxScale = 0.5;
    double yScale = 0.5;
    double zScale = 0.5;
    setScale(xxScale, yScale, zScale);

    setZoom(d / (2*sqrt(3.0)));

    coordinates()->axes[Z1].setTicLength(3*xxScale, xxScale);
    coordinates()->axes[Z2].setTicLength(3*xxScale, xxScale);
    coordinates()->axes[Z3].setTicLength(3*xxScale, xxScale);
    coordinates()->axes[Z4].setTicLength(3*xxScale, xxScale);

    coordinates()->axes[X1].setTicLength(3*yScale, yScale);
    coordinates()->axes[X2].setTicLength(3*yScale, yScale);
    coordinates()->axes[X3].setTicLength(3*yScale, yScale);
    coordinates()->axes[X4].setTicLength(3*yScale, yScale);

    coordinates()->axes[Y1].setTicLength(3*xxScale, xxScale);
    coordinates()->axes[Y2].setTicLength(3*xxScale, xxScale);
    coordinates()->axes[Y3].setTicLength(3*xxScale, xxScale);
    coordinates()->axes[Y4].setTicLength(3*xxScale, xxScale);

}









void Plot::calculateHull()
{
	if (actualData_p->empty())
		return;


ParallelEpiped hull(Triple(DBL_MAX,DBL_MAX,DBL_MAX),Triple(-DBL_MAX,-DBL_MAX,-DBL_MAX));



hull.minVertex.x = 0;
hull.maxVertex.x = 10;

hull.minVertex.y = 0;
hull.maxVertex.y = 10;

hull.minVertex.z = 0;
hull.maxVertex.z = 10;
/*
	*	if (data[i].x < hull.minVertex.x)
			hull.minVertex.x = data[i].x;
		if (data[i].y < hull.minVertex.y)
			hull.minVertex.y = data[i].y;
		if (data[i].z < hull.minVertex.z)
			hull.minVertex.z = data[i].z;
	
		if (data[i].x > hull.maxVertex.x)
			hull.maxVertex.x = data[i].x;
		if (data[i].y > hull.maxVertex.y)
			hull.maxVertex.y = data[i].y;
		if (data[i].z > hull.maxVertex.z)
			hull.maxVertex.z = data[i].z;*/

//	actualDataC_->setHull(hull);
















	setHull(hull);
}

    Plot::Plot()
    {
        setTitle("A Simple SurfacePlot Demonstration");

        /*Rosenbrock rosenbrock(this);

        rosenbrock.setMesh(41,31);
        rosenbrock.setDomain(-1.73,1.5,-1.5,1.5);
        rosenbrock.setMinZ(-10);

        rosenbrock.create();*/

        setRotation(30,0,15);
        setScale(1,1,1);
       // setShift(0.15,0,0);
        setZoom(0.9);
        showNormals(false);
/*
        for (unsigned i=0; i!=coordinates()->axes.size(); ++i)
        {
            coordinates()->axes[i].setMajors(7);
            coordinates()->axes[i].setMinors(4);
        }


        coordinates()->axes[X1].setLabelString("x-axis");
        coordinates()->axes[Y1].setLabelString("y-axis");
        coordinates()->axes[Z1].setLabelString(QChar (0x38f)); // Omega - see http://www.unicode.org/charts/


        setCoordinateStyle(BOX);
*/
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
            plot = new Qwt3D::Plot() ;
            QHBoxLayout layout;
            layout.addWidget(plot);
            plot->resize(800,600);

           // double data[10][10] = { {0.0} }; // all elements initialized to 0.

            unsigned int columns = 10;
            unsigned int rows = 10;

            double** dat = new double*[rows];
            for(int i = 0; i < rows; ++i)
                dat[i] = new double[columns];
                
            for(int y=0;y<columns;y++){
            for(int x=0;x<rows;x++){
                dat[x][y] = 1.3;
            }   
            }

            double minx=0.0,maxx=10.0,miny=0,maxy=10.0;
            plot->loadFromData 	(   dat,
			                        columns,
		  	                        rows,
		                            minx,
		                            maxx,
		                            miny,
                                    maxy	 
	                            ); 	

            setLayout(&layout);
        }

        fam_form::~fam_form()
        {
            delete d_timer;
            delete[] d_freq;
            delete d_symbol;
            delete d_manual_cb;
        }

        void fam_form::update(const float *d){
            int Np = 64  ;// 2xNp is the number of columns                                                                                                                                                                        
            int P = 256  ;// number of new items needed to calculate estimate                                                                                                                                                     
            int L = 2   ;

            unsigned int rows = 2 * Np;
            unsigned int columns = 2*P*L;

            double** dat = new double*[rows];
            for(int i = 0; i < rows; ++i)
                dat[i] = new double[columns];
                
            int z = 0.0;
            for(int y=0;y<columns;y++){
            for(int x=0;x<rows;x++){
                dat[x][y] = d[z];
                z++;
            }   
            }

            double minx=0.0,maxx=10.0,miny=0,maxy=10.0;
            plot->loadFromData 	(   dat,
			                        rows,
		  	                        columns,
		                            minx,
		                            maxx,
		                            miny,
                                    maxy	 
	                            ); 
            //plot->updateNormals();
           // plot->coordinates()->axes[0].recalculateTics();
            //plot->coordinates()->axes[1].recalculateTics();

            for (int i=0; i<4*3;i++)
                    plot->coordinates()->axes[i].setAutoScale(false);



/*
        plot->setRotation(30,0,15);
        plot->setScale(1,1,1);
       // setShift(0.15,0,0);
    plot->setIsolines(0);
        plot->setZoom(0.9);
        plot->showNormals(false);
*/
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

    }

}
