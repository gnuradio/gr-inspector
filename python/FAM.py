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
from __future__ import division, print_function, absolute_import
import tensorflow as tf    
from tensorflow.contrib.session_bundle import manifest_pb2
from tensorflow.contrib.session_bundle import constants
from tensorflow.contrib.session_bundle import session_bundle
 
import pmt
import numpy as np
from gnuradio import gr
import tensorflow as tf
from numpy import zeros, newaxis
import collections

Np = 64  # 2xNp is the number of columns
P = 256  # number of new items needed to calculate estimate
L = 2

MOD = ["fsk", "qam16", "qam64", "2psk", "4psk", "8psk", "gmsk", "wbfm", "nfm"]


class FAM(gr.sync_block):
    """
    docstring for block AMC
    """
    def __init__(self,dtype,vlen,graphfile,num_inputs,neurons=None):
    
        self.pmtin = False

        inputs = []

        if dtype == "message":
            self.pmtin = True
        else:
            for i in range(num_inputs):
                inputs.append((np.dtype(dtype),vlen))

        print(inputs)

        gr.sync_block.__init__(self,
            name="FAM",
            in_sig=inputs,
            out_sig=[])

        if self.pmtin:
            self.message_port_register_in(pmt.intern('in'))
            self.set_msg_handler(pmt.intern("in"), self.msg_handler)

        self.inputs = inputs
        sess,inp,out = self.load_graph(graphfile)  

        self.sess = sess
        self.inp = inp
        self.out = out

        self.old = collections.deque(maxlen=3)

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

        sess, meta_graph_def = session_bundle.LoadSessionBundleFromPath(output_graph_path) 

        with sess.as_default():

            collection_def = meta_graph_def.collection_def
            signatures_any = collection_def[constants.SIGNATURES_KEY].any_list.value
            signatures = manifest_pb2.Signatures()
            signatures_any[0].Unpack(signatures)
            default_signature = signatures.default_signature

            input_name = default_signature.classification_signature.input.tensor_name
            output_name = default_signature.classification_signature.scores.tensor_name
   
            print( default_signature.classification_signature ) 
            print("INP",sess.graph.get_tensor_by_name( input_name).get_shape())
            return (sess,input_name,output_name)

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
                outp = self.sess.run(self.out,feed_dict={self.inp: [[re,im]]})[0]
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
        inp = np.array(input_items[0][0])

        if np.mean(inp) == 0.0 :
            return len(input_items[0]) 

            
        print("MEAN",np.mean(inp))
        inp = (inp - np.mean(inp)) / np.std(inp)
        floats = np.reshape(inp, (2 * P * L, (2 * Np) - 0))
        tensordata.append(np.array([floats]))
        """
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
                    
            else:
                for v in range(len(input_items[0])):
                    in_v.append(np.array(input_items[i][v])[:, newaxis])
            
                    tensordata.append(in_v)
        """
    


        mod = ""     
        ne = []
        for v in tensordata:
            try:
                outp = self.sess.run(self.out,feed_dict={self.inp: [v]})[0]
                ne.append(outp)
            except tf.errors.InvalidArgumentError:
                print("Invalid size of input vector to TensorFlow model")
                quit()

        pmtv = pmt.make_dict()                                                                                                                         
        for outp in ne:
        
            c=0
            
            #if self.neuronsb:
            #mod = self.neurons [ np.argmax(outp) ] 
            
            pmtv = pmt.dict_add(pmtv, pmt.intern("Mod"), pmt.to_pmt(MOD[np.argmax(outp)]))
            pmtv = pmt.dict_add(pmtv, pmt.intern("Prob"), pmt.to_pmt(outp))
                
            """ 
                for o in outp:
                    o = o.astype(float)
                    pmtv = pmt.dict_add(pmtv, pmt.intern("out"+self.neurons[c]), pmt.from_double(o))
                    c=c+1
            """
            self.message_port_pub(pmt.intern("classification"),pmtv)  

        
        return len(input_items[0]) 

