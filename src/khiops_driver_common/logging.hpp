#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include "khiops_driver_common/backend.hpp"

namespace khiops_driver_common {

const spdlog::logger *GetLogger(const std::string &logger_name, const std::string &logfile_envvar_name, const std::string &loglevel_envvar_name);

std::string GetLastError();

} // namespace khiops_driver_common
