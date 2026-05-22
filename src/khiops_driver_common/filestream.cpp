#include "khiops_driver_common/filestream.hpp"
#include "khiops_driver_common/logging.hpp"
#include "khiops_driver_common/backend.hpp"
#include "khiops_driver_common/util.hpp"

using namespace std;

namespace khiops_driver_common {

int PopulateFileReader(FileReader *file_reader, const std::string &url) {
    // Number of fragments to pick at each end for header detection.
    const size_t NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END = 5;
    // Number of fragments to pick randomly for header detection.
    const size_t MAX_NUMBER_OF_FRAGMENTS_TO_PICK_RANDOMLY = 10;
    constexpr size_t MAX_HEADER_LENGTH = 8ULL * 1024ULL * 1024ULL;

    vector<string> fragment_urls;
    const size_t total_number_of_fragments = fragment_urls.size();
    size_t number_of_fragments_to_pick_randomly = min(total_number_of_fragments - 2 * NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END, MAX_NUMBER_OF_FRAGMENTS_TO_PICK_RANDOMLY);
    bool there_may_be_a_header = true;
    string header;
    string header_just_read;
    size_t possible_header_length = MAX_HEADER_LENGTH;

    if (ListObjects(&url) == 0) {
        for (size_t fragment_index = 0ULL; fragment_index < total_number_of_fragments; fragment_index++) {
            if (there_may_be_a_header) {
                // Determine if we need to fetch the header of the current fragment.
                bool should_read_header = false;
                if (fragment_index < NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END || total_number_of_fragments - NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END <= fragment_index) {
                    should_read_header = true;
                } else if (number_of_fragments_to_pick_randomly > 0 && util::random::RandomBool()) {
                    should_read_header = true;
                    number_of_fragments_to_pick_randomly--;
                }

                // Read the header.
                if (ReadHeader(&header_just_read, fragment_urls[fragment_index], possible_header_length) == 0) {
                    if (header_just_read.empty() || fragment_index > 0ULL && header_just_read != header) {
                        there_may_be_a_header = false;
                    } else if (fragment_index == 0ULL) {
                        possible_header_length = header_just_read.size();
                        header = header_just_read;
                    }
                } else {  // Failed to read header.
                    return -1;
                }
            }
        }
    } else {
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