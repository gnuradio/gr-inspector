/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include <map>
#include <vector>
namespace py = pybind11;

// Headers for binding functions
/**************************************/
// The following comment block is used for
// gr_modtool to insert function prototypes
// Please do not delete
/**************************************/
// BINDING_FUNCTION_PROTOTYPES(
void bind_ofdm_bouzegzi_c(py::module& m);
void bind_ofdm_synchronizer_cc(py::module& m);
void bind_ofdm_zkf_c(py::module& m);
void bind_qtgui_inspector_sink_vf(py::module& m);
void bind_signal_detector_cvf(py::module& m);
void bind_signal_extractor_c(py::module& m);
void bind_signal_separator_c(py::module& m);
// ) END BINDING_FUNCTION_PROTOTYPES


// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}
namespace std {
struct map_float_vector{
    map_float_vector(std::map<int, std::vector<int>> &vect) : vect(vect){}
    std::map<int, std::vector<int>> vect;
};
}

PYBIND11_MODULE(inspector_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Allow access to base block methods
    py::module::import("gnuradio.gr");
    py::module::import("gnuradio.qtgui.qtgui_python");


    /**************************************/
    // The following comment block is used for
    // gr_modtool to insert binding function calls
    // Please do not delete
    /**************************************/
    // BINDING_FUNCTION_CALLS(
    bind_ofdm_bouzegzi_c(m);
    bind_ofdm_synchronizer_cc(m);
    bind_ofdm_zkf_c(m);
    bind_qtgui_inspector_sink_vf(m);
    bind_signal_detector_cvf(m);
    bind_signal_extractor_c(m);
    bind_signal_separator_c(m);

    py::class_<std::map_float_vector>(m, "map_float_vector")
        .def(py::init<std::map<int, std::vector<int>>&>());
    // ) END BINDING_FUNCTION_CALLS
}
