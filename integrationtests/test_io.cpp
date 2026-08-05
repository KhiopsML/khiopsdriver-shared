#include "khiops_driver_common/driver.h"
#include "fixture_storage.hpp"
#include "returnval.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>

#include <boost/process/v2/environment.hpp>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;

void TestFSeek(string sUrl, bool bCrLf = false);

TEST_F(IoTest, FSeekSingleFile) { TestFSeek(url.File()); }

TEST_F(IoTest, FSeekMultipartFile) { TestFSeek(url.MultisplitFile(), true); }

void TestFSeek(string sUrl, bool bCrLf) {
  void *handle;
  char buffer[32];
  ASSERT_NE(handle = driver_fopen(sUrl.c_str(), 'r'), nullptr);

  ASSERT_EQ(driver_fseek(handle, bCrLf ? 929 : 922, 0), kSuccess);
  ASSERT_EQ(driver_fread(buffer, 1, 7, handle), 7);
  buffer[7] = 0;
  ASSERT_STREQ(buffer, "Jamaica");

  ASSERT_EQ(driver_fseek(handle, -55, 1), kSuccess);
  ASSERT_EQ(driver_fread(buffer, 1, 13, handle), 13);
  buffer[13] = 0;
  ASSERT_STREQ(buffer, "Other-service");

  ASSERT_EQ(driver_fseek(handle, bCrLf ? -664 : -658, 2), kSuccess);
  ASSERT_EQ(driver_fread(buffer, 1, 13, handle), 13);
  buffer[13] = 0;
  ASSERT_STREQ(buffer, "Never-married");

  ASSERT_EQ(driver_fclose(handle), kSuccess);
}

TEST_F(IoTest, FReadAtEndOfFile) {
  char ibuffer[64];
  void *ihandle;
  long long int filesize;

  // We want the file to be at least 10-byte long
  ASSERT_GT(filesize = driver_getFileSize(url.File().c_str()), 10);

  ASSERT_NE(ihandle = driver_fopen(url.File().c_str(), 'r'), nullptr);

  // Reading the first four bytes... OK
  ASSERT_EQ(driver_fread(ibuffer, 1, 4, ihandle), 4);
  ibuffer[4] = 0;
  ASSERT_STREQ(ibuffer, "Labe");
  // Reading the next four bytes... OK
  ASSERT_EQ(driver_fread(ibuffer, 1, 4, ihandle), 4);
  ibuffer[4] = 0;
  ASSERT_STREQ(ibuffer, "l\tag");
  ASSERT_EQ(driver_fseek(ihandle, -2, 2), kSuccess);
  // Trying to read four bytes from the last but one... it should read the last
  // two bytes
  ASSERT_EQ(driver_fread(ibuffer, 1, 4, ihandle), 2);
  ibuffer[2] = 0;
  ASSERT_STREQ(ibuffer, "e\n");
  // Trying to read four bytes while we are already at the end of the file...
  // it should raise an error
  ASSERT_EQ(driver_fread(ibuffer, 1, 4, ihandle), kFailure);
  ASSERT_THAT(driver_getlasterror(),
              testing::HasSubstr("Cannot read after end of file."));
  ASSERT_STREQ(ibuffer, "e\n"); // Buffer content unchanged

  ASSERT_EQ(driver_fclose(ihandle), kSuccess);
}

TEST_F(IoTest, FReadAtEndOfFileWithConcurrentWrite) {
  string file = url.RandomOutputFile();
  char ibuffer[64]{};
  void *ihandle;
  void *ohandle;

  // Write initial data to the file
  ASSERT_NE(ohandle = driver_fopen(file.c_str(), 'w'), nullptr);
  ASSERT_EQ(driver_fwrite("abc", 1, 3, ohandle), 3);
  ASSERT_EQ(driver_fflush(ohandle), kSuccess);
  // FIXME: should we test reading from a file that is still open for writing?
  //        It is currently not possible because all drivers do not behave the same, 
  //        (not all drivers actually write upon fflush, some only write upon fclose)
  //        but it could be a useful feature to add in the future. 
  //        For now we close the file after writing to be able to read from it.
  ASSERT_EQ(driver_fclose(ohandle), kSuccess);

  // Open the file for reading. Internally this will fetch the ETag of the file
  ASSERT_NE(ihandle = driver_fopen(file.c_str(), 'r'), nullptr);
  // This first reading operation should find an ETag identical to the one
  // fetched previously
  ASSERT_EQ(driver_fread(ibuffer, 1, 3, ihandle), 3);
  ASSERT_STREQ(ibuffer, "abc");

  // Add some data to the file. The ETag of the file will be changed
  // As in FIXME above, we have to reopen in append mode for drivers that do not
  // create the file upon fflush, forcing us to fclose.
  ASSERT_NE(ohandle = driver_fopen(file.c_str(), 'a'), nullptr);
  ASSERT_EQ(driver_fwrite("def", 1, 3, ohandle), 3);
  ASSERT_EQ(driver_fflush(ohandle), kSuccess);
  ASSERT_EQ(driver_fclose(ohandle), kSuccess);

  // This second reading operation should fail because we are at already at end of file.
  // This error case has priority above the "file modified" error case because the user could know they were at end of file, using driver_getFileSize.
  ASSERT_EQ(driver_fread(ibuffer, 1, 6, ihandle), kFailure);
  ASSERT_THAT(
      driver_getlasterror(),
      testing::HasSubstr("Cannot read after end of file."));
  ASSERT_STREQ(ibuffer, "abc"); // Input buffer content unchanged

  // Open file again. This will fetch the new ETag
  ASSERT_EQ(driver_fclose(ihandle), kSuccess);
  ASSERT_NE(ihandle = driver_fopen(file.c_str(), 'r'), nullptr);
  // Now read the content again. It should be the new one
  ASSERT_EQ(driver_fread(ibuffer, 1, 20, ihandle), 6);
  ASSERT_STREQ(ibuffer, "abcdef");

  ASSERT_EQ(driver_fclose(ihandle), kSuccess);
  ASSERT_EQ(driver_remove(file.c_str()), kOtherSuccess);
}

TEST_F(IoTest, FReadWithConcurrentWrite) {
  string file = url.RandomOutputFile();
  char ibuffer[64]{};
  void *ihandle;
  void *ohandle;

  // Write initial data to the file
  ASSERT_NE(ohandle = driver_fopen(file.c_str(), 'w'), nullptr);
  ASSERT_EQ(driver_fwrite("abcdef", 1, 6, ohandle), 6);
  ASSERT_EQ(driver_fflush(ohandle), kSuccess);
  // FIXME: should we test reading from a file that is still open for writing?
  //        It is currently not possible because all drivers do not behave the same, 
  //        (not all drivers actually write upon fflush, some only write upon fclose)
  //        but it could be a useful feature to add in the future. 
  //        For now we close the file after writing to be able to read from it.
  ASSERT_EQ(driver_fclose(ohandle), kSuccess);

  // Open the file for reading. Internally this will fetch the ETag of the file
  ASSERT_NE(ihandle = driver_fopen(file.c_str(), 'r'), nullptr);
  // This first reading operation should find an ETag identical to the one
  // fetched previously
  ASSERT_EQ(driver_fread(ibuffer, 1, 3, ihandle), 3);
  ASSERT_STREQ(ibuffer, "abc");

  // Add some data to the file. The ETag of the file will be changed
  // As in FIXME above, we have to reopen in append mode for drivers that do not 
  // create the file upon fflush, forcing us to fclose.
  ASSERT_NE(ohandle = driver_fopen(file.c_str(), 'a'), nullptr);
  ASSERT_EQ(driver_fwrite("ghi", 1, 3, ohandle), 3);
  ASSERT_EQ(driver_fflush(ohandle), kSuccess);
  ASSERT_EQ(driver_fclose(ohandle), kSuccess);

  // This second reading operation should fail because it should find an ETag
  // different to the one fetched by the driver_fopen call
  ASSERT_EQ(driver_fread(ibuffer, 1, 3, ihandle), kFailure);
  ASSERT_THAT(
      driver_getlasterror(),
      testing::HasSubstr("The file has been updated while reading it."));
  ASSERT_STREQ(ibuffer, "abc"); // Input buffer content unchanged

  // Open file again. This will fetch the new ETag
  ASSERT_EQ(driver_fclose(ihandle), kSuccess);
  ASSERT_NE(ihandle = driver_fopen(file.c_str(), 'r'), nullptr);
  // Now read the content again. It should be the new one
  ASSERT_EQ(driver_fread(ibuffer, 1, 20, ihandle), 9);
  ASSERT_STREQ(ibuffer, "abcdefghi");

  ASSERT_EQ(driver_fclose(ihandle), kSuccess);
  ASSERT_EQ(driver_remove(file.c_str()), kOtherSuccess);
}

// Scenario: a file contains 5 structs of 4 bytes each (2 uint16_t fields),
// followed by 2 trailing bytes ('A', 'B'), for a total of 22 bytes.
// The caller asks for 6 elements of 4 bytes (24 bytes), crossing EOF.
// Expected behaviour (mirroring fread from the C stdlib):
//   - driver_fread returns 5 (number of complete 4-byte elements read)
//   - the first 20 bytes of the buffer match the 5 written structs exactly
//   - the 2 trailing bytes ('A', 'B') are physically deposited at buffer[20]
//     and buffer[21], even though they do not form a complete element
//   - buffer[22] and buffer[23] are untouched (the 6th element slot is partial)
//   - the file offset is at EOF after the call
//   - a subsequent driver_fread returns kFailure (already at EOF)
TEST_F(IoTest, FReadPartialElementAtEOF) {
  // Layout of each struct: two uint16_t fields.
  // We use a plain struct with no padding (fields are 2 bytes each = 4 bytes total).
  struct TwoU16 {
    uint16_t a;
    uint16_t b;
  };
  static_assert(sizeof(TwoU16) == 4, "TwoU16 must be exactly 4 bytes");

  const string file = url.RandomOutputFile();
  PlanFileCleanup(file);

  // --- Write phase ---
  // Write 5 structs then 2 trailing bytes ('A', 'B') = 22 bytes total.
  {
    void *ohandle = driver_fopen(file.c_str(), 'w');
    ASSERT_NE(ohandle, nullptr);

    const TwoU16 structs[5] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}};
    ASSERT_EQ(driver_fwrite(structs, sizeof(TwoU16), 5, ohandle), 5)
        << "Expected 5 elements written";

    const char trailer[2] = {'A', 'B'};
    ASSERT_EQ(driver_fwrite(trailer, 1, 2, ohandle), 2)
        << "Expected 2 trailing bytes written";

    ASSERT_EQ(driver_fclose(ohandle), kSuccess);
  }

  // Verify the file size is exactly 22 bytes.
  ASSERT_EQ(driver_getFileSize(file.c_str()), 22LL)
      << "File must be exactly 22 bytes (5 structs * 4 bytes + 2 trailing bytes)";

  // --- Read phase ---
  // Ask for 6 elements of 4 bytes (24 bytes) while only 22 are available.
  // The buffer is zeroed so we can detect any unexpected overwrites.
  uint8_t rbuffer[6 * sizeof(TwoU16)] = {};
  void *ihandle = driver_fopen(file.c_str(), 'r');
  ASSERT_NE(ihandle, nullptr);

  const long long int elements_read = driver_fread(rbuffer, sizeof(TwoU16), 6, ihandle);

  // Only 5 complete elements fit: 22 / 4 == 5.
  ASSERT_EQ(elements_read, 5LL)
      << "driver_fread must return the number of complete elements read (5), not byte count";

  // The first 20 bytes (5 * 4) must match the 5 written structs byte-for-byte.
  const TwoU16 expected[5] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}};
  ASSERT_EQ(memcmp(rbuffer, expected, elements_read * sizeof(TwoU16)), 0)
      << "First " << (elements_read * sizeof(TwoU16))
      << " bytes of buffer must exactly match the written structs";

  // fread deposits ALL bytes it can read into the buffer, even the 2 trailing
  // bytes that do not form a complete element. So rbuffer[20]=='A' and
  // rbuffer[21]=='B' are physically written, while rbuffer[22..23] are untouched.
  ASSERT_EQ(rbuffer[20], static_cast<uint8_t>('A'))
      << "First trailing byte ('A') must be present in the buffer";
  ASSERT_EQ(rbuffer[21], static_cast<uint8_t>('B'))
      << "Second trailing byte ('B') must be present in the buffer";
  ASSERT_EQ(rbuffer[22], 0) << "Byte 22 must be untouched (zero-initialised)";
  ASSERT_EQ(rbuffer[23], 0) << "Byte 23 must be untouched (zero-initialised)";

  // A second read attempt, still at EOF, must fail.
  uint8_t dummy[4] = {};
  ASSERT_EQ(driver_fread(dummy, sizeof(TwoU16), 1, ihandle), kFailure)
      << "Reading past EOF must return kFailure";
  ASSERT_THAT(driver_getlasterror(), testing::HasSubstr("Cannot read after end of file."));

  ASSERT_EQ(driver_fclose(ihandle), kSuccess);
}
