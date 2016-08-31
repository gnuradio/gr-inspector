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


#ifndef INCLUDED_INSPECTOR_OFDM_ZKF_C_H
#define INCLUDED_INSPECTOR_OFDM_ZKF_C_H

#include <inspector/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief Parameter estimation block for OFDM signals
     * \ingroup inspector
     *
     * \details
     * This block performs a parameter estimation of an input OFDM signal.
     * Parameters that get estimated are FFT length, cyclic prefix length,
     * symbol time and subcarrier spacing. For the estimation, the algorithm
     * in [1] is used. Input is a complex stream, output is a message as
     * tupel of tupels containing analysis results description and values.
     *
     * [1] S. Koslowski, "Implementierung eines OFDM-Merkmalsklassifikators in einer SCA-Umgebung," Diploma Thesis, Karlsruhe Institute of Technology, 2011.
     */
    class INSPECTOR_API ofdm_zkf_c : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<ofdm_zkf_c> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::ofdm_zkf_c.
       *
       * \param samp_rate The samples rate of the input signal
       * \param signal The number of the analyzed signal
       * \param min_items Set minimum items required for each work call. Should be several FFT lengths.
       * \param typ_len Typical FFT lengths to search for. Can also be set to a fine grid when no knowledge about the signal is present.
       * \param typ_cp Typical cyclic prefix lengths to search for. Can also be set to a fine grid when no knowledge about the signal is present.
       */
      static sptr make(double samp_rate, int signal, int min_items, const std::vector<int> &typ_len, const std::vector<int> &typ_cp);
      virtual void set_samp_rate(double d_samp_rate) = 0;
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_OFDM_ZKF_C_H */
