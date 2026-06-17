/*
    The global state of the driver.
*/

#pragma once

#include <string>
#include <memory>
#include "khiops_driver_common/filestream_management.hpp"

namespace khiops_driver_common {

struct State {
    bool is_driver_initialized;
    OpenFileStreamCollection open_file_streams;
};

inline State *GetState() {
    static std::unique_ptr<State> state = nullptr;
    if (!state) {
        state = std::unique_ptr<State>(new State());
        state->is_driver_initialized = false;
    }
    return state.get();
}

}