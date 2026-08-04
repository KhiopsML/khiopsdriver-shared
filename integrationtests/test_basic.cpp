#include "khiops_driver_common/driver.h"
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
  ASSERT_EQ(driver_dirExists(url.InexistantDir().c_str()), kFalse);
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
  std::string sNewDir = url.NewRandomDir();
  ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kFalse);
  ASSERT_EQ(driver_mkdir(sNewDir.c_str()), kOtherSuccess);
  ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kTrue);
  ASSERT_EQ(driver_rmdir(sNewDir.c_str()), kOtherSuccess);
}

TEST_F(StorageTest, RmDir) {
  std::string sNewDir = url.NewRandomDir();
  ASSERT_EQ(driver_mkdir(sNewDir.c_str()), kOtherSuccess);
  ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kTrue);
  ASSERT_EQ(driver_rmdir(sNewDir.c_str()), kOtherSuccess);
  ASSERT_EQ(driver_dirExists(sNewDir.c_str()), kFalse);
}

TEST_F(StorageTest, CopyToLocalInexistantFile) {
  ASSERT_FALSE(LocalFileExists(sLocalFilePath)) << "The file exists locally before trying to copy from remote.";
  ASSERT_EQ(driver_copyToLocal(url.InexistantFile().c_str(), sLocalFilePath.c_str()), kOtherFailure) << "Copying inexistant file from remote to local did not indicate failure.";
  ASSERT_NE(driver_getlasterror(), nullptr) << "No error detected but there should have been one.";
  ASSERT_FALSE(LocalFileExists(sLocalFilePath)) << "The local file has been created.";
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

  ASSERT_EQ(driver_fileExists(output.c_str()), kFalse) << "The output file exists before concatenation.";
  // Copy sources. The temporary copies will be the actual sources of the concatenation.
  ASSERT_EQ(driver_dirExists(tmpdir.c_str()), kFalse) << "The temporary directory already exists.";
  ASSERT_EQ(driver_mkdir(tmpdir.c_str()), kOtherSuccess) << "Could not create temporary directory.";
  ASSERT_EQ(driver_dirExists(tmpdir.c_str()), kTrue) << "The temporary directory already exists.";
  for(size_t i = 0ULL; i < nsources; i++) {
    CopyFile(original_sources[i], tmpsources[i]);
  }
  // Concat
  ASSERT_EQ(driver_dirExists(outputdir.c_str()), kFalse) << "The destination directory already exists.";
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
  ASSERT_EQ(driver_dirExists(outputdir.c_str()), kFalse) << "Failed to remove destination directory.";
  ASSERT_EQ(driver_rmdir(tmpdir.c_str()), kOtherSuccess) << "Could not delete temporary directory.";
  ASSERT_EQ(driver_dirExists(tmpdir.c_str()), kFalse) << "Failed to remove temporary directory.";
}

TEST_F(StorageTest, ComposeMultifile) {
  if(IsAzuriteStorage()) {
    GTEST_SKIP() << "Azurite emulator does not support features needed for this operation.";
  }

  // Define source files assumed to exist in the test dataset
  const std::vector<std::string> original_sources = url.SplitFileParts();
  const size_t nsources = original_sources.size();
  ASSERT_GT(nsources, 0ULL) << "No source files available for composeMultifile test.";

  // Temporary directory for copied sources and renamed outputs
  const std::string tmpdir = url.NewRandomDir();

  // Destination globbing pattern: prefix*suffix
  const std::string dest_pattern = tmpdir + "compose-renamed-*.txt";

  // Full source paths (used for copy and existence checks)
  std::vector<std::string> tmpsources_full;
  tmpsources_full.reserve(nsources);

  // Relative source paths required by driver_composeMultifile (no scheme)
  std::vector<std::string> tmpsources_relative;
  tmpsources_relative.reserve(nsources);

  // C-string array passed to the driver
  std::vector<const char *> tmpsources_relative_cstr;
  tmpsources_relative_cstr.reserve(nsources);

  ASSERT_EQ(driver_mkdir(tmpdir.c_str()), kOtherSuccess) << "Could not create temporary directory.";

  // Copy sources to temporary location
  for(size_t i = 0ULL; i < nsources; ++i) {
    std::ostringstream name;
    name << tmpdir << "compose-src-" << std::setfill('0') << std::setw(3) << i << ".txt";
    const std::string tmpsource = name.str();

    CopyFile(original_sources[i], tmpsource);
    tmpsources_full.push_back(tmpsource);

    // Convert full URL to relative object path for composeMultifile
    // Expected blob URL format: scheme://bucket/object_path
    const std::size_t scheme_pos = tmpsource.find("://");
    ASSERT_NE(scheme_pos, std::string::npos)
        << "Invalid storage URL format for source: " << tmpsource;

    const std::size_t object_pos = tmpsource.find('/', scheme_pos + 3);
    ASSERT_NE(object_pos, std::string::npos)
        << "Cannot find object path in source URL: " << tmpsource;

    // Keep everything after bucket separator: "khiops_data/..."
    const std::string relative_path = tmpsource.substr(object_pos + 1);
    ASSERT_FALSE(relative_path.empty())
        << "Extracted empty relative path from source URL: " << tmpsource;

    tmpsources_relative.push_back(relative_path);
  }

  for(size_t i = 0ULL; i < tmpsources_relative.size(); ++i) {
    tmpsources_relative_cstr.push_back(tmpsources_relative[i].c_str());
  }

  // Execute composeMultifile
  ASSERT_EQ(driver_composeMultifile(dest_pattern.c_str(),
                                    tmpsources_relative_cstr.data(),
                                    nsources),
            kOtherSuccess) << "ComposeMultifile failed.";

  // Verify that sources were deleted
  for(size_t i = 0ULL; i < tmpsources_full.size(); ++i) {
    ASSERT_EQ(driver_fileExists(tmpsources_full[i].c_str()), kFalse)
        << "Source file was not deleted after composeMultifile: " << tmpsources_full[i];
  }

  // Verify renamed output files existence
  // Expected naming: prefix + 12-digit index + suffix
  std::vector<std::string> renamed_outputs;
  renamed_outputs.reserve(nsources);

  const std::string prefix = tmpdir + "compose-renamed-";
  const std::string suffix = ".txt";

  for(size_t i = 0ULL; i < nsources; ++i) {
    std::ostringstream out;
    out << prefix << std::setfill('0') << std::setw(12) << i << suffix;
    renamed_outputs.push_back(out.str());
  }

  for(size_t i = 0ULL; i < renamed_outputs.size(); ++i) {
    ASSERT_EQ(driver_fileExists(renamed_outputs[i].c_str()), kTrue)
        << "Renamed output does not exist: " << renamed_outputs[i];
    ASSERT_GT(driver_getFileSize(renamed_outputs[i].c_str()), 0)
        << "Renamed output is empty or unreadable: " << renamed_outputs[i];
  }

  // Cleanup
  for(size_t i = 0ULL; i < renamed_outputs.size(); ++i) {
    ASSERT_EQ(driver_remove(renamed_outputs[i].c_str()), kOtherSuccess)
        << "Failed to remove renamed output: " << renamed_outputs[i];
    ASSERT_EQ(driver_fileExists(renamed_outputs[i].c_str()), kFalse)
        << "Renamed output still exists after cleanup: " << renamed_outputs[i];
  }

  ASSERT_EQ(driver_rmdir(tmpdir.c_str()), kOtherSuccess) << "Could not delete temporary directory.";
}
