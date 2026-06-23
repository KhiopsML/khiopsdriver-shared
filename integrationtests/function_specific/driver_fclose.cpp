#include <string>
#include <gtest/gtest.h>
#include "../fixture_storage.hpp"
#include "khiops_driver_common/driver.h"
#include "../returnval.hpp"
#include "../errorstrings.hpp"
#include "../utils.hpp"

using namespace std;

class DriverFCloseTest : public StorageTest {};

TEST_F(DriverFCloseTest, FileClosingRightAfterWriteModeOpeningOK) {
    string new_file = this->url.RandomOutputFile();
    ASSERT_EQ(driver_fileExists(new_file.c_str()), kFalse) << "Randomly named file already exists: random name collision.";
    this->PlanFileCleanup(new_file);
    void *handle = driver_fopen(new_file.c_str(), 'w');
    ASSERT_NE(handle, nullptr) << "Failed to open file in write-mode.";
    ASSERT_EQ(driver_fclose(handle), kSuccess) << "Failed to close file.";
    ASSERT_EQ(driver_fileExists(new_file.c_str()), kTrue) << "File has not been created.";
    long long int size = driver_getFileSize(new_file.c_str());
    ASSERT_NE(size, -1LL) << "Failed to get file size.";
    ASSERT_EQ(size, 0LL) << "File is not empty.";
}