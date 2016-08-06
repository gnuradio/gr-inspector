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

## Modulation schemes
MOD = ["fsk", "qam16", "qam64", "2psk", "4psk", "8psk", "gmsk", "wbfm", "nfm"]

## GNU Radio block, for FAM classification
class FAM(gr.sync_block):

    # Create our block
    def __init__(self, dtype, vlen, graphfile):

        inputs = []
        inputs.append((np.dtype(dtype), vlen))

        gr.sync_block.__init__(self,
                               name="FAM",
                               in_sig=inputs,
                               out_sig=[])

        self.inputs = inputs
        sess, inp, out = self.load_graph(graphfile)

        self.sess = sess
        self.inp = inp
        self.out = out

        self.message_port_register_out(pmt.intern('classification'))

    ## Load graph from file through TensorFlow serving
    def load_graph(self, output_graph_path):

        sess, meta_graph_def = session_bundle.LoadSessionBundleFromPath(
            output_graph_path)

        with sess.as_default():

            collection_def = meta_graph_def.collection_def
            signatures_any = collection_def[
                constants.SIGNATURES_KEY].any_list.value
            signatures = manifest_pb2.Signatures()
            signatures_any[0].Unpack(signatures)
            default_signature = signatures.default_signature

            input_name = default_signature.classification_signature.input.tensor_name
            output_name = default_signature.classification_signature.scores.tensor_name

            return (sess, input_name, output_name)

    ## Work function to accept input FAM data, to reshape and pass to model
    def work(self, input_items, output_items):

        tensordata = []
        input_i = []
        shapev = np.array(input_items[0]).shape
        inp = np.array(input_items[0][0])

        if np.mean(inp) == 0.0:
            return len(input_items[0])

        ## Normalise data
        inp = (inp - np.mean(inp)) / np.std(inp)

        ## Reshape to 2D
        floats = np.reshape(inp, (2 * P * L, (2 * Np) - 0))

        tensordata.append(np.array([floats]))

        ne = []
        for v in tensordata:
            try:
                outp = self.sess.run(self.out, feed_dict={self.inp: [v]})[0]
                ne.append(outp)
            except tf.errors.InvalidArgumentError:
                print("Invalid size of input vector to TensorFlow model")
                quit()

        pmtv = pmt.make_dict()
        for outp in ne:
            pmtv = pmt.dict_add(pmtv, pmt.intern(
                "Mod"), pmt.to_pmt(MOD[np.argmax(outp)]))
            pmtv = pmt.dict_add(pmtv, pmt.intern("Prob"), pmt.to_pmt(outp))

            self.message_port_pub(pmt.intern("classification"), pmtv)

        return len(input_items[0])
