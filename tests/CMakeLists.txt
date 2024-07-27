
cmake_minimum_required(VERSION 3.10)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

add_executable(MemoryTest
    MemoryTest.cpp
    )
add_executable(ValueLockTest
    ValueLockTest.cpp
    )
add_executable(TypeTraitsTest
    TypeTraitsTest.cpp
    )

set_target_properties(MemoryTest ValueLockTest TypeTraitsTest
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
)


target_include_directories(MemoryTest  PRIVATE ..)
target_include_directories(ValueLockTest  PRIVATE  ..)
target_include_directories(TypeTraitsTest  PRIVATE  ..)

add_test(NAME MemoryTest COMMAND MemoryTest)
#For error code 255 increase timeout below
add_test(NAME ValueLockTest COMMAND ValueLockTest)
add_test(NAME TypeTraitsTest COMMAND TypeTraitsTest)

set_tests_properties(ValueLockTest PROPERTIES TIMEOUT 30)  