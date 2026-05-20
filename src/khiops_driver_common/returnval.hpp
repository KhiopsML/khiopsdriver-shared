/*
 * The return values of the functions exposed by the driver library.
 */

#pragma once

namespace khiops_driver_common {
namespace return_values {
constexpr int kFailure = -1;
constexpr int kSuccess = 0;
constexpr int kOtherFailure = 0;
constexpr int kOtherSuccess = 1;
constexpr int kFalse = 0;
constexpr int kTrue = 1;
} // namespace return_values
} // namespace khiops_driver_common
