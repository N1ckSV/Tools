
cmake_minimum_required(VERSION 3.16)

project(NickSVToolsBenchmark LANGUAGES C CXX VERSION 0.1.0)

include(GNUInstallDirs)
include(CheckSymbolExists)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(benchmark_DIR_HINTS "E:/SDK/google/benchmark/install")

find_package(benchmark REQUIRED
    HINTS ${benchmark_DIR_HINTS})


#-----------------------------------------------
#-----------------------------------------------
#-----------------------------------------------





#-----------------------------------------------
#CONFIGURING LIBRARY
#-----------------------------------------------
add_executable(NickSVToolsBenchmark
    ValueLockBenchmark.cpp
    )
#target_include_directories(NickSVToolsBenchmark PUBLIC
#    "$<BUILD_INTERFACE:${NickSVChat_INCLUDE_DIR}>"
#    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)
#target_include_directories(NickSVToolsBenchmark SYSTEM PUBLIC
#    "$<BUILD_INTERFACE:${NickSVChat_EXTERNAL_INCLUDE_DIRS}>"
#    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
#)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")

target_include_directories(NickSVToolsBenchmark  PUBLIC "${NickSVTools_INCLUDE_DIR}")

target_link_libraries(NickSVToolsBenchmark
    benchmark::benchmark
)
#-----------------------------------------------
#-----------------------------------------------
#-----------------------------------------------





