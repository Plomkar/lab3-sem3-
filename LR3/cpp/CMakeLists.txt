cmake_minimum_required(VERSION 3.16)
project(ds_lab3 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(DS_BUILD_TESTS "Build unit tests" ON)
option(DS_BUILD_BENCH "Build benchmarks" ON)
option(DS_COVERAGE "Enable coverage instrumentation" OFF)
option(DS_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

add_library(ds STATIC
    src/cli.cpp
    src/database.cpp
    src/hashes.cpp
    src/serialization.cpp)
target_include_directories(ds PUBLIC include)
target_compile_features(ds PUBLIC cxx_std_17)
target_compile_options(ds PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wold-style-cast)

if(DS_WARNINGS_AS_ERRORS)
    target_compile_options(ds PRIVATE -Werror)
endif()

if(DS_COVERAGE)
    target_compile_options(ds PUBLIC --coverage -O0 -g)
    target_link_options(ds PUBLIC --coverage)
endif()

add_executable(dsdb src/main.cpp)
target_link_libraries(dsdb PRIVATE ds)

if(DS_BUILD_BENCH)
    add_executable(ds_bench bench/bench.cpp)
    target_link_libraries(ds_bench PRIVATE ds)
endif()

if(DS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
