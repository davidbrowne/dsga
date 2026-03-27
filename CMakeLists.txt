cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dsga VERSION 2.0.5)

# file(GLOB SRC_FILES  CONFIGURE_DEPENDS src/*.cxx)
# file(GLOB TEST_FILES CONFIGURE_DEPENDS tests/*.cxx)
# file(GLOB EXAMPLE_FILES CONFIGURE_DEPENDS examples/*.cxx)

set(SRC_FILES
	src/main.cxx
	src/nanobench.cxx
)

set(TEST_FILES
	tests/assignment_test.cxx
	tests/comparison_test.cxx
	tests/constructor_test.cxx
	tests/conversion_test.cxx
	tests/function_test.cxx
	tests/operator_test.cxx
	tests/swizzle_test.cxx
	tests/traits_test.cxx
	tests/utility_test.cxx
)

set(EXAMPLE_FILES
	examples/stl.cxx
)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${TEST_FILES} ${EXAMPLE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/dev_3rd)

