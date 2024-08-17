
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

set_target_properties(MemoryTest ValueLockTest TypeTraitsTest
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
)

target_include_directories(MemoryTest  PUBLIC ..)
target_include_directories(ValueLockTest  PUBLIC  ..)
target_include_directories(TypeTraitsTest  PUBLIC  ..)

add_test(NAME MemoryTest COMMAND MemoryTest)
#For error code 255 increase timeout below
add_test(NAME ValueLockTest COMMAND ValueLockTest)
add_test(NAME TypeTraitsTest COMMAND TypeTraitsTest)

set_tests_properties(ValueLockTest PROPERTIES TIMEOUT 30)  