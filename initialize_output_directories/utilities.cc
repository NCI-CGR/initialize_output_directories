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

std::string initialize_output_directories::uppercase(const std::string &s) {
  std::string res = s;
  for (std::string::iterator iter = res.begin(); iter != res.end(); ++iter) {
    *iter = toupper(*iter);
  }
  return res;
}

std::string initialize_output_directories::lowercase(const std::string &s) {
  std::string res = s;
  for (std::string::iterator iter = res.begin(); iter != res.end(); ++iter) {
    *iter = tolower(*iter);
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

unsigned initialize_output_directories::wc(const std::string &filename) {
  std::ifstream input(filename.c_str());
  if (!input.is_open())
    throw std::runtime_error("cannot open file \"" + filename + "\"");
  // https://stackoverflow.com/questions/3072795/how-to-count-lines-of-a-file-in-c
  unsigned res = std::count(std::istreambuf_iterator<char>(input),
                            std::istreambuf_iterator<char>(), '\n');
  input.close();
  return res;
}
