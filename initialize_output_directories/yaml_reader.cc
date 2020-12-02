/*!
  \file yaml_reader.cc
  \brief implementation of yaml_reader class
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include "initialize_output_directories/yaml_reader.h"

void initialize_output_directories::yaml_reader::load_file(
    const std::string &filename) {
  _data = YAML::LoadFile(filename.c_str());
}
std::vector<std::string>
initialize_output_directories::yaml_reader::get_sequence(
    const std::vector<std::string> &queries) const {
  YAML::Node current = _data, next;
  std::vector<std::string> res;
  apply_queries(queries, &current, &next);
  if (current.Type() == YAML::NodeType::Scalar) {
    res.resize(1);
    res.at(0) = current.as<std::string>();
  } else if (current.Type() == YAML::NodeType::Sequence) {
    res.reserve(current.size());
    for (YAML::const_iterator iter = current.begin(); iter != current.end();
         ++iter) {
      res.push_back(iter->as<std::string>());
    }
  } else {
    throw std::runtime_error(
        "get_value: query chain does not end in "
        "compatible type");
  }
  return res;
}

std::vector<std::pair<std::string, std::string> >
initialize_output_directories::yaml_reader::get_map(
    const std::vector<std::string> &queries) const {
  YAML::Node current = _data, next;
  std::vector<std::pair<std::string, std::string> > res;
  apply_queries(queries, &current, &next);
  if (current.Type() == YAML::NodeType::Map) {
    res.reserve(current.size());
    for (YAML::const_iterator iter = current.begin(); iter != current.end();
         ++iter) {
      std::pair<std::string, std::string> val(iter->first.as<std::string>(),
                                              iter->second.as<std::string>());
      res.push_back(val);
    }
  } else {
    throw std::runtime_error(
        "get_value: query chain does not end in "
        "compatible type");
  }
  return res;
}

bool initialize_output_directories::yaml_reader::query_valid(
    const std::vector<std::string> &queries) const {
  YAML::Node current = _data, next;
  try {
    apply_queries(queries, &current, &next);
  } catch (const std::runtime_error &e) {
    return false;
  }
  return true;
}

void initialize_output_directories::yaml_reader::apply_queries(
    const std::vector<std::string> &queries, YAML::Node *current,
    YAML::Node *next) const {
  if (!current || !next)
    throw std::runtime_error("apply_queries: null pointer");
  for (unsigned i = 0; i < queries.size(); ++i) {
    *next = (*current)[queries.at(i)];
    switch (next->Type()) {
      case YAML::NodeType::Null:
        throw std::runtime_error("get_value: query \"" + queries.at(i) +
                                 "\" not present at query level");
      case YAML::NodeType::Scalar:
        break;
      case YAML::NodeType::Sequence:
        break;
      case YAML::NodeType::Map:
        break;
      case YAML::NodeType::Undefined:
        throw std::runtime_error("get_value: Undefined node type encountered");
      default:
        throw std::runtime_error("get_value: unrecognized node type?");
    }
    *current = *next;
  }
}
