#include "logging.hpp"
#include "util.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace khiops_driver_common {
namespace logging {

static string logstring;
static ostringstream logstringstream;
static shared_ptr<spdlog::sinks::ostream_sink_st> stringstreamsink;
static shared_ptr<spdlog::sinks::stderr_sink_st> stderrsink;
static shared_ptr<spdlog::sinks::basic_file_sink_st> filesink;
static vector<shared_ptr<spdlog::sinks::sink>> sinks;
static shared_ptr<spdlog::logger> logger;

namespace {
// Logging lazy initializer
struct LogInitializer {
  LogInitializer(string loggername, string loglevelstr, string logfile, bool logtostderr) {
    spdlog::level::level_enum loglevel = spdlog::level::from_str(loglevelstr);

    logstring.clear();
    logstringstream = ostringstream("");
    stringstreamsink =
        make_shared<spdlog::sinks::ostream_sink_st>(logstringstream);
    stringstreamsink->set_level(spdlog::level::err);
    sinks.push_back(stringstreamsink);

    if (logtostderr) {
      stderrsink = make_shared<spdlog::sinks::stderr_sink_st>();
      stderrsink->set_level(loglevel);
      sinks.push_back(stderrsink);
    }

    if (!logfile.empty()) {
      filesink = make_shared<spdlog::sinks::basic_file_sink_st>(logfile);
      filesink->set_level(loglevel);
      sinks.push_back(filesink);
    }

    logger = make_shared<spdlog::logger>(loggername, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace); // Let the sinks choose the log level.
    spdlog::register_logger(logger);
  }

  ~LogInitializer() {
    logstring.clear();
    logstringstream = ostringstream("");
    stringstreamsink.reset();
    stderrsink.reset();
    filesink.reset();
    sinks.clear();
    logger.reset();
  }
};

} // anonymous namespace

const shared_ptr<spdlog::logger> &getLogger(string loggername, string loglevelstr, string logfile, bool logtostderr) {
  static LogInitializer _(loggername, loglevelstr, logfile, logtostderr);
  return logger;
}

const string &getLastError() {
  logstring = logstringstream.str();
  logstringstream.str("");
  logstringstream.clear();
  return logstring;
}

} // namespace logging
} // namespace khiops_driver_common
