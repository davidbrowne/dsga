
//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

//#include "nanobench.h"
#include "dsga.hxx"
#include "../examples/format_output.hxx"

//
//
// This file contains main(), and it is used for both running the unit tests and
// for playing around with some example and ideas. This file doesn't have anything
// to demonstrate beyond that.
//
//


// this function is a place to just test out whatever
void sandbox_function()
{
	// put fun code here

#if defined(__cpp_lib_format)
	auto fmt_arr = std::array<double, 4>{1, 2, 3, 4};
	auto empty_arr = std::array<double, 0>{};
	auto one_arr = std::array<double, 1>{99};
	auto fmt_vec = dsga::dvec4(10, 20, 30, 40);
	auto fmt_mat = dsga::dmat3x2(1, 2, 3, 4, 5, 6);
	test_format_array(empty_arr);
	test_format_array(one_arr);
	test_format_array(fmt_arr);
	test_format_vector(fmt_vec);
	test_format_vector_base(fmt_vec);
	test_format_vector_base(fmt_vec.wzyx);
	test_format_indexed_vector(fmt_vec.wzyx);
	test_format_matrix(fmt_mat);
#endif
}

#if defined(__clang__) && (__clang_major__ < 13)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char *argv[])
{
	sandbox_function();


	//
	// doctest
	//

	int doctest_result = 0;

// comment out if we define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN somewhere to get the main() from doctest
#define DONT_USE_DOCTEST_MAIN

#if defined(DONT_USE_DOCTEST_MAIN)

	doctest::Context context;
	context.applyCommandLine(argc, argv);

	doctest_result = context.run();				// run

	if (context.shouldExit())					// important - query flags (and --exit) rely on the user doing this
		return doctest_result;					// propagate the result of the tests

#endif



	return EXIT_SUCCESS + doctest_result;
}
