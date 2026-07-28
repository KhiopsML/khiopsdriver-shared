#pragma once

#include "khiops_driver_common/driver.h"
#include "urls.hpp"
#include "returnval.hpp"
#include "utils.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <unordered_set>

namespace {
class NoErrorsLeftFixture : public testing::Test {
protected:
    void TearDown() override {
        // Make sure there are no errors left to read.
        const char *last_error = driver_getlasterror();
        ASSERT_EQ(last_error, nullptr) << "A driver error is still detected after the end of the test: '" << last_error << "'.";
        testing::Test::TearDown();
    }
};

class UrlFixture : public NoErrorsLeftFixture {
protected:
    void SetUp() override {
        NoErrorsLeftFixture::SetUp();
        url = StorageTestUrlProvider();
        std::ostringstream oss;
#ifdef _WIN32
        oss << std::getenv("TEMP") << "\\out-" << boost::uuids::random_generator()()
            << ".txt";
#else
        oss << "/tmp/out-" << boost::uuids::random_generator()() << ".txt";
#endif
        sLocalFilePath = oss.str();
    }
    StorageTestUrlProvider url;
    std::string sLocalFilePath;
};

class StorageTest : public UrlFixture {
protected:

    inline void SetUp() override {
        UrlFixture::SetUp();
        this->created_dirs.clear();
        this->created_files.clear();
        ASSERT_EQ(driver_connect(), kOtherSuccess) << "Driver failed to connect during test initialization.";
        ASSERT_EQ(driver_isConnected(), kTrue) << "After driver connected, it is still disconnected.";
    }

    inline void TearDown() override {
        for (const std::string &created_dir : this->created_dirs) {
            if (driver_dirExists(created_dir.c_str()) == kTrue) {
                ASSERT_EQ(driver_rmdir(created_dir.c_str()), kOtherSuccess) << "Failed to remove created directory.";
            }
        }
        for (const std::string &created_file : this->created_files) {
            if (driver_fileExists(created_file.c_str()) == kTrue) {
                ASSERT_EQ(driver_remove(created_file.c_str()), kOtherSuccess) << "Failed to remove created file.";
            }
        }
        ASSERT_EQ(driver_disconnect(), kOtherSuccess) << "Driver failed to disconnect during test finalization.";
        ASSERT_EQ(driver_isConnected(), kFalse) << "After driver disconnected, it is still connected.";
        UrlFixture::TearDown();
    }

    inline void PlanDirCleanup(const std::string &dir) {
        ASSERT_TRUE(this->created_dirs.insert(dir).second) << "Failed to plan directory clean-up (already planned?).";
    }

    inline void PlanFileCleanup(const std::string &file) {
        ASSERT_TRUE(this->created_files.insert(file).second) << "Failed to plan file clean-up (already planned?).";
    }

    inline void CreateEmptyFileAt(const std::string &file) {
        ASSERT_EQ(driver_fileExists(file.c_str()), kFalse) << "File already exists.";
        this->PlanFileCleanup(file);
        void *handle = driver_fopen(file.c_str(), 'w');
        ASSERT_NE(handle, nullptr) << "Failed to open file in write-mode.";
        ASSERT_EQ(driver_fclose(handle), kSuccess) << "Failed to close file.";
        ASSERT_EQ(driver_fileExists(file.c_str()), kTrue) << "File has not been created.";
    }

    inline void CreateRandomEmptyFile(std::string *created_file) {
        ASSERT_NE(created_file, nullptr);
        std::string new_file = this->url.RandomOutputFile();
        ASSERT_EQ(driver_fileExists(new_file.c_str()), kFalse) << "Randomly named file already exists: random name collision.";
        this->PlanFileCleanup(new_file);
        void *handle = driver_fopen(new_file.c_str(), 'w');
        ASSERT_NE(handle, nullptr) << "Failed to open file in write-mode.";
        ASSERT_EQ(driver_fclose(handle), kSuccess) << "Failed to close file.";
        ASSERT_EQ(driver_fileExists(new_file.c_str()), kTrue) << "File has not been created.";
        *created_file = new_file;
    }

    inline void CreateRandomFileWithContent(std::string *created_file) {
        ASSERT_NE(created_file, nullptr);
        std::string random_remote_file = url.RandomOutputFile();
        CopyFile(url.File(), random_remote_file);
        this->PlanFileCleanup(random_remote_file);
        *created_file = random_remote_file;
    }

    inline void CreateRandomMultiFileWithContent(std::string *created_multifile) {
        ASSERT_NE(created_multifile, nullptr);
        std::ostringstream oss;
        oss << url.RandomOutputFile() << ".multifile";
        std::string prefix = oss.str();
        oss << "*";
        std::string multifile_url = oss.str();
        std::vector<std::string> file_parts = url.SplitFileParts();
        for (size_t part_index = 0ULL; part_index != file_parts.size(); part_index++) {
            std::ostringstream file_part_oss;
            file_part_oss << prefix << part_index;
            std::string file_part = file_part_oss.str();
            CopyFile(file_parts[part_index], file_part);
            this->PlanFileCleanup(file_part);
        }
        *created_multifile = multifile_url;
    }

    inline void CreateDirAt(const std::string &dir) {
        ASSERT_EQ(driver_dirExists(dir.c_str()), kFalse) << "Remote directory already exists.";
        this->PlanDirCleanup(dir);
        ASSERT_EQ(driver_mkdir(dir.c_str()), kOtherSuccess) << "Failed to create remote directory.";
        ASSERT_EQ(driver_dirExists(dir.c_str()), kTrue) << "Remote directory not found after its creation.";
    }

    inline void CreateRandomDir(std::string *created_dir) {
        ASSERT_NE(created_dir, nullptr);
        std::string new_dir = url.NewRandomDir();
        ASSERT_EQ(driver_dirExists(new_dir.c_str()), kFalse) << "Randomly named remote directory already exists: random name collision.";
        this->PlanDirCleanup(new_dir);
        ASSERT_EQ(driver_mkdir(new_dir.c_str()), kOtherSuccess) << "Failed to create remote directory.";
        ASSERT_EQ(driver_dirExists(new_dir.c_str()), kTrue) << "Remote directory not found after its creation.";
        *created_dir = new_dir;
    }

private:

    std::unordered_set<std::string> created_dirs;
    std::unordered_set<std::string> created_files;
};

class IoTest : public StorageTest {};

class EndToEndTest : public StorageTest {};
}