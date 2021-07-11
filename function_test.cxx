
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
	TEST_CASE("vector angle and trigonometry functions")
	{
		const float my_pi = std::numbers::pi_v<float>;
		fvec3 degs(30, 45, 60);
		fvec3 rads(my_pi / 6, my_pi / 4, my_pi / 3);

		SUBCASE("radians and degrees")
		{
			// radians()
			
			auto my_rads = radians(degs);
			CHECK_EQ(my_rads, rads);

			// degrees()

			auto my_degs = degrees(rads);
			CHECK_EQ(my_degs, degs);
		}

		SUBCASE("basic trig")
		{
			// sin()

			CHECK_EQ(sin(radians(fvec2(30, 90))), fvec2(0.5f, 1));

			// cos()

			CHECK_EQ(cos(radians(fvec2(0, 180))), fvec2(1, -1));

			// tan()
			CHECK_EQ(tan(radians(fvec2(45, 0))), fvec2(1, 0));

			// asin()
			CHECK_EQ(asin(fvec2(0.5f, 1)), radians(fvec2(30, 90))) ;

			// acos()
			CHECK_EQ(acos(fvec2(1, -1)), radians(fvec2(0, 180)));

			// atan() (both 1 arg and 2 arg)
			CHECK_EQ(atan(fvec2(1, 0)), radians(fvec2(45, 0)));

			CHECK_EQ(atan(fvec2(1, -1), fvec2(-1, -1)), radians(fvec2(135, -135)));

		}

		SUBCASE("hyperbolic trig")
		{
			dvec3 vals(-2, 0, 2);

			// sinh()
			auto sinhs = sinh(vals);

			// cosh()
			auto coshs = cosh(vals);

			// tanh()
			auto tanhs = tanh(vals);

			// asinh()
			auto asinhs = asinh(sinhs);
			CHECK_EQ(vals, asinhs);

			// acosh()
			auto acoshs = acosh(coshs);
			CHECK_EQ(dvec3(2, 0, 2), acoshs);

			// atanh()
			auto atanhs = atanh(tanhs);
			CHECK_EQ(atanh(sinhs / coshs), atanhs);
		}
	}

	TEST_CASE("vector exponential functions")
	{
		SUBCASE("non-sqrt related")
		{
			// pow(), e.g., pow(2, 10) = 2^10 = 1024

			// exp(), e.g., e^x = exp(x)

			// log(), e.g., log(x) = ln(x) = log<base e>(x)

			// exp2(), e.g., 2^x = exp2(x)

			// log2(), e.g., log2(x) = lb(x) = log<base 2>(x)

		}

		SUBCASE("sqrt related")
		{
			//
			// cxcm constexpr versions of these may be off by an ulp from standard library.
			// Because these implementation results do not exactly match the standard's,
			// you have to explicitly opt-in to these constexpr versions of sqrt() and
			// inversesqrt() via define macro CXCM_APPROXIMATIONS_ALLOWED (prior to
			// including cxcm.hxx or dsga.hxx).
			//

			// sqrt()

			// inversesqrt()

		}
	}

	TEST_CASE("vector common functions")
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
#if defined(__cpp_lib_bit_cast)

			// floatBitsToInt()

			// floatBitsToUint()

			// doubleBitsToLongLong()

			// doubleBitsToUlongLong()

			// intBitsToFloat()

			// uintBitsToFloat()

			// longLongBitsToDouble()

			// ulongLongBitsToDouble()

#endif
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

	TEST_CASE("vector geometric functions")
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

	TEST_CASE("matrix functions")
	{
		// matrixCompMult()

		// outerProduct()

		// transpose()

		// determinant()

		// inverse()

	}

}
