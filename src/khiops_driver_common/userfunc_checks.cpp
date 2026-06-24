#include "khiops_driver_common/userfunc_checks.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <limits>
#include <vector>
#include "khiops_driver_common/logging.hpp"
#include "khiops_driver_common/checks.hpp"
#include "khiops_driver_common/stringify.hpp"
#include "khiops_driver_common/filestream_management.hpp"
#include "khiops_driver_common/globalstate.hpp"

using namespace std;

namespace khiops_driver_common {

int Check_driver_getDriverName() {
    GetLogger()->info("Retrieving driver name...");
    return 0;
}

int Check_driver_getVersion() {
    GetLogger()->info("Retrieving driver version...");
    return 0;
}

int Check_driver_getScheme() {
    GetLogger()->info("Retrieving driver scheme...");
    return 0;
}

int Check_driver_isReadOnly() {
    GetLogger()->info("Retrieving read-only state...");
    return 0;
}

int Check_driver_connect() {
    GetLogger()->info("Connecting...");
    return 0;
}

int Check_driver_disconnect() {
    GetLogger()->info("Disconnecting...");
    return 0;
}

int Check_driver_isConnected() {
    GetLogger()->info("Retrieving connection state...");
    return 0;
}

int Check_driver_getSystemPreferredBufferSize() {
    GetLogger()->info("Retrieving preferred buffer size...");
    return 0;
}

int Check_driver_fileExists(const char *sFilePathName) {
    GetLogger()->info("Checking if file exists at URL {}...", sFilePathName);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(sFilePathName, STRINGIFY(sFilePathName), "driver_fileExists")) return -1;
    if (CheckIsFileUrl(sFilePathName)) return -1;
    return 0;
}

int Check_driver_dirExists(const char *sFilePathName) {
    GetLogger()->info("Checking if directory exists at URL {}...", sFilePathName);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(sFilePathName, STRINGIFY(sFilePathName), "driver_dirExists")) return -1;
    if (CheckIsDirUrl(sFilePathName)) return -1;
    return 0;
}

int Check_driver_getFileSize(const char *filename) {
    GetLogger()->info("Retrieving size of file at URL {}...", filename);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(filename, STRINGIFY(filename), "driver_getFileSize")) return -1;
    if (CheckIsFileUrl(filename)) return -1;
    return 0;
}

int Check_driver_fopen(const char *filename, char mode) {
    GetLogger()->info("Opening file at URL {} in mode '{}'...", filename, mode);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(filename, STRINGIFY(filename), "driver_fopen")) return -1;
    if (CheckIsFileUrl(filename)) return -1;
    if (mode != 'r' && mode != 'w' && mode != 'a') {
        GetLogger()->error("Cannot open file with invalid mode '{}'.", mode);
        return -1;
    }
    return 0;
}

int Check_driver_fclose(void *stream) {
    GetLogger()->info("Closing file with handle {}...", stream);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(stream, STRINGIFY(stream), "driver_fclose")) return -1;
    if (CheckFileStream(GetState()->open_file_streams, stream)) return -1;
    return 0;
}

int Check_driver_fread(void *ptr, size_t size, size_t count, void *stream) {
    GetLogger()->info("Reading {}x{} bytes from file with handle {} to buffer at {}...", size, count, stream, ptr);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(ptr, STRINGIFY(ptr), "driver_fread")) return -1;
    if (CheckNotNull(stream, STRINGIFY(stream), "driver_fread")) return -1;
    if (size != 0 && count > numeric_limits<size_t>::max() / size) {
        GetLogger()->error("'size' x 'count' exceeds {}.", numeric_limits<size_t>::max());
        return -1;
    }
    if (CheckFileStream(GetState()->open_file_streams, stream, FileStreamMode::READ)) return -1;
    return 0;
}

int Check_driver_fseek(void *stream, long long int offset, int whence) {
    GetLogger()->info("Seeking offset {} from origin {} in file with handle {}...", offset, whence, stream);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(stream, STRINGIFY(stream), "driver_fseek")) return -1;
    if (whence != ios::beg && whence != ios::cur && whence != ios::end) {
        GetLogger()->error("Tried to seek from invalid origin '{}'.", whence);
        return -1;
    }
    if (CheckFileStream(GetState()->open_file_streams, stream, FileStreamMode::READ)) return -1;
    return 0;
}

int Check_driver_getlasterror() {
    GetLogger()->info("Retrieving last error...");
    return 0;
}

int Check_driver_fwrite(const void *ptr, size_t size, size_t count, void *stream) {
    GetLogger()->info("Writing {}x{} bytes from buffer at {} to file with handle {}...", size, count, ptr, stream);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(ptr, STRINGIFY(ptr), "driver_fwrite")) return -1;
    if (CheckNotNull(stream, STRINGIFY(stream), "driver_fwrite")) return -1;
    if (size != 0 && count > numeric_limits<size_t>::max() / size) {
        GetLogger()->error("'size' x 'count' exceeds {}.", numeric_limits<size_t>::max());
        return -1;
    }
    if (CheckFileStream(GetState()->open_file_streams, stream, vector<FileStreamMode>{FileStreamMode::WRITE, FileStreamMode::APPEND})) return -1;
    return 0;
}

int Check_driver_fflush(void *stream) {
    GetLogger()->info("Flushing file with handle {}...", stream);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(stream, STRINGIFY(stream), "driver_fflush")) return -1;
    if (CheckFileStream(GetState()->open_file_streams, stream, vector<FileStreamMode>{FileStreamMode::WRITE, FileStreamMode::APPEND})) return -1;
    return 0;
}

int Check_driver_remove(const char *filename) {
    GetLogger()->info("Removing file at URL {}...", filename);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(filename, STRINGIFY(filename), "driver_remove")) return -1;
    if (CheckIsFileUrl(filename)) return -1;
    return 0;
}

int Check_driver_mkdir(const char *pathname) {
    GetLogger()->info("Creating directory at URL {}...", pathname);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(pathname, STRINGIFY(pathname), "driver_mkdir")) return -1;
    if (CheckIsDirUrl(pathname)) return -1;
    return 0;
}

int Check_driver_rmdir(const char *pathname) {
    GetLogger()->info("Removing directory at URL {}...", pathname);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(pathname, STRINGIFY(pathname), "driver_rmdir")) return -1;
    if (CheckIsDirUrl(pathname)) return -1;
    return 0;
}

int Check_driver_diskFreeSpace(const char *filename) {
    GetLogger()->info("Retrieving free disk space at URL {}...", filename);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(filename, STRINGIFY(filename), "driver_diskFreeSpace")) return -1;
    return 0;
}

int Check_driver_copyToLocal(const char *sourcefilename, const char *destfilename) {
    GetLogger()->info("Copying file at URL {} to URL {}...", sourcefilename, destfilename);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(sourcefilename, STRINGIFY(sourcefilename), "driver_copyToLocal")) return -1;
    if (CheckNotNull(destfilename, STRINGIFY(destfilename), "driver_copyToLocal")) return -1;
    if (CheckIsFileUrl(sourcefilename)) return -1;
    if (CheckIsFileUrl(destfilename)) return -1;
    return 0;
}

int Check_driver_copyFromLocal(const char *sourcefilename, const char *destfilename) {
    GetLogger()->info("Copying file at URL {} to URL {}...", sourcefilename, destfilename);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(sourcefilename, STRINGIFY(sourcefilename), "driver_copyFromLocal")) return -1;
    if (CheckNotNull(destfilename, STRINGIFY(destfilename), "driver_copyFromLocal")) return -1;
    if (CheckIsFileUrl(sourcefilename)) return -1;
    if (CheckIsFileUrl(destfilename)) return -1;
    return 0;
}

int Check_driver_concat(const char *destfilename, const char **sourcefilenames, size_t sourcefilecount) {
    GetLogger()->info("Concatenating {} files to URL {}...", sourcefilecount, destfilename);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(destfilename, STRINGIFY(destfilename), "driver_concat")) return -1;
    if (CheckNotNull(sourcefilenames, STRINGIFY(sourcefilenames), "driver_concat")) return -1;
    for (size_t i = 0ULL; i < sourcefilecount; i++) {
        ostringstream oss;
        oss << STRINGIFY(sourcefilenames) << "[" << i << "]";
        string str = oss.str();
        if (CheckNotNull(sourcefilenames[i], str.c_str(), "driver_concat")) return -1;
    }
    for (size_t i = 0ULL; i < sourcefilecount; i++) {
        GetLogger()->info("  Source file #{}: {}", i + 1, sourcefilenames[i]);
    }
    if (CheckIsFileUrl(destfilename)) return -1;
    for (size_t i = 0ULL; i < sourcefilecount; i++) {
        if (CheckIsFileUrl(sourcefilenames[i])) return -1;
    }
    if (sourcefilecount < 2) { GetLogger()->error("Too few files to concatenate."); return -1; }
    return 0;
}

int Check_driver_composeMultifile(const char *sDestFilePathName, const char **sSourceFilePathNames, size_t nSourceFileCount) {
    GetLogger()->info("Composing {} files into one multifile {}...", nSourceFileCount, sDestFilePathName);
    if (CheckInitialized()) return -1;
    if (CheckNotNull(sDestFilePathName, STRINGIFY(sDestFilePathName), "driver_composeMultifile")) return -1;
    if (CheckNotNull(sSourceFilePathNames, STRINGIFY(sSourceFilePathNames), "driver_composeMultifile")) return -1;
    for (size_t i = 0; i < nSourceFileCount; i++) {
        GetLogger()->info("  Source file #{}: {}", i + 1, sSourceFilePathNames[i]);
    }
    if (CheckIsFileUrl(sDestFilePathName)) return -1;
    for (size_t i = 0ULL; i < nSourceFileCount; i++) {
        if (CheckIsFileUrl(sSourceFilePathNames[i])) return -1;
    }
    if (nSourceFileCount < 2) {
        GetLogger()->error("Too few files to compose a multifile.");
        return -1;
    }
    return 0;
}

}