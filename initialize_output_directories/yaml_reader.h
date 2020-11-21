/*!
  \file yaml_reader.h
  \brief interface for yaml config files
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#ifndef INITIALIZE_OUTPUT_DIRECTORIES_YAML_READER_H_
#define INITIALIZE_OUTPUT_DIRECTORIES_YAML_READER_H_

#include <string>
#include <utility>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace initialize_output_directories {
class yaml_reader {
 public:
  yaml_reader() {}
  explicit yaml_reader(const std::string &filename) { load_file(filename); }
  ~yaml_reader() throw() {}

  void load_file(const std::string &filename);
  std::string get_entry(const std::string &query) const {
    std::vector<std::string> queries;
    queries.push_back(query);
    return get_entry(queries);
  }
  std::string get_entry(const std::vector<std::string> &queries) const {
    std::vector<std::string> all_results;
    all_results = get_sequence(queries);
    if (all_results.size() != 1) {
      throw std::runtime_error(
          "invalid number of results for entry query "
          "ending in \"" +
          *queries.rbegin() + "\": found " +
          std::to_string(all_results.size()));
    }
    return *all_results.begin();
  }
  std::vector<std::string> get_sequence(const std::string &query) const {
    std::vector<std::string> queries;
    queries.push_back(query);
    return get_sequence(queries);
  }
  std::vector<std::string> get_sequence(
      const std::vector<std::string> &queries) const;
  std::vector<std::pair<std::string, std::string> > get_map(
      const std::string &query) const {
    std::vector<std::string> queries;
    queries.push_back(query);
    return get_map(queries);
  }
  std::vector<std::pair<std::string, std::string> > get_map(
      const std::vector<std::string> &queries) const;

 private:
  void apply_queries(const std::vector<std::string> &queries,
                     YAML::Node *current, YAML::Node *next) const;
  YAML::Node _data;
};

}  // namespace initialize_output_directories

#endif  // INITIALIZE_OUTPUT_DIRECTORIES_YAML_READER_H_
