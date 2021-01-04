
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

// compund assignments are implemented without regard to any specific dimension, so we can test generically
TEST_SUITE("test assignment")
{
	// all default assignment *is* implemented inside the class, so each dimension of basic_vector has its own implementation
	TEST_CASE("default assignment")
	{
		SUBCASE("1D default assignment =")
		{
		}

		SUBCASE("2D default assignment =")
		{
		}

		SUBCASE("3D default assignment =")
		{
		}

		SUBCASE("4D default assignment =")
		{
		}
	}

	TEST_CASE("operator +=")
	{
		SUBCASE("basic_vector operator +=")
		{

		}

		SUBCASE("indexed_vector operator +=")
		{

		}
	}

	TEST_CASE("operator -=")
	{
		SUBCASE("basic_vector operator -=")
		{

		}

		SUBCASE("indexed_vector operator -=")
		{

		}
	}

	TEST_CASE("operator *=")
	{
		SUBCASE("basic_vector operator *=")
		{

		}

		SUBCASE("indexed_vector operator *=")
		{

		}
	}

	TEST_CASE("operator /=")
	{
		SUBCASE("basic_vector operator /=")
		{

		}

		SUBCASE("indexed_vector operator /=")
		{

		}
	}

	TEST_CASE("operator %=")
	{
		SUBCASE("basic_vector operator %=")
		{

		}

		SUBCASE("indexed_vector operator %=")
		{

		}
	}

	TEST_CASE("operator <<=")
	{
		SUBCASE("basic_vector operator <<=")
		{

		}

		SUBCASE("indexed_vector operator <<=")
		{

		}
	}

	TEST_CASE("operator >>=")
	{
		SUBCASE("basic_vector operator >>=")
		{

		}

		SUBCASE("indexed_vector operator >>=")
		{

		}
	}

	TEST_CASE("operator &=")
	{
		SUBCASE("basic_vector operator &=")
		{

		}

		SUBCASE("indexed_vector operator &=")
		{

		}
	}

	TEST_CASE("operator |=")
	{
		SUBCASE("basic_vector operator |=")
		{

		}

		SUBCASE("indexed_vector operator |=")
		{

		}
	}

	TEST_CASE("operator ^=")
	{
		SUBCASE("basic_vector operator ^=")
		{

		}

		SUBCASE("indexed_vector operator ^=")
		{

		}
	}
}
