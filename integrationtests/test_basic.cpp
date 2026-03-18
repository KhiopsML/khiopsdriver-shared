#include "plugin.hpp"
#include "fixture_storage.hpp"
#include "utils.hpp"
#include "returnval.hpp"

#include <algorithm>

#include <boost/process/v2/environment.hpp>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <gtest/gtest.h>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

TEST(BasicTest, IsReadOnly) { ASSERT_EQ(driver_isReadOnly(), kFalse); }

TEST(BasicTest, GetSystemPreferredBufferSize) {
  ASSERT_EQ(driver_getSystemPreferredBufferSize(), 4 * 1024 * 1024);
}

TEST(BasicTest, Connect) {
  // check connection state before call to connect
  ASSERT_EQ(driver_isConnected(), kFalse);

  // call connect and check connection
  ASSERT_EQ(driver_connect(), kOtherSuccess);
  ASSERT_EQ(driver_isConnected(), kTrue);

  // call disconnect and check connection
  ASSERT_EQ(driver_disconnect(), kOtherSuccess);
  ASSERT_EQ(driver_isConnected(), kFalse);
}

TEST(BasicTest, Disconnect) {
  ASSERT_EQ(driver_connect(), kOtherSuccess);
  ASSERT_EQ(driver_disconnect(), kOtherSuccess);
  ASSERT_EQ(driver_isConnected(), kFalse);
}

TEST_F(StorageTest, GetFileSize) {
  ASSERT_EQ(driver_getFileSize(url.File().c_str()), 5585568);
}

TEST_F(StorageTest, GetMultipartFileSize) {
  ASSERT_EQ(driver_getFileSize(url.BQFile().c_str()), 5585568);
}

TEST_F(StorageTest, GetFileSizeNonexistentFailure) {
  ASSERT_EQ(driver_getFileSize(url.InexistantFile().c_str()), kFailure);
  ASSERT_STRNE(driver_getlasterror(), NULL);
}

TEST_F(StorageTest, FileExists) {
  ASSERT_EQ(driver_fileExists(url.File().c_str()), kTrue);
}

TEST_F(StorageTest, FileExistsNonExistentfile) {
  ASSERT_EQ(driver_fileExists(url.InexistantFile().c_str()), kFalse);
}

TEST_F(StorageTest, DirExists) {
  ASSERT_EQ(driver_dirExists(url.Dir().c_str()), kTrue);
}

TEST_F(StorageTest, DirExistsNonExistentDir) {
  if(GetStorageType() == StorageType::FILE) {
    ASSERT_EQ(driver_dirExists(url.InexistantDir().c_str()), kFalse);
  } else {
    ASSERT_EQ(driver_dirExists(url.InexistantDir().c_str()), kTrue);
  }
}

#ifndef _WIN32
// Setting of environment variables does not work on Windows
TEST(BasicTest, DriverConnectMissingCredentialsFailure) {
  GTEST_SKIP() << "To be implemented.";
}

// TODO: Move test to specific driver repo or fix it.
void setup_bad_credentials() {
  boost::process::v2::environment::set(
      "AZURE_STORAGE_CONNECTION_STRING",
      // Default Azurite credentials with AccountKey component slightly modified
      // (last "w" replaced by "W")
      "DefaultEndpointsProtocol=http;AccountName=devstoreaccount1;AccountKey="
      "Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/"
      "K1SZFPTOtr/"
      "KBHBeksoGMGW==;BlobEndpoint=http://localhost:10000/devstoreaccount1;");
}

void cleanup_bad_credentials() {
  boost::process::v2::environment::unset("AZURE_STORAGE_CONNECTION_STRING");
}

TEST_F(StorageTest, GetFileSizeInvalidCredentialsFailure) {
  GTEST_SKIP() << "To be fixed.";
  setup_bad_credentials();
  ASSERT_EQ(driver_getFileSize(url.File().c_str()), -1);
  ASSERT_STRNE(driver_getlasterror(), NULL);
  cleanup_bad_credentials();
}
#endif

TEST_F(StorageTest, MkDir) {
  if(GetStorageType() == StorageType::FILE) {
    std::string sNewDir = url.NewRandomDir();
    ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kFalse);
    ASSERT_EQ(driver_mkdir(sNewDir.c_str()), kOtherSuccess);
    ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kTrue);
    ASSERT_EQ(driver_rmdir(sNewDir.c_str()), kOtherSuccess);
  } else {
    ASSERT_EQ(driver_mkdir(url.NewRandomDir().c_str()), kOtherSuccess);
  }
}

TEST_F(StorageTest, RmDir) {
  if(GetStorageType() == StorageType::FILE) {
    std::string sNewDir = url.NewRandomDir();
    ASSERT_EQ(driver_mkdir(sNewDir.c_str()), kOtherSuccess);
    ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kTrue);
    ASSERT_EQ(driver_rmdir(sNewDir.c_str()), kOtherSuccess);
    ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kFalse);
  } else {
    ASSERT_EQ(driver_rmdir(url.NewRandomDir().c_str()), kOtherSuccess);
  }
}

TEST_F(StorageTest, Concat) {
  if(IsAzuriteStorage()) {
    GTEST_SKIP() << "Azurite emulator does not support features needed for server-side concatenation.";
  }

  // Define URLs
  const std::vector<std::string> original_sources = url.SplitFileParts();
  const std::string outputdir = url.NewRandomDir();
  const std::string output = outputdir + "driver_concat_test_output";
  const std::string reference = url.File();
  const std::string tmpdir = url.NewRandomDir();
  std::vector<std::string> tmpsources;
  std::vector<const char *> tmpsources_as_cstr;
  for(const std::string &original_source : original_sources) {
    std::string tmpsource = tmpdir + original_source.substr(original_source.rfind('/') + 1);
    tmpsources_as_cstr.push_back(tmpsource.c_str());
    tmpsources.push_back(std::move(tmpsource));
  }
  const size_t nsources = tmpsources.size();

  ASSERT_EQ(driver_connect(), kOtherSuccess) << "Failed to connect.";
  ASSERT_EQ(driver_fileExists(output.c_str()), kFalse) << "The output file exists before concatenation.";
  // Copy sources. The temporary copies will be the actual sources of the concatenation.
  if(GetStorageType() == StorageType::FILE) {
    ASSERT_EQ(driver_dirExists(tmpdir.c_str()), kFalse) << "The temporary directory already exists.";
  }
  ASSERT_EQ(driver_mkdir(tmpdir.c_str()), kOtherSuccess) << "Could not create temporary directory.";
  ASSERT_EQ(driver_dirExists(tmpdir.c_str()), kTrue) << "The temporary directory already exists.";
  for(size_t i = 0ULL; i < nsources; i++) {
    CopyFile(original_sources[i], tmpsources[i]);
  }
  // Concat
  if(GetStorageType() == StorageType::FILE) {
    ASSERT_EQ(driver_dirExists(outputdir.c_str()), kFalse) << "The destination directory already exists.";
  }
  ASSERT_EQ(driver_mkdir(outputdir.c_str()), kOtherSuccess) << "Could not create the destination directory.";
  ASSERT_EQ(driver_dirExists(outputdir.c_str()), kTrue) << "The destination directory already exists.";
  ASSERT_EQ(driver_concat(output.c_str(), tmpsources_as_cstr.data(), nsources), kOtherSuccess) << "Concatenation failed.";
  // Check
  for(const char *tmpsource : tmpsources_as_cstr) {
    ASSERT_EQ(driver_fileExists(tmpsource), kFalse) << "Source file " << tmpsource << " was not deleted after concatenation.";
  }
  ASSERT_EQ(driver_fileExists(output.c_str()), kTrue) << "The concatenation created no output file.";
  ASSERT_EQ(driver_getFileSize(output.c_str()), driver_getFileSize(reference.c_str())) << "Incorrect output file size.";
  // Cleanup
  ASSERT_EQ(driver_remove(output.c_str()), kOtherSuccess) << "Failed to remove output file.";
  ASSERT_EQ(driver_fileExists(output.c_str()), kFalse) << "Output file still exists after removal.";
  ASSERT_EQ(driver_rmdir(outputdir.c_str()), kOtherSuccess) << "Could not delete destination directory.";
  if(GetStorageType() == StorageType::FILE) {
    ASSERT_EQ(driver_dirExists(outputdir.c_str()), kFalse) << "Failed to remove destination directory.";
  }
  ASSERT_EQ(driver_rmdir(tmpdir.c_str()), kOtherSuccess) << "Could not delete temporary directory.";
  if(GetStorageType() == StorageType::FILE) {
    ASSERT_EQ(driver_dirExists(tmpdir.c_str()), kFalse) << "Failed to remove temporary directory.";
  }
  ASSERT_EQ(driver_disconnect(), kOtherSuccess) << "Failed to disconnect.";
}
