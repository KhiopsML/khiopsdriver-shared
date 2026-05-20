#pragma once

#include <string>
#include <vector>

struct Backend {
    int (*GetDriverName)(std::string *result);
    int (*GetDriverVersion)(std::string *result);
    int (*GetDriverScheme)(std::string *result);
    int (*IsReadOnly)(bool *result);
    int (*Initialize)();
    int (*Finalize)();
    int (*GetSystemPreferredBufferSize)(size_t *result);
    int (*FileExists)(bool *result, const std::string &sFilePathName);
    int (*DirExists)(bool *result, const std::string &sFilePathName);
    int (*GetFileSize)(size_t *result, const std::string &filename);
    int (*FOpenForReading)(void **result, const std::string &filename);
    int (*FOpenForWriting)(void **result, const std::string &filename);
    int (*FOpenForAppending)(void **result, const std::string &filename);
    int (*FClose)(void *stream);
    int (*FRead)(size_t *result, void *ptr, size_t size, size_t count, void *stream);
    int (*FSeek)(void *stream, long long int offset, int whence);
    int (*FWrite)(size_t *result, const void *ptr, size_t size, size_t count, void *stream);
    int (*FFlush)(void *stream);
    int (*Remove)(const std::string &filename);
    int (*Mkdir)(const std::string &pathname);
    int (*Rmdir)(const std::string &pathname);
    int (*DiskFreeSpace)(size_t *result, const std::string &filename);
    int (*CopyToLocal)(const std::string &sourcefilename, const std::string &destfilename);
    int (*CopyFromLocal)(const std::string &sourcefilename, const std::string &destfilename);
    int (*Concat)(const std::string &destfilename, const std::vector<std::string> &sourcefilenames, size_t sourcefilecount);
    int (*ComposeMultifile)(const std::string &sDestFilePathName, const std::vector<std::string> &sSourceFilePathNames, size_t nSourceFileCount);
};