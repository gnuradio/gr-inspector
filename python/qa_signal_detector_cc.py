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
import pmt
import numpy
import time
from signal_detector_cc import signal_detector_cc

class qa_signal_detector_cc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # cosine source
        src1 = analog.sig_source_c(32000, analog.GR_COS_WAVE,
                                          12500, 1)
        src2 = analog.sig_source_c(32000, analog.GR_COS_WAVE,
                                   5700, 1)
        add = blocks.add_cc()
        detector = signal_detector_cc(32000)
        # we dont need the samples anymore
        dst = blocks.null_sink(gr.sizeof_gr_complex*1)
        # check detection with message debug
        msg_dst = blocks.message_debug()

        # connections
        self.tb.connect(src1, (add,0))
        self.tb.connect(src2, (add,1))
        self.tb.connect(add, detector)
        self.tb.connect((detector, 0), dst)
        self.tb.msg_connect((detector, 'map_out'), (msg_dst, 'store'))

        # run
        self.tb.start()
        time.sleep(0.1)
        self.tb.stop()
        self.tb.wait()

        # evaluation
        msg = msg_dst.get_message(1)
        res_vector = numpy.empty([0,2])
        for i in range(pmt.length(msg)):
            row = pmt.vector_ref(msg, i)
            res_vector = numpy.vstack((res_vector, numpy.array(
                [pmt.f32vector_ref(row, 0), pmt.f32vector_ref(row, 1)]
            )))
        self.assertAlmostEqual(12500, res_vector[1,0], delta=50)
        self.assertAlmostEqual(12500, res_vector[1,1], delta=50)
        self.assertAlmostEqual(5700, res_vector[0,0], delta=50)
        self.assertAlmostEqual(5700, res_vector[0,1], delta=50)


if __name__ == '__main__':
    gr_unittest.run(qa_signal_detector_cc, "qa_signal_detector_cc.xml")
