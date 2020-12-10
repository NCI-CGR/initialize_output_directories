/*!
  \file tracking_files.h
  \brief class declaration for tracking file handler
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */
#ifndef INITIALIZE_OUTPUT_DIRECTORIES_TRACKING_FILES_H_
#define INITIALIZE_OUTPUT_DIRECTORIES_TRACKING_FILES_H_

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/iostreams/device/mapped_file.hpp"
#include "initialize_output_directories/yaml_reader.h"
#include "yaml-cpp/yaml.h"

namespace initialize_output_directories {
class categorical_variable {
 public:
  categorical_variable() {}
  categorical_variable(const categorical_variable &obj)
      : _reference_group(obj._reference_group),
        _comparison_groups(obj._comparison_groups) {}
  ~categorical_variable() throw() {}

  void set_reference_level(unsigned u) {
    _reference_group.clear();
    _reference_group.emplace(u);
  }
  void set_reference_level(const std::set<unsigned> &s) {
    _reference_group = s;
  }
  void add_comparison_group(unsigned u) {
    std::set<unsigned> s;
    s.emplace(u);
    add_comparison_group(s);
  }
  void add_comparison_group(const std::set<unsigned> &s) {
    _comparison_groups.push_back(s);
  }

  const std::set<unsigned> &get_reference_group() const {
    return _reference_group;
  }
  const std::set<unsigned> &get_comparison_group(unsigned index) const {
    if (index >= _comparison_groups.size())
      throw std::runtime_error("comparison index " + std::to_string(index) +
                               " out of bounds (total comparison set count " +
                               std::to_string(n_comparison_groups()) + ")");
    return _comparison_groups.at(index);
  }
  std::vector<std::set<unsigned> >::const_iterator comparison_begin() const {
    return _comparison_groups.begin();
  }
  std::vector<std::set<unsigned> >::const_iterator comparison_end() const {
    return _comparison_groups.end();
  }
  unsigned n_comparison_groups() const { return _comparison_groups.size(); }
  unsigned size() const { return n_comparison_groups() + 1; }

 private:
  std::set<unsigned> _reference_group;
  std::vector<std::set<unsigned> > _comparison_groups;
};

class model_matrix {
 public:
  model_matrix() : _id(""), _phenotype("") {}
  model_matrix(const model_matrix &obj)
      : _id(obj._id),
        _phenotype(obj._phenotype),
        _covariates(obj._covariates),
        _ids(obj._ids),
        _headers(obj._headers),
        _data(obj._data) {}
  ~model_matrix() throw() {}

  void set_id(const std::string &id) { _id = id; }

  const std::string &get_id() const { return _id; }

  void set_phenotype(const std::string &phenotype) { _phenotype = phenotype; }

  const std::string &get_phenotype() const { return _phenotype; }

  void set_covariates(const std::vector<std::string> &vec) {
    _covariates = vec;
  }

  void set_covariate(const std::string &s) {
    _covariates.clear();
    _covariates.push_back(s);
  }

  const std::vector<std::string> &get_covariates() const { return _covariates; }

  void load_data(const std::string &filename);

  const std::vector<std::string> get_ids() const { return _ids; }

  const std::vector<std::vector<std::string> > &get_data() const {
    return _data;
  }

  categorical_variable categorize(const std::string &name) const;

  bool operator==(const model_matrix &obj) const {
    if (get_ids().size() != obj.get_ids().size()) return false;
    if (get_data().size() != obj.get_data().size()) return false;
    for (unsigned i = 0; i < get_data().size(); ++i) {
      if (get_data().at(i).size() != obj.get_data().size()) return false;
      for (unsigned j = 0; j < get_data().at(i).size(); ++j) {
        if (get_data().at(i).at(j).compare(obj.get_data().at(i).at(j)))
          return false;
      }
    }
    return true;
  }

  bool operator!=(const model_matrix &obj) const { return !(*this == obj); }

  bool empty() const {
    return _headers.empty() && _ids.empty() && _data.empty();
  }

  void write(const std::string &filename) const;

 private:
  std::string _id;
  std::string _phenotype;
  std::vector<std::string> _covariates;
  std::vector<std::string> _ids;
  std::vector<std::string> _headers;
  std::vector<std::vector<std::string> > _data;
};

class extension_definition {
 public:
  extension_definition() : _name(""), _extension(""), _default("") {}
  extension_definition(const extension_definition &obj)
      : _name(obj._name),
        _extension(obj._extension),
        _default(obj._default),
        _permitted_values(obj._permitted_values) {}
  ~extension_definition() throw() {}

  void set_name(const std::string &s) { _name = s; }
  const std::string &get_name() const { return _name; }
  void set_extension(const std::string &s) { _extension = s; }
  const std::string &get_extension() const { return _extension; }
  void set_default(const std::string &s) { _default = s; }
  const std::string &get_default() const { return _default; }
  void add_permitted_value(const std::string &s) {
    _permitted_values[s] = true;
  }
  bool is_permitted_value(const std::string &s) const {
    return _permitted_values.find(s) != _permitted_values.end() ||
           _permitted_values.empty();
  }

 private:
  std::string _name;
  std::string _extension;
  std::string _default;
  std::map<std::string, bool> _permitted_values;
};

class tracking_files {
 public:
  tracking_files()
      : _output_prefix(""),
        _phenotype_dataset_suffix(""),
        _phenotype_suffix(""),
        _covariates_suffix(""),
        _categories_suffix(""),
        _finalized_suffix("") {}
  tracking_files(const std::string &s, const yaml_reader &config)
      : _output_prefix(s),
        _phenotype_dataset_suffix(""),
        _phenotype_suffix(""),
        _covariates_suffix(""),
        _categories_suffix(""),
        _finalized_suffix("") {
    initialize(config);
  }
  tracking_files(const tracking_files &obj)
      : _output_prefix(obj._output_prefix),
        _phenotype_dataset_suffix(obj._phenotype_dataset_suffix),
        _phenotype_suffix(obj._phenotype_suffix),
        _covariates_suffix(obj._covariates_suffix),
        _categories_suffix(obj._categories_suffix),
        _finalized_suffix(obj._finalized_suffix),
        _general_extensions(obj._general_extensions) {}
  ~tracking_files() throw() {}

  void initialize(const yaml_reader &config);
  bool check_phenotype_database(const yaml_reader &config,
                                const model_matrix &input_model,
                                const std::string &phenotype_filename,
                                bool pretend, bool force) const;
  bool check_files(const yaml_reader &config, const model_matrix &input_model,
                   const std::string &phenotype_filename, bool pretend,
                   bool force) const;
  bool check_file(const yaml_reader &config, const extension_definition &edef,
                  bool pretend, bool force, bool must_exist) const;
  void remove_finalization() const;
  void copy_trackers(unsigned comparison_number,
                     const std::set<unsigned> &reference,
                     const std::set<unsigned> &comparison) const;

 protected:
  const std::string &get_phenotype_dataset_suffix() const {
    return _phenotype_dataset_suffix;
  }
  const std::string &get_phenotype_suffix() const { return _phenotype_suffix; }
  const std::string &get_covariates_suffix() const {
    return _covariates_suffix;
  }
  const std::string &get_categories_suffix() const {
    return _categories_suffix;
  }
  const std::string &get_output_prefix() const { return _output_prefix; }
  const std::string &get_finalized_suffix() const { return _finalized_suffix; }
  void update_tracker(const std::string &filename,
                      const std::vector<std::string> &vec, bool append) const;

 private:
  std::string _output_prefix;
  std::string _phenotype_dataset_suffix;
  std::string _phenotype_suffix;
  std::string _covariates_suffix;
  std::string _categories_suffix;
  std::string _finalized_suffix;
  std::map<std::string, extension_definition> _general_extensions;
};
}  // namespace initialize_output_directories

#endif  // INITIALIZE_OUTPUT_DIRECTORIES_TRACKING_FILES_H_
