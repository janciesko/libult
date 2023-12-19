cmake_minimum_required(VERSION 3.18)
project(KokkosMPIConnector LANGUAGES CXX)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

include(CTest)

set (NAME libult)

option(LIBULT_ENABLE_TESTS "Whether to build tests" OFF)
set(SOURCES)
set(HEADERS)
set (SOURCE_DIRS "./src")

foreach(DIR ${SOURCE_DIRS})
  file(GLOB DIR_SRCS ${DIR}/*.cpp)
  file(GLOB DIR_HDRS ${DIR}/*.hpp)
  list(APPEND SOURCES ${DIR_SRCS})
  list(APPEND HEADERS ${DIR_HDRS})
endforeach()

add_library(${NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${NAME} PUBLIC ${PUBLIC_DEPS})

IF (LIBULT_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(test)
ENDIF()

