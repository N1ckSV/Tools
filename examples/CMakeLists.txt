
cmake_minimum_required(VERSION 3.10)


add_executable(
    ValueLockExample
    ValueLockExample.cpp
)
add_executable(
    MatrixExample
    MatrixExample.cpp
)
    

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")

target_include_directories(ValueLockExample  PUBLIC "${NickSVTools_INCLUDE_DIR}")
target_include_directories(MatrixExample  PUBLIC "${NickSVTools_INCLUDE_DIR}")
