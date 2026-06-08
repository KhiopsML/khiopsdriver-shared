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

std::vector<std::string> Split(const std::string &str, char delim, long long int nMaxSplits = -1, bool bRemoveEmpty = false);
bool StartsWith(const std::string &str, const std::string &prefix);
bool EndsWith(const std::string &str, const std::string &suffix);
std::string ToLower(const std::string &str);

bool RandomBool();

std::string GetEnvVar(const std::string &sVarName, bool bForbidLogging = false);
std::string GetEnvVarOrDefault(const std::string &sVarName, const std::string &sDefaultValue, bool bForbidLogging = false);

size_t FindGlobbingChar(const std::string &str);
bool IsGlobbingPattern(const std::string &str);
int CheckIsNotGlobbingPattern(const std::string &str);

// Function to detect if an URL points to a directory.
bool IsDirUrl(const std::string &url);

struct FileReader;  // Forward declaration
int FRead(size_t *result, void *ptr, FileReader *file_reader, size_t size, size_t count);

} // namespace khiops_driver_common