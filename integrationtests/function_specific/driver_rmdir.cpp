#include <string>
#include <gtest/gtest.h>
#include "../fixture_storage.hpp"
#include "khiops_driver_common/driver.h"
#include "../returnval.hpp"
#include "../errorstrings.hpp"
#include "../utils.hpp"

using namespace std;

class DriverRmdirTest : public StorageTest {};

TEST_F(DriverRmdirTest, SimplestCaseOK) {
    string created_dir; this->CreateRandomDir(&created_dir);
    // Make sure the remote directory exists.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kTrue) << "Remote directory not found after its creation.";
    // Remove the remote directory.
    ASSERT_EQ(driver_rmdir(created_dir.c_str()), kOtherSuccess) << "Failed to remove remote directory.";
    // Make sure the remote directory does not exist anymore.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kFalse) << "Remote directory still found after its removal.";
}

TEST_F(DriverRmdirTest, RecursiveRemovalOK) {
    string dir_root; this->CreateRandomDir(&dir_root);
    this->CreateDirAt(dir_root + "a/");
    this->CreateDirAt(dir_root + "a/aa/");
    this->CreateDirAt(dir_root + "b/");
    this->CreateDirAt(dir_root + "b/ba/");
    this->CreateEmptyFileAt(dir_root + "b/ba/baa");
    this->CreateDirAt(dir_root + "b/ba/bab/");
    this->CreateDirAt(dir_root + "b/bb/");
    ASSERT_EQ(driver_rmdir(dir_root.c_str()), kOtherSuccess);
    ASSERT_EQ(driver_dirExists((dir_root + "a/").c_str()), kFalse);
    ASSERT_EQ(driver_dirExists((dir_root + "a/aa/").c_str()), kFalse);
    ASSERT_EQ(driver_dirExists((dir_root + "b/").c_str()), kFalse);
    ASSERT_EQ(driver_dirExists((dir_root + "b/ba/").c_str()), kFalse);
    ASSERT_EQ(driver_fileExists((dir_root + "b/ba/baa").c_str()), kFalse);
    ASSERT_EQ(driver_dirExists((dir_root + "b/ba/bab/").c_str()), kFalse);
    ASSERT_EQ(driver_dirExists((dir_root + "b/bb/").c_str()), kFalse);
}