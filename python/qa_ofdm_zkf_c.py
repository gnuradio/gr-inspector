#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 Free Software Foundation, Inc..
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import inspector_swig as inspector
import numpy as np
import time
import pmt


class qa_ofdm_zkf_c(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        # set up fg
        fft_len = 256
        cp_len = 32
        samp_rate = 32000
        data = np.random.choice([-1, 1], [100, fft_len])

        timefreq = np.fft.ifft(data, axis=0)

        # add cp
        timefreq = np.hstack((timefreq[:, -cp_len:], timefreq))

        tx = np.reshape(timefreq, (1, -1))

        # GR time!
        src = blocks.vector_source_c(tx[0].tolist(), True, 1, [])
        analyzer = inspector.ofdm_zkf_c(samp_rate, 0, 7000, [128, 256, 512, 1024], [8, 16, 32, 64])
        snk = blocks.message_debug()

        # connect
        self.tb.connect(src, analyzer)
        self.tb.msg_connect((analyzer, 'ofdm_out'), (snk, 'store'))

        self.tb.start()
        time.sleep(0.25)
        self.tb.stop()
        self.tb.wait()

        # check data
        result = snk.get_message(0)

        fft_result = pmt.to_float(pmt.tuple_ref(pmt.tuple_ref(result, 4), 1))
        cp_result = pmt.to_float(pmt.tuple_ref(pmt.tuple_ref(result, 5), 1))

        self.assertAlmostEqual(fft_len, fft_result)
        self.assertAlmostEqual(cp_len, cp_result)


if __name__ == '__main__':
    gr_unittest.run(qa_ofdm_zkf_c)
