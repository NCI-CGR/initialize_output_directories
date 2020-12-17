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
    \brief determine whether the run is in pretend mode
    \return whether the run is in pretend mode

    Pretend mode disables tracker file updating, and just emits analysis
    prefixes for make dependency resolution. This is designed to be compatible
    with `make -n`.
   */
  bool pretend() const { return compute_flag("pretend"); }

  /*!
    \brief determine whether the run is in force mode
    \return whether the run is in force mode

    Force mode overrides standard contextual updates to tracker files
    and forces the updates regardless. Pretend mode takes precedence,
    so `--force --pretend` will not change tracking files, but is considered
    a valid run configuration for compatibility with `make -nB`.
   */
  bool force() const { return compute_flag("force"); }

  /*!
    \brief determine whether the run should be timed
    \return whether the run should be timed

    Almost all of this program is super fast, but one step, processing
    the phenotype database, can be comparatively slow. to ease the
    comparison of different implementations, this flag will turn on
    an internal timer that will report the elapsed runtime.
   */
  bool timer() const { return compute_flag("timer"); }

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
    \brief get the user-specified phenotype database filename
    \return the user-specified phenotype database filename

    The phenotype database is dynamically parsed out to determine
    if changes in the phenotype database used for an association test
    have any impact on the particular analysis being run. If the changes,
    in the phenotype file do not impact any of the variables in the
    model matrix, then the analysis trackers will not be updated
    unless there are other changes to the configuration specification
    that do impact the run output.
   */
  std::string get_phenotype_database() const {
    return compute_parameter<std::string>("phenotype-database");
  }

  /*!
    \brief get the user-specified name of the subject ID column
    in the phenotype dataset
    \return the user-specified name of the subject ID column
    in the phenotype dataset

    This is required in case there are sorting changes in the phenotype
    database that do not actually impact the model matrix. For the PLCO
    project, this value should be "plco_id".
   */
  std::string get_phenotype_id_colname() const {
    return compute_parameter<std::string>("phenotype-id-colname");
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
    \brief get the user-specified prefix to all bgen imputation files
    \return the user-specified prefix to all bgen imputation files

    This is expected to be the top level directory containing the bgen
    pipeline from the plco-analysis project. Subdirectories in the
    format "platform/ancestry" should contain bgen and sample files,
    cleaned (NAs removed if using plink) and indexed.
   */
  std::string get_bgen_prefix() const {
    return compute_parameter<std::string>("bgen-dir");
  }

  /*!
    \brief get the user-specified software in use
    \return the user-specified software in use

    The preprocessing step of the analysis pipelines is where
    the pipeline figures out if a config file is relevant to the pipeline
    in use. So the user specifies "saige" for example, and then this
    software determines whether "saige" is present in the corresponding
    phenotype yaml.

    Supported options: saige, boltlmm
   */
  std::string get_software() const {
    return compute_parameter<std::string>("software");
  }

  /*!
    \brief get minimum sample size for the requested software
    \return the minimum sample size for the requested software

    Each of the analysis tools supported by the pipeline has a different
    heuristic minimum sample size requirement. This lets the user
    specify that as needed.
   */
  unsigned get_software_min_sample_size() const {
    return compute_parameter<unsigned>("software-min-sample-size");
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
