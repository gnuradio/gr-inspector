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

class AMC_f(gr.sync_block):
    """
    docstring for block AMC
    """
    def __init__(self,vlen):
        gr.sync_block.__init__(self,
            name="AMC",
            in_sig=[(numpy.float32,vlen)],
            out_sig=[])
        self.message_port_register_out(pmt.intern('classification'))



    def work(self, input_items, output_items):
        in0 = input_items[0]
        #out = output_items[0]
        # <+signal processing here+>
        #out[:] = in0


        a = pmt.make_dict()
        a = pmt.dict_add(a, pmt.intern("psk"), pmt.from_double(0.34))
        self.message_port_pub(pmt.intern("classification"),a)
        
        return 0
        #return len(output_items[0])

