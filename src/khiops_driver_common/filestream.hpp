#pragma once

#include <vector>
#include <string>

namespace khiops_driver_common {

struct FileReader {
    struct Fragment {
        // The user offset is the start position of this fragment in the whole
        // file as seen by the user. If the file contains a header that is
        // repeated in each fragment, the user only sees the header at the
        // beginning of the first fragment. The user offset is always zero for
        // the first fragment.
        size_t user_offset;
        // The content size includes the header length only for the first fragment.
        size_t content_size;
        // The version of the file is used to detect if the file has been modified before a reading.
        void *version = nullptr;
        // This function must be implemented by the driver to free the memory allocated to the "version" member above.
        void FreeVersion();
        // The destructor then calls "FreeVersion".
        inline ~Fragment() { FreeVersion(); }
    };
    std::vector<Fragment> fragments;
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