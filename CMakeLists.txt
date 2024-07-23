cmake_minimum_required(VERSION 3.18)
project(libult VERSION 0.1)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

include(CTest)

option(LIBULT_ENABLE_TESTS "Whether to build tests" OFF)
option(LIBULT_ENABLE_PTHREADS "Whether to build with pthreads support" ON)
option(LIBULT_ENABLE_QTHREADS "Whether to build with Qthreads support" OFF)
option(LIBULT_ENABLE_ARGOBOTS "Whether to build with Argobots support" OFF)

add_subdirectory(src)

export (
    TARGETS libult
    NAMESPACE "${PROJECT_NAME}::"
    FILE ${PROJECT_BINARY_DIR}/cmake/libult/libultTargets.cmake
)

install(
    TARGETS libult
    EXPORT LibultTargets # does this do anything?
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

