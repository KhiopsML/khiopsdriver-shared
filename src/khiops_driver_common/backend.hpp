#pragma once

#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include "khiops_driver_common/filestream.hpp"

namespace khiops_driver_common {

/*****************************************
 *** FUNCTIONS PROVIDED BY THE DRIVER. ***
 *****************************************/

spdlog::logger *GetLogger();
int GetDriverName(std::string *result);
int GetDriverVersion(std::string *result);
int GetDriverScheme(std::string *result);
int IsReadOnly(bool *result);
int Initialize();
int Finalize();
int GetSystemPreferredBufferSize(size_t *result);
int FileExists(bool *result, const std::string &sFilePathName);
int DirExists(bool *result, const std::string &sFilePathName);
int GetFileSize(size_t *result, const std::string &filename);
int FOpen(khiops_driver_common::FileStream &stream, const std::string &filename);
int FClose(const khiops_driver_common::FileStream &stream);
int FRead(size_t *result, void *ptr, size_t size, size_t count, khiops_driver_common::FileStream &stream);
int FSeek(khiops_driver_common::FileStream &stream, long long int offset, int whence);
int FWrite(size_t *result, const void *ptr, size_t size, size_t count, const khiops_driver_common::FileStream &stream);
int FFlush(const khiops_driver_common::FileStream &stream);
int Remove(const std::string &filename);
int Mkdir(const std::string &pathname);
int Rmdir(const std::string &pathname);
int DiskFreeSpace(size_t *result, const std::string &filename);
int CopyToLocal(const std::string &sourcefilename, const std::string &destfilename);
int CopyFromLocal(const std::string &sourcefilename, const std::string &destfilename);
int Concat(const std::string &destfilename, const std::vector<std::string> &sourcefilenames);
int ComposeMultifile(const std::string &sDestFilePathName, const std::vector<std::string> &sSourceFilePathNames);

}