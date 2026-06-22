#pragma once

#include "khiops_driver_common/driver.h"
#include "urls.hpp"
#include "returnval.hpp"
#include <cstddef>
#include <gtest/gtest.h>
#include <string>

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
  void SetUp() override {
    UrlFixture::SetUp();
    ASSERT_EQ(driver_connect(), kOtherSuccess) << "Driver failed to connect during test initialization.";
    ASSERT_EQ(driver_isConnected(), kTrue) << "After driver connected, it is still disconnected.";
  }
  void TearDown() override {
    ASSERT_EQ(driver_disconnect(), kOtherSuccess) << "Driver failed to disconnect during test finalization.";
    ASSERT_EQ(driver_isConnected(), kFalse) << "After driver disconnected, it is still connected.";
    UrlFixture::TearDown();
  }
};

class IoTest : public StorageTest {};

class EndToEndTest : public StorageTest {};
}