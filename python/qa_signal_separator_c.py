#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2016 <+YOU OR YOUR COMPANY+>.
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
from gnuradio import blocks, analog, filter
import inspector_swig as inspector_test
import inspector
import pmt
import numpy
import time
from gnuradio.filter import firdes

class qa_signal_separator_c (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def no_test_sebastian (self):
        src = blocks.vector_source_c(range(100), False, 1, [])
        separator = inspector_test.signal_separator_c(32000, firdes.WIN_HAMMING, 0.1, 100)

        # pack message
        msg = pmt.make_vector(1, pmt.PMT_NIL)
        flanks = pmt.make_f32vector(2, 0.0)
        pmt.f32vector_set(flanks, 0, 12490)
        pmt.f32vector_set(flanks, 1, 12510)
        pmt.vector_set(msg, 0, flanks)

        msg_src = blocks.message_strobe(msg, 100)

        self.tb.connect(src, separator)
        self.tb.msg_connect((msg_src, 'strobe'), (separator, 'map_in'))

        self.tb.run()

    def test_001_t (self):
        # set up fg
        src = blocks.vector_source_c(range(1024), False, 1, [])
        # pack message
        msg = pmt.make_vector(1, pmt.PMT_NIL)
        flanks = pmt.make_f32vector(2, 0.0)
        pmt.f32vector_set(flanks, 0, 12500)
        pmt.f32vector_set(flanks, 1, 8)
        pmt.vector_set(msg, 0, flanks)

        msg_src = blocks.message_strobe(msg, 100)


        separator = inspector_test.signal_separator_c(1024, firdes.WIN_HAMMING, 0.1, 1)
        extractor = inspector.signal_extractor_c(0)

        taps = firdes.low_pass(1, 1024, 2, 0.1*2, firdes.WIN_HAMMING, 6.76)
        reference = numpy.convolve(range(1024), taps, 'same')
        #xlator = filter.freq_xlating_fir_filter_ccf(160, taps, 12500, 32000)
        stv1 = blocks.stream_to_vector(gr.sizeof_gr_complex, 128)
        #stv2 = blocks.stream_to_vector(gr.sizeof_gr_complex, 128)
        snk1 = blocks.vector_sink_c(128)
        #snk2 = blocks.vector_sink_c(128)
        null1 = blocks.null_sink(gr.sizeof_gr_complex)
        # connect this
        self.tb.connect(src, separator)
        self.tb.msg_connect((msg_src, 'strobe'), (separator, 'map_in'))
        #self.tb.connect(src, xlator)
        #self.tb.connect(xlator, null1)
        self.tb.msg_connect((separator, 'msg_out'), (extractor, 'sig_in'))
        self.tb.connect(extractor, stv1)
        #self.tb.connect(xlator, stv2)
        self.tb.connect(stv1, snk1)
        #self.tb.connect(stv2, snk2)

        self.tb.start()
        time.sleep(0.5)
        self.tb.stop()
        # check data
        data1 = snk1.data()
        #data2 = snk2.data()
        data2 = reference[0::8]

        for i in range(min(len(data2), len(data1))):
            self.assertComplexAlmostEqual(data1[i], data2[i], 4)

if __name__ == '__main__':
    gr_unittest.run(qa_signal_separator_c, "qa_signal_separator_c.xml")
