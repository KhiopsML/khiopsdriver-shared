#include "khiops_driver_common/util.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

/*** String stream logger for driver_getlasterror ***/
static ostringstream logstringstream;
static shared_ptr<spdlog::sinks::ostream_sink_st> stringstreamsink;

/*** File logger used if the logfile is set using the driver-specific environment variable and is not an empty string, and if the loglevel is set using the driver-specific environment variable and is not "off" ***/
static shared_ptr<spdlog::sinks::basic_file_sink_st> filesink;

namespace khiops_driver_common {

spdlog::logger *GetLogger(const std::string &logger_name, const string &logfile_envvar_name, const string &loglevel_envvar_name) {
  static shared_ptr<spdlog::logger> logger = nullptr;
  if (logger == nullptr) {
    /*** Create an ampty vector of sinks. ***/
    vector<shared_ptr<spdlog::sinks::sink>> sinks;

    /*** Create the string stream logger and add it to the vector of sinks. ***/
    logstringstream = ostringstream("");
    stringstreamsink = make_shared<spdlog::sinks::ostream_sink_st>(logstringstream);
    stringstreamsink->set_level(spdlog::level::err);
    sinks.push_back(stringstreamsink);

    /*** Create the file logger if configured and add it to the vector of sinks. ***/
    string logfile = GetEnvVar(logfile_envvar_name, true);
    if (!logfile.empty()) {
      spdlog::level::level_enum loglevel = spdlog::level::from_str(GetEnvVarOrDefault(loglevel_envvar_name, "off", true));
      filesink = make_shared<spdlog::sinks::basic_file_sink_st>(logfile);
      filesink->set_level(loglevel);
      sinks.push_back(filesink);
    }

    /*** Create the logger with the vector of sinks. ***/
    logger = make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);  // Set loglevel to the most verbose one to let the sinks choose the actual log level.
    spdlog::register_logger(logger);
  }
  return logger.get();
}

string GetLastError() {
  string logstring = logstringstream.str();
  logstringstream.str("");
  logstringstream.clear();
  return logstring;
}

} // namespace khiops_driver_common
