# CMakeList.txt : CMake project for wudcrgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("udcrgen")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# All implementation details of the recognition algorithm, generator etc.
# are compiled in this library, on which all the executables depend.
add_library(udcr STATIC
	"embed.cpp" "embed.h"
	"config.cpp" "config.h"
	"output/svg.cpp" "output/svg.h"
	"output/ipe.cpp" "output/ipe.h"
	"output/translate.cpp" "output/translate.h"
	"utility/graph.cpp" "utility/graph.h"
	"utility/geometry.cpp" "utility/geometry.h"
	"utility/util.cpp" "utility/util.h")

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
add_executable(unittests "test/tests.cpp")
target_link_libraries(unittests ${GTEST_BOTH_LIBRARIES} udcr)
add_test(unittests unittests)
