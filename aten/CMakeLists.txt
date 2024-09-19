cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")
set(CMAKE_C_STANDARD 11 CACHE STRING "The C standard whose features are requested.")

file(GLOB_RECURSE ATen_CORE_HEADERS "src/ATen/core/*.h")
file(GLOB_RECURSE ATen_CORE_SRCS "src/ATen/core/*.cpp")

# add_library(aten ${ATen_CORE_SRCS} ${ATen_CORE_HEADERS})
