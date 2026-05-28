#pragma once

#include <vector>
#include <string>

namespace khiops_driver_common {

struct FileReader {
    struct Fragment {
        // The URL of the remote object representing a fragment of the whole file.
        std::string url;
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
    // Fragments can have an empty content or may be considered to have an empty content if there is a repeated header, if they are not the first fragment and there content consists only of the header.
    // Empty fragments are not removed from the vector below because they can be modified between readings and become non-empty.
    // This is a case where the reading will abort in error due to a file fragment version mismatch.
    // Keeping the empty fragments allow us to detect that error case.
    std::vector<Fragment> fragments;
    // Total size of the file as seen by the user, that is, if there is a repeated header, its length is included only in the size of the first fragment.
    size_t total_size;
    // The header length is zero if there is no repeated header in this file.
    size_t header_length;
    // The current position in the file, as seen by the user.
    size_t current_position;
};

int PopulateFileReader(FileReader *file_reader, const std::string &url);
int FragmentIndexOfUserOffset(size_t *result, const FileReader &file_reader, size_t user_offset);

struct FileWriter {};

}