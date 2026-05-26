#pragma once

#include <vector>
#include <string>

namespace khiops_driver_common {

struct FileReader {
    std::vector<struct Fragment {
        // The user offset is the start position of this fragment in the whole
        // file as seen by the user. If the file contains a header that is
        // repeated in each fragment, the user only sees the header at the
        // beginning of the first fragment. The user offset is always zero for
        // the first fragment.
        size_t user_offset;
        // The content size includes the header length only for the first fragment.
        size_t content_size;
    }> fragments;
    size_t total_size;
};

int PopulateFileReader(FileReader *file_reader, const std::string &url);

struct FileStream {
    std::string url;
    enum struct Mode {NONE = 0, READ, WRITE, APPEND} mode;
    FileStream();
    FileStream(FileStream &&source);
};

int FileModeCharToFileStreamMode(FileStream::Mode *result, char mode);

}