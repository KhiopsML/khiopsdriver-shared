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
#include "logging.hpp"

namespace khiops_driver_common {
namespace util {

namespace str {

inline std::vector<std::string> Split(const std::string &str, char delim, long long int nMaxSplits = -1, bool bRemoveEmpty = false) {
  size_t nStrLen = str.length();
  std::vector<std::string> fragments;
  size_t nOffset = 0;
  size_t nDelimPos;
  std::string sFragment;
  for (size_t nSplits = 0; nMaxSplits == -1 || nSplits <= static_cast<size_t>(nMaxSplits); nSplits++) {
    nDelimPos = nSplits == static_cast<size_t>(nMaxSplits)
        ? std::string::npos
        : str.find(delim, nOffset); sFragment = nOffset == nStrLen ? "" : str.substr(nOffset, nDelimPos - nOffset);
    if (!sFragment.empty() || !bRemoveEmpty) {
      fragments.push_back(std::move(sFragment));
    }
    if (nDelimPos == std::string::npos) {
      break;
    }
    nOffset = nDelimPos + 1;
  }
  return fragments;
}

inline bool StartsWith(const std::string &str, const std::string &prefix) {
  size_t strLen = str.length();
  size_t prefixLen = prefix.length();
  return prefixLen <= strLen && !str.compare(0, prefixLen, prefix);
}

inline bool EndsWith(const std::string &str, const std::string &suffix) {
  size_t strLen = str.length();
  size_t suffixLen = suffix.length();
  return suffixLen <= strLen &&
         !str.compare(strLen - suffixLen, suffixLen, suffix);
}

inline std::string ToLower(const std::string &str) {
  std::string lower(str.length(), '\0');
  std::transform(str.begin(), str.end(), lower.begin(), [](char ch) { return (char)tolower((int)ch); });
  return lower;
}

} // namespace str

namespace random {

inline bool RandomBool() {
  static std::random_device randomDevice;
  static std::minstd_rand::result_type seed =
      randomDevice() ^
      ((std::minstd_rand::result_type)std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::system_clock::now().time_since_epoch())
           .count() +
       (std::minstd_rand::result_type)std::chrono::duration_cast<std::chrono::microseconds>(
           std::chrono::high_resolution_clock::now().time_since_epoch())
           .count());
  static std::minstd_rand generator(seed);
  return (bool)(generator() % 2 == 1);
}

} // namespace random

namespace env {

inline std::string GetEnvVar(const std::string &sVarName, bool bForbidLogging = false) {
  char *loglevel_cstr, *logfile_cstr;
  std::string loglevel = "";
  std::string logfile = "";
  if (!bForbidLogging) {
    if ((loglevel_cstr = getenv("DRIVER_COMMON_LOGLEVEL"))) {
      loglevel = loglevel_cstr;
    }
    if ((logfile_cstr = getenv("DRIVER_COMMON_LOGFILE"))) {
      logfile = logfile_cstr;
    }
  }
  char *sValue = getenv(sVarName.c_str());
  if (!sValue) {
    if (!bForbidLogging) {
      khiops_driver_common::logging::getLogger("drivercommon", loglevel, logfile, false)->debug("Environment variable {} is not set.", sVarName);
    }
    return "";
  }
  if (strlen(sValue) == 0ULL) {
    if (!bForbidLogging) {
      khiops_driver_common::logging::getLogger("drivercommon", loglevel, logfile, false)->debug("Environment variable {} is empty.", sVarName);
    }
    return "";
  }
  if (!bForbidLogging) {
    khiops_driver_common::logging::getLogger("drivercommon", loglevel, logfile, false)->debug("Environment variable {} is set to: {}.", sVarName,
                       sValue);
  }
  return sValue;
}

inline std::string GetEnvVarOrDefault(const std::string &sVarName, const std::string &sDefaultValue, bool bForbidLogging = false) {
  std::string sEnvval = GetEnvVar(sVarName, bForbidLogging);
  if (sEnvval.empty()) {
    return sDefaultValue;
  }
  return sEnvval;
}

} // namespace env

namespace glob {

inline size_t FindGlobbingChar(const std::string &str) {
  std::smatch match;
  return std::regex_search(str, match,
                      std::regex("[^\\]([*?![^])", std::regex_constants::extended))
             ? match.position(1)
             : std::string::npos;
}

} // namespace glob

} // namespace util
} // namespace khiops_driver_common