#pragma once

namespace khiops_driver_common {

struct FileStream {
    enum struct Mode {NONE = 0, READ, WRITE, APPEND} mode;

    FileStream():
        mode(Mode::NONE)
    {}
};

int FileModeCharToFileStreamMode(FileStream::Mode *result, char mode) {
    if (mode == 'r') {
        *result = FileStream::Mode::READ;
    } else if (mode == 'w') {
        *result = FileStream::Mode::WRITE;
    } else if (mode == 'a') {
        *result = FileStream::Mode::APPEND;
    } else {
        return -1;
    }
    return 0;
}

}