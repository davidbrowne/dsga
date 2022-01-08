
//          Copyright David Browne 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

//#include "dev_3rd/nanobench.h"
#include "dsga.hxx"


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
}


// reversing parameter pack stuff


// https://twitter.com/the_whole_daisy/status/1379580525078147072
// https://godbolt.org/z/h5P1Mxsrz
// TLDR -- operator precedence ('=' is right to left, ',' is left to right) matters with binary fold expressions.
//
//#include <iostream>
//
//template <class... Ts>
//void print_args_backwards(Ts... ts)
//{
//	auto print_one = [](auto t)
//	{
//		std::cout << t << std::endl;
//
//		// Anything with a reasonable assignment operator will work here
//		return std::type_identity<void>{};
//	};
//
//	// Backwards:
//	(print_one(ts) = ...);
//
//	// Forwards:
//	(print_one(ts), ...);
//}
//
//int main()
//{
//	print_args_backwards(1, 2, "hello", 3, 4, "world");
//}


// https://stackoverflow.com/questions/51408771/c-reversed-integer-sequence-implementation
//
//#include <utility>
//#include <type_traits>
//
//template <std::size_t ... Is>
//constexpr auto indexSequenceReverse (std::index_sequence<Is...> const &)
//-> decltype( std::index_sequence<sizeof...(Is)-1U-Is...>{} );
//
//template <std::size_t N>
//using makeIndexSequenceReverse = decltype(indexSequenceReverse(std::make_index_sequence<N>{}));
//
//int main()
//{
//	static_assert( std::is_same<std::index_sequence<4U, 3U, 2U, 1U, 0U>,
//				   makeIndexSequenceReverse<5U>>::value, "!" );
//}


// https://stackoverflow.com/questions/47303466/compile-time-reversal-of-parameter-pack-expansion
//
//template<typename T>
//struct Test
//{
//	template <std::size_t...Is, typename... B>
//	Test(std::index_sequence<Is...>, B&&... vv) :
//		b{std::get<sizeof...(Is) - 1 - Is>(std::tie(vv...))...}
//	{}
//
//	public:
//		template<typename... B>
//		explicit Test(B... vv) : Test(std::index_sequence_for<B...>{}, vv...) {}
//	private:
//		std::byte b[sizeof(T)];
//};


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
