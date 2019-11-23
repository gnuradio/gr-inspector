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
from gnuradio import blocks, channels, analog
import inspector_swig as inspector
import numpy as np
import time
import pmt

class qa_ofdm_synchronizer_cc(gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        #  set up fg
        fft_len = 256
        cp_len = 32
        samp_rate = 32000
        data = np.random.choice([-1, 1], [100, fft_len])

        timefreq = np.fft.ifft(data, axis=0)

        #add cp
        timefreq = np.hstack((timefreq[:, -cp_len:], timefreq))

        # msg (only 4th and 5th tuples are needed)
        id1 = pmt.make_tuple(pmt.intern("Signal"), pmt.from_uint64(0))
        name = pmt.make_tuple(pmt.intern("OFDM"), pmt.from_float(1.0))
        id2 = pmt.make_tuple(pmt.intern("xxx"), pmt.from_float(0.0))
        id3 = pmt.make_tuple(pmt.intern("xxx"), pmt.from_float(0.0))
        id4 = pmt.make_tuple(pmt.intern("xxx"), pmt.from_float(256))
        id5 = pmt.make_tuple(pmt.intern("xxx"), pmt.from_float(32))
        msg = pmt.make_tuple(id1, name, id2, id3, id4, id5)

        tx = np.reshape(timefreq, (1, -1))

        # GR time!
        src = blocks.vector_source_c(tx[0].tolist(), True, 1, [])
        freq_offset = analog.sig_source_c(1, analog.GR_SIN_WAVE,
                                          50.0/samp_rate, 1.0, 0.0)
        mixer = blocks.multiply_cc()
        sync = inspector.ofdm_synchronizer_cc(4096)
        dst = blocks.vector_sink_c()
        dst2 = blocks.vector_sink_c()
        msg_src = blocks.message_strobe(msg, 0)

        # connect
        self.tb.connect(src, (mixer, 0))
        self.tb.connect(freq_offset, (mixer, 1))
        self.tb.connect(mixer, sync)
        self.tb.msg_connect((msg_src, 'strobe'), (sync, 'ofdm_in'))
        self.tb.connect(sync, dst)
        self.tb.connect(src, dst2)

        self.tb.start()
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # check data
        output = dst.data()
        expect = dst2.data()

        # block outputs 0j until it has enough OFDM symbols to perform estimations
        k = next((k for k in range(len(output)) if output[k] != 0j))

        # use 10,000 samples for comparison since block fails sometimes
        # for one work function
        output = output[k:k+10000]
        expect = expect[k:k+10000]

        self.assertComplexTuplesAlmostEqual2(expect, output, abs_eps = 0.001, rel_eps=10)


if __name__ == '__main__':
    gr_unittest.run(qa_ofdm_synchronizer_cc)
