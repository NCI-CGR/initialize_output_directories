/*!
  \file utilities.h
  \brief random helper content
  \copyright Released under the MIT License.
  Copyright 2020 Cameron Palmer.
 */

#ifndef INITIALIZE_OUTPUT_DIRECTORIES_UTILITIES_H_
#define INITIALIZE_OUTPUT_DIRECTORIES_UTILITIES_H_

#include <cctype>
#include <string>
#include <vector>

namespace initialize_output_directories {
std::string strreplace(const std::string &input, char query, char replacement);

std::string uppercase(const std::string &s);

std::string lowercase(const std::string &s);

bool cicompare(const std::string &s1, const std::string &s2);

bool find_entry(const std::string &query,
                const std::vector<std::string> &sequence);

}  // namespace initialize_output_directories

#endif  // INITIALIZE_OUTPUT_DIRECTORIES_UTILITIES_H_
