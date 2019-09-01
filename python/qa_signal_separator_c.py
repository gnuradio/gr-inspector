#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 Free Software Foundation, Inc.
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
from gnuradio import blocks, filter
import inspector_swig as inspector_test
import pmt
import numpy
import time
from gnuradio.filter import firdes


class qa_signal_separator_c (gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        src = blocks.vector_source_c(range(10000), False, 1, [])
        separator = inspector_test.signal_separator_c(32000, firdes.WIN_HAMMING, 0.1, 100, False,
                                                      inspector_test.map_float_vector({0.0: [0.0]}))
        vec_sink = blocks.vector_sink_c(1)
        ext = inspector_test.signal_extractor_c(0)
        snk = blocks.vector_sink_c(1)
        # pack message
        msg = pmt.make_vector(1, pmt.PMT_NIL)
        flanks = pmt.make_f32vector(2, 0.0)
        pmt.f32vector_set(flanks, 0, 12500)
        pmt.f32vector_set(flanks, 1, 20)
        pmt.vector_set(msg, 0, flanks)

        msg_src = blocks.message_strobe(msg, 100)

        taps = filter.firdes.low_pass(
            1, 32000, 500, 50, firdes.WIN_HAMMING, 6.76)

        self.tb.connect(src, separator)
        self.tb.connect(src, vec_sink)
        self.tb.msg_connect((msg_src, 'strobe'), (separator, 'map_in'))
        self.tb.msg_connect((separator, 'sig_out'), (ext, 'sig_in'))
        self.tb.connect(ext, snk)

        self.tb.start()
        time.sleep(0.3)
        self.tb.stop()
        self.tb.wait()

        data = vec_sink.data()

        sig = numpy.zeros(len(vec_sink.data()), dtype=complex)
        for i in range(len(vec_sink.data())):
            sig[i] = data[i]*numpy.exp(-1j*2*numpy.pi*12500*i*1/32000)

        taps = filter.firdes.low_pass(
            1, 32000, 900, 90, firdes.WIN_HAMMING, 6.76)
        sig = numpy.convolve(sig, taps, 'full')
        out = numpy.empty([0])
        decim = int(32000 / 20 / 100)
        j = 0
        for i in range(int(len(sig) // decim)):
            out = numpy.append(out, sig[j])
            j += decim

        data = snk.data()
        for i in range(min(len(out), len(data))):
            self.assertComplexAlmostEqual2(out[i], data[i], abs_eps=0.01)


if __name__ == '__main__':
    gr_unittest.run(qa_signal_separator_c)
