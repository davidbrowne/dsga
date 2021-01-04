
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

TEST_SUITE("test constructors")
{
	constexpr ivec4	cx_four		(0, 1, 2, 3);
	constexpr ivec3	cx_three	(4, 5, 6);
	constexpr ivec2	cx_two		(7, 8);
	constexpr iscal	cx_one		(9);

	TEST_CASE("1D constructors")
	{
		// the four non-defaulted constructors
		fscal c1d_1(cx_four);		// #1 - implicit type conversion
		iscal cld_2(c1d_1);			// #2
		fscal cld_3(3);				// #3 - implicit type conversion
		iscal cld_4(2.5);			// #4

		bscal b1d_1(true);					// #4
		bscal b1d_2(bvec2(false, false));	// #2
		bscal b1d_3(1);						// #4
		bscal b1d_4(cx_four);				// #2

		CHECK_EQ(c1d_1, fscal(0));
		CHECK_NE(c1d_1, fscal(3));

		CHECK_EQ(cld_2, iscal(0));
		CHECK_NE(cld_2, iscal(3));

		CHECK_EQ(cld_3, fscal(3));
		CHECK_NE(cld_3, fscal(0));

		CHECK_EQ(cld_4, iscal(2));
		CHECK_NE(cld_4, iscal(3));
	}

	TEST_CASE("2D constructors")
	{
		// the five non-defaulted constructors
		ivec2 c2d_1(17);			// #1
		fvec2 c2d_2(50, 75.5);		// #2
		ivec2 c2d_3(100, c2d_2);	// #3
		fvec2 c2d_4(c2d_3);			// #4 - implicit type conversion
		ivec2 c2d_5(c2d_4);			// #5

		bvec2 b2d_1(17);			// #1
		bvec2 b2d_2(false, 75.5);	// #2
		bvec2 b2d_3(100, cx_four);	// #3
		fvec2 b2d_4(b2d_3);			// #5
		bvec2 b2d_5(cx_three);		// #5

		CHECK_EQ(c2d_1, ivec2(17, 17));
		CHECK_NE(c2d_1, ivec2(17, 0));

		CHECK_EQ(c2d_2, fvec2(50.f, 75.5f));
		CHECK_NE(c2d_2, fvec2(50, 75));

		CHECK_EQ(c2d_3, ivec2(100, 50));
		CHECK_NE(c2d_3, ivec2(100, 75));

		CHECK_EQ(c2d_4, fvec2(100.0f, 50.0f));
		CHECK_NE(c2d_4, fvec2(100.0f, 75.5f));

		CHECK_EQ(c2d_5, ivec2(100, 50));
		CHECK_NE(c2d_5, ivec2(100, 75));
	}

	TEST_CASE("3D constructors")
	{
		// the eight non-defaulted constructors
		ivec3 c3d_1(23);				// #1
		ivec3 c3d_2(200, 300, 400);		// #2
		ivec3 c3d_3(15, 25, cx_two);	// #3
		fvec3 c3d_4(cx_four);			// #4 - implicit type conversion
		ivec3 c3d_5(c3d_4);				// #5
		ivec3 c3d_6(cx_two, 99);		// #6
		ivec3 c3d_7(cx_two, cx_three);	// #7
		ivec3 c3d_8(42, cx_four);		// #8

		bvec3 b3d_1(23);				// #1
		bvec3 b3d_2(200, 300, 400);		// #2
		bvec3 b3d_3(15, 25, cx_two);	// #3
		fvec3 b3d_4(b3d_3);				// #5
		bvec3 b3d_5(c3d_4);				// #5
		bvec3 b3d_6(cx_two, 99);		// #6
		bvec3 b3d_7(cx_two, cx_three);	// #7
		bvec3 b3d_8(42, cx_four);		// #8

		CHECK_EQ(c3d_1, ivec3(23, 23, 23));
		CHECK_NE(c3d_1, ivec3(23, 0, 0));

		CHECK_EQ(c3d_2, ivec3(200, 300, 400));
		CHECK_NE(c3d_2, ivec3(0, 0, 0));

		CHECK_EQ(c3d_3, ivec3(15, 25, 7));
		CHECK_NE(c3d_3, ivec3(15, 25, 8));

		CHECK_EQ(c3d_4, fvec3(0.0f, 1.0f, 2.0f));
		CHECK_NE(c3d_4, fvec3(1.0f, 2.0f, 3.0f));

		CHECK_EQ(c3d_5, ivec3(0, 1, 2));
		CHECK_NE(c3d_5, ivec3(1, 2, 3));

		CHECK_EQ(c3d_6, ivec3(7, 8, 99));
		CHECK_NE(c3d_6, ivec3(99, 99, 99));

		CHECK_EQ(c3d_7, ivec3(7, 8, 4));
		CHECK_NE(c3d_7, ivec3(7, 8, 6));

		CHECK_EQ(c3d_8, ivec3(42, 0, 1));
		CHECK_NE(c3d_8, ivec3(42, 2, 3));
	}

	TEST_CASE("4D constructors")
	{
		// the fourteen non-defaulted constructors
		ivec4 c4d_01(-8);					// #1
		ivec4 c4d_02(11, 22, 33, 44);		// #2
		ivec4 c4d_03(15, 25, 30, cx_two);	// #3
		fvec4 c4d_04(cx_four);				// #4 - implicit type conversion
		ivec4 c4d_05(c4d_04);				// #5
		ivec4 c4d_06(cx_three, 99);			// #6
		ivec4 c4d_07(cx_three, cx_three);	// #7
		ivec4 c4d_08(42, cx_four);			// #8
		ivec4 c4d_09(cx_two, cx_three);		// #9
		ivec4 c4d_10(cx_two, -3, -5);		// #10
		ivec4 c4d_11(cx_two, 13, cx_two);	// #11
		ivec4 c4d_12(19, cx_two, 99);		// #12
		ivec4 c4d_13(19, cx_two, cx_three);	// #13
		ivec4 c4d_14(42, 24, cx_four);		// #14

		bvec4 b4d_01(-8);					// #1
		bvec4 b4d_02(11, 22, 33, 44);		// #2
		bvec4 b4d_03(15, 25, 30, cx_two);	// #3
		fvec4 b4d_04(b4d_03);				// #5
		bvec4 b4d_05(c4d_04);				// #5
		bvec4 b4d_06(cx_three, 99);			// #6
		bvec4 b4d_07(cx_three, cx_three);	// #7
		bvec4 b4d_08(42, cx_four);			// #8
		bvec4 b4d_09(cx_two, cx_three);		// #9
		bvec4 b4d_10(cx_two, -3, -5);		// #10
		bvec4 b4d_11(cx_two, 13, cx_two);	// #11
		bvec4 b4d_12(19, cx_two, 99);		// #12
		bvec4 b4d_13(19, cx_two, cx_three);	// #13
		bvec4 b4d_14(42, 24, cx_four);		// #14

		CHECK_EQ(c4d_01, ivec4(-8, -8, -8, -8));
		CHECK_NE(c4d_01, ivec4(-8, 0, 0, 0));

		CHECK_EQ(c4d_02, ivec4(11, 22, 33, 44));
		CHECK_NE(c4d_02, ivec4(11, 22, 33, 0));

		CHECK_EQ(c4d_03, ivec4(15, 25, 30, 7));
		CHECK_NE(c4d_03, ivec4(15, 25, 30, 8));

		CHECK_EQ(c4d_04, fvec4(0.0f, 1.0f, 2.0f, 3.0f));
		CHECK_NE(c4d_04, fvec4(0.0f, 1.0f, 2.0f, 0.0f));

		CHECK_EQ(c4d_05, ivec4(0, 1, 2, 3));
		CHECK_NE(c4d_05, ivec4(0, 1, 2, 0));

		CHECK_EQ(c4d_06, ivec4(4, 5, 6, 99));
		CHECK_NE(c4d_06, ivec4(4, 5, 6, 0));

		CHECK_EQ(c4d_07, ivec4(4, 5, 6, 4));
		CHECK_NE(c4d_07, ivec4(4, 5, 6, 6));

		CHECK_EQ(c4d_08, ivec4(42, 0, 1, 2));
		CHECK_NE(c4d_08, ivec4(42, 1, 2, 3));

		CHECK_EQ(c4d_09, ivec4(7, 8, 4, 5));
		CHECK_NE(c4d_09, ivec4(7, 8, 5, 6));

		CHECK_EQ(c4d_10, ivec4(7, 8, -3, -5));
		CHECK_NE(c4d_10, ivec4(7, 8, -3, 0));

		CHECK_EQ(c4d_11, ivec4(7, 8, 13, 7));
		CHECK_NE(c4d_11, ivec4(7, 8, 13, 8));

		CHECK_EQ(c4d_12, ivec4(19, 7, 8, 99));
		CHECK_NE(c4d_12, ivec4(19, 7, 8, 0));

		CHECK_EQ(c4d_13, ivec4(19, 7, 8, 4));
		CHECK_NE(c4d_13, ivec4(19, 7, 8, 6));

		CHECK_EQ(c4d_14, ivec4(42, 24, 0, 1));
		CHECK_NE(c4d_14, ivec4(42, 24, 2, 3));
	}
}
