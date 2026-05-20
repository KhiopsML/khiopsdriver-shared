#pragma once

#include <spdlog/spdlog.h>
#include <string>

namespace khiops_driver_common {
namespace logging {

const spdlog::logger *GetLogger(const std::string &logger_name, const std::string &logfile_envvar_name, const std::string &loglevel_envvar_name);

// Reference to the GetLogger function provided by the driver.
extern const spdlog::Logger *GetLogger();

std::string GetLastError();

} // namespace logging
} // namespace khiops_driver_common
