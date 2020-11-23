/*!
  \file main.cc
  \brief main entry point for software
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "initialize_output_directories/cargs.h"
#include "initialize_output_directories/tracking_files.h"
#include "initialize_output_directories/utilities.h"
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
  std::string phenotype_config_filename = ap.get_phenotype_config();
  std::string extension_config_filename = ap.get_extension_config();
  std::string bgen_prefix = ap.get_bgen_prefix();
  std::string phenotype_database = ap.get_phenotype_database();
  std::string phenotype_id_colname = ap.get_phenotype_id_colname();
  std::string results_dir = ap.get_results_dir();
  std::string software = ap.get_software();
  unsigned software_min_sample_size = ap.get_software_min_sample_size();
  bool pretend = ap.pretend();
  bool force = ap.force();
  bool debug = ap.debug();
  // read configuration files
  initialize_output_directories::yaml_reader pheno_config(
      phenotype_config_filename);
  initialize_output_directories::yaml_reader extension_config(
      extension_config_filename);

  // read required entries from phenotype configuration
  std::string analysis_prefix = pheno_config.get_entry("analysis_prefix");
  std::vector<std::string> chips = pheno_config.get_sequence("chips");
  std::vector<std::string> ancestries = pheno_config.get_sequence("ancestries");
  std::vector<std::string> algorithms = pheno_config.get_sequence("algorithm");
  std::string phenotype = pheno_config.get_entry("phenotype");

  // if one of the algorithms is what was requested on the command line
  if (initialize_output_directories::find_entry("saige", algorithms)) {
    // compute groups and sizes, combining any group with N<100 into
    //    a single meta-group
  }
  // for each chip
  for (std::vector<std::string>::const_iterator chip = chips.begin();
       chip != chips.end(); ++chip) {
    // for each ancestry
    for (std::vector<std::string>::const_iterator ancestry = ancestries.begin();
         ancestry != ancestries.end(); ++ancestry) {
      // if the requested software is specified in the pheno config
      //    and the bgen directory for this chip/ancestry combination exists
      //    and the "chr22-filtered-noNAs.sample" file exists in that directory
      std::string bgen_directory =
          bgen_prefix + "/" +
          initialize_output_directories::strreplace(*chip, '_', '/') + "/" +
          *ancestry;
      std::string bgen_samplefile =
          bgen_directory + "/chr22-filtered-noNAs.sample";
      if (initialize_output_directories::find_entry(software, algorithms) &&
          std::filesystem::is_directory(
              std::filesystem::path(bgen_directory)) &&
          std::filesystem::is_regular_file(
              std::filesystem::path(bgen_samplefile))) {
        // compute number of subjects in this sample file
        unsigned n_subjects =
            initialize_output_directories::wc(bgen_samplefile) - 2;
        // if there are enough subjects in this sample file to run this
        // particular software
        if (n_subjects >= software_min_sample_size) {
          // build the results directory name:
          // {results/phenotype/ancestry/SOFTWARE}
          std::string results_prefix =
              results_dir + "/" + analysis_prefix + "/" + *ancestry + "/" +
              initialize_output_directories::uppercase(software) + "/" +
              analysis_prefix + "." + *chip + "." +
              initialize_output_directories::lowercase(software);
          // presumably build a tracker class and initialize an instance of it
          //   and make the directory if needed
          initialize_output_directories::tracking_files tf(results_prefix,
                                                           extension_config);

          // make the tracker class determine if updates are needed for:
          // phenotype database used and history
          // phenotype variable
          // covariates
          // frequency type
          // ID format
          // phenotype transformation
          // sex-specific analysis
          // control inclusions
          // control exclusions
          // finalized analysis indicator

          // finally, trigger update if needed. for categoricals (n comparisons
          // > 1)
          //    emit results in "comparison[1-n]" subdirectories only;
          //    otherwise, emit directly to the top-level directory.
        }
      }
    }
  }

  // once all of this is done, echo analysis prefixes for all valid targets;
  // basically,
  //    the directories that would be written to.

  std::cout << "all done" << std::endl;
  return 0;
}
