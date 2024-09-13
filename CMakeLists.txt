cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(MyTorch CXX C)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")
set(CMAKE_C_STANDARD 11 CACHE STRING "The C standard whose features are requested.")

include(CMakeDependentOption)

option(DEBUG "Enable debug symbols" ON)
if(DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_CXX_FLAGS "-g -O0")
  add_compile_definitions(DEBUG)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

enable_testing()
option(BUILD_TEST "Build the test" ON)
cmake_dependent_option(INSTALL_TEST "Install the test" ON "BUILD_TEST" OFF)

add_subdirectory(test)

if(BUILD_TEST)
  add_subdirectory(third_party/googletest)
endif()
