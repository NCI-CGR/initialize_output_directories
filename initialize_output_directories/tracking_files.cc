/*!
  \file tracking_files.cc
  \brief implementation for tracking files classes
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include "initialize_output_directories/tracking_files.h"

void initialize_output_directories::model_matrix::load_data(
    const std::string &filename) {
  // https://stackoverflow.com/questions/17925051/fast-textfile-reading-in-c
  boost::iostreams::mapped_file input(filename.c_str(),
                                      boost::iostreams::mapped_file::readonly);
  const char *f = input.const_data(), *f_next = 0;
  const char *l = f + input.size();
  std::string line = "", catcher = "";
  std::map<std::string, bool> targets;
  targets[get_phenotype()] = true;
  for (std::vector<std::string>::const_iterator iter = get_covariates().begin();
       iter != get_covariates().end(); ++iter) {
    targets[*iter] = true;
  }
  std::vector<bool> include_column;
  unsigned id_colnum = 0, nfound = 0;
  _headers.clear();
  _ids.clear();
  if ((f_next = static_cast<const char *>(memchr(f, '\n', l - f)))) {
    line = std::string(f, f_next - f);
    std::istringstream strm1(line);
    while (strm1 >> catcher) {
      include_column.push_back(targets.find(catcher) != targets.end());
      if (targets.find(catcher) != targets.end()) ++nfound;
      if (*include_column.rbegin()) _headers.push_back(catcher);
      if (!catcher.compare(get_id())) {
        id_colnum = include_column.size() - 1;
      }
    }
    f = f_next + 1;
  } else {
    throw std::runtime_error("no header in phenotype file \"" + filename +
                             "\"");
  }
  _data.clear();
  _data.resize(nfound);
  while (f && f != l) {
    nfound = 0;
    for (unsigned i = 0; i < include_column.size(); ++i) {
      if (!f || f == l) {
        throw std::runtime_error("ran out of tokens for phenotype file \"" +
                                 filename + "\"");
      }
      if ((f_next = static_cast<const char *>(memchr(
               f, (i == include_column.size() - 1 ? '\n' : '\t'), l - f)))) {
        if (i == id_colnum) {
          catcher = std::string(f, f_next - f);
          _ids.push_back(catcher);
        }
        if (include_column.at(i)) {
          catcher = std::string(f, f_next - f);
          _data.at(nfound).push_back(catcher);
          ++nfound;
        }
      } else {
        throw std::runtime_error("insufficient tokens in phenotype file \"" +
                                 filename + "\"");
      }
      f = f_next + 1;
    }
  }
  input.close();
}

void initialize_output_directories::model_matrix::write(
    const std::string &filename) const {
  std::ofstream output;
  output.open(filename.c_str());
  if (!output.is_open())
    throw std::runtime_error("cannot write model_matrix file \"" + filename +
                             "\"");
  output << get_id();
  for (std::vector<std::string>::const_iterator iter = _headers.begin();
       iter != _headers.end(); ++iter) {
    output << '\t' << *iter;
  }
  output << std::endl;
  for (unsigned i = 0; i < _data.size(); ++i) {
    if (_ids.size() != _data.at(i).size())
      throw std::runtime_error(
          "jagged model matrix: " + std::to_string(_ids.size()) +
          " (ids) versus " + std::to_string(_data.at(i).size()) + " (" +
          std::to_string(i) + ")");
  }
  for (unsigned i = 0; i < _data.at(0).size(); ++i) {
    output << _ids.at(i);
    for (unsigned j = 0; j < _data.size(); ++j) {
      output << '\t' << _data.at(j).at(i);
    }
    output << std::endl;
  }
  output.close();
}

initialize_output_directories::categorical_variable
initialize_output_directories::model_matrix::categorize(
    const std::string &name) const {
  std::map<std::string, unsigned> res;
  std::map<std::string, unsigned>::iterator finder;
  unsigned target_colnum = 0;
  for (; target_colnum < _headers.size(); ++target_colnum) {
    if (!_headers.at(target_colnum).compare(name)) break;
  }
  if (target_colnum == _headers.size())
    throw std::runtime_error("categorize: unable to find header \"" + name +
                             "\"");
  for (std::vector<std::string>::const_iterator iter =
           _data.at(target_colnum).begin();
       iter != _data.at(target_colnum).end(); ++iter) {
    if ((finder = res.find(*iter)) == res.end()) {
      finder = res.insert(std::make_pair(*iter, 0)).first;
    }
    ++finder->second;
  }
  // so realistically, there shouldn't be more than like...
  //   15 or so categories? probably want to configure this
  //   in a user-accessible way later. alternate groups with <100
  //   subjects should be merged into a single meta-group according
  //   to the PLCO analysis plan. this code doesn't currently check
  //   that the resultant metagroup has more than 100 subjects in it....
  //   will probably add that as a debug flagged option at least. i mean,
  //   from an atlas perspective, what do you even do about it...
  categorical_variable cv;
  std::set<unsigned> combined_alternate;
  unsigned combined_alternate_meta_count = 0;
  for (std::map<std::string, unsigned>::const_iterator iter = res.begin();
       iter != res.end(); ++iter) {
    if (iter->first.find_first_not_of("0123456789") == std::string::npos) {
      std::istringstream strm1(iter->first);
      unsigned val = 0;
      if (!(strm1 >> val))
        throw std::runtime_error(
            "confusingly unable to convert to integer: \"" + iter->first +
            "\"");
      if (iter->second < 100) {
        combined_alternate.emplace(val);
        combined_alternate_meta_count += iter->second;
      } else {
        if (iter == res.begin()) {
          cv.set_reference_level(val);
        } else {
          cv.add_comparison_group(val);
        }
      }
    }
  }
  if (!combined_alternate.empty()) {
    cv.add_comparison_group(combined_alternate);
  }
  return cv;
}

void initialize_output_directories::tracking_files::initialize(
    const yaml_reader &config) {
  // create the target directory if needed
  std::string target_dir =
      get_output_prefix().substr(0, get_output_prefix().rfind("/"));
  boost::filesystem::path target_dir_path = target_dir;
  boost::filesystem::create_directories(target_dir_path);
  // pull the required presets from this extension configuration
  _phenotype_dataset_suffix = config.get_entry("phenotype-dataset");
  _phenotype_suffix = config.get_entry("phenotype");
  _covariates_suffix = config.get_entry("covariates");
  // the categories tracking suffix is permitted to be NA in the original code
  // which indicated the use of BOLT or fastGWA, one of the methods that doesn't
  // operate on categorical/binary traits. but now, the categorical tracker is
  // specified once in the global tracker extension file and never again.
  // so, it's appropriate to unconditionally search for it here, but later
  // on the tracker should only run when the trait is actually categorical or
  // binary
  _categories_suffix = config.get_entry("categories");
  _finalized_suffix = config.get_entry("finalization");
  // get as many custom extensions as are available
  YAML::Node data;
  data = config.get_node("general-extensions");
  // should be a map
  if (data.Type() != YAML::NodeType::Map) {
    throw std::runtime_error(
        "initialize: extensions config entry 'general-extensions'"
        " is supposed to be a YAML Map, but is not :(");
  }
  std::string observed_name = "", observed_suffix = "", observed_default = "",
              observed_option = "";
  for (YAML::const_iterator iter = data.begin(); iter != data.end(); ++iter) {
    if (iter->second["suffix"] && iter->second["default"]) {
      if (iter->second["suffix"].IsScalar() &&
          iter->second["default"].IsScalar()) {
        extension_definition edef;
        observed_name = iter->first.as<std::string>();
        observed_suffix = iter->second["suffix"].as<std::string>();
        observed_default = iter->second["default"].as<std::string>();
        edef.set_name(observed_name);
        edef.set_extension(observed_suffix);
        edef.set_default(observed_default);
        // there is an optional "options" section that contains the predefined
        // permitted values for this extension tracker
        if (iter->second["options"]) {
          std::vector<std::string> extension_options;
          if (iter->second["options"].Type() == YAML::NodeType::Scalar) {
            observed_option = iter->second["options"].as<std::string>();
            edef.add_permitted_value(observed_option);
          } else if (iter->second["options"].Type() ==
                     YAML::NodeType::Sequence) {
            for (YAML::const_iterator options_iter =
                     iter->second["options"].begin();
                 options_iter != iter->second["options"].end();
                 ++options_iter) {
              observed_option = options_iter->as<std::string>();
              edef.add_permitted_value(observed_option);
            }
          } else {
            throw std::runtime_error(
                "initialize: extensions config entry 'general-extensions'"
                " key '" +
                observed_name +
                "' section 'options' contains "
                "invalid data type (likely a Map which is not supported");
          }
        }
        _general_extensions[iter->first.as<std::string>()] = edef;
      } else {
        throw std::runtime_error(
            "initialize: extensions config entry 'general-extensions'"
            " key '" +
            iter->first.as<std::string>() +
            "' is supposed to contain "
            "simple key:value pairs, but is not for either entry 'suffix' or "
            "'default'");
      }
    } else {
      throw std::runtime_error(
          "initialize: extensions config entry 'general-extensions'"
          " should have maps containing at least keys 'suffix' and "
          "'default', but key '" +
          iter->first.as<std::string>() + "' does not");
    }
  }
}

bool initialize_output_directories::tracking_files::check_file(
    const yaml_reader &config, const extension_definition &edef, bool pretend,
    bool force, bool must_exist) const {
  std::string tag = edef.get_name();
  std::string suffix = edef.get_extension();
  std::string value_default = edef.get_default();
  if (pretend) return false;
  std::string line = "";
  std::vector<std::string> values;
  std::map<std::string, std::string> mapped_values;
  std::string filename = get_output_prefix() + suffix;
  // weird corner case: current config doesn't have entry but previous run did
  if (!config.query_valid(tag)) {
    if (must_exist && value_default.empty())
      throw std::runtime_error("check_file: essential tag \"" + tag +
                               "\" not found "
                               "in configuration for analysis \"" +
                               get_output_prefix() + "\"");
    values.push_back(value_default);
  } else {
    YAML::Node head = config.get_node(tag);
    if (head.Type() == YAML::NodeType::Scalar) {
      values.push_back(head.as<std::string>());
    } else if (head.Type() == YAML::NodeType::Sequence) {
      for (YAML::const_iterator iter = head.begin(); iter != head.end();
           ++iter) {
        values.push_back(iter->as<std::string>());
      }
    } else if (head.Type() == YAML::NodeType::Map) {
      for (YAML::const_iterator iter = head.begin(); iter != head.end();
           ++iter) {
        mapped_values[iter->first.as<std::string>()] =
            iter->second.as<std::string>();
      }
    } else {
      throw std::runtime_error("check_file: tracker '" + tag +
                               "' entry "
                               "type not recognized");
    }
  }
  // confirm that all entries in "values" are valid options for this tracker
  // currently only enabled for sequence-type input; map type input is targeted
  // at variable names, which won't ever be enumerated at config level,
  // hopefully?
  for (std::vector<std::string>::const_iterator iter = values.begin();
       iter != values.end(); ++iter) {
    if (!edef.is_permitted_value(*iter)) {
      throw std::runtime_error("check_file: tracker '" + tag + "' value '" +
                               *iter +
                               "' not among permitted values for this tracker");
    }
  }
  if (!boost::filesystem::is_regular_file(boost::filesystem::path(filename))) {
    if (values.empty()) {
      update_tracker(filename, mapped_values, false);
    } else {
      update_tracker(filename, values, false);
    }
    return true;
  }
  std::ifstream input(filename.c_str());
  if (!input.is_open())
    throw std::runtime_error("cannot open \"" + tag + "\" tracking file \"" +
                             filename + "\"");
  // for esoteric reasons, need to be careful about how this is computed.
  // acquire the (guaranteed very small) file contents into a single string;
  // do the same for the new values; compare those
  std::ostringstream existing_data, new_data;
  while (input.peek() != EOF) {
    getline(input, line);
    existing_data << line << std::endl;
  }
  input.close();
  if (values.empty()) {
    for (std::map<std::string, std::string>::const_iterator iter =
             mapped_values.begin();
         iter != mapped_values.end(); ++iter) {
      new_data << (iter == mapped_values.begin() ? "" : ",") << iter->first
               << '\t' << iter->second;
    }
  } else {
    for (std::vector<std::string>::const_iterator iter = values.begin();
         iter != values.end(); ++iter) {
      new_data << (iter == values.begin() ? "" : ",") << *iter;
    }
  }
  new_data << std::endl;
  if (existing_data.str().compare(new_data.str())) {
    if (values.empty()) {
      update_tracker(filename, mapped_values, false);
    } else {
      update_tracker(filename, values, false);
    }
    return true;
  }
  return false;
}

bool initialize_output_directories::tracking_files::check_phenotype_database(
    const yaml_reader &config, const model_matrix &input_model,
    const std::string &phenotype_filename, bool pretend, bool force) const {
  // logic is as follows:
  // - if a pretend run (make -n), leave everything as is, return false
  // - if a force run (make -B), or
  // -    if the database version tracking file does not exist, create the file
  // and put this database in it, return true
  // - if the database version tracking file does exist and contains the current
  // file, return false
  // - if the database version tracking file does exist but doesn't contain the
  // current file:
  // --- run model matrix calculation on new and old database
  // --- if there is any meaningful difference between the versions, delete the
  // tracker, create a version with this version, return true
  // --- if there is no meaningful difference between the versions, append this
  // filename to the tracker, return false
  if (pretend) return false;
  std::string line = "";
  std::string filename = get_output_prefix() + get_phenotype_dataset_suffix();
  std::vector<std::string> update_contents;
  update_contents.push_back(phenotype_filename);
  if (!boost::filesystem::is_regular_file(boost::filesystem::path(filename)) ||
      force) {
    update_tracker(filename, update_contents, false);
    return true;
  } else {
    std::ifstream input(filename.c_str());
    if (!input.is_open())
      throw std::runtime_error("cannot read phenotype dataset tracker file \"" +
                               filename + "\"");
    bool found_match = false;
    std::vector<std::string> previous_datasets;
    while (input.peek() != EOF) {
      getline(input, line);
      if (!line.compare(phenotype_filename)) found_match = true;
      previous_datasets.push_back(line);
    }
    input.close();
    if (found_match) return false;
    // tracker file exists but does not contain the current phenotype file
    model_matrix new_mm, old_mm;
    if (input_model.empty()) {
      new_mm.set_id(input_model.get_id());
      new_mm.set_phenotype(config.get_entry("phenotype"));
      if (config.query_valid("covariates")) {
        new_mm.set_covariates(config.get_sequence("covariates"));
      }
      new_mm.load_data(phenotype_filename);
    } else {
      new_mm = input_model;
    }
    old_mm.set_id(new_mm.get_id());
    old_mm.set_phenotype(config.get_entry("phenotype"));
    if (config.query_valid("covariates")) {
      old_mm.set_covariates(config.get_sequence("covariates"));
    }
    for (std::vector<std::string>::const_iterator iter =
             previous_datasets.begin();
         iter != previous_datasets.end(); ++iter) {
      if (boost::filesystem::is_regular_file(boost::filesystem::path(*iter))) {
        old_mm.load_data(*iter);
        break;
      }
    }
    // not necessarily any guarantee any old dataset is still present for
    // comparison
    if (old_mm.empty() || old_mm != new_mm) {
      // overwrite the tracker with the new dataset, as change state is
      // undefined
      update_tracker(filename, update_contents, false);
      return true;
    } else {  // no contextual change
      // append current dataset to tracker and return no change
      update_tracker(filename, update_contents, true);
      return false;
    }
  }
}

bool initialize_output_directories::tracking_files::check_files(
    const yaml_reader &config, const model_matrix &input_model,
    const std::string &phenotype_filename, bool pretend, bool force) const {
  bool res = check_phenotype_database(config, input_model, phenotype_filename,
                                      pretend, force);
  extension_definition extension_pheno;
  extension_pheno.set_name("phenotype");
  extension_pheno.set_extension(get_phenotype_suffix());
  res |= check_file(config, extension_pheno, pretend, force, true);
  extension_definition extension_covar;
  extension_covar.set_name("covariates");
  extension_covar.set_extension(get_covariates_suffix());
  res |= check_file(config, extension_covar, pretend, force, false);
  for (std::map<std::string, extension_definition>::const_iterator iter =
           _general_extensions.begin();
       iter != _general_extensions.end(); ++iter) {
    res |= check_file(config, iter->second, pretend, force, false);
  }
  return res;
}

void initialize_output_directories::tracking_files::remove_finalization()
    const {
  std::string finalization_file = get_output_prefix() + get_finalized_suffix();
  boost::filesystem::remove(boost::filesystem::path(finalization_file));
}

void initialize_output_directories::tracking_files::update_tracker(
    const std::string &filename, const std::vector<std::string> &vec,
    bool append) const {
  std::ofstream output;
  output.open(filename.c_str(),
              append ? std::ios_base::app : std::ios_base::out);
  if (!output.is_open())
    throw std::runtime_error("cannot write tracking file \"" + filename + "\"");
  unsigned counter = 0;
  for (std::vector<std::string>::const_iterator iter = vec.begin();
       iter != vec.end(); ++iter, ++counter) {
    if (!(output << *iter))
      throw std::runtime_error("cannot write entry to tracking file \"" +
                               filename + "\"");
    if (counter < vec.size() - 1) {
      if (!(output << ','))
        throw std::runtime_error(
            "cannot write comma delimiter to tracking file \"" + filename +
            "\"");
    }
  }
  if (!(output << std::endl))
    throw std::runtime_error("cannot write newline to tracking file \"" +
                             filename + "\"");
  output.close();
}

void initialize_output_directories::tracking_files::update_tracker(
    const std::string &filename,
    const std::map<std::string, std::string> &values, bool append) const {
  std::ofstream output;
  output.open(filename.c_str(),
              append ? std::ios_base::app : std::ios_base::out);
  if (!output.is_open())
    throw std::runtime_error("cannot write tracking file \"" + filename + "\"");
  for (std::map<std::string, std::string>::const_iterator iter = values.begin();
       iter != values.end(); ++iter) {
    if (!(output << iter->first << '\t' << iter->second << std::endl))
      throw std::runtime_error("cannot write entry to tracking file \"" +
                               filename + "\"");
  }
  output.close();
}

void initialize_output_directories::tracking_files::copy_trackers(
    unsigned comparison_number, const std::set<unsigned> &reference,
    const std::set<unsigned> &comparison) const {
  std::string file_prefix =
      get_output_prefix().substr(get_output_prefix().rfind("/") + 1);
  std::string target_dir =
      get_output_prefix().substr(0, get_output_prefix().rfind("/")) +
      "/comparison" + std::to_string(comparison_number);
  std::string target_prefix = target_dir + "/" + file_prefix;
  boost::filesystem::create_directories(boost::filesystem::path(target_dir));
  // acquire suffixes of copyable files
  std::vector<std::string> suffixes;
  suffixes.push_back(get_phenotype_dataset_suffix());
  suffixes.push_back(get_phenotype_suffix());
  suffixes.push_back(get_covariates_suffix());
  for (std::map<std::string, extension_definition>::const_iterator iter =
           _general_extensions.begin();
       iter != _general_extensions.end(); ++iter) {
    suffixes.push_back(iter->second.get_extension());
  }
  // for each suffix, test to see if the source file exists; if so, copy to
  // target
  for (std::vector<std::string>::const_iterator iter = suffixes.begin();
       iter != suffixes.end(); ++iter) {
    if (boost::filesystem::is_regular_file(
            boost::filesystem::path(get_output_prefix() + *iter))) {
      // test whether the target already exists
      if (boost::filesystem::is_regular_file(
              boost::filesystem::path(target_prefix + *iter))) {
        // if the contents of source and target are identical, do nothing
        std::ifstream in1, in2;
        std::string line1 = "", line2 = "";
        in1.open((get_output_prefix() + *iter).c_str());
        in2.open((target_prefix + *iter).c_str());
        while (in1.peek() != EOF && in2.peek() != EOF &&
               !line1.compare(line2)) {
          getline(in1, line1);
          getline(in2, line2);
        }
        if (!line1.compare(line2) && in1.peek() == EOF && in2.peek() == EOF) {
          // files are the same
          continue;
        }
        // delete the target file so boost::filesystem::copy works without issue
        boost::filesystem::remove(
            boost::filesystem::path(target_prefix + *iter));
      }
      boost::filesystem::copy(
          boost::filesystem::path(get_output_prefix() + *iter),
          boost::filesystem::path(target_prefix + *iter));
    }
  }
  // only for the comparison level tracker: emit the comparison level
  // information I'm still not really convinced what to do about these levels.
  // this version is a modification from the old code in that it reports
  // reference and comparison labels for each variable level, introducing the
  // possibility of arbitrary level specification, whereas before arbitrary
  // levels were only accepted in the comparison group. even this change
  // requires a downstream change in ./shared-source/construct_model_matrix.R,
  // but that needs rewriting from scratch anyway. that may be the next issue to
  // address.
  // formatting is a bit wonky here, as most of the trackers are reporting
  // comma-delimited lists, but this is traditionally one token per line,
  // now extended to two tokens per line. so this code now slightly hacks
  // the behavior of update_tracker to support arbitrary line formatting.
  std::vector<std::string> category_tracker_data;
  std::ostringstream o;
  for (std::set<unsigned>::const_iterator iter = reference.begin();
       iter != reference.end(); ++iter) {
    o << *iter << "\treference\n";
  }
  unsigned counter = 0;
  for (std::set<unsigned>::const_iterator iter = comparison.begin();
       iter != comparison.end(); ++iter, ++counter) {
    o << *iter << "\tcomparison";
    if (counter < comparison.size() - 1) o << '\n';
  }
  category_tracker_data.push_back(o.str());
  update_tracker(target_prefix + get_categories_suffix(), category_tracker_data,
                 false);
}
