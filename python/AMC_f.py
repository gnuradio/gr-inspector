#!/usr/bin/env python2
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
import pmt
import numpy
from gnuradio import gr
from AMC import * 
import tensorflow as tf
from tensor import *

class AMC_f(gr.sync_block):
    """
    docstring for block AMC
    """
    def __init__(self,vlen,graphfile):

        gr.sync_block.__init__(self,
            name="AMC",
            in_sig=[(numpy.float32,vlen)],
            out_sig=[])

        sess,inp,out = load_graph(graphfile)        
        self.sess = sess
        self.inp = inp
        self.out = out        

        self.message_port_register_out(pmt.intern('classification'))

    def work(self, input_items, output_items):
        for i in input_items:
            for v in i:
                a = pmt.make_dict()
                outp = self.sess.run(self.out,feed_dict={self.inp: [v]})[0]
                c=0
                for o in outp:
                    print(type(o))
                    o = o.astype(float)               
                    a = pmt.dict_add(a, pmt.intern("out"+str(c)), pmt.from_double(o))
                    c=c+1 
                self.message_port_pub(pmt.intern("classification"),a)
        return 0

