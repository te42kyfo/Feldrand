cmake_minimum_required(VERSION 2.6)
project(FELDRAND)

cmake_policy(SET CMP0003 NEW)

set(FELDRAND_VERSION_MAJOR 0)
set(FELDRAND_VERSION_MINOR 1)
set(FELDRAND_CORE_SOVERSION 0)
set(FELDRAND_VISUALISATION_SOVERSION 0)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_COMPILER "/usr/bin/g++-4.8") # the only tested compiler so far
message("Using compiler ${CMAKE_CXX_COMPILER}")

# compiler magic (blatantly stolen from avogadro, which stole from KDE)
if (CMAKE_COMPILER_IS_GNUCXX)
  # I need c++11 and pthread
  set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")

  set(CMAKE_CXX_FLAGS_WARN "-Wnon-virtual-dtor -Wno-long-long -Wcast-align -Wchar-subscripts -Wall -Wextra -Wpointer-arith -Wformat-security -fno-check-new -fno-common -pedantic")
  # On Linux with GCC 4.3+ these flags generate thousands of warnings, so they
  # are now optional. Most of the warnings are in included headers.
  option(EXTRA_WARNINGS
    "Enable extra GCC warnings - can generate thousands" OFF)
  if(EXTRA_WARNINGS)
    # Enable extra compiler warnings for extra checking
    set (CMAKE_CXX_FLAGS_WARN "${CMAKE_CXX_FLAGS_WARN} -Wconversion -Woverloaded-virtual -Wfloat-equal")
  endif()
  # Set some linker flags
  if(CMAKE_SYSTEM_NAME MATCHES Linux)
    set(CMAKE_SHARED_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS
      "-Wl,--enable-new-dtags ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS
      "-Wl,--enable-new-dtags ${CMAKE_SHARED_LINKER_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS
      "-Wl,--fatal-warnings -Wl,--no-undefined -lc ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS
      "-Wl,--enable-new-dtags ${CMAKE_EXE_LINKER_FLAGS}")
  endif()
  # determining gcc version
  exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE	_gcc_version_info)
  string (REGEX MATCH "[345]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
  # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the
  # patch level, handle this here:
  if (NOT _gcc_version)
    string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
  endif ()

  # Set up additional build flags for particular build types. These will be added to CMAKE_CXX_FLAGS,
  # they will not replace them.
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DQT_NO_DEBUG_OUTPUT -Wl,-s ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline ${CMAKE_CXX_FLAGS_WARN}")

  # define WITH_SSE2 to enable the SSE2 instruction set.
  # Available on Pentium 4, Athlon 64, and newer CPUs.
  # Enabled by default on the x86_64 architecture, but not on the x86 architecture.
  if(WITH_SSE2)
    message(STATUS "Enabling the SSE2 instruction set")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
  endif()
endif()


set(LIBRARY_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/lib
  CACHE PATH
  "Directory for all libraries")

set(EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory for all executables")

set(FELDRAND_INCLUDE_DIR
  ${PROJECT_BINARY_DIR}/include
  CACHE PATH
  "Directory for all executables")

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
