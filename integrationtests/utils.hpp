#pragma once

#include "khiops_driver_common/driver.h"
#include "returnval.hpp"
#include <gtest/gtest.h>
#include <string>
#include <cstdlib>
#include <fstream>

inline bool IsAzuriteStorage() {
  char *azure_emulated_storage = getenv("AZURE_EMULATED_STORAGE");
  if(azure_emulated_storage) {
    std::string azure_emulated_storage_str(azure_emulated_storage);
    if(!azure_emulated_storage_str.empty() && azure_emulated_storage_str != "false") {
      return true;
    }
  }
  return false;
}

inline void CopyFile(std::string source, std::string dest) {
  void *sourceptr, *destptr;
  long long int buffersize, nread;
  ASSERT_NE((buffersize = driver_getSystemPreferredBufferSize()), -1LL) << "Could not get preferred buffer size.";
  std::vector<char> buffer(buffersize);

  ASSERT_EQ(driver_fileExists(source.c_str()), kTrue) << "Source file does not exist: '" << source << "'.";
  ASSERT_EQ(driver_fileExists(dest.c_str()), kFalse) << "Destination file already exists: '" << dest << "'.";
  long long int filesize = driver_getFileSize(source.c_str());
  ASSERT_NE(filesize, kFailure) << "Failed to get size of file to copy.";
  ASSERT_NE((sourceptr = driver_fopen(source.c_str(), 'r')), nullptr) << "Could not open source file: '" << source << "'.";
  ASSERT_NE((destptr = driver_fopen(dest.c_str(), 'w')), nullptr) << "Could not open destination file: '" << dest << "'.";
  for (long long int ntotalcopied = 0LL; ntotalcopied != filesize; ntotalcopied += std::min(buffersize, filesize - ntotalcopied)) {
    nread = driver_fread(reinterpret_cast<void *>(buffer.data()), 1, std::min(buffersize, filesize - ntotalcopied), sourceptr);
    ASSERT_EQ(nread, std::min(buffersize, filesize - ntotalcopied)) << "Failed to read from source file: '" << source << "'.";
    ASSERT_EQ(driver_fwrite(reinterpret_cast<void *>(buffer.data()), 1, nread, destptr), nread) << "Failed to write to destination file: '" << dest << "'.";
  }
  ASSERT_EQ(driver_fclose(destptr), kSuccess) << "Could not close destination file: '" << dest << "'.";
  ASSERT_EQ(driver_fclose(sourceptr), kSuccess) << "Could not close source file: " << source << "'.";
  ASSERT_EQ(driver_fileExists(source.c_str()), kTrue) << "Source file does not exist anymore: '" << source << "'.";
  ASSERT_EQ(driver_fileExists(dest.c_str()), kTrue) << "Destination file has not been created: '" << dest << "'.";

  const char *last_error = driver_getlasterror();
  ASSERT_EQ(last_error, nullptr) << "A driver error has been detected : '" << last_error << "'.";
}

inline bool LocalFileExists(const std::string &filename) {
  std::ifstream ifstream(filename);
  bool file_exists = ifstream.is_open();
  return file_exists;
}
