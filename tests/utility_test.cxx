
//          Copyright David Browne 2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
using namespace dsga;

#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_SUITE("test utility support")
{
	TEST_CASE("test index ranges")
	{

		SUBCASE("half-closed")
		{
			CHECK_UNARY(std::same_as<dsga::make_index_range<5, 2>, std::index_sequence<5, 4, 3>>);
			CHECK_UNARY(std::same_as<dsga::make_index_range<2, 5>, std::index_sequence<2, 3, 4>>);
			CHECK_UNARY(std::same_as<dsga::make_index_range<2, 2>, std::index_sequence<>>);
		}

		SUBCASE("closed")
		{
			CHECK_UNARY(std::same_as<dsga::make_closed_index_range<5, 2>, std::index_sequence<5, 4, 3, 2>>);
			CHECK_UNARY(std::same_as<dsga::make_closed_index_range<2, 5>, std::index_sequence<2, 3, 4, 5>>);
			CHECK_UNARY(std::same_as<dsga::make_closed_index_range<2, 2>, std::index_sequence<2>>);
		}
	}
}
