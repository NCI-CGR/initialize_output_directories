/*!
  \file cargs.cc
  \brief method implementation for command line argument parser class
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
*/

#include "initialize_output_directories/cargs.h"

void initialize_output_directories::cargs::initialize_options() {
  _desc.add_options()("help,h", "emit this help message");
}
