/*!
  \file cargs.cc
  \brief method implementation for command line argument parser class
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
*/

#include "initialize_output_directories/cargs.h"

void initialize_output_directories::cargs::initialize_options() {
  _desc.add_options()("help,h", "emit this help message")(
      "debug,d", "run in debug mode with more verbose output")(
      "extension-config,e", boost::program_options::value<std::string>(),
      "file extension configuration file, yaml format")(
      "force,B", "force updates to all tracking files unless in pretend mode")(
      "phenotype-config,p", boost::program_options::value<std::string>(),
      "phenotype configuration file, yaml format")(
      "phenotype-database,D", boost::program_options::value<std::string>(),
      "name of current phenotype dataset in use")(
      "phenotype-id-colname,I", boost::program_options::value<std::string>(),
      "column header for subject IDs in phenotype dataset")(
      "pretend,n",
      "emit analysis target directories but do not write any changes to disk")(
      "bgen-dir,b", boost::program_options::value<std::string>(),
      "top level directory containing imputed bgen files")(
      "results-dir,r", boost::program_options::value<std::string>(),
      "top level directory containing analysis results")(
      "software,s", boost::program_options::value<std::string>(),
      "requested software (e.g. SAIGE, BOLTLMM)")(
      "software-min-sample-size,N", boost::program_options::value<unsigned>(),
      "minimum heuristic sample size for software")(
      "timer,t", "emit elapsed runtime at end of program execution");
}
