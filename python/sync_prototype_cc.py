#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2016 Sebastian Müller.
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

import numpy
from gnuradio import gr
import matplotlib.pyplot as plt

class sync_prototype_cc(gr.sync_block):
    """
    docstring for block sync_prototype_cc
    """
    def __init__(self, samp_rate):
        gr.sync_block.__init__(self,
            name="sync_prototype_cc",
            in_sig=[numpy.complex64],
            out_sig=[numpy.float32])
        self.taus = [0, 255]

    def autocorr(self, in0, tau):
        R = 0
        Rxx = numpy.empty(0)
        for i in numpy.arange(len(in0)-tau-1,-1,-1):
            R *= len(Rxx)
            R += in0[i+tau]*numpy.conj(in0[i])
            R *= 1.0/(len(Rxx)+1)
            Rxx = numpy.hstack((R, Rxx))
        return Rxx

    def cyclic_corr(self, in0):
        R = numpy.fft.fftshift(numpy.fft.fft(in0))
        R = R[len(R)/2:]
        return R

    def work(self, input_items, output_items):
        in0 = input_items[0]
        out = output_items[0]
        if(len(in0) < 1024):
            return 0
        # <+signal processing here+>
        l = len(self.cyclic_corr(self.autocorr(in0, 256)))
        R = numpy.empty([0, l])
        for t in self.taus:
            R = numpy.vstack((R, self.cyclic_corr(self.autocorr(in0, t))[:l]))

        n = numpy.mean([1/(2*numpy.pi*k/288) * numpy.angle(R[0][k]) for k in range(1, len(R[0]))])
        fo = numpy.mean([1/(2*numpy.pi*256) * numpy.angle(R[1][k] * numpy.exp(1j*2*numpy.pi*k/288*n)) for k in range(1, len(R[1]))])

        print "n = " + str(n)
        print "fo = " + str(fo)


        out[:len(R[0])] = numpy.abs(R[0])
        return len(R[0])

