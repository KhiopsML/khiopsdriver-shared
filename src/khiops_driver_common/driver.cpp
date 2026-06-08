#ifdef __CYGWIN__
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <memory>
#include <string>
#include <unordered_map>
#include <fstream>
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
// It is variadic just to avoid splitting the code on commas outside of parentheses (otherwise the preprocessor thinks there are multiple macro arguments).
#define CATCH_ALL(...) \
    do { \
        try { \
            __VA_ARGS__ \
        } catch (const exception &exc) { \
            GetLogger()->error("An exception has been raised: {}", exc.what()); \
        } catch (...) { \
            GetLogger()->error("An unknown exception has been raised."); \
        } \
        return KO; \
    } while(0)

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
    const char *const KO = nullptr;
    CATCH_ALL(
        GetLogger()->info("Retrieving driver name...");
        static string driver_name;
        if (GetDriverName(&driver_name) != 0) return KO;
        return driver_name.c_str();
    );
}

const char *driver_getVersion() {
    const char *const KO = nullptr;
    CATCH_ALL(
        GetLogger()->info("Retrieving driver version...");
        static string driver_version;
        if (GetDriverVersion(&driver_version) != 0) return KO;
        return driver_version.c_str();
    );
}

const char *driver_getScheme() {
    const char *const KO = nullptr;
    CATCH_ALL(
        GetLogger()->info("Retrieving driver scheme...");
        static string driver_scheme;
        if (GetDriverScheme(&driver_scheme) != 0) return KO;
        return driver_scheme.c_str();
    );
}

int driver_isReadOnly() {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Retrieving read-only state...");
        bool is_readonly;
        if (IsReadOnly(&is_readonly) != 0) return KO;
        return is_readonly ? kTrue : kFalse;
    );
}

int driver_connect() {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Connecting...");
        if (!CheckNotInitialized()) return KO;
        if (Initialize() != 0) return KO;
        GetState()->is_driver_initialized = true;
        return kOtherSuccess;
    );
}

int driver_disconnect() {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Disconnecting...");
        if (!CheckInitialized()) return KO;
        if (Finalize() != 0) return KO;
        GetState()->is_driver_initialized = false;
        return kOtherSuccess;
    );
}

int driver_isConnected() {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Retrieving connection state...");
        return GetState()->is_driver_initialized ? kTrue : kFalse;
    );
}

long long int driver_getSystemPreferredBufferSize() {
    const long long int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Retrieving preferred buffer size...");
        size_t buffer_size;
        if (GetSystemPreferredBufferSize(&buffer_size) != 0) return KO;
        return static_cast<long long int>(buffer_size);
    );
}

int driver_exist(const char *filename) {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->warn("Function {} is deprecated. Consider using the more specific driver_fileExists or driver_dirExists function.", __func__);
        GetLogger()->info("Checking if file or directory exists at URL {}...", filename);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(filename, STRINGIFY(filename), __func__)) return KO;
        if (util::IsDirUrl(filename)) {
            bool dir_exists;
            if (DirExists(&dir_exists, filename) != 0) return KO;
            return dir_exists ? kTrue : kFalse;
        } else {
            bool file_exists;
            if (FileExists(&file_exists, filename) != 0) return KO;
            return file_exists ? kTrue : kFalse;
        }
    );
}

int driver_fileExists(const char *sFilePathName) {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Checking if file exists at URL {}...", sFilePathName);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(sFilePathName, STRINGIFY(sFilePathName), __func__)) return KO;
        if (!CheckIsFileUrl(sFilePathName)) return KO;
        bool file_exists;
        if (FileExists(&file_exists, sFilePathName) != 0) return KO;
        return file_exists ? kTrue : kFalse;
    );
}

int driver_dirExists(const char *sFilePathName) {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Checking if directory exists at URL {}...", sFilePathName);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(sFilePathName, STRINGIFY(sFilePathName), __func__)) return KO;
        if (!CheckIsDirUrl(sFilePathName)) return KO;
        bool dir_exists;
        if (DirExists(&dir_exists, sFilePathName) != 0) return KO;
        return dir_exists ? kTrue : kFalse;
    );
}

long long int driver_getFileSize(const char *filename) {
    const long long int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Retrieving size of file at URL {}...", filename);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(filename, STRINGIFY(filename), __func__)) return KO;
        if (!CheckIsFileUrl(filename)) return KO;
        size_t file_size;
        if (GetFileSize(&file_size, filename) != 0) return KO;
        return static_cast<long long int>(file_size);
    );
}

void *driver_fopen(const char *filename, char mode) {
    void *const KO = nullptr;
    CATCH_ALL(
        GetLogger()->info("Opening file at URL {} in mode '{}'...", filename, mode);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(filename, STRINGIFY(filename), __func__)) return KO;
        if (!CheckIsFileUrl(filename)) return KO;
        if (mode != 'r' && mode != 'w' && mode != 'a') {
            GetLogger()->error("Cannot open file with invalid mode '{}'.", mode);
            return KO;
        }
        if (mode == 'r') {
            unique_ptr<FileReader> file_reader = make_unique<FileReader>();
            void *handle = static_cast<void *>(file_reader.get());
            if (PopulateFileReader(file_reader.get(), filename) != 0) return KO;
            if (!GetState()->file_readers.insert({handle, std::move(file_reader)}).second) {
                GetLogger()->error("Failed to register file stream.");
                return KO;
            }
            return handle;
        } else if (mode == 'w' || mode == 'a') {
            unique_ptr<FileWriter> file_writer = make_unique<FileWriter>();
            void *handle = static_cast<void *>(file_writer.get());
            file_writer->url = filename;
            if (mode == 'w') {
                file_writer->current_position = 0;
                if (InitializeFileWriterWithWriteMode(file_writer.get()) != 0) return KO;
            } else if (mode == 'a') {
                size_t file_size;
                if (GetFileSize(&file_size, filename) != 0) return KO;
                file_writer->current_position = file_size;
                if (InitializeFileWriterWithAppendMode(file_writer.get()) != 0) return KO;
            }
            if (!GetState()->file_writers.insert({handle, std::move(file_writer)}).second) {
                GetLogger()->error("Failed to register file stream.");
                return KO;
            }
            return handle;
        } else {
            // Should never happen but need to make the compiler happy.
            return KO;
        }
    );
}

int driver_fclose(void *stream) {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Closing file with handle {}...", stream);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(stream, STRINGIFY(stream), __func__)) return KO;
        FileReader *file_reader; FileWriter *file_writer;
        if (FindFileReader(&file_reader, stream)) {
            if (FCloseReader(*file_reader) != 0) return KO;
            if (GetState()->file_readers.erase(stream) == 0ULL) {
                GetLogger()->error("Failed to unregister file stream.");
                return KO;
            } else {
                return kSuccess;
            }
        } else if (FindFileWriter(&file_writer, stream)) {
            if (FCloseWriter(*file_writer) != 0) return KO;
            if (GetState()->file_writers.erase(stream) == 0ULL) {
                GetLogger()->error("Failed to unregister file stream.");
                return KO;
            } else {
                return kSuccess;
            }
        } else {
            GetLogger()->error("No file open with handle {}.", stream);
            return KO;
        }
    );
}

long long int driver_fread(void *ptr, size_t size, size_t count, void *stream) {
    const long long int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Reading {}x{} bytes from file with handle {} to buffer at {}...", size, count, stream, ptr);
        
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(ptr, STRINGIFY(ptr), __func__)) return KO;
        if (!CheckNotNull(stream, STRINGIFY(stream), __func__)) return KO;
        FileReader *file_reader;
        if (GetFileReader(&file_reader, stream) != 0) return KO;
        if (size != 0 && count > numeric_limits<size_t>::max() / size) { GetLogger()->error("'size' x 'count' exceeds {}.", numeric_limits<size_t>::max()); return KO; }
        
        size_t nread;
        if (khiops_driver_common::FRead(&nread, ptr, file_reader, size, count) != 0) return KO;

        return static_cast<long long int>(nread);
    );
}

int driver_fseek(void *stream, long long int offset, int whence) {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Seeking offset {} from origin {} in file with handle {}...", offset, whence, stream);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(stream, STRINGIFY(stream), __func__)) return KO;
        FileReader *file_reader;
        if (whence != ios::beg && whence != ios::cur && whence != ios::end) {
            GetLogger()->error("Tried to seek from invalid origin '{}'.", whence);
            return KO;
        }
        if (GetFileReader(&file_reader, stream) != 0) return KO;
        bool seek_out_of_range = false;
        if (whence == ios::beg) {
            if (offset < 0LL || static_cast<size_t>(offset) > file_reader->total_size) {
                seek_out_of_range = true;
            } else {
                file_reader->current_position = static_cast<size_t>(offset);
            }
        } else if (whence == ios::cur) {
            if (offset < 0LL) {
                size_t positive_offset = static_cast<size_t>(-(offset + 1LL)) + 1ULL;
                if (positive_offset > file_reader->current_position) {
                    seek_out_of_range = true;
                } else {
                    file_reader->current_position -= positive_offset;
                }
            } else {
                if (static_cast<size_t>(offset) > file_reader->total_size - file_reader->current_position) {
                    seek_out_of_range = true;
                } else {
                    file_reader->current_position += static_cast<size_t>(offset);
                }
            }
        } else if (whence == ios::end) {
            if (offset < 0LL) {
                size_t positive_offset = static_cast<size_t>(-(offset + 1LL)) + 1ULL;
                if (positive_offset > file_reader->total_size) {
                    seek_out_of_range = true;
                } else {
                    file_reader->current_position = file_reader->total_size - positive_offset;
                }
            } else if (offset == 0LL) {
            }
        }
        if (seek_out_of_range) {
            GetLogger()->error("Seeking out of file's range.");
            return KO;
        } else {
            return kSuccess;
        }
    );
}

const char *driver_getlasterror() {
    const char *const KO = "Error while trying to fetch last error.";
    CATCH_ALL(
        GetLogger()->info("Retrieving last error...");
        static string last_error = GetLastError();
        return last_error.empty() ? nullptr : last_error.c_str();
    );
}

long long int driver_fwrite(const void *ptr, size_t size, size_t count, void *stream) {
    const long long int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Writing {}x{} bytes from buffer at {} to file with handle {}...", size, count, ptr, stream);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(ptr, STRINGIFY(ptr), __func__)) return KO;
        if (!CheckNotNull(stream, STRINGIFY(stream), __func__)) return KO;
        FileWriter *file_writer;
        if (GetFileWriter(&file_writer, stream) != 0) return KO;
        size_t nwritten;
        if (FWrite(&nwritten, file_writer, ptr, size, count) != 0) return KO;
        return static_cast<long long int>(nwritten);
    );
}

int driver_fflush(void *stream) {
    const int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Flushing file with handle {}...", stream);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(stream, STRINGIFY(stream), __func__)) return KO;
        FileWriter *file_writer;
        if (GetFileWriter(&file_writer, stream) != 0) return KO;
        if (FFlush(*file_writer) != 0) return KO;
        return kSuccess;
    );
}

int driver_remove(const char *filename) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Removing file at URL {}...", filename);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(filename, STRINGIFY(filename), __func__)) return KO;
        if (!CheckIsFileUrl(filename)) return KO;
        if (Remove(filename) != 0) return KO;
        return kOtherSuccess;
    );
}

int driver_mkdir(const char *pathname) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Creating directory at URL {}...", pathname);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(pathname, STRINGIFY(pathname), __func__)) return KO;
        if (!CheckIsDirUrl(pathname)) return KO;
        if (Mkdir(pathname) != 0) return KO;
        return kOtherSuccess;
    );
}

int driver_rmdir(const char *pathname) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Removing directory at URL {}...", pathname);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(pathname, STRINGIFY(pathname), __func__)) return KO;
        if (!CheckIsDirUrl(pathname)) return KO;
        if (Rmdir(pathname) != 0) return KO;
        return kOtherSuccess;
    );
}

long long int driver_diskFreeSpace(const char *filename) {
    const long long int KO = kFailure;
    CATCH_ALL(
        GetLogger()->info("Retrieving free disk space at URL {}...", filename);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(filename, STRINGIFY(filename), __func__)) return KO;
        size_t free_space;
        if (DiskFreeSpace(&free_space, filename) != 0) return KO;
        return free_space;
    );
}

int driver_copyToLocal(const char *sourcefilename, const char *destfilename) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Copying file at URL {} to URL {}...", sourcefilename, destfilename);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(sourcefilename, STRINGIFY(sourcefilename), __func__)) return KO;
        if (!CheckNotNull(destfilename, STRINGIFY(destfilename), __func__)) return KO;
        if (!CheckIsFileUrl(sourcefilename)) return KO;
        if (!CheckIsFileUrl(destfilename)) return KO;
        int status;
        FileReader file_reader;
        size_t buffer_size;
        unique_ptr<char[]> buffer;
        ofstream ofs;
        size_t ntotalcopied = 0ULL, nread, ntocopy;

        if (PopulateFileReader(&file_reader, sourcefilename) != 0) return -1;
        if (file_reader.total_size == 0ULL) { GetLogger()->trace("Nothing to copy."); return 0; }
        if (GetSystemPreferredBufferSize(&buffer_size) != 0) return -1;
        buffer = make_unique<char[]>(buffer_size);
        ofs = ofstream(destfilename, ios::binary);
        if (!ofs) { GetLogger()->error("Failed to open local destination file."); return -1; }
        status = kOtherSuccess;
        while (ntotalcopied < file_reader.total_size) {
            ntocopy = min(buffer_size, file_reader.total_size - ntotalcopied);
            GetLogger()->trace("Copying {} bytes from remote to local file...", ntocopy);
            if (FRead(&nread, buffer.get(), &file_reader, 1, ntocopy) != 0) { status = KO; break; }
            if (nread != ntocopy) { GetLogger()->error("Tried to copy {} bytes but read only {}.", ntocopy, nread); status = KO; break; }
            ofs.write(buffer.get(), (streamsize)ntocopy);
            if (!ofs) { GetLogger()->error("Failed to write to local destination file."); status = KO; break; }
            ntotalcopied += ntocopy;
        }
        if (FCloseReader(file_reader) != 0) return -1;
        return status;
    );
}

int driver_copyFromLocal(const char *sourcefilename, const char *destfilename) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Copying file at URL {} to URL {}...", sourcefilename, destfilename);
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(sourcefilename, STRINGIFY(sourcefilename), __func__)) return KO;
        if (!CheckNotNull(destfilename, STRINGIFY(destfilename), __func__)) return KO;
        if (!CheckIsFileUrl(sourcefilename)) return KO;
        if (!CheckIsFileUrl(destfilename)) return KO;
        int status;
        FileWriter file_writer;
        unique_ptr<char[]> buffer;
        size_t buffer_size;
        ifstream ifs;
        size_t ntotalcopied = 0ULL, ntocopy, nread, nwritten, total_size;

        if (InitializeFileWriterWithWriteMode(&file_writer) != 0) return -1;
        if (GetSystemPreferredBufferSize(&buffer_size) != 0) return -1;
        buffer = make_unique<char[]>(buffer_size);
        ifs = ifstream(sourcefilename, ios::binary);
        if (!ifs) { GetLogger()->error("Failed to open local source file."); return -1; }
        ifs.seekg(0, ios::end);
        streampos end = ifs.tellg();
        if (end == streampos(-1)) { GetLogger()->error("Failed to get local source file size."); return -1; }
        ifs.seekg(0, ios::beg);
        total_size = static_cast<size_t>(end);
        if (total_size == 0ULL) { GetLogger()->trace("Nothing to copy."); return 0; }
        status = kOtherSuccess;
        while (ntotalcopied < total_size) {
            ntocopy = min(buffer_size, total_size - ntotalcopied);
            GetLogger()->trace("Copying {} bytes from local file to remote...", ntocopy);
            ifs.read(buffer.get(), ntocopy);
            if (!ifs) { GetLogger()->error("Failed to read from local source file."); status = KO; break; }
            nread = static_cast<size_t>(ifs.gcount());
            if (nread != ntocopy) { GetLogger()->error("Tried to copy {} bytes but read only {}.", ntocopy, nread); status = KO; break; }
            if (FWrite(&nwritten, &file_writer, buffer.get(), 1, ntocopy) != 0) { status = KO; break; }
            if (nwritten != ntocopy) { GetLogger()->error("Tried to copy {} bytes but wrote only {}.", ntocopy, nwritten); status = KO; break; }
            ntotalcopied += ntocopy;
        }
        if (FCloseWriter(file_writer) != 0) return -1;
        return status;
    );
}

int driver_concat(const char *destfilename, const char **sourcefilenames, size_t sourcefilecount) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Concatenating {} files to URL {}...", sourcefilecount, destfilename);
        for (size_t i = 0; i < sourcefilecount; i++) {
            GetLogger()->info("  Source file #{}: {}", i + 1, sourcefilenames[i]);
        }
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(destfilename, STRINGIFY(destfilename), __func__)) return KO;
        if (!CheckNotNull(sourcefilenames, STRINGIFY(sourcefilenames), __func__)) return KO;
        if (sourcefilecount < 2) { GetLogger()->error("Too few files to concatenate."); return KO; }
        if (Concat(destfilename, vector<string>(sourcefilenames, sourcefilenames + sourcefilecount)) != 0) return KO;
        return kOtherSuccess;
    );
}

int driver_composeMultifile(const char *sDestFilePathName, const char **sSourceFilePathNames, size_t nSourceFileCount) {
    const int KO = kOtherFailure;
    CATCH_ALL(
        GetLogger()->info("Composing {} files into one multifile {}...", nSourceFileCount, sDestFilePathName);
        for (size_t i = 0; i < nSourceFileCount; i++) {
            GetLogger()->info("  Source file #{}: {}", i + 1, sSourceFilePathNames[i]);
        }
        if (!CheckInitialized()) return KO;
        if (!CheckNotNull(sDestFilePathName, STRINGIFY(sDestFilePathName), __func__)) return KO;
        if (!CheckNotNull(sSourceFilePathNames, STRINGIFY(sSourceFilePathNames), __func__)) return KO;
        if (nSourceFileCount < 2) { GetLogger()->error("Too few files to compose a multifile."); return KO; }
        if (Concat(sDestFilePathName, vector<string>(sSourceFilePathNames, sSourceFilePathNames + nSourceFileCount)) != 0) return KO;
        return kOtherSuccess;
    );
}