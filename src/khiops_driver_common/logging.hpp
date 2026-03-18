#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace khiops_driver_common {
namespace logging {

const std::shared_ptr<spdlog::logger> &getLogger(std::string loggername, std::string loglevelstr, std::string logfile, bool logtostderr);
const std::string &getLastError();

} // namespace logging
} // namespace khiops_driver_common
