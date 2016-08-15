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