#ifdef __CYGWIN__
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <memory>
#include <string>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include "khiops_driver_common/driver.h"
#include "khiops_driver_common/backend.hpp"
#include "khiops_driver_common/backendimport.hpp"
#include "khiops_driver_common/returnval.hpp"
#include "khiops_driver_common/logging.hpp"
#include "khiops_driver_common/filestreamregistry.hpp"

// Compiling this file means we are currently compiling the driver, so export public functions.
#define CLOUD_STORAGE_DRIVER_EXPORT

using namespace std;
using namespace khiops_driver_common;

// Macro that must be used in all public functions to avoid raising exceptions
#define CATCH_ALL(stmt) \
    try { \
        stmt \
    } catch (const exception &exc) { \
        GetLogger()->error("An exception has been raised: {}", exc.what()); \
    } catch (...) { \
        GetLogger()->error("An non-exception value has been raised as an exception."); \
    }

namespace khiops_driver_common {

// Global state
struct State {
    bool is_driver_initialized;
    FileStreamRegistry file_stream_registry;
};
static State *GetState() {
    static unique_ptr<State> state = nullptr;
    if (state == nullptr) {
        state = make_unique<State>();
        state->is_driver_initialized = false;
    }
    return state.get();
}

// Function to check that the driver has been initialized and log an error if it is not the case
static bool CheckInitialized() {
    if (GetState()->is_driver_initialized) {
        return true;
    } else {
        GetLogger()->error("Operation cannot be performed when disconnected.");
        return false;
    }
}

// Function to check that the driver has NOT been initialized and log an error if it has been initialized
static bool CheckNotInitialized() {
    if (!GetState()->is_driver_initialized) {
        return true;
    } else {
        GetLogger()->error("Operation cannot be performed when connected.");
        return false;
    }
}

// Function to check that an argument is not a null pointer and log and error if it is a null pointer
static bool CheckNotNull(const void *arg, const char *param, const char *func) {
    if (arg != nullptr) {
        return true;
    } else {
        GetLogger()->error("Error calling function '{}': passing null pointer as argument '{}'.", func, param);
        return false;
    }
}

} // namespace khiops_driver_common



/************************
 *** PUBLIC FUNCTIONS ***
 ************************/

const char *driver_getDriverName() {
    CATCH_ALL({
        GetLogger()->info("Retrieving driver name...");
        string driver_name;
        if (backend.GetDriverName(&driver_name) == 0) {
            return driver_name.c_str()
        }
    })
    return nullptr;
}

const char *driver_getVersion() {
    CATCH_ALL({
        GetLogger()->info("Retrieving driver version...");
        string driver_version;
        if (backend.GetDriverVersion(&driver_version) == 0) {
            return driver_version.c_str();
        }
    })
    return nullptr;
}

const char *driver_getScheme() {
    CATCH_ALL({
        GetLogger()->info("Retrieving driver scheme...");
        string driver_scheme;
        if (backend.GetDriverScheme(&driver_scheme) == 0) {
            return driver_scheme.c_str();
        }
    })
    return nullptr;
}

int driver_isReadOnly() {
    CATCH_ALL({
        GetLogger()->info("Retrieving read-only state...");
        bool is_readonly;
        if (backend.IsReadOnly(&is_readonly) == 0) {
            return is_readonly ? kTrue : kFalse;
        }
    })
    return kFailure;
}

int driver_connect() {
    CATCH_ALL({
        GetLogger()->info("Connecting...");
        if (CheckNotInitialized() && backend.Initialize() == 0) {
            GetState()->is_driver_initialized = true;
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_disconnect() {
    CATCH_ALL({
        GetLogger()->info("Disconnecting...");
        if (CheckInitialized() && backend.Finalize() == 0) {
            GetState()->is_driver_initialized = false;
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_isConnected() {
    CATCH_ALL({
        GetLogger()->info("Retrieving connection state...");
        return GetState()->is_driver_initialized ? kTrue : kFalse;
    })
    return kFailure;
}

long long int driver_getSystemPreferredBufferSize() {
    CATCH_ALL({
        GetLogger()->info("Retrieving preferred buffer size...");
        size_t buffer_size;
        if (backend.GetSystemPreferredBufferSize(&buffer_size) == 0) {
            return static_cast<long long int>(buffer_size);
        }
    })
    return kFailure;
}

int driver_exist(const char *filename) {
    CATCH_ALL({
        GetLogger()->warn("Function {} is deprecated. Consider using the more specific driver_fileExists or driver_dirExists function.", __func__);
        GetLogger()->info("Checking if file or directory exists at URL {}...", filename);
        if (CheckInitialized() && CheckNotNull(filename, KHIOPS_STR(filename), __func__)) {
            string filename_as_string = filename;
            if (filename_as_string.size() > 0 && filename_as_string[-1] == '/') {  // Directory
                bool dir_exists;
                if (backend.DirExists(&dir_exists, filename_as_string) == 0) {
                    return dir_exists ? kTrue : kFalse;
                }
            } else {  // File
                bool file_exists;
                if (backend.FileExists(&file_exists, filename_as_string) == 0) {
                    return file_exists ? kTrue : kFalse;
                }
            }
        }
    })
    return kFailure;
}

int driver_fileExists(const char *sFilePathName) {
    CATCH_ALL({
        GetLogger()->info("Checking if file exists at URL {}...", sFilePathName);
        bool file_exists;
        if (CheckInitialized() && CheckNotNull(sFilePathName, KHIOPS_STR(sFilePathName), __func__) && backend.FileExists(&file_exists, sFilePathName) == 0) {
            return file_exists ? kTrue : kFalse;
        }
    })
    return kFailure;
}

int driver_dirExists(const char *sFilePathName) {
    CATCH_ALL({
        GetLogger()->info("Checking if directory exists at URL {}...", sFilePathName);
        bool dir_exists;
        if (CheckInitialized() && CheckNotNull(sFilePathName, KHIOPS_STR(sFilePathName), __func__) && backend.DirExists(&dir_exists, sFilePathName) == 0) {
            return dir_exists ? kTrue : kFalse;
        }
    })
    return kFailure;
}

long long int driver_getFileSize(const char *filename) {
    CATCH_ALL({
        GetLogger()->info("Retrieving size of file at URL {}...", filename);
        size_t file_size;
        if (CheckInitialized() && CheckNotNull(filename, KHIOPS_STR(filename), __func__) && backend.GetFileSize(&file_size, filename) == 0) {
            return static_cast<long long int>(file_size);
        }
    })
    return kFailure;
}

void *driver_fopen(const char *filename, char mode) {
    CATCH_ALL({
        GetLogger()->info("Opening file at URL {} in mode {}...", filename, mode);

        if (CheckInitialized() && CheckNotNull(filename, KHIOPS_STR(filename), __func__)) {
            FileStream stream;
            if (FileModeCharToFileStreamMode(&stream->mode, mode) == 0) {
                if (backend.FOpen(&stream, filename) == 0) {
                    void *handle;
                    if (GetState()->file_stream_registry.AddStream(&handle, std::move(stream)) == 0) {
                        return handle;
                    }
                }
            } else {
                GetLogger()->error("Tried to open file '{}' with invalid mode '{}'.", filename, mode);
            }
        }
    })
    return nullptr;
}

int driver_fclose(void *stream) {
    CATCH_ALL({
        GetLogger()->info("Closing file with handle {}...", stream);
        if (CheckInitialized() && CheckNotNull(stream, KHIOPS_STR(stream), __func__) && backend.FClose(stream) == 0) {
            GetState()->streams.erase(stream);
            return kSuccess;
        }
    })
    return kFailure;
}

long long int driver_fread(void *ptr, size_t size, size_t count, void *stream) {
    CATCH_ALL({
        GetLogger()->info("Reading {}x{} bytes from file with handle {} to {}...", size, count, stream, ptr);
        size_t nread;
        if (CheckInitialized() && CheckNotNull(ptr, KHIOPS_STR(ptr), __func__) && CheckNotNull(stream, KHIOPS_STR(stream), __func__)) {
            FileStream *file_stream = GetState()->file_stream_registry.GetReaderStream(stream);
            if (file_stream != nullptr && backend.FRead(&nread, ptr, size, count, *file_stream) == 0 && nread != 0ULL) {
                return static_cast<long long int>(nread);
            }
        }
    })
    return kFailure;
}

int driver_fseek(void *stream, long long int offset, int whence) {
    CATCH_ALL({
        GetLogger()->info("Seeking offset {} from origin {} in file with handle {}...", offset, whence, stream);
        if (CheckInitialized() && CheckNotNull(stream, KHIOPS_STR(stream), __func__)) {
            if (0 <= whence && whence <= 2) {
                if (backend.FSeek(stream, offset, whence) == 0) {
                    return kSuccess;
                }
            } else {
                GetLogger()->error("Tried to seek from invalid origin '{}'.", whence);
            }
        }
    })
    return kFailure;
}

const char *driver_getlasterror() {
    CATCH_ALL({
        GetLogger()->info("Retrieving last error...");
        string last_error = GetLastError();
        return last_error.empty() ? nullptr : last_error.c_str();
    })
    return "Error while trying to fetch last error.";
}

long long int driver_fwrite(const void *ptr, size_t size, size_t count, void *stream) {
}

int driver_fflush(void *stream) {
}

int driver_remove(const char *filename) {
}

int driver_mkdir(const char *pathname) {
}

int driver_rmdir(const char *pathname) {
}

long long int driver_diskFreeSpace(const char *filename) {
}

int driver_copyToLocal(const char *sourcefilename, const char *destfilename) {
}

int driver_copyFromLocal(const char *sourcefilename, const char *destfilename) {
}

int driver_concat(const char *destfilename, const char **sourcefilenames, size_t sourcefilecount) {
}

int driver_composeMultifile(const char *sDestFilePathName, const char **sSourceFilePathNames, size_t nSourceFileCount) {
}