#include "khiops_driver_common/util.hpp"
#include "khiops_driver_common/backend.hpp"

namespace khiops_driver_common { namespace util { namespace str {
std::vector<std::string> Split(const std::string &str, char delim, long long int nMaxSplits, bool bRemoveEmpty) {
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
bool StartsWith(const std::string &str, const std::string &prefix) {
    size_t strLen = str.length();
    size_t prefixLen = prefix.length();
    return prefixLen <= strLen && !str.compare(0, prefixLen, prefix);
}
bool EndsWith(const std::string &str, const std::string &suffix) {
    size_t strLen = str.length();
    size_t suffixLen = suffix.length();
    return suffixLen <= strLen && !str.compare(strLen - suffixLen, suffixLen, suffix);
}
std::string ToLower(const std::string &str) {
    std::string lower(str.length(), '\0');
    std::transform(str.begin(), str.end(), lower.begin(), [](char ch) { return (char)tolower((int)ch); });
    return lower;
}
}}}

namespace khiops_driver_common { namespace util { namespace random {
bool RandomBool() {
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
}}}

namespace khiops_driver_common { namespace util { namespace env {
std::string GetEnvVar(const std::string &sVarName, bool bForbidLogging) {
    const char *value = getenv(sVarName.c_str());
    if (value) {
        if (strlen(value) > 0ULL) {
            if (!bForbidLogging) {
                khiops_driver_common::GetLogger()->debug("Environment variable {} is set to: {}.", sVarName, sVarName.find("CONNECTION_STRING") == std::string::npos ? value : "**REDACTED**");
            }
            return value;
        } else if (!bForbidLogging) {
            khiops_driver_common::GetLogger()->debug("Environment variable {} is empty.", sVarName);
        }
    } else if (!bForbidLogging) {
        khiops_driver_common::GetLogger()->debug("Environment variable {} is not set.", sVarName);
    }
    return "";
}
std::string GetEnvVarOrDefault(const std::string &sVarName, const std::string &sDefaultValue, bool bForbidLogging) {
    std::string sEnvval = GetEnvVar(sVarName, bForbidLogging);
    if (sEnvval.empty()) {
        return sDefaultValue;
    }
  return sEnvval;
}
}}}

namespace khiops_driver_common { namespace util { namespace glob {
size_t FindGlobbingChar(const std::string &str) {
    std::smatch match;
    return std::regex_search(str, match, std::regex("[^\\]([*?![^])", std::regex_constants::extended)) ? match.position(1) : std::string::npos;
}
bool IsGlobbingPattern(const std::string &str) {
    return FindGlobbingChar(str) != std::string::npos;
}
int CheckIsNotGlobbingPattern(const std::string &str) {
    if (IsGlobbingPattern(str)) {
        khiops_driver_common::GetLogger()->error("Passed a globbing URL while a real URL was expected.");
        return -1;
    } else {
        return 0;
    }
}
}}}

namespace khiops_driver_common { namespace util {
bool IsDirUrl(const std::string &url) {
    return url.size() > 0 && url.back() == '/';
}
}}