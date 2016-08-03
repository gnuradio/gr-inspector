/* -*- c++ -*- */
/* 
 * Copyright 2016 Sebastian Müller.
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


#ifndef INCLUDED_INSPECTOR_FM_DEMOD_CF_H
#define INCLUDED_INSPECTOR_FM_DEMOD_CF_H

#include <inspector/api.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief <+description of block+>
     * \ingroup inspector
     *
     */
    class INSPECTOR_API fm_demod_cf : virtual public gr::sync_decimator
    {
     public:
      typedef boost::shared_ptr<fm_demod_cf> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::fm_demod_cf.
       *
       * To avoid accidental use of raw pointers, inspector::fm_demod_cf's
       * constructor is in a private implementation
       * class. inspector::fm_demod_cf::make is the public interface for
       * creating new instances.
       */
      static sptr make(int signal, int audio_rate);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_FM_DEMOD_CF_H */

