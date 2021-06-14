
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
	TEST_CASE("angle and trigonometry functions")
	{
		SUBCASE("radians and degrees")
		{
			// radians()

			// degrees()

		}

		SUBCASE("basic trig")
		{
			// sin()

			// cos()

			// tan()

			// asin()

			// acos()

			// atan() (both 1 arg and 2 arg)

		}

		SUBCASE("hyperbolic trig")
		{
			// sinh()

			// cosh()

			// tanh()

			// asinh()

			// acosh()

			// atanh()
		}
	}

	TEST_CASE("exponential functions")
	{
		SUBCASE("non-sqrt related")
		{
			// pow()

			// exp()

			// log()

			// exp2()

			// log2()

		}

		SUBCASE("sqrt related")
		{
			// sqrt()

			// inversesqrt()

		}
	}

	TEST_CASE("common functions")
	{
		SUBCASE("most common")
		{
			// abs()

			// sign()

			// floor()

			// trunc()

			// round()

			// roundEven()

			// ceil()

			// fract()

			// mod()

			// modf()

		}

		SUBCASE("in range functions")
		{
			// min()

			// max()

			// clamp()

			// mix()

			// step()

			// smoothstep()

		}

		SUBCASE("bit changing functions")
		{
			// floatBitsToInt()

			// floatBitsToUint()

			// doubleBitsToLongLong()

			// doubleBitsToUlongLong()

			// intBitsToFloat()

			// uintBitsToFloat()

			// longLongBitsToDouble()

			// ulongLongBitsToDouble()

		}

		SUBCASE("other common functions")
		{
			// isnan()

			// isinf()

			// fma()

			// frexp()

			// ldexp()

		}
	}

	TEST_CASE("geometric functions")
	{
		SUBCASE("generic vector functions")
		{
			// length()

			// distance

			// dot()

			// cross()

			// normalize()

		}

		SUBCASE("'normal' vector functions")
		{
			// faceforward()

			// reflect()

			// refract()

		}
	}

	TEST_CASE("vector relational functions")
	{
		SUBCASE("standard relational functions")
		{
			// lessThan()

			// lessThanEqual()

			// greaterThan()

			// greaterThanEqual()

			// equal()

			// notEqual()

		}

		SUBCASE("reduced relational functions")
		{
			// any()

			// all()

			// not() - c++ doesn't allow not(), so Not()
		}
	}
}