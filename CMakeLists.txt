cmake_minimum_required(VERSION 2.6)
project(FELDRAND)

cmake_policy(SET CMP0003 NEW)

set(FELDRAND_VERSION_MAJOR 0)
set(FELDRAND_VERSION_MINOR 2)
set(FELDRAND_CORE_SOVERSION 0)
set(FELDRAND_VISUALISATION_SOVERSION 0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Build type. Avilable types are Debug and Release")
endif()

# compiler magic (blatantly stolen from avogadro, which stole from KDE)
if (CMAKE_COMPILER_IS_GNUCXX)
  # I need c++11 and pthread
  set(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

  # Did I forget anything?
  set(CMAKE_CXX_FLAGS_WARN "-Wnon-virtual-dtor -Wno-long-long -Wcast-align -Wchar-subscripts -Wall -Wextra -Wpointer-arith -Wformat-security -fno-check-new -fno-common -pedantic")

  # Set up additional build flags for particular build types. These will be added to CMAKE_CXX_FLAGS,
  # they will not replace them.
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DQT_NO_DEBUG_OUTPUT -Wl,-s ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_CXX_FLAGS_NATIVE "-O3 -DNDEBUG -DQT_NO_DEBUG_OUTPUT -march=native -Wl,-s ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_CXX_FLAGS_PROFILE "-O2 -g -pg ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_LD_FLAGS_PROFILE "-O2 -g -pg")
endif()





set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(LIBRARY_OUTPUT_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/lib
  CACHE PATH
  "Directory for all libraries" FORCE)

set(EXECUTABLE_OUTPUT_PATH
  ${CMAKE_CURRENT_BINARY_DIR}/bin
  CACHE PATH
  "Directory for all executables" FORCE)

set(FELDRAND_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  CACHE PATH
  "Directory for all executables" FORCE)

message(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(
  ${FELDRAND_INCLUDE_DIR}/config.hpp.in
  ${FELDRAND_INCLUDE_DIR}/config.hpp
  )

MARK_AS_ADVANCED (
  LIBRARY_OUTPUT_PATH
  EXECUTABLE_OUTPUT_PATH
  FELDRAND_INCLUDE_DIR
  )

add_subdirectory(src)
