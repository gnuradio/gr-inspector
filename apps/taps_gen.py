from gnuradio.filter import firdes
import numpy

print "gr-inspector taps file generator"
print ""
trans = input("Rel. trans. width ")
print "Input relative cutoffs here (fc/fs):"
start = input("Start cutoff ")
stop = input("End cutoff ")
step = input("Step ")

cutoffs = numpy.arange(start, stop, step)

for i in cutoffs:
    firdes.low_pass(1, i, trans*i, )
