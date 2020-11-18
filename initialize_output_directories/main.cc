/*!
  \file main.cc
  \brief main entry point for software
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include <iostream>
#include <stdexcept>

#include "initialize_output_directories/cargs.h"
#include "initialize_output_directories/yaml_reader.h"

int main(int argc, char **argv) {
  // parse command line input
  initialize_output_directories::cargs ap(argc, argv);
  // if help is requested or no flags specified
  if (ap.help() || argc == 1) {
    // print a help message and exist
    ap.print_help(std::cout);
    return 0;
  }

  std::cout << "all done" << std::endl;
  return 0;
}
