#pragma once

#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include "khiops_driver_common/filestream.hpp"

namespace khiops_driver_common {

/******************************************************
 *** FUNCTIONS TO BE PROVIDED BY THE ACTUAL DRIVER. ***
 ******************************************************/

spdlog::logger *GetLogger();
void FreeFileReaderFragmentVersion(void *version);
void FreeFileWriterUserData(void *user_data);
int InitializeFileWriterWithWriteMode(FileWriter *file_writer);
int InitializeFileWriteWithAppendMode(FileWriter *file_writer);
int ListFragments(std::vector<std::string> *result, const std::string &url);
int GetFragmentSizeAndVersion(size_t *size_result, void **version_result, const std::string &url);
int ReadFragment(std::string *result, bool *stopped_on_termchar, const std::string &url, void *version, size_t offset, size_t maxlength);
int ReadFragment(std::string *result, bool *stopped_on_termchar, const std::string &url, void *version, size_t offset, size_t maxlength, char termchar);
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
int FCloseReader(const FileReader &stream);
int FCloseWriter(const FileWriter &stream);
// int FRead(size_t *result, void *ptr, FileReader *file_reader, size_t size, size_t count);
int FWrite(size_t *result, FileWriter *file_writer, const void *ptr, size_t size, size_t count);
int FFlush(const FileWriter &file_writer);
int Remove(const std::string &filename);
int Mkdir(const std::string &pathname);
int Rmdir(const std::string &pathname);
int DiskFreeSpace(size_t *result, const std::string &filename);
int Concat(const std::string &destfilename, const std::vector<std::string> &sourcefilenames);
int ComposeMultifile(const std::string &sDestFilePathName, const std::vector<std::string> &sSourceFilePathNames);

}