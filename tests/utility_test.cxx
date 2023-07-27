
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

template<std::size_t ...Args1, std::size_t ...Args2>
requires (sizeof...(Args1) == sizeof...(Args2))
bool operator ==(std::index_sequence<Args1...>, std::index_sequence<Args2...>) noexcept
{
	return ((Args1 == Args2) && ...);
}

TEST_SUITE("test utility support")
{
	TEST_CASE("test index ranges")
	{

		SUBCASE("half-closed")
		{
			auto foo1 = dsga::make_index_range<5, 2>();
			auto bar1 = dsga::make_index_range<2, 5>();
			auto baz1 = dsga::make_index_range<2, 2>();

			CHECK_UNARY(foo1 == std::index_sequence<5, 4, 3>());
			CHECK_UNARY(bar1 == std::index_sequence<2, 3, 4>());
			CHECK_UNARY(baz1 == std::index_sequence<>());
		}

		SUBCASE("closed")
		{
			auto foo2 = dsga::make_closed_index_range<5, 2>();
			auto bar2 = dsga::make_closed_index_range<2, 5>();
			auto baz2 = dsga::make_closed_index_range<2, 2>();

			CHECK_UNARY(foo2 == std::index_sequence<5, 4, 3, 2>());
			CHECK_UNARY(bar2 == std::index_sequence<2, 3, 4, 5>());
			CHECK_UNARY(baz2 == std::index_sequence<2>());
		}
	}
}
