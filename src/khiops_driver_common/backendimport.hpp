#pragma once

#include "backend.hpp"

namespace khiops_driver_common {

// Reference to the backend object provided by the driver.
extern const Backend backend;
// Alias for a frequently used function.
auto &GetLogger = backend.GetLogger;

}