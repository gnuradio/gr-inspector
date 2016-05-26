#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2016 GNU Radio.
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
from scipy import signal
import pmt

class signal_detector_cc(gr.sync_block):
    """
    block to detect signals in input spectrum. Uses enegry detection
    with manual or auto threshold. Passes the PDF as complex output
    and the signal edges as messages.
    """
    def __init__(self, samp_rate, fft_len=1024, window='hamming',
                 threshold=0.7, sensitivity=0.2, auto=True):
        gr.sync_block.__init__(self,
            name="signal_detector_cc",
            in_sig=[numpy.complex64],
            out_sig=[numpy.complex64])
        # store parameters in local variables
        self.samp_rate = samp_rate
        self.fft_len = fft_len
        self.window = window
        self.threshold = threshold
        # register message port
        self.message_port_register_out(pmt.intern('map_out'))
        self.auto_threshold = auto
        self.sensitivity = sensitivity
        self.signal_edges = numpy.empty([0,2])


    def find_signal_edges(self, pos):
        """
        find edges of adjacent frequency bin positions
        """
        # if no bins detected, return empty array
        if(len(pos)) == 0:
            return numpy.empty([0,2])
        # else try to group bins
        i = 1
        # first pos is always signal bound
        curr_signal = numpy.array(pos[0])
        # -> no search for new signal right now
        new_signal = False
        # create empty result array
        flanks = numpy.empty([0, 2])
        # if only one bin detected, use this as both flanks
        if len(pos) == 1:
            flanks = numpy.array([[pos[0], pos[0]]])
        # if only two bins detected
        elif len(pos) == 2:
            if pos[0] + 1 == pos[1]:
                # one signal with two adjacent bins
                flanks = numpy.array([[pos[0], pos[1]]])
            else:
                # two signals with only one bin
                flanks = numpy.array([[pos[0], pos[0]],
                                      [pos[1], pos[1]]])
        # else: find signal flanks >2 dynamically
        else:
            while i < len(pos):
                # if array end is reached use last element for
                # signal flank
                if new_signal:
                    curr_signal = numpy.array([pos[i]])
                    new_signal = False
                    # if end of position, repeat last edge
                    if i == len(pos) - 1:
                        curr_signal = numpy.append(curr_signal, pos[i])
                        flanks = numpy.vstack((flanks, curr_signal))
                    i += 1
                else:
                    if i < len(pos)-1:
                        if pos[i+1] <> pos[i] + 1:
                            curr_signal = numpy.append(curr_signal,
                                                   pos[i])
                            flanks = numpy.vstack((flanks, curr_signal))
                            new_signal = True
                    else:
                        curr_signal = numpy.append(curr_signal,
                                                   pos[i])
                        flanks = numpy.vstack((flanks, curr_signal))
                    i += 1

        return flanks.astype(int)

    def pack_message(self):
        """
        generate message compatible pmt out of array
        result is pmt vector of pmt vectors with float values in it
        """
        signal_cnt = self.signal_edges.shape[0]
        msg = pmt.make_vector(signal_cnt, pmt.PMT_NIL)
        for i in range(signal_cnt):
            curr_edge = pmt.make_f32vector(2, 0.0)
            pmt.f32vector_set(curr_edge, 0, self.signal_edges[i,0])
            pmt.f32vector_set(curr_edge, 1, self.signal_edges[i,1])
            pmt.vector_set(msg, i, curr_edge)

        return msg

    def set_threshold(self, Pxx):
        """
        Set automatic threshold. Find lowest frequency bins that have
        similar absolute value and set threshold to largest
        """
        pos = len(Pxx)-1
        Pxx = numpy.sort(abs(Pxx))
        for i in range(len(Pxx)-2):
            if abs(Pxx[i+1]) - abs(Pxx[i]) > 1-self.sensitivity:
                pos = i
                break

        self.threshold = max(Pxx[0:pos])

    def compare_signal_edges(self, edges):
        """
        check if rf map changed (within differences of 1 Hz)
        """
        change = False
        if edges.shape[0] == len(self.signal_edges):
            #print "same length"
            for i in range(edges.shape[0]):
                #print "look at sig "+str(i)
                if abs(edges[i][0] - self.signal_edges[i][0]) > 1:
                    change = True
                    #print "starts changed"
                if abs(edges[i][1] - self.signal_edges[i][1]) > 1:
                    change = True
                    #print "stops changed"
        else:
            return True
        return change


    def work(self, input_items, output_items):
        in0 = input_items[0]
        out = output_items[0]
        # psd estimation
        freq, Pxx = signal.periodogram(in0, self.samp_rate,
                                       self.window, self.fft_len)
        # bartlett estimation:
        # freq, Pxx = signal.welch(in0, fself.samp_rate, self.window,
        #                         256, 0, self.fft_len)
        # nomize psd (TODO: remove this)
        Pxx = Pxx/max(abs(Pxx))
        # use auto threshold finding
        if(self.auto_threshold):
            self.set_threshold(Pxx)
        # find bins over threshold
        pos = numpy.where(Pxx > self.threshold)[0]
        # group bins to signal edges
        pos = self.find_signal_edges(pos)
        # get frequency values

        # only send message if something changed
        if(self.compare_signal_edges(freq[pos])):
            self.signal_edges = freq[pos]
            #TODO: create message here
            self.message_port_pub(pmt.intern('map_out'),
                            self.pack_message())
        # debug values
        #print "\nDebug Info: \n"
        #print "input len: " + str(len(in0))
        # print "fft len: " + str(self.fft_len)
        # print "samp rate: " + str(self.samp_rate)
        # print "freq_vec len:" + str(len(freq))
        # print "freq_max: " + str(max(freq))
        # print "Threshold: " + str(self.threshold)
        # print ""
        #print self.compare_signal_edges(freq[pos])
        #print freq[pos]


        # output psd
        # out[:] = abs(Pxx)
        # output unaltered signal
        out[:] = in0
        return len(output_items[0])

