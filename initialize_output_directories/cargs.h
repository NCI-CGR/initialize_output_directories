/*!
 \file cargs.h
 \brief command line argument handling
 \author Cameron Palmer
 \copyright Released under the MIT License.
 Copyright 2020 Cameron Palmer

 Thanks to
 https://www.boost.org/doc/libs/1_70_0/doc/html/program_options/tutorial.html#id-1.3.32.4.3
 */

#ifndef INITIALIZE_OUTPUT_DIRECTORIES_CARGS_H_
#define INITIALIZE_OUTPUT_DIRECTORIES_CARGS_H_

#include <stdexcept>
#include <string>

#include "boost/program_options.hpp"

namespace initialize_output_directories {
/*!
  \class cargs
  \brief command line argument parser using boost::program_options
 */
class cargs {
 public:
  /*!
    \brief constructor with program arguments
    @param argc number of arguments including program name
    @param argv string array containing actual arguments
   */
  cargs(int argc, char **argv) : _desc("Recognized options") {
    initialize_options();
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, _desc), _vm);
    boost::program_options::notify(_vm);
  }
  /*!
    \brief copy constructor
    @param obj existing cargs object
   */
  cargs(const cargs &obj) : _desc(obj._desc), _vm(obj._vm) {}
  /*!
    \brief destructor
   */
  ~cargs() throw() {}

  /*!
    \brief set user help documentation and default values for parameters as
    needed

    Note the weird syntax with overloaded () operators. The lists are not
    separated by commas.
   */
  void initialize_options();

  /*!
    \brief determine whether the user has requested help documentation
    \return whether the user has requested help documentation

    This test is separate from whether the user has run the software with no
    flags
   */
  bool help() const { return compute_flag("help"); }

  /*!
    \brief get the user-specified phenotype configuration file
    \return the user-specified phenotype configuration file

    This file should be in yaml format and specify each of the
    phenotype run configuration options required for the analysis.
    Yaml format is checked by yaml-cpp but any config file extension
    is allowed.
   */
  std::string get_phenotype_config() const {
    return compute_parameter<std::string>("phenotype-config");
  }

  /*!
    \brief get the user-specified extension configuration file
    \return the user-specified extension configuration file

    This file should be in yaml format and specify each of the
    file extensions used for config parameter tracking in the results
    directories and downstream workflows. Yaml format is checked by
    yaml-cpp but any config file extension is allowed.
   */
  std::string get_extension_config() const {
    return compute_parameter<std::string>("extension-config");
  }

  /*!
    \brief get the user-specified top level results directory
    \return the user-specified top level results directory
   */
  std::string get_results_dir() const {
    return compute_parameter<std::string>("results-dir");
  }

  /*!
    \brief find status of arbitrary flag
    @param tag name of flag
    \return whether the flag is set

    This is the underlying accessor function used by the custom flag accessors,
    and can be used for arbitrary flag additions if you don't want to type out
    the custom access functions or want to allow dynamic specification from a
    config file.
   */
  bool compute_flag(const std::string &tag) const { return _vm.count(tag); }

  /*!
    \brief find value of arbitrary parameter
    @tparam value_type class to which the value should be cast
    @param tag name of parameter
    \return value of parameter if specified

    \warning throws exception if parameter was not set and had no default
   */
  template <class value_type>
  value_type compute_parameter(const std::string &tag) const {
    if (_vm.count(tag)) {
      return _vm[tag].as<value_type>();
    }
    throw std::domain_error("cargs: requested parameter \"" + tag + "\" unset");
  }

  /*!
    \brief report help documentation to arbitrary output stream
    @param out stream to which to write help documentation

    Parameter should probably be std::cout or std::cerr at your preference.
   */
  void print_help(std::ostream &out) {
    if (!(out << _desc))
      throw std::domain_error("cargs::print_help: unable to write to stream");
  }

 private:
  /*!
    \brief default constructor
    \warning disabled
   */
  cargs() { throw std::domain_error("cargs: do not use default constructor"); }
  boost::program_options::options_description
      _desc;  //!< help documentation string
  boost::program_options::variables_map
      _vm;  //!< storage of parsed command line settings
};
}  // namespace initialize_output_directories

#endif  // INITIALIZE_OUTPUT_DIRECTORIES_CARGS_H_
