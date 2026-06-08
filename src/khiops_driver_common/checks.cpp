#include "khiops_driver_common/checks.hpp"
#include "khiops_driver_common/util.hpp"
#include "khiops_driver_common/globalstate.hpp"
#include "khiops_driver_common/logging.hpp"

namespace khiops_driver_common {

int CheckIsDirUrl(const std::string &url) {
    if (!IsDirUrl(url)) {
        GetLogger()->error("URL {} indicates a file, not a directory.", url);
        return -1;
    }
    return 0;
}

int CheckIsFileUrl(const std::string &url) {
    if (IsDirUrl(url)) {
        GetLogger()->error("URL {} indicates a directory, not a file.", url);
        return -1;
    }
    return 0;
}

int CheckInitialized() {
    if (!GetState()->is_driver_initialized) {
        GetLogger()->error("Operation cannot be performed when disconnected.");
        return -1;
    }
    return 0;
}

int CheckNotInitialized() {
    if (GetState()->is_driver_initialized) {
        GetLogger()->error("Operation cannot be performed when connected.");
        return -1;
    }
    return 0;
}

int CheckNotNull(const void *arg, const char *param, const char *func) {
    if (arg == nullptr) {
        GetLogger()->error("Error calling function '{}': passing null pointer as argument '{}'.", func, param);
        return -1;
    }
    return 0;
}

}