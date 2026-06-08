#pragma once

#include <string>

namespace khiops_driver_common {

// Function to check that an URL points to directory and log an error if it is not the case.
int CheckIsDirUrl(const std::string &url);

// Function to check that an URL points to file and log an error if it is not the case.
// "File" here refers to an non-directory object, not necessarily a file object stored in a file share (it can be a blob, too).
int CheckIsFileUrl(const std::string &url);

// Function to check that the driver has been initialized and log an error if it is not the case
int CheckInitialized();

// Function to check that the driver has NOT been initialized and log an error if it has been initialized
int CheckNotInitialized();

// Function to check that an argument is not a null pointer and log and error if it is a null pointer
int CheckNotNull(const void *arg, const char *param, const char *func);

}