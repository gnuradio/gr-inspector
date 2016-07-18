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

class ofdm_prototype_c(gr.sync_block):
    """
    docstring for block ofdm_prototype_c
    """
    def __init__(self, samp_rate):
        gr.sync_block.__init__(self,
            name="ofdm_prototype_c",
            in_sig=[numpy.complex64],
            out_sig=[numpy.float32])
        self.d_samp_rate = samp_rate
        self.typ_a = [32, 64, 128, 256, 512, 1024]
        self.typ_cp = [4, 8 , 16, 32, 64, 128, 256]

    def autocorr(self, in0):
        akf = numpy.correlate(in0, in0, mode='full')
        akf = akf[len(akf)/2:]
        return akf

    def work(self, input_items, output_items):
        in0 = input_items[0]
        out = output_items[0]
        # <+signal processing here+>
        if len(in0) < 4096:
            #print "Got only " + str(len(in0)) + " items!"
            return 0
        akf = abs(self.autocorr(in0))
        a = numpy.argmax(akf[16:1025])+16
        a = min(self.typ_a, key=lambda x:abs(x-a))
        print "a = " +str(a)

        Rxx = numpy.empty(0)
        R = 0
        for i in numpy.arange(len(in0)-a-1,-1,-1):
            R += in0[i+a]*numpy.conj(in0[i])
            R *= 1.0/(len(Rxx)+1)
            Rxx = numpy.hstack((R, Rxx))

        fft = abs(numpy.fft.fftshift(numpy.fft.fft(Rxx, n=8192)))
        fft = fft[len(fft)/2:]
        b = numpy.argmax(fft[int(2*len(fft)/(a+256)):int(2*len(fft)/(a+4))])+int(2*len(fft)/(a+256))
        b = min(self.typ_cp, key=lambda x:abs(x-a+b))

        print "b = " + str(2*len(fft)/b)
        #print "FFT = " + str(fft)
        print ""
        #out[:4096] = fft[:4096]
        #print "len = "+ str(len(akf[:4096]))

        #cxx = numpy.fft(akf);

        return len(in0)


