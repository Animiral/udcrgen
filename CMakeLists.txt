# CMakeList.txt : CMake project for wudcrgen, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("wudcrgen")

# Add source to this project's executable.
add_executable (wudcrgen "wudcrgen.cpp"  "graph.h" "tests.cpp" "graph.cpp" "main.cpp" "wudcrgen.h" "svg.cpp" "svg.h")

# TODO: Add tests and install targets if needed.
