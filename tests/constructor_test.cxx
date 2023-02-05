
//          Copyright David Browne 2020-2023.
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

TEST_SUITE("test constructors")
{
	constexpr ivec4	cx_four		(0, 1, 2, 3);
	constexpr ivec3	cx_three	(4, 5, 6);
	constexpr ivec2	cx_two		(7, 8);
	[[ maybe_unused ]] constexpr iscal	cx_one		(9);

	TEST_CASE("vectors from length 1 vectors (scalars)")
	{
		// make sure constructors work with length 1 vector (dscal, fscal, etc.) as if it were a scalar
		dscal eight(8);
		dvec2 dv2(eight);
		dvec3 dv3(eight);
		dvec4 dv4(eight);
		CHECK_EQ(dv2, dvec2(8, 8));
		CHECK_EQ(dv3, dvec3(8, 8, 8));
		CHECK_EQ(dv4, dvec4(8, 8, 8, 8));
	}

	TEST_CASE("vector 1D constructors")
	{
		// the four non-defaulted constructors
		fscal c1d_1(cx_four);		// #1 - implicit type conversion
		iscal c1d_2(c1d_1);			// #2
		fscal c1d_3(3);				// #3 - implicit type conversion
		iscal c1d_4(2.5);			// #4
		iscal c1d_5(c1d_3.x);
		iscal c1d_6(mat2(77));

		[[ maybe_unused ]] bscal b1d_1(true);					// #4
		[[ maybe_unused ]] bscal b1d_2(bvec2(false, false));	// #2
		[[ maybe_unused ]] bscal b1d_3(1);						// #4
		[[ maybe_unused ]] bscal b1d_4(cx_four);				// #2

		CHECK_EQ(c1d_1, fscal(0));
		CHECK_NE(c1d_1, fscal(3));

		CHECK_EQ(c1d_2, iscal(0));
		CHECK_NE(c1d_2, iscal(3));

		CHECK_EQ(c1d_3, fscal(3));
		CHECK_NE(c1d_3, fscal(0));

		CHECK_EQ(c1d_4, iscal(2));
		CHECK_NE(c1d_4, iscal(3));

		CHECK_EQ(c1d_5, iscal(3));
		CHECK_EQ(c1d_6, iscal(77));
	}

	TEST_CASE("vector 2D constructors")
	{
		// the five non-defaulted constructors
		ivec2 c2d_1(17);			// #1
		fvec2 c2d_2(50, 75.5);		// #2
		ivec2 c2d_3(100, c2d_2);	// #3
		fvec2 c2d_4(c2d_3);			// #4 - implicit type conversion
		ivec2 c2d_5(c2d_4);			// #5
		ivec2 c2d_6(c2d_4.yx);
		ivec2 c2d_7(mat2(77, 66, 55, 44));
		ivec2 c2d_8(34, mat2(77, 66, 55, 44));

		[[ maybe_unused ]] bvec2 b2d_1(17);			// #1
		[[ maybe_unused ]] bvec2 b2d_2(false, 75.5);	// #2
		bvec2 b2d_3(100, cx_four);	// #3
		[[ maybe_unused ]] fvec2 b2d_4(b2d_3);			// #5
		[[ maybe_unused ]] bvec2 b2d_5(cx_three);		// #5

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

		CHECK_EQ(c2d_6, ivec2(50, 100));
		CHECK_EQ(c2d_7, ivec2(77, 66));
		CHECK_EQ(c2d_8, ivec2(34, 77));
	}

	TEST_CASE("vector 3D constructors")
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
		ivec3 c3d_9(c3d_4.yzx);
		ivec3 c3d_10(mat2(77, 66, 55, 44));
		ivec3 c3d_11(dscal(34), mat2(77, 66, 55, 44));

		[[ maybe_unused ]] bvec3 b3d_1(23);				// #1
		[[ maybe_unused ]] bvec3 b3d_2(200, 300, 400);		// #2
		bvec3 b3d_3(15, 25, cx_two);	// #3
		[[ maybe_unused ]] fvec3 b3d_4(b3d_3);				// #5
		[[ maybe_unused ]] bvec3 b3d_5(c3d_4);				// #5
		[[ maybe_unused ]] bvec3 b3d_6(cx_two, 99);		// #6
		[[ maybe_unused ]] bvec3 b3d_7(cx_two, cx_three);	// #7
		[[ maybe_unused ]] bvec3 b3d_8(42, cx_four);		// #8

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

		CHECK_EQ(c3d_9, ivec3(1, 2, 0));
		CHECK_EQ(c3d_10, ivec3(77, 66, 55));
		CHECK_EQ(c3d_11, ivec3(34, 77, 66));
	}

	TEST_CASE("vector 4D constructors")
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
		ivec4 c4d_15(c4d_04.yzwx);
		ivec4 c4d_16(mat2(77, 66, 55, 44));
		ivec4 c4d_17(dscal(34), ivec2(2, 99), mat2(77, 66, 55, 44));
		ivec4 c4d_18(dscal(34), bvec2(true, false), mat2(77, 66, 55, 44));

		[[ maybe_unused ]] bvec4 b4d_01(-8);					// #1
		[[ maybe_unused ]] bvec4 b4d_02(11, 22, 33, 44);		// #2
		bvec4 b4d_03(15, 25, 30, cx_two);	// #3
		[[ maybe_unused ]] fvec4 b4d_04(b4d_03);				// #5
		[[ maybe_unused ]] bvec4 b4d_05(c4d_04);				// #5
		[[ maybe_unused ]] bvec4 b4d_06(cx_three, 99);			// #6
		[[ maybe_unused ]] bvec4 b4d_07(cx_three, cx_three);	// #7
		[[ maybe_unused ]] bvec4 b4d_08(42, cx_four);			// #8
		[[ maybe_unused ]] bvec4 b4d_09(cx_two, cx_three);		// #9
		[[ maybe_unused ]] bvec4 b4d_10(cx_two, -3, -5);		// #10
		[[ maybe_unused ]] bvec4 b4d_11(cx_two, 13, cx_two);	// #11
		[[ maybe_unused ]] bvec4 b4d_12(19, cx_two, 99);		// #12
		[[ maybe_unused ]] bvec4 b4d_13(19, cx_two, cx_three);	// #13
		[[ maybe_unused ]] bvec4 b4d_14(42, 24, cx_four);		// #14

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

		CHECK_EQ(c4d_15, ivec4(1, 2, 3, 0));
		CHECK_EQ(c4d_16, ivec4(77, 66, 55, 44));
		CHECK_EQ(c4d_17, ivec4(34, 2, 99, 77));
	}

	TEST_CASE("CTAD deduction guide construction")
	{
		auto v1 = dsga::basic_vector{1, 2, 3};
		auto v2 = dsga::basic_vector{2.2, 3, 4, 5.5};
		auto v3 = dsga::basic_vector{true};

		auto v4 = dsga::basic_vector{v1};
		auto v5 = dsga::basic_vector{v2.zy};

		CHECK_NE(v1, ivec3(1, 2, 4));
		CHECK_EQ(v1, ivec3(1, 2, 3));
		CHECK_EQ(v2, dvec4(2.2, 3, 4, 5.5));
		CHECK_EQ(v3, bscal(true));
		CHECK_EQ(v4, v1);
		CHECK_EQ(v5, v2.zy);
	}

	TEST_CASE("matrix constructors")
	{
		SUBCASE("diagonal constructor")
		{
			auto m1 = mat2(7);
			auto m2 = dmat3(3);
			auto m3 = mat4(-2);

			CHECK_EQ(m1, mat2x2(vec2(7, 0), vec2(0, 7)));
			CHECK_EQ(m2, dmat3x3(dvec3(3, 0, 0), dvec3(0, 3, 0), dvec3(0, 0, 3)));
			CHECK_EQ(m3, mat4x4(vec4(-2, 0, 0, 0), vec4(0, -2, 0, 0), vec4(0, 0, -2, 0), vec4(0, 0, 0, -2)));

			// make sure constructors work with length 1 vector (dscal, fscal, etc.) as if it were a scalar
			dscal eight(8);
			dmat2 dm2(eight);
			dmat3 dm3(eight.x);
			dmat4 dm4(eight);
			CHECK_EQ(dm2, dmat2(dvec2(8, 0), dvec2(0, 8)));
			CHECK_EQ(dm3, dmat3(dvec3(8, 0, 0), dvec3(0, 8, 0), dvec3(0, 0, 8)));
			CHECK_EQ(dm4, dmat4(dvec4(8, 0, 0, 0), dvec4(0, 8, 0, 0), dvec4(0, 0, 8, 0), dvec4(0, 0, 0, 8)));
		}

		SUBCASE("variadic component constructor")
		{
			dscal one(1);
			dvec2 two(2);
			dvec3 three(3);
			dvec4 four(4);
			mat2  two_by_two(10, 9, 8, 7);

			double rando = 5;

			auto m1 = dmat4(four, rando, three, rando, rando, one, two, rando, three);
			auto m2 = mat2x3(four, rando, four);
			auto m3 = mat3x4(one, three, rando, two, three, rando, one);
			auto m4 = dmat3x3(two, three, four);
			auto m5 = dmat4(two_by_two, two_by_two, two_by_two, two_by_two);

			CHECK_EQ(m1, dmat4x4(4, 4, 4, 4, 5, 3, 3, 3, 5, 5, 1, 2, 2, 5, 3, 3));
			CHECK_EQ(m2, mat2x3(4, 4, 4, 4, 5, 4));
			CHECK_EQ(m3, mat3x4(1, 3, 3, 3, 5, 2, 2, 3, 3, 3, 5, 1));
			CHECK_EQ(m4, dmat3x3(2, 2, 3, 3, 3, 4, 4, 4, 4));
			CHECK_EQ(m5, dmat4(10, 9, 8, 7, 10, 9, 8, 7, 10, 9, 8, 7, 10, 9, 8, 7));
		}

		// this subcase is more about compiling without warnings or errors
		SUBCASE("Implicitly convertible type constructor, same dimensions")
		{
			constexpr mat2 m2{};
			constexpr mat3 m3{};
			constexpr mat4 m4{};
			constexpr mat2x3 m23{};
			constexpr mat2x4 m24{};
			constexpr mat3x2 m32{};
			constexpr mat3x4 m34{};
			constexpr mat4x2 m42{};
			constexpr mat4x3 m43{};

			[[maybe_unused]] dmat2 dm2{m2};
			[[maybe_unused]] dmat3 dm3{m3};
			[[maybe_unused]] dmat4 dm4{m4};
			[[maybe_unused]] dmat2x3 dm23{m23};
			[[maybe_unused]] dmat2x4 dm24{m24};
			[[maybe_unused]] dmat3x2 dm32{m32};
			[[maybe_unused]] dmat3x4 dm34{m34};
			[[maybe_unused]] dmat4x2 dm42{m42};
			[[maybe_unused]] dmat4x3 dm43{m43};
		}

		SUBCASE("matrix argument constructor")
		{
			mat2x2 m22(2, 2, 2, 2);
			mat2x3 m23(3, 3, 3, 3, 3, 3);
			mat2x4 m24(4, 4, 4, 4, 4, 4, 4, 4);
			mat3x2 m32(5, 5, 5, 5, 5, 5);
			mat3x3 m33(6, 6, 6, 6, 6, 6, 6, 6, 6);
			mat3x4 m34(7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			mat4x2 m42(8, 8, 8, 8, 8, 8, 8, 8);
			mat4x3 m43(9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9);
			mat4x4 m44(11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11);

			mat2x2 m22_22(m22);
			mat2x2 m22_23(m23);
			mat2x2 m22_24(m24);
			mat2x2 m22_32(m32);
			mat2x2 m22_33(m33);
			mat2x2 m22_34(m34);
			mat2x2 m22_42(m42);
			mat2x2 m22_43(m43);
			mat2x2 m22_44(m44);

			CHECK_EQ(m22_22, m22);
			CHECK_EQ(m22_23, mat2x2(3, 3, 3, 3));
			CHECK_EQ(m22_24, mat2x2(4, 4, 4, 4));
			CHECK_EQ(m22_32, mat2x2(5, 5, 5, 5));
			CHECK_EQ(m22_33, mat2x2(6, 6, 6, 6));
			CHECK_EQ(m22_34, mat2x2(7, 7, 7, 7));
			CHECK_EQ(m22_42, mat2x2(8, 8, 8, 8));
			CHECK_EQ(m22_43, mat2x2(9, 9, 9, 9));
			CHECK_EQ(m22_44, mat2x2(11, 11, 11, 11));

			mat2x3 m23_22(m22);
			mat2x3 m23_23(m23);
			mat2x3 m23_24(m24);
			mat2x3 m23_32(m32);
			mat2x3 m23_33(m33);
			mat2x3 m23_34(m34);
			mat2x3 m23_42(m42);
			mat2x3 m23_43(m43);
			mat2x3 m23_44(m44);

			CHECK_EQ(m23_22, mat2x3(2, 2, 0, 2, 2, 0));
			CHECK_EQ(m23_23, m23);
			CHECK_EQ(m23_24, mat2x3(4, 4, 4, 4, 4, 4));
			CHECK_EQ(m23_32, mat2x3(5, 5, 0, 5, 5, 0));
			CHECK_EQ(m23_33, mat2x3(6, 6, 6, 6, 6, 6));
			CHECK_EQ(m23_34, mat2x3(7, 7, 7, 7, 7, 7));
			CHECK_EQ(m23_42, mat2x3(8, 8, 0, 8, 8, 0));
			CHECK_EQ(m23_43, mat2x3(9, 9, 9, 9, 9, 9));
			CHECK_EQ(m23_44, mat2x3(11, 11, 11, 11, 11, 11));

			mat2x4 m24_22(m22);
			mat2x4 m24_23(m23);
			mat2x4 m24_24(m24);
			mat2x4 m24_32(m32);
			mat2x4 m24_33(m33);
			mat2x4 m24_34(m34);
			mat2x4 m24_42(m42);
			mat2x4 m24_43(m43);
			mat2x4 m24_44(m44);

			CHECK_EQ(m24_22, mat2x4(2, 2, 0, 0, 2, 2, 0, 0));
			CHECK_EQ(m24_23, mat2x4(3, 3, 3, 0, 3, 3, 3, 0));
			CHECK_EQ(m24_24, m24);
			CHECK_EQ(m24_32, mat2x4(5, 5, 0, 0, 5, 5, 0, 0));
			CHECK_EQ(m24_33, mat2x4(6, 6, 6, 0, 6, 6, 6, 0));
			CHECK_EQ(m24_34, mat2x4(7, 7, 7, 7, 7, 7, 7, 7));
			CHECK_EQ(m24_42, mat2x4(8, 8, 0, 0, 8, 8, 0, 0));
			CHECK_EQ(m24_43, mat2x4(9, 9, 9, 0, 9, 9, 9, 0));
			CHECK_EQ(m24_44, mat2x4(11, 11, 11, 11, 11, 11, 11, 11));

			mat3x2 m32_22(m22);
			mat3x2 m32_23(m23);
			mat3x2 m32_24(m24);
			mat3x2 m32_32(m32);
			mat3x2 m32_33(m33);
			mat3x2 m32_34(m34);
			mat3x2 m32_42(m42);
			mat3x2 m32_43(m43);
			mat3x2 m32_44(m44);

			CHECK_EQ(m32_22, mat3x2(2, 2, 2, 2, 0, 0));
			CHECK_EQ(m32_23, mat3x2(3, 3, 3, 3, 0, 0));
			CHECK_EQ(m32_24, mat3x2(4, 4, 4, 4, 0, 0));
			CHECK_EQ(m32_32, m32);
			CHECK_EQ(m32_33, mat3x2(6, 6, 6, 6, 6, 6));
			CHECK_EQ(m32_34, mat3x2(7, 7, 7, 7, 7, 7));
			CHECK_EQ(m32_42, mat3x2(8, 8, 8, 8, 8, 8));
			CHECK_EQ(m32_43, mat3x2(9, 9, 9, 9, 9, 9));
			CHECK_EQ(m32_44, mat3x2(11, 11, 11, 11, 11, 11));

			mat3x3 m33_22(m22);
			mat3x3 m33_23(m23);
			mat3x3 m33_24(m24);
			mat3x3 m33_32(m32);
			mat3x3 m33_33(m33);
			mat3x3 m33_34(m34);
			mat3x3 m33_42(m42);
			mat3x3 m33_43(m43);
			mat3x3 m33_44(m44);

			CHECK_EQ(m33_22, mat3x3(2, 2, 0, 2, 2, 0, 0, 0, 1));
			CHECK_EQ(m33_23, mat3x3(3, 3, 3, 3, 3, 3, 0, 0, 1));
			CHECK_EQ(m33_24, mat3x3(4, 4, 4, 4, 4, 4, 0, 0, 1));
			CHECK_EQ(m33_32, mat3x3(5, 5, 0, 5, 5, 0, 5, 5, 1));
			CHECK_EQ(m33_33, m33);
			CHECK_EQ(m33_34, mat3x3(7, 7, 7, 7, 7, 7, 7, 7, 7));
			CHECK_EQ(m33_42, mat3x3(8, 8, 0, 8, 8, 0, 8, 8, 1));
			CHECK_EQ(m33_43, mat3x3(9, 9, 9, 9, 9, 9, 9, 9, 9));
			CHECK_EQ(m33_44, mat3x3(11, 11, 11, 11, 11, 11, 11, 11, 11));

			mat3x4 m34_22(m22);
			mat3x4 m34_23(m23);
			mat3x4 m34_24(m24);
			mat3x4 m34_32(m32);
			mat3x4 m34_33(m33);
			mat3x4 m34_34(m34);
			mat3x4 m34_42(m42);
			mat3x4 m34_43(m43);
			mat3x4 m34_44(m44);

			CHECK_EQ(m34_22, mat3x4(2, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0));
			CHECK_EQ(m34_23, mat3x4(3, 3, 3, 0, 3, 3, 3, 0, 0, 0, 0, 0));
			CHECK_EQ(m34_24, mat3x4(4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0));
			CHECK_EQ(m34_32, mat3x4(5, 5, 0, 0, 5, 5, 0, 0, 5, 5, 0, 0));
			CHECK_EQ(m34_33, mat3x4(6, 6, 6, 0, 6, 6, 6, 0, 6, 6, 6, 0));
			CHECK_EQ(m34_34, m34);
			CHECK_EQ(m34_42, mat3x4(8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0));
			CHECK_EQ(m34_43, mat3x4(9, 9, 9, 0, 9, 9, 9, 0, 9, 9, 9, 0));
			CHECK_EQ(m34_44, mat3x4(11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11));

			mat4x2 m42_22(m22);
			mat4x2 m42_23(m23);
			mat4x2 m42_24(m24);
			mat4x2 m42_32(m32);
			mat4x2 m42_33(m33);
			mat4x2 m42_34(m34);
			mat4x2 m42_42(m42);
			mat4x2 m42_43(m43);
			mat4x2 m42_44(m44);

			CHECK_EQ(m42_22, mat4x2(2, 2, 2, 2, 0, 0, 0, 0));
			CHECK_EQ(m42_23, mat4x2(3, 3, 3, 3, 0, 0, 0, 0));
			CHECK_EQ(m42_24, mat4x2(4, 4, 4, 4, 0, 0, 0, 0));
			CHECK_EQ(m42_32, mat4x2(5, 5, 5, 5, 5, 5, 0, 0));
			CHECK_EQ(m42_33, mat4x2(6, 6, 6, 6, 6, 6, 0, 0));
			CHECK_EQ(m42_34, mat4x2(7, 7, 7, 7, 7, 7, 0, 0));
			CHECK_EQ(m42_42, m42);
			CHECK_EQ(m42_43, mat4x2(9, 9, 9, 9, 9, 9, 9, 9));
			CHECK_EQ(m42_44, mat4x2(11, 11, 11, 11, 11, 11, 11, 11));

			mat4x3 m43_22(m22);
			mat4x3 m43_23(m23);
			mat4x3 m43_24(m24);
			mat4x3 m43_32(m32);
			mat4x3 m43_33(m33);
			mat4x3 m43_34(m34);
			mat4x3 m43_42(m42);
			mat4x3 m43_43(m43);
			mat4x3 m43_44(m44);

			CHECK_EQ(m43_22, mat4x3(2, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0));
			CHECK_EQ(m43_23, mat4x3(3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0));
			CHECK_EQ(m43_24, mat4x3(4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0));
			CHECK_EQ(m43_32, mat4x3(5, 5, 0, 5, 5, 0, 5, 5, 0, 0, 0, 0));
			CHECK_EQ(m43_33, mat4x3(6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0));
			CHECK_EQ(m43_34, mat4x3(7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0));
			CHECK_EQ(m43_42, mat4x3(8, 8, 0, 8, 8, 0, 8, 8, 0, 8, 8, 0));
			CHECK_EQ(m43_43, m43);
			CHECK_EQ(m43_44, mat4x3(11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11));

			mat4x4 m44_22(m22);
			mat4x4 m44_23(m23);
			mat4x4 m44_24(m24);
			mat4x4 m44_32(m32);
			mat4x4 m44_33(m33);
			mat4x4 m44_34(m34);
			mat4x4 m44_42(m42);
			mat4x4 m44_43(m43);
			mat4x4 m44_44(m44);

			CHECK_EQ(m44_22, mat4x4(2, 2, 0, 0, 2, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
			CHECK_EQ(m44_23, mat4x4(3, 3, 3, 0, 3, 3, 3, 0, 0, 0, 1, 0, 0, 0, 0, 1));
			CHECK_EQ(m44_24, mat4x4(4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 1, 0, 0, 0, 0, 1));
			CHECK_EQ(m44_32, mat4x4(5, 5, 0, 0, 5, 5, 0, 0, 5, 5, 1, 0, 0, 0, 0, 1));
			CHECK_EQ(m44_33, mat4x4(6, 6, 6, 0, 6, 6, 6, 0, 6, 6, 6, 0, 0, 0, 0, 1));
			CHECK_EQ(m44_34, mat4x4(7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 1));
			CHECK_EQ(m44_42, mat4x4(8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 1, 0, 8, 8, 0, 1));
			CHECK_EQ(m44_43, mat4x4(9, 9, 9, 0, 9, 9, 9, 0, 9, 9, 9, 0, 9, 9, 9, 1));
			CHECK_EQ(m44_44, m44);
		}
	}
}
