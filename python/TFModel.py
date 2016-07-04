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
import tensorflow as tf

class TFModel(gr.sync_block):
    """
    docstring for block AMC
    """
    def __init__(self,dtype,vlen,graphfile,itensor,otensor):

        gr.sync_block.__init__(self,
            name="TFModel",
            in_sig=[(numpy.dtype(dtype),vlen)],
            out_sig=[])

        self.itensor = itensor
        self.otensor = otensor

        sess,inp,out = self.load_graph(graphfile)  

        self.sess = sess
        self.inp = inp
        self.out = out

        self.message_port_register_out(pmt.intern('classification'))

    def load_graph(self,output_graph_path):
        with tf.Graph().as_default():
            output_graph_def = tf.GraphDef()
            with open(output_graph_path, "rb") as f:
                output_graph_def.ParseFromString(f.read())
                _ = tf.import_graph_def(output_graph_def, name="")
    
            with tf.Session() as sess:
                n_input = sess.graph.get_tensor_by_name("%s:0" % self.itensor)
                output = sess.graph.get_tensor_by_name("%s:0" % self.otensor)
                return (sess,n_input,output)

    def work(self, input_items, output_items):
        for i in input_items:
            for v in i:
                a = pmt.make_dict()                                                                                                                         
                try:
                    outp = self.sess.run(self.out,feed_dict={self.inp: [v]})[0]
                except tf.errors.InvalidArgumentError:
                    print("Invalid size of input vector to TensorFlow model")
                    quit()
                c=0
                for o in outp:
                    o = o.astype(float)
                    a = pmt.dict_add(a, pmt.intern("out"+str(c)), pmt.from_double(o))
                    c=c+1
                
                self.message_port_pub(pmt.intern("classification"),a)  
        return 0

