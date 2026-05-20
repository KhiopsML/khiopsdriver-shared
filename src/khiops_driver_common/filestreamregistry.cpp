#include "khiops_driver_common/filestreamregistry.hpp"
#include "khiops_driver_common/logging.hpp"

using khiops_driver_common::logging::GetLogger;

namespace khiops_driver_common {
namespace filestream {

const FileStream *FileStreamRegistry::get_reader_stream(void *handle) const {
    FileStream *stream = this->get_stream(handle);
    if (stream->mode == FileStream::Mode::READ) {
        return stream;
    } else {
        GetLogger()->error("File stream with handle '{}' exists but is not a reader stream.", handle);
    }
}

const FileStream *FileStreamRegistry::get_writer_stream(void *handle) const {
    FileStream *stream = this->get_stream(handle);
    if (stream->mode == FileStream::Mode::WRITE) {
        return stream;
    } else {
        GetLogger()->error("File stream with handle '{}' exists but is not a writer stream.", handle);
    }
}

const FileStream *FileStreamRegistry::get_appender_stream(void *handle) const {
    FileStream *stream = this->get_stream(handle);
    if (stream->mode == FileStream::Mode::APPEND) {
        return stream;
    } else {
        GetLogger()->error("File stream with handle '{}' exists but is not an appender stream.", handle);
    }
}

const FileStream *FileStreamRegistry::get_stream(void *handle) const {
    auto it = streams.find(handle);
    if (it != streams.end()) {
        return &it->second;
    } else {
        GetLogger()->error("File stream with handle '{}' not found.", handle);
        return nullptr;
    }
}

}
}