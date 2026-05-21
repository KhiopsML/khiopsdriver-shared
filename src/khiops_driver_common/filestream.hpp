#pragma once

namespace khiops_driver_common {

struct FileStream {
    enum struct Mode {NONE = 0, READ, WRITE, APPEND} mode;

    FileStream():
        mode(Mode::NONE)
    {}
};

}