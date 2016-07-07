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
import pmt
from gnuradio import gr

class ofdm_prototype_c(gr.sync_block):
    """
    docstring for block ofdm_prototype_c
    """
    def __init__(self, samp_rate):
        gr.sync_block.__init__(self,
            name="ofdm_prototype_c",
            in_sig=[numpy.complex64],
            out_sig=None)
        self.Nb = 5

        self.busy = False
        self.iter = 0
        self.correct = 0
        self.alpha_range = [16, 32, 64, 128, 256, 512, 1024]
        self.beta_range = [4, 8, 16, 32]
        self.samp_rate = samp_rate
        self.message_port_register_out(pmt.intern("ofdm_out"))

    def autocorr(self, sig, a, b, p):
        '''
        implement eq. 26
        '''
        M = len(sig)
        R = 0
        for m in range(0,M-1-a):
            R += sig[m+a]*numpy.conj(sig[m])*numpy.exp(
                -1j*2*numpy.pi*m*p/(a*(1+b))
            )
        R = R/M
        return R

    def cost_func(self, sig, a, b):
        '''
        implement eq. 23
        '''
        J = 0
        for p in range(-self.Nb, self.Nb):
            J += numpy.abs(self.autocorr(sig, a, b, p))**2
        J = J/(2*self.Nb+1)
        return J


    def work(self, input_items, output_items):
        in0 = input_items[0]
        if self.busy:
            return len(in0)
        self.iter += 1

        #print("Got " +str(len(in0))+" items")

        self.busy = True

        # <+signal processing here+>
        J = -1000 # small value?
        a_res = 0
        b_res = 0
        #print("Optimizing... please be patient")
        for a in self.alpha_range:
            for b in self.beta_range:
                J_new = self.cost_func(in0, a, 1/float(b))
                #print("a = "+str(a)+", b = 1/"+str(b)+", J = "+str(J_new))

                if J_new > J:
                    J = J_new
                    a_res = a
                    b_res = b
        print("")
        print("------- Result #"+str(self.iter)+ " -------")
        print("FFT len = " + str(a_res))
        print("CP len = " + str(1.0/float(b_res)*float(a_res)))
        print("Subcarr. Spacing = " + str(self.samp_rate/a_res) + " Hz")
        print("Symbol time = " + str(float(float(a_res)*(1.0+1/float(b_res))*1000.0/self.samp_rate)) + " ms")
        #if a_res == 512 and b_res == 32:
        #    self.correct += 1
        #    print("Correct estimation " + str(self.correct ) + "/" + str(self.iter))
        #    print("(a = " + str(a_res) + ", b = " + str(1.0/float(b_res)*float(a_res)) + ")")
        #else:
        #    print("Wrong estimation " + str(self.iter-self.correct) + "/" + str(self.iter))
        #    print("(a = " + str(a_res) + ", b = " + str(1.0/float(b_res)*float(a_res)) + ")")
        #print("")
        #numpy.sqrt(100*(10**(-SNR/10.0)))

        self.busy = False
        return len(in0)

