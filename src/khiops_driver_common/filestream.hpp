#pragma once

#include "khiops_driver_common/logging.hpp"

namespace khiops_driver_common {

struct FileStream {
    enum struct Mode {NONE = 0, READ, WRITE, APPEND} mode;

    FileStream():
        mode(Mode::NONE)
    {}

    FileStream(FileStream &&source):
        mode(source.mode)
    {}
};

inline int FileModeCharToFileStreamMode(FileStream::Mode *result, char mode) {
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