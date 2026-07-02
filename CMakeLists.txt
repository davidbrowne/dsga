cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dsga VERSION 3.0.0)

set(SRC_FILES
	src/main.cxx
	src/nanobench.cxx
	examples/stl.cxx
)

set(TEST_FILES
	tests/main.cxx
	tests/assignment_test.cxx
	tests/comparison_test.cxx
	tests/constructor_test.cxx
	tests/conversion_test.cxx
	tests/function_test.cxx
	tests/iterator_test.cxx
	tests/operator_test.cxx
	tests/swizzle_test.cxx
	tests/traits_test.cxx
	tests/utility_test.cxx
	tests/negative_test.cxx
)

set(EXAMPLE_FILES
	examples/stl.cxx
)

# --- Remove /Ob2 from CMake's default MSVC Release flags to avoid conflict with our /Ob3 ---
if(MSVC)
	string(REPLACE "/Ob0" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# --- Interface library target (header-only) ---
add_library(dsga INTERFACE)
add_library(dsga::dsga ALIAS dsga)

target_include_directories(dsga INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include/dsga>
)

target_compile_features(dsga INTERFACE cxx_std_20)

# --- Compiler flags for dev targets ---
# Applied only to dsga_dev, dsga_tests, dsga_examples — never to the INTERFACE target.
if(MSVC)
	set(DSGA_COMPILE_FLAGS
		/W4             # warning level 4
		/WX             # warnings as errors
		/sdl            # SDL security checks
		/permissive-    # strict conformance mode
		/bigobj         # large object file support
		/MP             # multi-processor compilation
		/Ob3            # aggressive inlining
		/guard:cf       # control flow guard
		/D_MSVC_STL_DESTRUCTOR_TOMBSTONES
		/D_MSVC_STL_HARDENING
		/DDOCTEST_CONFIG_USE_STD_HEADERS  # avoid forbidden std:: forward declarations
	)
	set(DSGA_COMPILE_FLAGS_DEBUG
		/RTC1           # basic runtime checks
	)
	set(DSGA_COMPILE_FLAGS_RELEASE
		/Oi             # intrinsic functions
		/Gy             # function-level linking
		/GL             # whole program optimization (pairs with /LTCG)
	)
	set(DSGA_LINK_FLAGS_RELEASE "")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(DSGA_COMPILE_FLAGS
		-Wpedantic
		-Wall
		-Werror
		-Wextra
		-Wconversion
		-Wsign-conversion
		-Wshadow
		-fstack-protector-strong
	)
	set(DSGA_COMPILE_FLAGS_DEBUG   "")
	set(DSGA_COMPILE_FLAGS_RELEASE
		-O3
		-flto=auto
	)
	# GCC on Windows (MinGW) has lld interop issues with LTO: lld defaults to the
	# Windows GUI subsystem and fails with "undefined symbol: WinMain". Use the
	# default linker there; lld is reliable for GCC/Clang on Linux and Clang on Windows.
	if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(DSGA_LINK_FLAGS_RELEASE "")
	else()
		set(DSGA_LINK_FLAGS_RELEASE
			-fuse-ld=lld    # use lld for LTO link performance
		)
	endif()
endif()

# Helper function to apply compiler and linker flags to a dev target
function(dsga_set_compile_flags target)
	target_compile_options(${target} PRIVATE
		${DSGA_COMPILE_FLAGS}
		$<$<CONFIG:Debug>:${DSGA_COMPILE_FLAGS_DEBUG}>
		$<$<CONFIG:Release>:${DSGA_COMPILE_FLAGS_RELEASE}>
	)
	target_link_options(${target} PRIVATE
		$<$<CONFIG:Release>:${DSGA_LINK_FLAGS_RELEASE}>
	)
endfunction()

# --- Top-level only: install rules, dev targets, tests, examples ---
if(PROJECT_IS_TOP_LEVEL)

	# --- Install rules ---
	include(GNUInstallDirs)
	include(CMakePackageConfigHelpers)

	install(TARGETS dsga
		EXPORT dsga-targets
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)

	install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dsga
	)

	install(EXPORT dsga-targets
		FILE dsga-targets.cmake
		NAMESPACE dsga::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dsga
	)

	# --- Package config files ---
	configure_package_config_file(
		${CMAKE_CURRENT_SOURCE_DIR}/cmake/dsga-config.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/dsga-config.cmake
		INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dsga
	)

	write_basic_package_version_file(
		${CMAKE_CURRENT_BINARY_DIR}/dsga-config-version.cmake
		VERSION ${PROJECT_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/dsga-config.cmake
		${CMAKE_CURRENT_BINARY_DIR}/dsga-config-version.cmake
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dsga
	)

	# --- Dev scratchpad (always built) ---
	add_executable(dsga_dev ${SRC_FILES})
	target_link_libraries(dsga_dev PRIVATE dsga::dsga)
	target_include_directories(dsga_dev PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dev_3rd)
	dsga_set_compile_flags(dsga_dev)

	# --- Tests ---
	option(BUILD_TESTS "Build the dsga test suite" ON)
	if(BUILD_TESTS)
		enable_testing()

		add_executable(dsga_tests ${TEST_FILES})
		target_link_libraries(dsga_tests PRIVATE dsga::dsga)
		target_include_directories(dsga_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dev_3rd)
		dsga_set_compile_flags(dsga_tests)

		include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/doctest.cmake")
		doctest_discover_tests(dsga_tests)
	endif()

	# --- Examples ---
	option(BUILD_EXAMPLES "Build the dsga examples" OFF)
	if(BUILD_EXAMPLES)
		add_executable(dsga_examples ${EXAMPLE_FILES})
		target_link_libraries(dsga_examples PRIVATE dsga::dsga)
		target_include_directories(dsga_examples PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dev_3rd)
		dsga_set_compile_flags(dsga_examples)
	endif()

	# --- Shader demo ---
	option(BUILD_SHADER_DEMO "Build the shader demo" ON)
	if(BUILD_SHADER_DEMO)
		add_executable(dsga_shader_demo examples/shader_demo.cxx)
		target_link_libraries(dsga_shader_demo PRIVATE dsga::dsga)
		dsga_set_compile_flags(dsga_shader_demo)
	endif()

	# --- Line closest points example ---
	option(BUILD_LINE_CLOSEST_POINTS "Build the line closest points example" ON)
	if(BUILD_LINE_CLOSEST_POINTS)
		add_executable(dsga_line_closest_points examples/line_closest_points.cxx)
		target_link_libraries(dsga_line_closest_points PRIVATE dsga::dsga)
		dsga_set_compile_flags(dsga_line_closest_points)
	endif()

endif()

