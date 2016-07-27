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
import numpy as np
from gnuradio import gr
import tensorflow as tf
from numpy import zeros, newaxis
import collections

class TFModel(gr.sync_block):
    """
    docstring for block AMC
    """
    def __init__(self,dtype,vlen,graphfile,itensor,otensor,num_inputs,neurons=None):
    
        self.pmtin = False

        inputs = []

        if dtype == "message":
            self.pmtin = True
        else:
            for i in range(num_inputs):
                inputs.append((np.dtype(dtype),vlen))

        print(inputs)

        gr.sync_block.__init__(self,
            name="TFModel",
            in_sig=inputs,
            out_sig=[])

        if self.pmtin:
            self.message_port_register_in(pmt.intern('in'))
            self.set_msg_handler(pmt.intern("in"), self.msg_handler)

        self.inputs = inputs

        self.itensor = itensor
        self.otensor = otensor

        sess,inp,out = self.load_graph(graphfile)  

        self.sess = sess
        self.inp = inp
        self.out = out

        self.old = collections.deque(maxlen=3)


        self.keep = sess.graph.get_tensor_by_name("drop1/cond/dropout/keep_prob:0")

        
    
        if neurons == None:
            self.neuronsb = False
        else: 
            self.neuronsb = True
            try:
                self.neurons =  sess.run(sess.graph.get_tensor_by_name("%s:0" % neurons))
                print(self.neurons)
            except:
                self.neuronsb = False

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

    def msg_handler(self,msg):
        msg = pmt.to_python(msg)

        for v in msg:

            # v[0] Signal ID
            # v[1] Signal
        
            signal = v[1]

            print("SIGLEN",len(signal))
            if not len(signal) > 128:
                continue
                     
            re = signal[0:128].real[:,newaxis]
            im = signal[0:128].imag[:,newaxis]

            pmtv = pmt.make_dict()                                                                                                                         
            try:
                outp = self.sess.run(self.out,feed_dict={self.inp: [[re,im]],self.keep: 1.0})[0]
            except tf.errors.InvalidArgumentError:
                print("Invalid size of input vector to TensorFlow model")
                quit()
    
            if self.neuronsb:
                mod = self.neurons [ np.argmax(outp) ] 
                #pmtv = pmt.dict_add(pmtv, pmt.intern("Mod"), pmt.intern(mod))
                outp = pmt.to_pmt([v[0],mod])
            
        
                self.message_port_pub(pmt.intern("classification"),outp)  


        return len(msg)



    def work(self, input_items, output_items):
        #output_items[0][:] = input_items[0] * input_items[0] # Only works because numpy.array
        tensordata = []
        input_i = []
        shapev = np.array(input_items[0]).shape
        print(shapev)
        inp = input_items[0]
        items = np.array(input_items).shape[0] * np.array(input_items).shape[1] 
        
        for i in range(shapev[0]):
            in_v = []
            re = []
            im = []

            for v in range(shapev[1]):
                #print("i",i,"v",v,len(input_items[i]))
                re.append(inp[i][v].real)
                im.append(inp[i][v].imag)
                    
            in_v.append(np.array(re)[:,newaxis])
            in_v.append(np.array(im)[:,newaxis])

            #print(len(in_v))


            tensordata.append(in_v)
                    

            """
            else:
                for v in range(len(input_items[0])):
                    in_v.append(np.array(input_items[i][v])[:, newaxis])
            
                    tensordata.append(in_v)
            """
    


        mod = ""     
        ne = []
        for v in tensordata:
            pmtv = pmt.make_dict()                                                                                                                         
            try:
                outp = self.sess.run(self.out,feed_dict={self.inp: [v],self.keep: 1.0})[0]
                ne.append(outp)
            except tf.errors.InvalidArgumentError:
                print("Invalid size of input vector to TensorFlow model")
                quit()

        mn = ne
        for outp in mn:
        
            c=0
            
            if self.neuronsb:
                mod = self.neurons [ np.argmax(outp) ] 
                pmtv = pmt.dict_add(pmtv, pmt.intern("Mod"), pmt.intern(mod))
            
                for o in outp:
                    o = o.astype(float)
                    pmtv = pmt.dict_add(pmtv, pmt.intern("out"+self.neurons[c]), pmt.from_double(o))
                    c=c+1
            self.message_port_pub(pmt.intern("classification"),pmtv)  

        
        return len(input_items[0]) 

