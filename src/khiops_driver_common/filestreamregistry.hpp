#pragma once

#include <unordered_set>
#include "khiops_driver_common/filestream.hpp"

namespace khiops_driver_common {

class FileStreamRegistry {
public:
    int AddStream(void **handle_result, FileStream &&file_stream);
    int GetReaderStream(const FileStream **result, void *handle) const;
    int GetWriterStream(const FileStream **result, void *handle) const;
    int GetAppenderStream(const FileStream **result, void *handle) const;
    int RemoveStream(void *handle);
    ~FileStreamRegistry();
private:
    std::unordered_set<FileStream *> streams;
    int GetStream(const FileStream **result, void *handle) const;
};

}