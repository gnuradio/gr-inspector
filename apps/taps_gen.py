#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# Copyright 2016 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
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

from gnuradio.filter import firdes
import json
import numpy

print "GR-INSPECTOR TAPS FILE GENERATOR"
print ""
print "This tool creates filter taps to use as precalculated taps in \
the signal separator block. It outputs a JSON file."
print "Windows:"
print "WIN_NONE = -1"
print "WIN_HAMMING = 0"
print "WIN_HANN = 1"
print "WIN_BLACKMAN = 2"
print "WIN_RECTANGULAR = 3"
print "WIN_KAISER = 4"
print "WIN_BLACKMAN_HARRIS = 5"
print "WIN_BARTLETT = 6"
print "WIN_FLATTOP = 7"
win = input("Window type: ")
trans = input("Rel. trans. width (trans_width/cutoff): ")
print "Input relative cutoffs for the following (fc/fs):"
start = input("Start cutoff: ")
stop = input("End cutoff: ")
step = input("Step: ")

cutoffs = numpy.arange(start, stop, step)

dict = {i : firdes.low_pass(1, 1, i, trans*i, win, 6.76) for i in cutoffs}

with open('taps.json', 'w') as outfile:
    json.dump(dict, outfile, sort_keys=True, indent=4, separators=(',', ': '))
print "Saved "+str(len(cutoffs))+" filters in taps.json"
