
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

//#include "nanobench.h"
#include "dsga.hxx"
#include <iostream>

//
//
// This file contains main(), and it is used for both running the unit tests and
// for playing around with some example and ideas. This file doesn't have anything
// to demonstrate beyond that.
//
//

// print current version number
void print_dsga_version()
{
	std::cout << "\ndsga version: v" << DSGA_MAJOR_VERSION << "." << DSGA_MINOR_VERSION << "." << DSGA_PATCH_VERSION << "\n\n";
}

// this function is a place to just test out whatever
void sandbox_function()
{
	// put fun code here

}

#if defined(__clang__) && (__clang_major__ < 13)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main([[ maybe_unused ]] int argc, [[ maybe_unused ]] char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	// watch window:
	// {,,ucrtbased.dll}_crtBreakAlloc
	// {,,ucrtbased.dll}__acrt_current_request_number
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	print_dsga_version();

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
