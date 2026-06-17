#pragma once

#include <spdlog/spdlog.h>
#include <string>

namespace khiops_driver_common {

// To be implemtented by the driver.
spdlog::logger *GetLogger();

spdlog::logger *GetLogger(const std::string &logger_name, const std::string &logfile_envvar_name, const std::string &loglevel_envvar_name);

std::string GetLastError();

} // namespace khiops_driver_common
