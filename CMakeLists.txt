# CMakeList.txt : CMake project for wudcrgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("udcrgen")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# All implementation details of the recognition algorithm, generator etc.
# are compiled in this library, on which all the executables depend.
add_library(udcr STATIC "embed.cpp"  "graph.h" "graph.cpp" "embed.h"
"svg.cpp" "svg.h" "geometry.cpp" "geometry.h" "config.cpp" "config.h" "util.h" "util.cpp"
"ipe.h" "ipe.cpp" "translate.h" "translate.cpp")

# This is the main executable.
add_executable(udcrgen "main.cpp")
target_link_libraries(udcrgen udcr)

# This is the generator executable, which produces test cases.
add_executable(gencases "gencases.cpp")
target_link_libraries(gencases udcr)

# GTest
enable_testing()
find_package(GTest REQUIRED)

# Unit Tests
add_executable(unittests tests.cpp)
target_link_libraries(unittests ${GTEST_BOTH_LIBRARIES} udcr)
add_test(unittests unittests)
