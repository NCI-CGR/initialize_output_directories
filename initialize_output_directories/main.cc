/*!
  \file main.cc
  \brief main entry point for software
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include <chrono>  // NOLINT [build/c++11]
#include <iostream>
#include <stdexcept>

#include "boost/filesystem.hpp"
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
  bool timer = ap.timer();

  std::chrono::time_point<std::chrono::high_resolution_clock> start_time,
      end_time;
  if (timer) {
    start_time = std::chrono::high_resolution_clock::now();
  }

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

  initialize_output_directories::model_matrix mm;
  mm.set_id(phenotype_id_colname);
  mm.set_phenotype(pheno_config.get_entry("phenotype"));
  if (pheno_config.query_valid("covariates")) {
    mm.set_covariates(pheno_config.get_sequence("covariates"));
  }
  // if one of the algorithms is what was requested on the command line
  initialize_output_directories::categorical_variable categories;
  if (initialize_output_directories::find_entry("saige", algorithms)) {
    // compute groups and sizes, combining any group with N<100 into
    //    a single meta-group
    mm.load_data(phenotype_database);
    categories = mm.categorize(pheno_config.get_entry("phenotype"));
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
      boost::filesystem::path bgen_dir_path = bgen_directory;
      boost::filesystem::path bgen_sample_path = bgen_samplefile;
      if (initialize_output_directories::find_entry(software, algorithms) &&
          boost::filesystem::is_directory(bgen_dir_path) &&
          boost::filesystem::is_regular_file(bgen_sample_path)) {
        // compute number of subjects in this sample file
        // deduct 2 because of .sample file header conventions
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
          // make the tracker class determine if updates are needed
          bool updated = tf.check_files(pheno_config, mm, phenotype_database,
                                        pretend, force);
          // for categoricals (n comparisons > 1)
          //    copy top-level trackers into "comparison[1-n]" subdirectories
          if (updated) {
            tf.remove_finalization();
            // if there are more than two categories
            if (categories.size() > 2) {
              unsigned comparison_count = 1;
              for (std::vector<std::set<unsigned> >::const_iterator iter =
                       categories.comparison_begin();
                   iter != categories.comparison_end();
                   ++iter, ++comparison_count) {
                // at some point, this will have to be moved outside of this
                // conditional, as the assignment of reference and comparison
                // groups will be exposed as a configuration variable. but for
                // now, comparison groups are determined by phenotype database
                // counts, which is deterministic pending things that guarantee
                // `updated == true`
                tf.copy_trackers(comparison_count,
                                 categories.get_reference_group(), *iter);
              }
            } else if (categories.size() == 2) {
              tf.report_categories(tf.get_output_prefix(),
                                   categories.get_reference_group(),
                                   *categories.comparison_begin());
            }
          }
          // actually emit output prefixes as appropriate
          if (categories.size() > 2) {
            // for categorical data, suppress the top level directory
            // as an analysis target, and just emit the comparison
            // subdirectories
            for (unsigned i = 1; i <= categories.n_comparison_groups(); ++i) {
              std::cout << results_prefix.substr(0, results_prefix.rfind("/"))
                        << "/comparison" << i
                        << results_prefix.substr(results_prefix.rfind("/"))
                        << std::endl;
            }
          } else {
            std::cout << results_prefix << std::endl;
          }
        }
      }
    }
  }
  if (timer) {
    end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                              start_time);
    std::cout << "Time taken by run: " << elapsed.count() << " milliseconds"
              << std::endl;
  }
  return 0;
}
