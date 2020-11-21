/*!
  \file utilities.cc
  \brief implementations of random helper content
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#include "initialize_output_directories/utilities.h"

std::string initialize_output_directories::strreplace(const std::string &input,
                                                      char query,
                                                      char replacement) {
  std::string res = input;
  for (std::string::iterator iter = res.begin(); iter != res.end(); ++iter) {
    if (*iter == query) *iter = replacement;
  }
  return res;
}

bool initialize_output_directories::cicompare(const std::string &s1,
                                              const std::string &s2) {
  if (s1.size() != s2.size()) return false;
  for (unsigned i = 0; i < s1.size(); ++i) {
    if (toupper(s1.at(i)) != toupper(s2.at(i))) return false;
  }
  return true;
}

bool initialize_output_directories::find_entry(
    const std::string &query, const std::vector<std::string> &sequence) {
  for (std::vector<std::string>::const_iterator iter = sequence.begin();
       iter != sequence.end(); ++iter) {
    if (cicompare(query, *iter)) return true;
  }
  return false;
}
