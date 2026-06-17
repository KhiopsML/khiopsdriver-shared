#include "khiops_driver_common/util.hpp"
#include "khiops_driver_common/globalstate.hpp"
#include "khiops_driver_common/checks.hpp"
#include "khiops_driver_common/stringify.hpp"
#include <fstream>

using namespace std;

namespace khiops_driver_common {

vector<string> Split(const string &str, char delim, long long int nMaxSplits, bool bRemoveEmpty) {
    size_t nStrLen = str.length();
    vector<string> fragments;
    size_t nOffset = 0;
    size_t nDelimPos;
    string sFragment;
    for (size_t nSplits = 0; nMaxSplits == -1 || nSplits <= static_cast<size_t>(nMaxSplits); nSplits++) {
        nDelimPos = nSplits == static_cast<size_t>(nMaxSplits)
            ? string::npos
            : str.find(delim, nOffset); sFragment = nOffset == nStrLen ? "" : str.substr(nOffset, nDelimPos - nOffset);
        if (!sFragment.empty() || !bRemoveEmpty) {
            fragments.push_back(std::move(sFragment));
        }
        if (nDelimPos == string::npos) {
            break;
        }
        nOffset = nDelimPos + 1;
    }
    return fragments;
}

bool StartsWith(const string &str, const string &prefix) {
    size_t strLen = str.length();
    size_t prefixLen = prefix.length();
    return prefixLen <= strLen && !str.compare(0, prefixLen, prefix);
}

bool EndsWith(const string &str, const string &suffix) {
    size_t strLen = str.length();
    size_t suffixLen = suffix.length();
    return suffixLen <= strLen && !str.compare(strLen - suffixLen, suffixLen, suffix);
}

string ToLower(const string &str) {
    string lower(str.length(), '\0');
    transform(str.begin(), str.end(), lower.begin(), [](char ch) { return (char)tolower((int)ch); });
    return lower;
}

bool RandomBool() {
  static random_device randomDevice;
  static minstd_rand::result_type seed =
      randomDevice() ^
      ((minstd_rand::result_type)chrono::duration_cast<chrono::seconds>(
           chrono::system_clock::now().time_since_epoch())
           .count() +
       (minstd_rand::result_type)chrono::duration_cast<chrono::microseconds>(
           chrono::high_resolution_clock::now().time_since_epoch())
           .count());
  static minstd_rand generator(seed);
  return (bool)(generator() % 2 == 1);
}

string GetEnvVar(const string &sVarName, bool bForbidLogging) {
    const char *value = getenv(sVarName.c_str());
    if (value) {
        if (strlen(value) > 0ULL) {
            if (!bForbidLogging) {
                GetLogger()->debug("Environment variable {} is set to: {}.", sVarName, sVarName.find("CONNECTION_STRING") == string::npos ? value : "**REDACTED**");
            }
            return value;
        } else if (!bForbidLogging) {
            GetLogger()->debug("Environment variable {} is empty.", sVarName);
        }
    } else if (!bForbidLogging) {
        GetLogger()->debug("Environment variable {} is not set.", sVarName);
    }
    return "";
}

string GetEnvVarOrDefault(const string &sVarName, const string &sDefaultValue, bool bForbidLogging) {
    string sEnvval = GetEnvVar(sVarName, bForbidLogging);
    if (sEnvval.empty()) {
        return sDefaultValue;
    }
  return sEnvval;
}

size_t FindGlobbingChar(const string &str) {
    bool escaped = false;
    for (size_t i = 0ULL; i < str.size(); i++) {
        const char c = str[i];

        if (escaped) {  // The current character has been previously escaped.
            escaped = false;
            continue;
        }

        if (c == '\\') {  // The next character, if any, will be escaped.
            escaped = true;
            continue;
        }

        if (c == '*' || c == '?' || c == '!' || c == '[' || c == '^') {  // Globbing character found!
            return i;
        }
    }
    return string::npos;  // Globbing character not found.
}

bool IsGlobbingPattern(const string &str) {
    return FindGlobbingChar(str) != string::npos;
}

int CheckIsNotGlobbingPattern(const string &str) {
    if (IsGlobbingPattern(str)) {
        GetLogger()->error("Passed a globbing URL while a real URL was expected.");
        return -1;
    }
    return 0;
}

bool IsDirUrl(const string &url) {
    return url.size() > 0 && url.back() == '/';
}

#if defined(__linux__)
int FindCertificate(string *result) {
    if (CheckNotNull(result, STRINGIFY(result), __func__)) return -1;

    vector<string> file_candidates = {
        "/etc/ssl/certs/ca-certificates.crt",                 // Debian/Ubuntu/Arch/Gentoo
        "/etc/pki/tls/certs/ca-bundle.crt",                   // RHEL/CentOS/Rocky/AlmaLinux
        "/etc/ssl/cert.pem",                                  // Alpine Linux (commonly used)
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",  // RHEL-family (alternative path)
        "/etc/ssl/ca-bundle.pem"                              // SUSE/openSUSE (common path)
    };

    string ssl_cert_file = GetEnvVar("SSL_CERT_FILE");
    if (!ssl_cert_file.empty()) file_candidates.push_back(ssl_cert_file);

    for (const auto &path : file_candidates) {
        ifstream f(path.c_str(), ios::in | ios::binary);
        if (f.good()) {
            *result = path;
            return 0;
        }
    }

    GetLogger()->error("Did not find SSL/TLS certificate.");
    return -1;
}
#endif

}