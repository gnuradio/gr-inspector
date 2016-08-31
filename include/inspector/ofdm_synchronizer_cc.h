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


#ifndef INCLUDED_INSPECTOR_OFDM_SYNCHRONIZER_CC_H
#define INCLUDED_INSPECTOR_OFDM_SYNCHRONIZER_CC_H

#include <inspector/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace inspector {

    /*!
     * \brief OFDM frequency synchronizer and timing estimator
     * \ingroup inspector
     *
     * \details
     * This block can be inserted downstream to the OFDM parameter
     * estimation block. It performs a frequency offset estimation and
     * symbol timing estimation according to [1]. Frequency offsets smaller than the
     * subcarrier spacing will be corrected to obtain orthogonality of
     * the carriers. Output is a stream with stream tags at the estimated
     * OFDM symbol beginnings (PMT symbol "symbol").
     *
     * Connect output of OFDM parameter estimator to message port and
     * signal itself to the stream input.
     *
     * [1] J. J. van de Beek, M. Sandell, and P. O. Borjesson,
     * “ML estimation of time and frequency offset in OFDM systems,”
     * IEEE Transactions on Signal Processing, vol. 45, no. 7, pp.
     * 1800–1805, 1997. [Online]. Available: http://pure.ltu.se/portal/files/1612155/Article.pdf.
     */
    class INSPECTOR_API ofdm_synchronizer_cc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<ofdm_synchronizer_cc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of inspector::ofdm_synchronizer_cc.
       *
       * \param min_items Set minimum items required for each work call. Should be several FFT lengths.
       */
      static sptr make(int min_items);
    };

  } // namespace inspector
} // namespace gr

#endif /* INCLUDED_INSPECTOR_OFDM_SYNCHRONIZER_CC_H */
