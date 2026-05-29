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
using CustomVoidUniquePtr = std::unique_ptr<void, void (*)(void *)>;
namespace util {
namespace str {
std::vector<std::string> Split(const std::string &str, char delim, long long int nMaxSplits = -1, bool bRemoveEmpty = false);
bool StartsWith(const std::string &str, const std::string &prefix);
bool EndsWith(const std::string &str, const std::string &suffix);
std::string ToLower(const std::string &str);
} // namespace str
namespace random {
bool RandomBool();
} // namespace random
namespace env {
std::string GetEnvVar(const std::string &sVarName, bool bForbidLogging = false);
std::string GetEnvVarOrDefault(const std::string &sVarName, const std::string &sDefaultValue, bool bForbidLogging = false);
} // namespace env
namespace glob {
size_t FindGlobbingChar(const std::string &str);
bool IsGlobbingPattern(const std::string &str);
int CheckIsNotGlobbingPattern(const std::string &str);
} // namespace glob
// Function to detect if an URL points to a directory.
bool IsDirUrl(const std::string &url);
} // namespace util
} // namespace khiops_driver_common