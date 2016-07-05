/* -*- c++ -*- */

#define INSPECTOR_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "inspector_swig_doc.i"

%{
#include "inspector/signal_separator_c.h"
#include "inspector/signal_detector_cvf.h"
#include "inspector/signal_extractor_c.h"
#include "inspector/qtgui_inspector_sink_vf.h"
%}
%include "inspector/signal_separator_c.h"
GR_SWIG_BLOCK_MAGIC2(inspector, signal_separator_c);
%include "inspector/signal_detector_cvf.h"
GR_SWIG_BLOCK_MAGIC2(inspector, signal_detector_cvf);
%include "inspector/signal_extractor_c.h"
GR_SWIG_BLOCK_MAGIC2(inspector, signal_extractor_c);
%include "inspector/qtgui_inspector_sink_vf.h"
GR_SWIG_BLOCK_MAGIC2(inspector, qtgui_inspector_sink_vf);
