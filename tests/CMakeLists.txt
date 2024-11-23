
cmake_minimum_required(VERSION 3.10)

include(CTest)

add_executable(
    MemoryTest
    MemoryTest.cpp
    )
add_executable(
    ValueLockTest
    ValueLockTest.cpp
    )
add_executable(
    TypeTraitsTest
    TypeTraitsTest.cpp
    )

add_executable(
    MatrixTest
    MatrixTest.cpp
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

target_include_directories(MemoryTest PUBLIC "${NickSVTools_INCLUDE_DIR}")
target_include_directories(ValueLockTest PUBLIC "${NickSVTools_INCLUDE_DIR}")
target_include_directories(TypeTraitsTest PUBLIC "${NickSVTools_INCLUDE_DIR}")
target_include_directories(MatrixTest PUBLIC "${NickSVTools_INCLUDE_DIR}")

message(STATUS "NickSVTools_INCLUDE_DIR: ${NickSVTools_INCLUDE_DIR}")

add_test(NAME MemoryTest COMMAND MemoryTest)
#For error code 255 increase timeout below
add_test(NAME ValueLockTest COMMAND ValueLockTest)
add_test(NAME TypeTraitsTest COMMAND TypeTraitsTest)
add_test(NAME MatrixTest COMMAND MatrixTest)

set_tests_properties(ValueLockTest PROPERTIES TIMEOUT 30)  