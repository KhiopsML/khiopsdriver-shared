#pragma once

#include <unordered_set>
#include "khiops_driver_common/filestream.hpp"

namespace khiops_driver_common {

class FileStreamRegistry {
public:
    int AddStream(void **handle_result, FileStream &&file_stream);
    int GetStream(FileStream **result, void *handle) const;
    int GetReaderStream(FileStream **result, void *handle) const;
    int GetWriterStream(FileStream **result, void *handle) const;
    int GetAppenderStream(FileStream **result, void *handle) const;
    int GetWriterOrAppenderStream(FileStream **result, void *handle) const;
    int RemoveStream(void *handle);
    ~FileStreamRegistry();
private:
    std::unordered_set<FileStream *> streams;
};

}