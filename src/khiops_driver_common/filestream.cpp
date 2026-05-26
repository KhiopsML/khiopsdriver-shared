#include "khiops_driver_common/filestream.hpp"
#include "khiops_driver_common/logging.hpp"
#include "khiops_driver_common/backend.hpp"
#include "khiops_driver_common/util.hpp"

using namespace std;

namespace khiops_driver_common {

int PopulateFileReader(FileReader *file_reader, const std::string &url) {
    // Number of fragments to pick at each end for header detection.
    const size_t NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END = 5;
    // Number of fragments to pick randomly for header detection. This is only a maximum since priority is given to the "pick 'n' at each end" rule.
    const size_t MAX_NUMBER_OF_FRAGMENTS_TO_PICK_RANDOMLY = 10;
    // The maximal size of a header is defined to 8 MiB. If there is a longer header, it is not considered to be a header.
    constexpr size_t MAX_HEADER_LENGTH = 8ULL * 1024ULL * 1024ULL;

    // The fragment vector may contain:
    //   - multiple URLs matching remote objects if the user-provided URL is a globbing pattern
    // OR
    //   - one URL matching a remote object if the user-provided URL is NOT a globbing pattern.
    vector<string> fragment_urls;
    const size_t total_number_of_fragments = fragment_urls.size();

    size_t number_of_fragments_picked_randomly = 0ULL;
    
    // Before any readings are performed, we do not know it there is a header, so there MAY BE one.
    bool there_may_be_a_header = true;

    string possible_header;
    string header_just_read;
    size_t possible_header_length = MAX_HEADER_LENGTH;

    if (ListRemoteObjects(&fragment_urls, &url) == 0) {
        for (size_t fragment_index = 0ULL; fragment_index < total_number_of_fragments; fragment_index++) {
            if (there_may_be_a_header) {
                // Determine if we need to fetch the header of the current fragment.
                bool should_read_header = false;
                if (fragment_index < NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END || total_number_of_fragments - NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END <= fragment_index) {
                    should_read_header = true;
                } else if (number_of_fragments_picked_randomly < MAX_NUMBER_OF_FRAGMENTS_TO_PICK_RANDOMLY && util::random::RandomBool()) {
                    should_read_header = true;
                    number_of_fragments_picked_randomly++;
                }

                // Read the header.
                if (RemoteRead(&header_just_read, fragment_urls[fragment_index], 0ULL, possible_header_length, '\n') == 0) {
                    if (header_just_read.empty() || fragment_index > 0ULL && header_just_read != possible_header) {
                        there_may_be_a_header = false;
                    } else if (fragment_index == 0ULL) {
                        possible_header_length = header_just_read.size();
                        possible_header = header_just_read;
                    }
                } else {  // Failed to read header.
                    return -1;
                }
            }
        }
    } else {  // Failed to list remote objects matching the user-provided URL.
        return -1;
    }
    return 0;
}

FileStream::FileStream():
    url(""),
    mode(Mode::NONE)
{}

FileStream::FileStream(FileStream &&source):
    url(std::move(source.url)),
    mode(std::move(source.mode))
{}

int FileModeCharToFileStreamMode(FileStream::Mode *result, char mode) {
    if (mode == 'r') {
        *result = FileStream::Mode::READ;
    } else if (mode == 'w') {
        *result = FileStream::Mode::WRITE;
    } else if (mode == 'a') {
        *result = FileStream::Mode::APPEND;
    } else {
        GetLogger()->error("Invalid file stream mode '{}'.", mode);
        return -1;
    }
    return 0;
}

}