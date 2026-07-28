#include <string>
#include <vector>
#include <cstdio>
#include <gtest/gtest.h>
#include "../fixture_storage.hpp"
#include "khiops_driver_common/driver.h"
#include "../returnval.hpp"
#include "../errorstrings.hpp"
#include "../utils.hpp"

using namespace std;

class DriverRemoveTest : public StorageTest {};

TEST_F(DriverRemoveTest, SimplestCaseOK) {
    string new_file; this->CreateRandomFileWithContent(&new_file);

    // Remove remote file: should succeed.
    ASSERT_EQ(driver_remove(new_file.c_str()), kOtherSuccess);
    // Remote file should not exist on remote filesystem anymore.
    ASSERT_EQ(driver_fileExists(new_file.c_str()), kFalse);
}

TEST_F(DriverRemoveTest, DoubleRemovalOK) {
    // It is not clear at the moment whether the driver_remove function should be idempotent or more like C's remove function.
    GTEST_SKIP();

    string new_file; this->CreateRandomFileWithContent(&new_file);

    // First removal.
    ASSERT_EQ(driver_remove(new_file.c_str()), kOtherSuccess);
    // Remote file should not exist on remote filesystem anymore.
    ASSERT_EQ(driver_fileExists(new_file.c_str()), kFalse);
    
    // Double removal: should succeed because removal operation is idempotent.
    ASSERT_EQ(driver_remove(new_file.c_str()), kOtherSuccess);
    // Remote file should not exist on remote filesystem.
    ASSERT_EQ(driver_fileExists(new_file.c_str()), kFalse);
}

TEST_F(DriverRemoveTest, RemoveNonexistentOK) {
    // It is not clear at the moment whether the driver_remove function should be idempotent or more like C's remove function.
    GTEST_SKIP();

    string nonexistent_file = url.RandomOutputFile();
    ASSERT_EQ(driver_fileExists(nonexistent_file.c_str()), kFalse) << "Randomly named remote file exists: random name collision.";

    // Try to remove nonexistent file: should succeed because removal operation is idempotent.
    ASSERT_EQ(driver_remove(nonexistent_file.c_str()), kOtherSuccess);
    // Remote file should not exist on remote filesystem.
    ASSERT_EQ(driver_fileExists(nonexistent_file.c_str()), kFalse);
}

TEST_F(DriverRemoveTest, RemoveMultifileOK) {
    string new_multifile; this->CreateRandomMultiFileWithContent(&new_multifile);
    ASSERT_EQ(driver_remove(new_multifile.c_str()), kOtherSuccess);
    ASSERT_EQ(driver_fileExists(new_multifile.c_str()), kFalse);
}

TEST_F(DriverRemoveTest, NotConnectedKO) {
    if (string(driver_getDriverName()) == "GCS driver") GTEST_SKIP();  // https://github.com/KhiopsML/khiopsdriver-gcs/issues/54
    string new_file; this->CreateRandomFileWithContent(&new_file);

    // Disconnect.
    ASSERT_EQ(driver_disconnect(), kOtherSuccess);

    // Try to remove remote file while not connected: should fail.
    ASSERT_EQ(driver_remove(new_file.c_str()), kOtherFailure);
    // ASSERT_NE(string(driver_getlasterror()).find(ERR_NOT_CONNECTED), string::npos);
    ASSERT_NE(driver_getlasterror(), nullptr);

    // Connect to verify file still exists on remote filesystem.
    ASSERT_EQ(driver_connect(), kOtherSuccess);
    ASSERT_EQ(driver_fileExists(new_file.c_str()), kTrue);
}

TEST_F(DriverRemoveTest, NullPointerKO) {
    // Pass null pointer to removal function: should fail.
    ASSERT_EQ(driver_remove(nullptr), kOtherFailure);
    // char formatted_null_ptr_error[256];
    // snprintf(formatted_null_ptr_error, 256, ERR_NULL_PTR, "driver_remove", "filename");
    // ASSERT_NE(string(driver_getlasterror()).find(formatted_null_ptr_error), string::npos);
    ASSERT_NE(driver_getlasterror(), nullptr);
}

TEST_F(DriverRemoveTest, InvalidURLKO) {
    // Pass invalid URL to removal function: should fail.
    ASSERT_EQ(driver_remove("invalid URL"), kOtherFailure);
    ASSERT_NE(driver_getlasterror(), nullptr);
}