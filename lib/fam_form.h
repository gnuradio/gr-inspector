/* -*- c++ -*- */
/*
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
#include <qwtplot3d/qwt3d_surfaceplot.h>
#include <qwtplot3d/qwt3d_function.h>
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


        const QEvent::Type MY_CUSTOM_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
        const QEvent::Type ROT_EVENT = static_cast<QEvent::Type>(QEvent::User + 2);

        class MyCustomEvent : public QEvent
        {
        public:
            MyCustomEvent( double ** customData1, const double customData2):
                QEvent(MY_CUSTOM_EVENT),
                m_customData1(customData1),
                m_customData2(customData2)
            {
            }

            double ** getCustomData1() const
            {
                return m_customData1;
            }

            double getCustomData2() const
            {
                return m_customData2;
            }

        private:
            double ** m_customData1;
            double m_customData2;
        };


        class RotEvent : public QEvent
        {
        public:
            RotEvent(bool customData1):
                QEvent(ROT_EVENT),
                m_customData1(customData1)
            {
            }

            bool getCustomData1() const
            {
                return m_customData1;
            }

       private:
            bool m_customData1;
        };


        class fam_form : public QMainWindow // public QWidget
        {
            Q_OBJECT

        public:
            void set_axis_x(float start, float stop);
            void msg_received();
            void set_cfreq(float freq);
            void spawn_signal_selector();
            void add_msg_queue(float cfreq, float bandwidth);
            float freq_to_x(float freq);
            float x_to_freq(float x);
        

            Qwt3D::Plot * plot;

            void drawOverlay();
            fam_form(QWidget *parent,int,int,int , int);
            ~fam_form();

            bool updated=false;

        private:

            enum markerType
            {
                NONE,
                LEFT,
                CENTER,
                RIGHT
            };

            int width;  
            int height;

            int gwidth;  
            int gheight;

            int d_interval, d_fft_len, d_marker_count;
            int *d_rf_unit;
            bool *d_manual;
            std::vector<float> d_axis_x, d_axis_y;
            std::vector<double> *d_buffer;
            float d_max, d_min, d_cfreq, d_mouse_offset;
            double* d_freq;
            std::vector<std::vector<float> >* d_rf_map;
            markerType d_clicked_marker;

            QwtSymbol *d_symbol;
            QTimer *d_timer;
            QCheckBox* d_manual_cb;
            gr::msg_queue* d_msg_queue;

            gr::thread::mutex d_mutex;

        protected:
            void resizeEvent(QResizeEvent * event);

        public slots:

            void btn3d();
            void btn2d();
            void update(double * *d,double maxz);
        public:
            void postMyCustomEvent(double* * customData1, const int customData2);
        protected:
            void customEvent(QEvent *event); // This overrides QObject::customEvent()
        private:
            void handleMyCustomEvent(const MyCustomEvent *event);
            void handleRotEvent(const RotEvent *event);



        };

    }
}

#endif
