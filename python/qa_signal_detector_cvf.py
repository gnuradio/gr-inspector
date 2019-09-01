#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 Free Software Foundation Inc..
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
from gnuradio.filter import firdes
import time
import numpy
import pmt
import inspector_swig as inspector


class qa_signal_detector_cvf (gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_t(self):
        src1 = analog.sig_source_c(32000, analog.GR_COS_WAVE,
                                   12500, 3)

        src2 = analog.sig_source_c(32000, analog.GR_COS_WAVE,
                                   9800, 3)

        add = blocks.add_cc()
        detector = inspector.signal_detector_cvf(32000,  4096,
                                                 firdes.WIN_BLACKMAN_hARRIS,
                                                 -80, 0.6, False, 0.5, 0.001)
        dst1 = blocks.null_sink(gr.sizeof_float*4096)
        msg_dst = blocks.message_debug()

        # connections
        self.tb.connect(src1, (add, 0))
        self.tb.connect(src2, (add, 1))
        self.tb.connect(add, detector)
        self.tb.connect((detector, 0), dst1)
        self.tb.msg_connect((detector, 'map_out'), (msg_dst, 'store'))
        self.tb.start()
        time.sleep(0.5)
        self.tb.stop()
        self.tb.wait()

        # take most recent message
        msg = msg_dst.get_message(msg_dst.num_messages()-1)
        res_vector = numpy.empty([0, 2])
        for i in range(pmt.length(msg)):
            row = pmt.vector_ref(msg, i)
            res_vector = numpy.vstack((res_vector, numpy.array(
                [pmt.f32vector_ref(row, 0), pmt.f32vector_ref(row, 1)]
            )))

        self.assertAlmostEqual(9800.0, res_vector[0][0], delta=50)
        self.assertAlmostEqual(0.0, res_vector[0][1], delta=100)
        self.assertAlmostEqual(12500.0, res_vector[1][0], delta=50)
        self.assertAlmostEqual(0.0, res_vector[1][1], delta=100)


if __name__ == '__main__':
    gr_unittest.run(qa_signal_detector_cvf)
