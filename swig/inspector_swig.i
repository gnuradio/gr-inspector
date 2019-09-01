/* -*- c++ -*- */

#define INSPECTOR_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "inspector_swig_doc.i"

%include "std_map.i"

namespace std {
  %template(map_float_vector) map<float, vector<float> >;
};

%{
#include "inspector/ofdm_synchronizer_cc.h"
#include "inspector/ofdm_bouzegzi_c.h"
#include "inspector/ofdm_zkf_c.h"
#include "inspector/qtgui_inspector_sink_vf.h"
#include "inspector/signal_detector_cvf.h"
#include "inspector/signal_extractor_c.h"
#include "inspector/signal_separator_c.h"
%}

%include "inspector/ofdm_synchronizer_cc.h"
GR_SWIG_BLOCK_MAGIC2(inspector, ofdm_synchronizer_cc);
%include "inspector/ofdm_bouzegzi_c.h"
GR_SWIG_BLOCK_MAGIC2(inspector, ofdm_bouzegzi_c);
%include "inspector/ofdm_zkf_c.h"
GR_SWIG_BLOCK_MAGIC2(inspector, ofdm_zkf_c);
%include "inspector/qtgui_inspector_sink_vf.h"
GR_SWIG_BLOCK_MAGIC2(inspector, qtgui_inspector_sink_vf);
%include "inspector/signal_detector_cvf.h"
GR_SWIG_BLOCK_MAGIC2(inspector, signal_detector_cvf);
%include "inspector/signal_extractor_c.h"
GR_SWIG_BLOCK_MAGIC2(inspector, signal_extractor_c);
%include "inspector/signal_separator_c.h"
GR_SWIG_BLOCK_MAGIC2(inspector, signal_separator_c);
