/*
A collection of utilities.
*/

#pragma once

// getenv would be more secure in C++ than in C and thus getenv_s would not be available in C++?
#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <random>
#include <regex>
#include <string>
#include <vector>
#include "khiops_driver_common/logging.hpp"
#include <spdlog/spdlog.h>

namespace khiops_driver_common {

// Split string using a delimiter character.
std::vector<std::string> Split(const std::string &str, char delim, long long int nMaxSplits = -1, bool bRemoveEmpty = false);
// Check if string has a given prefix.
bool StartsWith(const std::string &str, const std::string &prefix);
// Check if string has a given suffix.
bool EndsWith(const std::string &str, const std::string &suffix);
// Get a new string which is a lowercase version of the given string.
std::string ToLower(const std::string &str);

// Randomly return true or false.
bool RandomBool();

// Get environment variable, returning an empty string if not found.
std::string GetEnvVar(const std::string &sVarName, bool bForbidLogging = false);
// Get environment variable, returning a given default value if not found.
std::string GetEnvVarOrDefault(const std::string &sVarName, const std::string &sDefaultValue, bool bForbidLogging = false);

// Find the position of the first globbing character inside a string.
size_t FindGlobbingChar(const std::string &str);
// Test if a string contains globbing characters.
bool IsGlobbingPattern(const std::string &str);
// Check that a string does not contain globbing characters.
int CheckIsNotGlobbingPattern(const std::string &str);

// Detect if a URL refers to a directory, that is, if it ends with a slash.
bool IsDirUrl(const std::string &url);

#if defined(__linux__)
// Find the path to the SSL/TLS certificate file.
int FindCertificate(std::string *result);
#endif

} // namespace khiops_driver_common