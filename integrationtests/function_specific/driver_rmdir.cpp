#include <string>
#include <gtest/gtest.h>
#include "../fixture_storage.hpp"
#include "khiops_driver_common/driver.h"
#include "../returnval.hpp"
#include "../errorstrings.hpp"

using namespace std;

class DriverRmdirTest : public StorageTest {
protected:
    inline void CreateRandomDir(string *created_dir) const {
        ASSERT_NE(created_dir, nullptr);
        string new_dir = url.NewRandomDir();
        ASSERT_EQ(driver_dirExists(new_dir.c_str()), kFalse) << "Randomly named remote directory already exists: random name collision.";
        ASSERT_EQ(driver_mkdir(new_dir.c_str()), kOtherSuccess) << "Failed to create remote directory.";
        ASSERT_EQ(driver_dirExists(new_dir.c_str()), kTrue) << "Remote directory not found after its creation.";
        *created_dir = new_dir;
    }
};

TEST_F(DriverRmdirTest, SimplestCaseOK) {
    string created_dir; this->CreateRandomDir(&created_dir);
    // Make sure the remote directory exists.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kTrue) << "Remote directory not found after its creation.";
    // Remove the remote directory.
    ASSERT_EQ(driver_rmdir(created_dir.c_str()), kOtherSuccess) << "Failed to remove remote directory.";
    // Make sure the remote directory does not exist anymore.
    ASSERT_EQ(driver_dirExists(created_dir.c_str()), kFalse) << "Remote directory still found after its removal.";
}