/*
 * Copyright 2022 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(ofdm_bouzegzi_c.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(facb676dd012e68cd2b28cd0b5d57468)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/inspector/ofdm_bouzegzi_c.h>
// pydoc.h is automatically generated in the build directory
#include <ofdm_bouzegzi_c_pydoc.h>

void bind_ofdm_bouzegzi_c(py::module& m)
{

    using ofdm_bouzegzi_c    = ::gr::inspector::ofdm_bouzegzi_c;


    py::class_<ofdm_bouzegzi_c, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<ofdm_bouzegzi_c>>(m, "ofdm_bouzegzi_c", D(ofdm_bouzegzi_c))

        .def(py::init(&ofdm_bouzegzi_c::make),
           py::arg("samp_rate"),
           py::arg("Nb"),
           py::arg("alpha"),
           py::arg("beta"),
           D(ofdm_bouzegzi_c,make)
        )
        



        ;




}








