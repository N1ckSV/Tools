
cmake_minimum_required(VERSION 3.10)


add_executable(
    ValueLockExample
    ValueLockExample.cpp
    )
    

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")

target_include_directories(ValueLockExample  PUBLIC "${NickSVTools_INCLUDE_DIR}")
