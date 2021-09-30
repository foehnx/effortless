cmake_minimum_required(VERSION 2.8.12...3.13)
project(effortless VERSION 0.0.1)
set(EFFORTLESS_INCLUDE_DIR include REALPATH)
include_directories("include")

################################################################################
# Options
################################################################################

option(EFFORTLESS_QUIET "Suppress configuration output from efforless" ON)
option(EFFORTLESS_TESTS "Building the tests" OFF)
option(EFFORTLESS_DEBUG "Enable all debug logging" OFF)

# DebugLogging
if(EFFORTLESS_DEBUG)
  if(NOT EFFORTLESS_QUIET)
    message(STATUS "Enable Debug Logs!")
  endif()
  add_definitions(-DDEBUG_LOG)
endif()

# Change to minimal catkin build
if(DEFINED CATKIN_DEVEL_PREFIX)
  include(cmake/catkin.cmake)
  return()
endif()

# Build Tests
if(NOT EFFORTLESS_TESTS)
  return()
else()
  include(cmake/catch2.cmake)
endif()

################################################################################
# Setup Optional Compilation for Tests
################################################################################

# Check for ccache
if(NOT DEFINED CATKIN_DEVEL_PREFIX)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_ARCH_FLAGS " -march=native")

add_definitions(-std=c++17)
add_definitions(-Ofast)
add_definitions(-fno-finite-math-only)
add_definitions(-Werror)
add_definitions(-Wextra)
add_definitions(-Wall)
add_definitions(-Wpedantic)
add_definitions(-Wunused)
add_definitions(-Wundef)
add_definitions(-Wconversion)
add_definitions(-Wdouble-promotion)
add_definitions(-Wcast-align)
add_definitions(-Wmissing-declarations)
add_definitions(-Wmissing-include-dirs)
add_definitions(-Wnon-virtual-dtor)
add_definitions(-Wredundant-decls)
add_definitions(-Wodr)
add_definitions(-Wunreachable-code)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

# Compiler Specific
set(EFFORTLESS_COMPILER_LIBRARIES)
if (CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
  add_definitions(-Wno-shorten-64-to-32)
  add_definitions(-Wno-sign-conversion)
  add_definitions(-Wno-implicit-int-float-conversion)
  add_definitions(-Wno-c99-extensions)
  add_definitions(-Wno-implicit-int-conversion)
else()
  add_definitions(-Wno-deprecated-copy)
  set(EFFORTLESS_COMPILER_LIBRARIES 'stdc++fs')
endif()

# Set default build type
if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

################################################################################
# Specify Build Resources
################################################################################

file(GLOB_RECURSE EFFORTLESS_EXAMPLES
  examples/*.cpp
)

################################################################################
# Setup Build
################################################################################

# Build tests
enable_testing()
add_executable(tests ${EFFORTLESS_EXAMPLES})
target_link_libraries(tests PRIVATE
  Catch2::Catch2
  ${EFFORTLESS_COMPILER_LIBRARIES}
)
add_test(tests tests)
