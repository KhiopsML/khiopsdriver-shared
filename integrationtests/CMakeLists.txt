if(NOT DEFINED storage_driver_target_name)
  message(FATAL_ERROR "CMake variable storage_driver_target_name is not defined.")
endif()

# Fetch googletest from its Git repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://github.com/google/googletest.git"
  GIT_TAG "v1.14.0")

set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Find dependencies
find_package(
  Boost
  COMPONENTS process
  REQUIRED)

add_executable(
  basic_test
  test_basic.cpp
  test_io.cpp
  test_end_to_end.cpp
  fixture_storage.hpp
  returnval.hpp
  utils.hpp
  urls.hpp)

target_compile_options(
  basic_test
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4;/wd4101;/wd4625;/wd4710;/wd4711;/permissive->
  PRIVATE $<$<CXX_COMPILER_ID:AppleClang>:-Wall;-Wextra;-pedantic;-Wno-unused-lambda-capture>
  PRIVATE $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-pedantic>)

# MSVC requires /bigobj to build in debug config
target_compile_options(basic_test PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/bigobj>)

if(WIN32)
  target_link_libraries(basic_test PRIVATE
    GTest::gtest GTest::gmock GTest::gmock_main
    ${Boost_LIBRARIES}
    fmt::fmt
    ${storage_driver_target_name}
    bcrypt)
else()
  target_link_libraries(basic_test PRIVATE
    GTest::gtest GTest::gmock GTest::gmock_main
    ${Boost_LIBRARIES}
    fmt::fmt
    ${storage_driver_target_name})
endif()

gtest_discover_tests(basic_test PROPERTIES ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/lsan_suppressions.txt")
