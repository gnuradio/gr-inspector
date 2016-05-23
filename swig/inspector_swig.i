/* -*- c++ -*- */

#define INSPECTOR_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "inspector_swig_doc.i"

%{
%}
GR_SWIG_BLOCK_MAGIC2(inspector, inspector_gui_cc);
