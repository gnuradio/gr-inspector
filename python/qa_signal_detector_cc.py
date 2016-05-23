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
from gnuradio import blocks, analog
import inspector
import pmt
import numpy
from signal_detector_cc import signal_detector_cc

class qa_signal_detector_cc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        src = analog.sig_source_c(32000, analog.GR_COS_WAVE,
                                          12500, 1)
        detector = inspector.signal_detector_cc(32000)
        dst = blocks.null_sink(gr.sizeof_gr_complex*1)
        msg_dst = blocks.message_debug()
        self.tb.connect(src, detector)
        self.tb.connect((detector, 0), dst)
        self.tb.msg_connect((detector, 'map_out'), (msg_dst, 'print'))
        self.tb.run()
        msg = msg_dst.get_message(0)
        self.tb.stop()
        res_vector = numpy.empty([0,2])
        for i in pmt.length(msg):
            row = pmt.vector_ref(msg, i)
            numpy.vstack((res_vector, numpy.array([
                pmt.f32vector_ref(row, 0), pmt.f32vector_ref(row, 1)
            ])))

        self.assertFloatTuplesAlmostEqual(12500, res_vector[0,0], 2)
        self.assertFloatTuplesAlmostEqual(12500, res_vector[0,1], 2)

        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_signal_detector_cc, "qa_signal_detector_cc.xml")
