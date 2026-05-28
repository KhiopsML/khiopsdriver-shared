#ifdef __CYGWIN__
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <memory>
#include <string>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include "khiops_driver_common/driver.h"
#include "khiops_driver_common/backend.hpp"
#include "khiops_driver_common/returnval.hpp"
#include "khiops_driver_common/logging.hpp"
#include "khiops_driver_common/util.hpp"
#include "khiops_driver_common/stringify.hpp"

// Compiling this file means we are currently compiling the driver, so export public functions.
#define CLOUD_STORAGE_DRIVER_EXPORT

using namespace std;

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
    unordered_map<void *, unique_ptr<FileReader>> file_readers;
    unordered_map<void *, unique_ptr<FileWriter>> file_writers;
};
static State *GetState() {
    static unique_ptr<State> state = nullptr;
    if (state == nullptr) {
        state = make_unique<State>();
        state->is_driver_initialized = false;
    }
    return state.get();
}

static bool FindFileReader(FileReader **result, void *handle) {
    auto it = GetState()->file_readers.find(handle);
    if (it != GetState()->file_readers.end()) {
        *result = it->second.get();
        return true;
    }
    return false;
}

static int GetFileReader(FileReader **result, void *handle) {
    if (FindFileReader(result, handle)) {
        return 0;
    } else {
        GetLogger()->error("No file open in read mode with handle {}.", handle);
    }
    return -1;
}

static bool FindFileWriter(FileWriter **result, void *handle) {
    auto it = GetState()->file_writers.find(handle);
    if (it != GetState()->file_writers.end()) {
        *result = it->second.get();
        return true;
    }
    return false;
}

static int GetFileWriter(FileWriter **result, void *handle) {
    if (FindFileWriter(result, handle)) {
        return 0;
    } else {
        GetLogger()->error("No file open in write or append mode with handle {}.", handle);
    }
    return -1;
}

// Function to check that an URL points to directory and log an error if it is not the case.
static bool CheckIsDirUrl(const std::string &url) {
    if (util::IsDirUrl(url)) {
        return true;
    } else {
        GetLogger()->error("URL {} indicates a file, not a directory.", url);
        return false;
    }
}

// Function to check that an URL points to file and log an error if it is not the case.
// "File" here refers to an non-directory object, not necessarily a file object stored in a file share (it can be a blob, too).
static bool CheckIsFileUrl(const std::string &url) {
    if (!util::IsDirUrl(url)) {
        return true;
    } else {
        GetLogger()->error("URL {} indicates a directory, not a file.", url);
        return false;
    }
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


using namespace khiops_driver_common;

/************************
 *** PUBLIC FUNCTIONS ***
 ************************/

const char *driver_getDriverName() {
    CATCH_ALL({
        GetLogger()->info("Retrieving driver name...");
        static string driver_name;
        if (GetDriverName(&driver_name) == 0) {
            return driver_name.c_str();
        }
    })
    return nullptr;
}

const char *driver_getVersion() {
    CATCH_ALL({
        GetLogger()->info("Retrieving driver version...");
        static string driver_version;
        if (GetDriverVersion(&driver_version) == 0) {
            return driver_version.c_str();
        }
    })
    return nullptr;
}

const char *driver_getScheme() {
    CATCH_ALL({
        GetLogger()->info("Retrieving driver scheme...");
        static string driver_scheme;
        if (GetDriverScheme(&driver_scheme) == 0) {
            return driver_scheme.c_str();
        }
    })
    return nullptr;
}

int driver_isReadOnly() {
    CATCH_ALL({
        GetLogger()->info("Retrieving read-only state...");
        bool is_readonly;
        if (IsReadOnly(&is_readonly) == 0) {
            return is_readonly ? kTrue : kFalse;
        }
    })
    return kFailure;
}

int driver_connect() {
    CATCH_ALL({
        GetLogger()->info("Connecting...");
        if (CheckNotInitialized() && Initialize() == 0) {
            GetState()->is_driver_initialized = true;
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_disconnect() {
    CATCH_ALL({
        GetLogger()->info("Disconnecting...");
        if (CheckInitialized() && Finalize() == 0) {
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
        if (GetSystemPreferredBufferSize(&buffer_size) == 0) {
            return static_cast<long long int>(buffer_size);
        }
    })
    return kFailure;
}

int driver_exist(const char *filename) {
    CATCH_ALL({
        GetLogger()->warn("Function {} is deprecated. Consider using the more specific driver_fileExists or driver_dirExists function.", __func__);
        GetLogger()->info("Checking if file or directory exists at URL {}...", filename);
        if (CheckInitialized() && CheckNotNull(filename, STRINGIFY(filename), __func__)) {
            if (util::IsDirUrl(filename)) {
                bool dir_exists;
                if (DirExists(&dir_exists, filename) == 0) {
                    return dir_exists ? kTrue : kFalse;
                }
            } else {
                bool file_exists;
                if (FileExists(&file_exists, filename) == 0) {
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
        if (CheckInitialized() && CheckNotNull(sFilePathName, STRINGIFY(sFilePathName), __func__) && CheckIsFileUrl(sFilePathName) && FileExists(&file_exists, sFilePathName) == 0) {
            return file_exists ? kTrue : kFalse;
        }
    })
    return kFailure;
}

int driver_dirExists(const char *sFilePathName) {
    CATCH_ALL({
        GetLogger()->info("Checking if directory exists at URL {}...", sFilePathName);
        bool dir_exists;
        if (CheckInitialized() && CheckNotNull(sFilePathName, STRINGIFY(sFilePathName), __func__) && CheckIsDirUrl(sFilePathName) && DirExists(&dir_exists, sFilePathName) == 0) {
            return dir_exists ? kTrue : kFalse;
        }
    })
    return kFailure;
}

long long int driver_getFileSize(const char *filename) {
    CATCH_ALL({
        GetLogger()->info("Retrieving size of file at URL {}...", filename);
        size_t file_size;
        if (CheckInitialized() && CheckNotNull(filename, STRINGIFY(filename), __func__) && CheckIsFileUrl(filename) && GetFileSize(&file_size, filename) == 0) {
            return static_cast<long long int>(file_size);
        }
    })
    return kFailure;
}

void *driver_fopen(const char *filename, char mode) {
    CATCH_ALL({
        GetLogger()->info("Opening file at URL {} in mode {}...", filename, mode);
        if (
            CheckInitialized() && CheckNotNull(filename, STRINGIFY(filename), __func__) && CheckIsFileUrl(filename)
         ) {
            if (mode == 'r') {
                unique_ptr<FileReader> file_reader = make_unique<FileReader>();
                if (PopulateFileReader(file_reader.get(), filename) == 0) {
                    if (GetState()->file_readers.insert({static_cast<void *>(file_reader.get()), std::move(file_reader)}).second) {
                        return static_cast<void *>(file_reader.get());
                    } else {
                        GetLogger()->error("Failed to register file stream.");
                    }
                }
            } else if (mode == 'w' || mode == 'a') {
                unique_ptr<FileWriter> file_writer = make_unique<FileWriter>();
            } else {
                GetLogger()->error("Invalid file stream mode '{}'.", mode);
            }
            // stream.url = filename;
            // if (FOpen(stream, filename) == 0 && GetState()->file_stream_registry.AddStream(&handle, std::move(stream)) == 0) {
            //     return handle;
            // }
        }
    })
    return nullptr;
}

int driver_fclose(void *stream) {
    CATCH_ALL({
        GetLogger()->info("Closing file with handle {}...", stream);
        if (CheckInitialized() && CheckNotNull(stream, STRINGIFY(stream), __func__)) {
            FileReader *file_reader;
            FileWriter *file_writer;
            if (FindFileReader(&file_reader, stream)) {
                if (FCloseReader(*file_reader) == 0 && GetState()->file_readers.erase(stream) == 1ULL) {
                    return kSuccess;
                } else {
                    GetLogger()->error("Failed to unregister file stream.");
                }
            } else if (FindFileWriter(&file_writer, stream)) {
                if (FCloseWriter(*file_writer) == 0 && GetState()->file_writers.erase(stream) == 1ULL) {
                    return kSuccess;
                } else {
                    GetLogger()->error("Failed to unregister file stream.");
                }
            } else {
                GetLogger()->error("No file open with handle {}.", stream);
            }
        }
    })
    return kFailure;
}

long long int driver_fread(void *ptr, size_t size, size_t count, void *stream) {
    CATCH_ALL({
        GetLogger()->info("Reading {}x{} bytes from file with handle {} to {}...", size, count, stream, ptr);
        FileReader *file_reader;
        size_t nread;
        if (
            CheckInitialized() && CheckNotNull(ptr, STRINGIFY(ptr), __func__) && CheckNotNull(stream, STRINGIFY(stream), __func__)
            && GetFileReader(&file_reader, stream) == 0
            && FRead(&nread, ptr, *file_reader, size, count) == 0 && nread != 0ULL
        ) {
            return static_cast<long long int>(nread);
        }
    })
    return kFailure;
}

int driver_fseek(void *stream, long long int offset, int whence) {
    CATCH_ALL({
        GetLogger()->info("Seeking offset {} from origin {} in file with handle {}...", offset, whence, stream);
        FileReader *file_reader;
        if (CheckInitialized() && CheckNotNull(stream, STRINGIFY(stream), __func__)) {
            if (whence == ios::beg || whence == ios::cur || whence == ios::end) {
                if (GetFileReader(&file_reader, stream) == 0) {
                    if (whence == ios::beg) {
                        if (0LL <= offset && offset <= file_reader->total_size) {
                            file_reader->current_position = static_cast<size_t>(offset);
                            return kSuccess;
                        }
                    } else if (whence == ios::cur) {
                        if (offset < 0LL) {
                            size_t positive_offset = static_cast<size_t>(-(offset + 1LL)) + 1ULL;
                            if (positive_offset <= file_reader->current_position) {
                                file_reader->current_position -= positive_offset;
                                return kSuccess;
                            }
                        } else {
                            if (static_cast<size_t>(offset) <= file_reader->total_size - file_reader->current_position) {
                                file_reader->current_position += static_cast<size_t>(offset);
                                return kSuccess;
                            }
                        }
                    } else if (whence == ios::end) {
                        if (offset < 0LL) {
                            size_t positive_offset = static_cast<size_t>(-(offset + 1LL)) + 1ULL;
                            if (positive_offset <= file_reader->total_size) {
                                file_reader->current_position = file_reader->total_size - positive_offset;
                                return kSuccess;
                            }
                        } else if (offset == 0LL) {
                            return kSuccess;
                        }
                    }
                    GetLogger()->error("Seeking out of file's range.");
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
        static string last_error = GetLastError();
        return last_error.empty() ? nullptr : last_error.c_str();
    })
    return "Error while trying to fetch last error.";
}

long long int driver_fwrite(const void *ptr, size_t size, size_t count, void *stream) {
    CATCH_ALL({
        GetLogger()->info("Writing {}x{} bytes from {} to file with handle {}...", size, count, ptr, stream);
        FileWriter *file_writer;
        size_t nwritten;
        if (
            CheckInitialized() && CheckNotNull(ptr, STRINGIFY(ptr), __func__) && CheckNotNull(stream, STRINGIFY(stream), __func__)
            && GetFileWriter(&file_writer, stream) == 0 && FWrite(&nwritten, *file_writer, ptr, size, count) == 0
        ) {
            return static_cast<long long int>(nwritten);
        }
    })
    return kFailure;
}

int driver_fflush(void *stream) {
    CATCH_ALL({
        GetLogger()->info("Flushing file with handle {}...", stream);
        FileWriter *file_writer;
        if (
            CheckInitialized() && CheckNotNull(stream, STRINGIFY(stream), __func__)
            && GetFileWriter(&file_writer, stream) == 0 && FFlush(*file_writer)
        ) {
            return kSuccess;
        }
    })
    return kFailure;
}

int driver_remove(const char *filename) {
    CATCH_ALL({
        GetLogger()->info("Removing file at URL {}...", filename);
        if (CheckInitialized() && CheckNotNull(filename, STRINGIFY(filename), __func__) && CheckIsFileUrl(filename) && Remove(filename) == 0) {
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_mkdir(const char *pathname) {
    CATCH_ALL({
        GetLogger()->info("Creating directory at URL {}...", pathname);
        if (CheckInitialized() && CheckNotNull(pathname, STRINGIFY(pathname), __func__) && CheckIsDirUrl(pathname) && Mkdir(pathname) == 0) {
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_rmdir(const char *pathname) {
    CATCH_ALL({
        GetLogger()->info("Removing directory at URL {}...", pathname);
        if (CheckInitialized() && CheckNotNull(pathname, STRINGIFY(pathname), __func__) && CheckIsDirUrl(pathname) && Rmdir(pathname) == 0) {
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

long long int driver_diskFreeSpace(const char *filename) {
    CATCH_ALL({
        GetLogger()->info("Retrieving free disk space at URL {}...", filename);
        size_t free_space;
        if (CheckInitialized() && CheckNotNull(filename, STRINGIFY(filename), __func__) && DiskFreeSpace(&free_space, filename) == 0) {
            return free_space;
        }
    })
    return kFailure;
}

int driver_copyToLocal(const char *sourcefilename, const char *destfilename) {
    CATCH_ALL({
        GetLogger()->info("Copying file at URL {} to URL {}...", sourcefilename, destfilename);
        if (
            CheckInitialized() && CheckNotNull(sourcefilename, STRINGIFY(sourcefilename), __func__) && CheckNotNull(destfilename, STRINGIFY(destfilename), __func__)
            && CheckIsFileUrl(sourcefilename) && CheckIsFileUrl(destfilename)
            && CopyToLocal(sourcefilename, destfilename) == 0
        ) {
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_copyFromLocal(const char *sourcefilename, const char *destfilename) {
    CATCH_ALL({
        GetLogger()->info("Copying file at URL {} to URL {}...", sourcefilename, destfilename);
        if (
            CheckInitialized() && CheckNotNull(sourcefilename, STRINGIFY(sourcefilename), __func__) && CheckNotNull(destfilename, STRINGIFY(destfilename), __func__)
            && CheckIsFileUrl(sourcefilename) && CheckIsFileUrl(destfilename)
            && CopyFromLocal(sourcefilename, destfilename) == 0
        ) {
            return kOtherSuccess;
        }
    })
    return kOtherFailure;
}

int driver_concat(const char *destfilename, const char **sourcefilenames, size_t sourcefilecount) {
    CATCH_ALL({
        GetLogger()->info("Concatenating {} files to URL {}...", sourcefilecount, destfilename);
        for (size_t i = 0; i < sourcefilecount; i++) {
            GetLogger()->info("  Source file #{}: {}", i + 1, sourcefilenames[i]);
        }
        if (
            CheckInitialized() && CheckNotNull(destfilename, STRINGIFY(destfilename), __func__)
            && CheckNotNull(sourcefilenames, STRINGIFY(sourcefilenames), __func__)
            && sourcefilecount >= 2
        ) {
            if (Concat(destfilename, vector<string>(sourcefilenames, sourcefilenames + sourcefilecount)) == 0) {
                return kOtherSuccess;
            }
        } else {
            GetLogger()->error("Too few files to concatenate.");
        }
    })
    return kOtherFailure;
}

int driver_composeMultifile(const char *sDestFilePathName, const char **sSourceFilePathNames, size_t nSourceFileCount) {
    CATCH_ALL({
        GetLogger()->info("Composing {} files into one multifile {}...", nSourceFileCount, sDestFilePathName);
        for (size_t i = 0; i < nSourceFileCount; i++) {
            GetLogger()->info("  Source file #{}: {}", i + 1, sSourceFilePathNames[i]);
        }
        if (
            CheckInitialized() && CheckNotNull(sDestFilePathName, STRINGIFY(sDestFilePathName), __func__)
            && CheckNotNull(sSourceFilePathNames, STRINGIFY(sSourceFilePathNames), __func__)
            && nSourceFileCount >= 2
        ) {
            if (Concat(sDestFilePathName, vector<string>(sSourceFilePathNames, sSourceFilePathNames + nSourceFileCount)) == 0) {
                return kOtherSuccess;
            }
        } else {
            GetLogger()->error("Too few files to compose a multifile.");
        }
    })
    return kOtherFailure;
}