#pragma once

#include <cstddef>

namespace khiops_driver_common {

int Check_driver_getDriverName();

int Check_driver_getVersion();

int Check_driver_getScheme();

int Check_driver_isReadOnly();

int Check_driver_connect();

int Check_driver_disconnect();

int Check_driver_isConnected();

int Check_driver_getSystemPreferredBufferSize();

int Check_driver_fileExists(const char *sFilePathName);

int Check_driver_dirExists(const char *sFilePathName);

int Check_driver_getFileSize(const char *filename);

int Check_driver_fopen(const char *filename, char mode);

int Check_driver_fclose(void *stream);

int Check_driver_fread(void *ptr, size_t size, size_t count, void *stream);

int Check_driver_fseek(void *stream, long long int offset, int whence);

int Check_driver_getlasterror();

int Check_driver_fwrite(const void *ptr, size_t size, size_t count, void *stream);

int Check_driver_fflush(void *stream);

int Check_driver_remove(const char *filename);

int Check_driver_mkdir(const char *pathname);

int Check_driver_rmdir(const char *pathname);

int Check_driver_diskFreeSpace(const char *filename);

int Check_driver_copyToLocal(const char *sourcefilename, const char *destfilename);

int Check_driver_copyFromLocal(const char *sourcefilename, const char *destfilename);

int Check_driver_concat(const char *destfilename, const char **sourcefilenames, size_t sourcefilecount);

int Check_driver_composeMultifile(const char *sDestFilePathName, const char **sSourceFilePathNames, size_t nSourceFileCount);

}