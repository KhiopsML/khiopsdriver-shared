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

namespace khiops_driver_common {
int FRead(size_t *result, void *ptr, FileReader *file_reader, size_t size, size_t count) {
    const size_t ntotaltoread = size * count;
    size_t nlefttoread = ntotaltoread, ntotalread = 0ULL, ntoread, nread;
    size_t offset_inside_first_fragment_to_read, fragment_remote_offset;
    size_t absolute_fragment_index, relative_fragment_index;
    std::string globalread, read;
    bool stopped_on_term_char;
    
    GetLogger()->debug("Reading starting position: {}  |  Total number of bytes to read: {}  |  Total file size: {}.", file_reader->current_position, ntotaltoread, file_reader->total_size);
    
    if (ntotaltoread == 0ULL) {
        GetLogger()->trace("0 byte to read => fast-exit.");
        *result = 0ULL;
        return 0;
    }

    if (FragmentIndexOfUserOffset(&absolute_fragment_index, *file_reader, file_reader->current_position) != 0) return -1;
    GetLogger()->debug("Selected fragment #{}.", absolute_fragment_index);

    for (relative_fragment_index = 0ULL; nlefttoread > 0ULL ; relative_fragment_index++, absolute_fragment_index++) {
        const FileReader::Fragment &fragment = file_reader->fragments[absolute_fragment_index];
        if (relative_fragment_index == 0ULL) {
            offset_inside_first_fragment_to_read = file_reader->current_position - fragment.user_offset;
            fragment_remote_offset = (absolute_fragment_index == 0ULL ? 0ULL : file_reader->header_length) + offset_inside_first_fragment_to_read;
            ntoread = std::min(nlefttoread, fragment.content_size - offset_inside_first_fragment_to_read);
        } else {
            fragment_remote_offset = file_reader->header_length;
            ntoread = std::min(nlefttoread, fragment.content_size);
        }
        if (ReadFragment(&read, &stopped_on_term_char, fragment, fragment_remote_offset, ntoread) != 0) {
            GetLogger()->error("Failed to read.");
            return -1;
        }
        nread = read.size();
        GetLogger()->trace("File fragment #{} (absolute #{}): read {} bytes.", relative_fragment_index, absolute_fragment_index, nread);
        if (nread != ntoread) { GetLogger()->error("Number of bytes read does not match number of bytes to read."); return -1; }
        ntotalread += nread;
        globalread.append(read);
        nlefttoread -= nread;
        file_reader->current_position += nread;
    }

    *result = ntotalread;
    memcpy(ptr, globalread.data(), ntotalread);
    return 0;
}
}