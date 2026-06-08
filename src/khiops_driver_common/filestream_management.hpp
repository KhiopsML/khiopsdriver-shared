#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include "khiops_driver_common/logging.hpp"

namespace khiops_driver_common {
    
enum struct FileStreamMode { READ, WRITE, APPEND };
inline std::string FormatFileStreamMode(FileStreamMode mode) {
    switch (mode) {
        case FileStreamMode::READ:
            return "read";
        case FileStreamMode::WRITE:
            return "write";
        case FileStreamMode::APPEND:
            return "append";
        default:
            return "<invalid>";
    }
}

struct OpenFileStreamCollection {
    std::unordered_map<void *, FileStreamMode> file_streams;
};

inline int RegisterFileStream(OpenFileStreamCollection *open_file_streams, void *handle, FileStreamMode mode) {
    if (!open_file_streams->file_streams.insert({handle, mode}).second) {
        khiops_driver_common::GetLogger()->error("Failed to register file stream with handle {} and mode {}.", handle, FormatFileStreamMode(mode));
        return -1;
    }
    return 0;
}

inline int CheckFileStream(const OpenFileStreamCollection &open_file_streams, void *handle) {
    if (open_file_streams.file_streams.find(handle) == open_file_streams.file_streams.end()) {
        khiops_driver_common::GetLogger()->error("No file streams have been registered with handle {}.", handle);
        return -1;
    }
    return 0;
}

inline int CheckFileStream(const OpenFileStreamCollection &open_file_streams, void *handle, FileStreamMode mode) {
    if (CheckFileStream(open_file_streams, handle)) return -1;
    FileStreamMode detected_mode = open_file_streams.file_streams.at(handle);
    if (detected_mode != mode) {
        khiops_driver_common::GetLogger()->error("File stream registered with handle {} is in mode \"{}\", not \"{}\".", handle, FormatFileStreamMode(detected_mode), FormatFileStreamMode(mode));
        return -1;
    }
    return 0;
}

inline int CheckFileStream(const OpenFileStreamCollection &open_file_streams, void *handle, std::vector<FileStreamMode> modes) {
    if (CheckFileStream(open_file_streams, handle)) return -1;
    FileStreamMode detected_mode = open_file_streams.file_streams.at(handle);
    if (std::find(modes.begin(), modes.end(), detected_mode) == modes.end()) {
        std::ostringstream oss;
        oss << "{";
        for (size_t i = 0ULL; i < modes.size(); i++) {
            if (i != 0ULL) oss << ", ";
            oss << "\"" << FormatFileStreamMode(modes[i]) << "\"";
        }
        oss << "}";
        khiops_driver_common::GetLogger()->error("File stream registered with handle {} is in mode \"{}\", not one of {}.", handle, FormatFileStreamMode(detected_mode), oss.str());
        return -1;
    }
    return 0;
}

inline int UnregisterFileStream(OpenFileStreamCollection *open_file_streams, void *handle) {
    if (CheckFileStream(*open_file_streams, handle)) return -1;
    if (open_file_streams->file_streams.erase(handle) != 1ULL) {
        khiops_driver_common::GetLogger()->error("Failed to unregister file stream with handle {}, although it is properly detected as registered.", handle);
        return -1;
    }
    return 0;
}

}