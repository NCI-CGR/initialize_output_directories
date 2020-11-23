/*!
  \file tracking_files.cc
  \brief implementation for tracking files classes
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include "initialize_output_directories/tracking_files.h"

void initialize_output_directories::model_matrix::load_data(
    const std::string &filename) {
  std::ifstream input;
  std::string line = "", catcher = "";
  input.open(filename.c_str());
  if (!input.is_open())
    throw std::runtime_error("unable to open phenotype file \"" + filename +
                             "\"");
  getline(input, line);
  std::map<std::string, bool> targets;
  targets[get_phenotype()] = true;
  for (std::vector<std::string>::const_iterator iter = get_covariates().begin();
       iter != get_covariates().end(); ++iter) {
    targets[*iter] = true;
  }
  std::vector<bool> include_column;
  unsigned id_colnum = 0;
  std::istringstream strm1(line);
  while (strm1 >> catcher) {
    include_column.push_back(targets.find(catcher) != targets.end());
    if (!catcher.compare(get_id())) {
      id_colnum = include_column.size() - 1;
    }
  }
  _data.clear();
  _data.resize(1 + get_covariates().size());
  while (input.peek() != EOF) {
    getline(input, line);
    std::istringstream strm1(line);
    unsigned nfound = 0;
    for (unsigned i = 0; i < include_column.size(); ++i) {
      if (!(strm1 >> catcher))
        throw std::runtime_error(
            "header/line token count mismatch for line \"" + line + "\"");
      if (i == id_colnum) {
        _ids.push_back(catcher);
      }
      if (include_column.at(i)) {
        _data.at(nfound).push_back(catcher);
        ++nfound;
      }
    }
  }
  input.close();
}

void initialize_output_directories::tracking_files::initialize(
    const yaml_reader &config) {
  // pull the required presets from this extension configuration
  _phenotype_dataset_suffix = config.get_entry("phenotype-dataset");
  _phenotype_suffix = config.get_entry("phenotype");
  _covariates_suffix = config.get_entry("covariates");
  _categories_suffix = config.get_entry("categories");
  _finalized_suffix = config.get_entry("finalization");
  // get as many custom extensions as are available
  std::vector<std::pair<std::string, std::string> > data;
  data = config.get_map("general-extensions");
  for (std::vector<std::pair<std::string, std::string> >::const_iterator iter =
           data.begin();
       iter != data.end(); ++iter) {
    _general_extensions[iter->first] = iter->second;
  }
}
