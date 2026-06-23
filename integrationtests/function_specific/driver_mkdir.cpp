#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "../fixture_storage.hpp"
#include "khiops_driver_common/driver.h"
#include "../returnval.hpp"
#include "../errorstrings.hpp"
#include "../utils.hpp"

using namespace std;

class DriverMkdirTest : public StorageTest {};

TEST_F(DriverMkdirTest, SimplestCaseOK) {
    if (GetStorageType() == StorageType::BLOB) {
        // driver_mkdir does nothing when using a blob service.
        GTEST_SKIP();
    }

    string created_dir = this->url.NewRandomDir();
    // Make sure the remote directory does not already exist.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kFalse) << "Randomly named remote directory already exists: random name collision.";
    // Create the remote directory.
    ASSERT_EQ(driver_mkdir(created_dir.c_str()), kOtherSuccess) << "Failed to create remote directory.";
    this->PrepareDirForCleanup(created_dir);
    // Make sure the remote directory now exists.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kTrue) << "Remote directory not found after its creation.";
}