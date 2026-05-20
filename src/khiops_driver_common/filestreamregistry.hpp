#pragma once

#include <unordered_map>
#include "khiops_driver_common/filestream.hpp"

namespace khiops_driver_common {
namespace filestream {

class FileStreamRegistry {
public:
    const FileStream *get_reader_stream(void *handle) const;
    const FileStream *get_writer_stream(void *handle) const;
    const FileStream *get_appender_stream(void *handle) const;
private:
    std::unordered_map<void *, FileStream> streams;
    const FileStream *get_stream(void *handle) const;
};

}
}