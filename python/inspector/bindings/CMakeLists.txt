# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT inspector_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT inspector_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND inspector_python_files
        ofdm_bouzegzi_c_python.cc
        ofdm_synchronizer_cc_python.cc
        ofdm_zkf_c_python.cc
        qtgui_inspector_sink_vf_python.cc
        signal_detector_cvf_python.cc
        signal_extractor_c_python.cc
        signal_separator_c_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(inspector
   ../../..
   gr::inspector
   "${inspector_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(TARGET inspector_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:inspector_python>
    ${CMAKE_BINARY_DIR}/test_modules/gnuradio/inspector/
)

install(TARGETS inspector_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/inspector COMPONENT pythonapi)
