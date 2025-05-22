cmake_minimum_required (VERSION 3.19)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_CXX_STANDARD 20)
project (CryptomatteAPIBuild)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

if (MAIN_PROJECT)
    set(CRYPTOMATTE_API_EXTENDED_WARNINGS ON)
    set(CRYPTOMATTE_API_IMAGE_USE_VCPKG ON)
    set(CRYPTOMATTE_API_BUILD_TESTS ON)
endif()


option(CRYPTOMATTE_API_EXTENDED_WARNINGS OFF)
option(CRYPTOMATTE_API_IMAGE_USE_VCPKG OFF)
option(CRYPTOMATTE_API_BUILD_TESTS OFF)
option(CRYPTOMATTE_API_BUILD_EXAMPLES OFF)
option(CRYPTOMATTE_API_BUILD_DOCS OFF)
option(CRYPTOMATTE_API_BUILD_BENCHMARKS OFF)

if (CRYPTOMATTE_API_IMAGE_USE_VCPKG)
    include("${PROJECT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message("CryptomatteAPI: Picked up toolchain at '${PROJECT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake'")
endif()

# Add thirdparty libraries
# --------------------------------------------------------------------------

find_package(OpenImageIO CONFIG REQUIRED)

# CompressedImage for loading images into a compressed memory buffer as well as compressing the mattes in-memory
set(COMPRESSED_IMAGE_USE_VCPKG OFF)
add_subdirectory(thirdparty/compressed-image)

# spdlog for loggig
set(SPDLOG_USE_STD_FORMAT ON)
add_subdirectory(thirdparty/spdlog)

# Projects
# --------------------------------------------------------------------------
add_subdirectory(cryptomatte_api)

if (CRYPTOMATTE_API_BUILD_TESTS)
    add_library(doctest INTERFACE)
    target_include_directories(doctest SYSTEM INTERFACE thirdparty/doctest/doctest)
    add_subdirectory(test)
endif()

if (CRYPTOMATTE_API_BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_INSTALL OFF)
    set(BENCHMARK_INSTALL_DOCS OFF)
    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
    add_subdirectory(thirdparty/benchmark)
    add_subdirectory(benchmark)
endif()

if (COMPRESSED_IMAGE_BUILD_DOCS)
    add_subdirectory(docs)
endif()
