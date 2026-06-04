//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
//#include "doctest.h"

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
