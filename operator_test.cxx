
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"


#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// operators are implemented without regard to any specific dimension, so we can test generically
TEST_SUITE("test operators")
{
	TEST_CASE("unary operator +")
	{
	}

	TEST_CASE("unary operator -")
	{
	}

	TEST_CASE("unary pre-increment operator ++")
	{
	}

	TEST_CASE("unary post-increment operator ++")
	{
	}

	TEST_CASE("unary pre-decrement operator --")
	{
	}

	TEST_CASE("unary post-decrement operator --")
	{
	}

	TEST_CASE("binary operator +")
	{
		// commutative
	}

	TEST_CASE("binary operator -")
	{
		// non-commutative
	}

	TEST_CASE("binary operator *")
	{
		// commutative
	}

	TEST_CASE("binary operator /")
	{
		// non-commutative
	}

	TEST_CASE("binary operator %")
	{
		// non-commutative
	}

	TEST_CASE("unary operator ~")
	{
	}

	TEST_CASE("binary operator <<")
	{
		// non-commutative
	}

	TEST_CASE("binary operator >>")
	{
		// non-commutative
	}

	TEST_CASE("binary operator &")
	{
		// commutative
	}

	TEST_CASE("binary operator |")
	{
		// commutative
	}

	TEST_CASE("binary operator ^")
	{
		// commutative
	}
}
