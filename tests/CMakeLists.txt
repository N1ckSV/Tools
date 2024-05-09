
cmake_minimum_required(VERSION 3.10)

include(CTest)

add_executable(MemoryTest
    MemoryTest.cpp
    )
add_executable(ValueLockTest
    ValueLockTest.cpp
    )

set_target_properties(MemoryTest ValueLockTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
)


target_include_directories(MemoryTest
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    )

target_include_directories(ValueLockTest
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    )

add_test(NAME MemoryTest COMMAND MemoryTest)
add_test(NAME ValueLockTest COMMAND ValueLockTest)

set_tests_properties(ValueLockTest PROPERTIES TIMEOUT 10) 