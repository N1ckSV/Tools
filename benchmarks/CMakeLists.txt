
cmake_minimum_required(VERSION 3.16)

project(NickSVToolsBenchmark LANGUAGES C CXX VERSION 0.1.0)

include(GNUInstallDirs)
include(CheckSymbolExists)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(benchmark_DIR_HINTS "E:/SDK/google/benchmark/install")

find_package(benchmark REQUIRED
    HINTS ${benchmark_DIR_HINTS})


message(STATUS "\n\n")
message(STATUS "C++ Standart: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Make program: ${CMAKE_MAKE_PROGRAM}")
message(STATUS "C++ Make generator: ${CMAKE_GENERATOR}")
message(STATUS "C++ Make generator: ${CMAKE_GENERATOR}")
message(STATUS "C++ Make generator instance: ${CMAKE_GENERATOR_INSTANCE}")
message(STATUS "C++ Make generator platform: ${CMAKE_GENERATOR_PLATFORM}")
message(STATUS "C++ Make generator toolset: ${CMAKE_GENERATOR_TOOLSET}")
message(STATUS "\n\n")
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

target_link_libraries(NickSVToolsBenchmark
    benchmark::benchmark
)
#-----------------------------------------------
#-----------------------------------------------
#-----------------------------------------------





