/* -*- c++ -*- */
/*
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <iostream>
#include <fstream>
#include "tap_parser.h"

namespace gr {
  namespace inspector {

    tap_parser::tap_parser(std::string filepath) {
      d_filepath = filepath;
      json = new jsonxx::Object();

      try {
        std::ifstream file(filepath);
        filebuffer << file.rdbuf();
      }
      catch(int e) {
        std::cout << "Taps JSON file not found!" << std::endl;
        return;
      }
      //std::cout << filebuffer.str() << std::endl;
      assert(json->parse(filebuffer.str()));
      assert(json->has<jsonxx::Array>("0.01"));
    }

    tap_parser::~tap_parser() {
      delete json;
    }

  }
}