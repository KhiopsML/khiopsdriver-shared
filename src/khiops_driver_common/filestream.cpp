#include "khiops_driver_common/filestream.hpp"
#include "khiops_driver_common/logging.hpp"
#include "khiops_driver_common/backend.hpp"
#include "khiops_driver_common/util.hpp"
#include <algorithm>

using namespace std;

namespace khiops_driver_common {

int PopulateFileReader(FileReader *file_reader, const string &url) {
    if (file_reader == nullptr) {
        GetLogger()->error("Null pointer passed to function {}.", __func__);
        return -1;
    }

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
    
    size_t number_of_fragments_picked_randomly = 0ULL;
    
    //     ** A NOTE ABOUT REPEATED HEADERS **
    //
    // A repeated header is a header that is repeated at the beginning of every fragment.
    // From the user point of view, the header is a part of the content of the first fragment only.
    
    // Before any readings are performed, we do not know it there is a repeated header, so there MAY BE one.
    bool there_may_be_a_header = true;

    string possible_header;
    string header_just_read;
    size_t possible_header_length = MAX_HEADER_LENGTH;
    
    
    if (ListFragments(&fragment_urls, url) != 0) {
        // Failed to list remote objects matching the user-provided URL.
        return -1;
    }

    const size_t total_number_of_fragments = fragment_urls.size();
    vector<size_t> fragment_sizes(total_number_of_fragments);
    vector<CustomVoidUniquePtr> fragment_versions;
    fragment_versions.reserve(total_number_of_fragments);
    
    for (size_t fragment_index = 0ULL; fragment_index < total_number_of_fragments; fragment_index++) {
        void *current_fragment_version;
        if (GetFragmentSizeAndVersion(&fragment_sizes[fragment_index], &current_fragment_version, fragment_urls[fragment_index]) != 0) {
            // Failed to get the size or the version of the current remote object.
            return -1;
        }
        fragment_versions.emplace_back(current_fragment_version, &FreeFileReaderFragmentVersion);

        if (there_may_be_a_header) {
            if (fragment_index > 0ULL && fragment_sizes[fragment_index] < possible_header_length) {
                // A header has previously been detected but it is too big to fit inside the current fragment.
                there_may_be_a_header = false;
            }

            // Determine if we need to fetch the header of the current fragment.
            bool should_read_header = false;
            if (fragment_index < NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END || total_number_of_fragments <= fragment_index + NUMBER_OF_FRAGMENTS_TO_PICK_AT_EACH_END) {
                should_read_header = true;
            } else if (number_of_fragments_picked_randomly < MAX_NUMBER_OF_FRAGMENTS_TO_PICK_RANDOMLY && util::random::RandomBool()) {
                should_read_header = true;
                number_of_fragments_picked_randomly++;
            }

            if (should_read_header) {
                // Read the header.
                bool stopped_on_termchar;
                if (ReadFragment(&header_just_read, &stopped_on_termchar, fragment_urls[fragment_index], fragment_versions[fragment_index].get(), 0ULL, possible_header_length, '\n') != 0) {
                    // Failed to read the header.
                    return -1;
                }

                if (!stopped_on_termchar || header_just_read.empty() || (fragment_index > 0ULL && header_just_read != possible_header)) {
                    there_may_be_a_header = false;
                } else if (fragment_index == 0ULL) {
                    possible_header_length = header_just_read.size();
                    possible_header = header_just_read;
                }
            }
        }
    }

    // From now on, we know if there is a repeated header or not, and if there is one we know its content and, more importantly, its size.
    bool there_is_a_header = there_may_be_a_header;
    size_t header_length = there_is_a_header ? possible_header_length : 0ULL;
    
    file_reader->total_size = 0ULL;
    file_reader->header_length = header_length;
    file_reader->current_position = 0ULL;

    size_t current_user_offset = 0ULL;
    for (size_t fragment_index = 0ULL; fragment_index < total_number_of_fragments; fragment_index++) {
        // Only the first fragment will include the header in its content.
        size_t fragment_content_size = there_is_a_header && fragment_index > 0ULL ? fragment_sizes[fragment_index] - header_length : fragment_sizes[fragment_index];

        // Create the fragment object and add it into the file reader's vector of fragments.
        FileReader::Fragment fragment;
        fragment.url = fragment_urls[fragment_index];
        fragment.user_offset = current_user_offset;
        fragment.content_size = fragment_content_size;
        fragment.version = std::move(fragment_versions[fragment_index]);
        file_reader->fragments.push_back(std::move(fragment));

        // Update the total size of the file.
        file_reader->total_size += fragment_content_size;

        // Update the current user offset.
        current_user_offset += fragment_content_size;
    }

    return 0;
}

int FragmentIndexOfUserOffset(size_t *result, const FileReader &file_reader, size_t user_offset) {
    if (file_reader.fragments.empty()) { GetLogger()->error("No fragments found."); return -1; }
    *result = static_cast<size_t>(
        find_if(
            file_reader.fragments.begin(), file_reader.fragments.end(),
            [user_offset](const FileReader::Fragment &fragment) { return user_offset < fragment.user_offset; }
        )
        - 1
        - file_reader.fragments.begin()
    );
    return 0;
}

}