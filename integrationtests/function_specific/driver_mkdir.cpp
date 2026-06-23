#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "../fixture_storage.hpp"
#include "khiops_driver_common/driver.h"
#include "../returnval.hpp"
#include "../errorstrings.hpp"

using namespace std;

class DriverMkdirTest : public StorageTest {
protected:
    vector<string> created_dirs;
    inline void SetUp() override {
        StorageTest::SetUp();
        this->created_dirs.clear();
    }
    inline void TearDown() override {
        for (const string &created_dir : this->created_dirs) {
            if (driver_dirExists(created_dir.c_str()) == kTrue) {
                ASSERT_EQ(driver_rmdir(created_dir.c_str()), kOtherSuccess) << "Failed to remove created directory.";
            }
        }
        StorageTest::TearDown();
    }
};

TEST_F(DriverMkdirTest, SimplestCaseOK) {
    string created_dir = this->url.NewRandomDir();
    // Make sure the remote directory does not already exist.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kFalse) << "Randomly named remote directory already exists: random name collision.";
    // Create the remote directory.
    ASSERT_EQ(driver_mkdir(created_dir.c_str()), kOtherSuccess) << "Failed to create remote directory.";
    // Save the directory URI for removal during clean-up.
    this->created_dirs.push_back(created_dir);
    // Make sure the remote directory now exists.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kTrue) << "Remote directory not found after its creation.";
}