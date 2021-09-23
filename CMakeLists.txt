# CMakeList.txt : CMake project for wudcrgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("wudcrgen")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# All implementation details of the recognition algorithm, generator etc.
# are compiled in this library, on which all the executables depend.
add_library(udcr STATIC "embed.cpp"  "graph.h" "tests.cpp" "graph.cpp" "embed.h"
"svg.cpp" "svg.h" "geometry.cpp" "geometry.h" "config.cpp" "config.h" "util.h" "util.cpp"
"ipe.h" "ipe.cpp" "translate.h" "translate.cpp")

# This is the main executable.
add_executable (wudcrgen "main.cpp")
target_link_libraries(wudcrgen udcr)

# This is the generator executable, which produces test cases.
add_executable (gencases "gencases.cpp")
target_link_libraries(gencases udcr)

# TODO: Add tests and install targets if needed.
