cmake_minimum_required (VERSION 3.8)

project ("udcrgen")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# All implementation details of the recognition algorithm, generator etc.
# are compiled in this library, on which all the executables depend.
add_library(udcr STATIC
	"src/dynamic.cpp" "src/dynamic.h"
	"src/heuristic.cpp" "src/heuristic.h"
	"src/embed.cpp" "src/embed.h"
	"src/enumerate.cpp" "src/enumerate.h"
	"src/config.cpp" "src/config.h"
	"src/input/input.cpp" "src/input/input.h"
	"src/output/csv.cpp" "src/output/csv.h"
	"src/output/svg.cpp" "src/output/svg.h"
	"src/output/ipe.cpp" "src/output/ipe.h"
	"src/output/archive.h" "src/output/archive.cpp"
	"src/output/translate.cpp" "src/output/translate.h"
	"src/utility/geometry.cpp" "src/utility/geometry.h"
	"src/utility/graph.cpp" "src/utility/graph.h"
	"src/utility/grid.cpp" "src/utility/grid.h"
	"src/utility/util.cpp" "src/utility/util.h"
	"src/utility/exception.cpp" "src/utility/exception.h"
	"src/utility/stat.h")
target_include_directories(udcr PRIVATE ${PROJECT_SOURCE_DIR}/src)

# This is the main executable.
add_executable(udcrgen "src/main.cpp"
	"src/config.h" "src/embed.h" "src/heuristic.h" "src/dynamic.h" "src/enumerate.h"
	"src/utility/graph.h" "src/utility/exception.h" "src/utility/grid.h" "src/utility/geometry.h" "src/utility/stat.h"
	"src/output/translate.h" "src/output/ipe.h" "src/output/svg.h" "src/output/csv.h" "src/output/archive.h")
target_link_libraries(udcrgen udcr)

# This is the generator executable, which produces test cases.
add_executable(gencases "src/gencases.cpp" "src/utility/graph.h" "src/utility/geometry.h")
target_link_libraries(gencases udcr)

# Unit Tests
enable_testing()
find_package(GTest REQUIRED)

add_executable(unittests
	"test/test_embed.cpp" "src/embed.h"
	"test/test_graph.cpp" "src/utility/graph.h"
	"test/test_grid.cpp" "src/utility/grid.h"
	"test/test_dynamic.cpp" "src/dynamic.h"
	"test/test_heuristic.cpp" "src/heuristic.h"
	"test/test_enumerate.cpp" "src/enumerate.h" "src/output/csv.h" "src/output/svg.h" "src/output/translate.h"
	"test/test_config.cpp" "src/config.h"
	"test/test_output.cpp" "src/output/archive.h"
	"src/utility/exception.h" "src/utility/geometry.h" "src/utility/stat.h")
target_link_libraries(unittests ${GTEST_BOTH_LIBRARIES} udcr)
target_include_directories(unittests PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(unittests unittests)
