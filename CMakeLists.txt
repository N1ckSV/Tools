
cmake_minimum_required(VERSION 3.10)


project(
    NickSVTools
    VERSION 1.0.0
    LANGUAGES CXX
)

set(NickSVTools_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/")


include(GNUInstallDirs)

option(NickSVTools_BUILD_EXAMPLES "Build examples" OFF)
option(NickSVTools_BUILD_TESTS "Build tests" OFF)
option(NickSVTools_BUILD_BENCHMARKS "Build benchmarks" OFF)

add_library(NickSVTools INTERFACE)

target_include_directories(NickSVTools
    INTERFACE
    "$<BUILD_INTERFACE:${NickSVTools_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# examples
if (NickSVTools_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# tests
if (NickSVTools_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# tests
if (NickSVTools_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

install(
    DIRECTORY   ${NickSVTools_INCLUDE_DIR}
    DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "cmake/NickSVToolsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/NickSVToolsConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/NickSVTools"
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/NickSVToolsConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/NickSVTools"
)
