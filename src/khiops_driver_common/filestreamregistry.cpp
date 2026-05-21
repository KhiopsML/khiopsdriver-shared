#include "khiops_driver_common/filestreamregistry.hpp"
#include "khiops_driver_common/backendimport.hpp"

using khiops_driver_common::GetLogger;

namespace khiops_driver_common {

int FileStreamRegistry::AddStream(void **handle_result, FileStream &&file_stream) {
    auto insertion_result = this->streams.insert(new FileStream(file_stream));
    if (insertion_result.second) {
        *handle_result = static_cast<void *>(*insertion_result.first);
        return 0;
    } else {
        GetLogger()->error("Failed to register file stream.");
    }
    return -1;
}

int FileStreamRegistry::GetReaderStream(const FileStream **result, void *handle) const {
    if (this->GetStream(result, handle) == 0) {
        if (stream->mode == FileStream::Mode::READ) {
            return 0;
        } else {
            GetLogger()->error("File stream exists but is not a reader stream.");
        }
    }
    return -1;
}

int FileStreamRegistry::GetWriterStream(const FileStream **result, void *handle) const {
    if (this->GetStream(result, handle) == 0) {
        if (stream->mode == FileStream::Mode::WRITE) {
            return 0;
        } else {
            GetLogger()->error("File stream exists but is not a writer stream.");
        }
    }
    return -1;
}

int FileStreamRegistry::GetAppenderStream(const FileStream **result, void *handle) const {
    if (this->GetStream(result, handle) == 0) {
        if (stream->mode == FileStream::Mode::APPEND) {
            return 0;
        } else {
            GetLogger()->error("File stream exists but is not an appender stream.");
        }
    }
    return -1;
}

int FileStreamRegistry::RemoveStream(void *handle) {
    FileStream *file_stream = static_cast<FileStream *>(handle);
    if (this->streams.erase(file_stream) == 1ULL) {
        delete file_stream;
        return 0;
    } else {
        GetLogger()->error("Failed to unregister file stream.");
    }
    return -1;
}

FileStreamRegistry::~FileStreamRegistry() {
    for (const auto &file_stream : this->streams) {
        delete file_stream;
    }
}

int FileStreamRegistry::GetStream(const FileStream **result, void *handle) const {
    auto it = this->streams.find(static_cast<FileStream *>(handle));
    if (it != this->streams.end()) {
        *result = *it;
        return 0;
    } else {
        GetLogger()->error("File stream not found.");
    }
    return -1;
}

}