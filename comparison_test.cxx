
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

//
// make sure that exact equality comparisons (==, !=) work so we can trust the rest of the tests.
// these tests DO NOT perform floating point epsilon types of checks.
//

TEST_SUITE("test comparisons")
{
	static ivec4	four	(0, 1, 2, 3);
	static ivec3	three	(0, 1, 2);
	static ivec2	two		(0, 1);
	static iscal	one		(0);

	TEST_CASE("basic_vector comparisons")
	{
		SUBCASE("1D basic_vector comparisons")
		{
			CHECK_EQ(one, iscal(0));
			CHECK_EQ(iscal(0), iscal(0));
			CHECK_NE(iscal(1), iscal(-1));
			CHECK_NE(one, iscal(1));
			CHECK_NE(one, iscal(-1));
		}

		SUBCASE("2D basic_vector comparisons")
		{
			CHECK_EQ(two, ivec2(0, 1));
			CHECK_EQ(ivec2(0, 1), ivec2(0, 1));
			CHECK_NE(ivec2(1, -1), ivec2(-1, 1));
			CHECK_NE(two, ivec2(1, 0));
			CHECK_NE(two, ivec2(0, -1));
		}

		SUBCASE("3D basic_vector comparisons")
		{
			CHECK_EQ(three, ivec3(0, 1, 2));
			CHECK_EQ(ivec3(0, 1, 2), ivec3(0, 1, 2));
			CHECK_NE(ivec3(0, 1, -1), ivec3(0, -1, 1));
			CHECK_NE(three, ivec3(1, 0, 2));
			CHECK_NE(three, ivec3(0, -1, 2));
		}

		SUBCASE("4D basic_vector comparisons")
		{
			CHECK_EQ(four, ivec4(0, 1, 2, 3));
			CHECK_EQ(ivec4(0, 1, 2, 3), ivec4(0, 1, 2, 3));
			CHECK_NE(ivec4(0, 1, -1, 3), ivec4(0, -1, 1, 3));
			CHECK_NE(four, ivec4(1, 0, 2, 3));
			CHECK_NE(four, ivec4(0, -1, 2, 3));
		}
	}

	TEST_CASE("indexed_vector comparisons")
	{
		SUBCASE("1D indexed_vector comparison")
		{
			CHECK_EQ(one.x, two.x);
			CHECK_EQ(one.x, three.x);
			CHECK_EQ(one.x, four.x);
			CHECK_NE(one.x, two.y);
			CHECK_EQ(two.y, three.y);
			CHECK_EQ(two.y, four.y);
			CHECK_NE(two.y, three.z);
			CHECK_EQ(three.z, four.z);
			CHECK_NE(three.z, four.w);
		}

		SUBCASE("2D indexed_vector comparison")
		{
			CHECK_EQ(one.xx, two.xx);
			CHECK_EQ(one.xx, three.xx);
			CHECK_EQ(one.xx, four.xx);
			CHECK_NE(one.xx, two.xy);
			CHECK_EQ(two.xy, three.xy);
			CHECK_EQ(two.xy, four.xy);
			CHECK_NE(two.xy, three.yz);
			CHECK_EQ(three.yz, four.yz);
			CHECK_NE(three.yz, four.yw);
		}

		SUBCASE("3D indexed_vector comparison")
		{
			CHECK_EQ(one.xxx, two.xxx);
			CHECK_EQ(one.xxx, three.xxx);
			CHECK_EQ(one.xxx, four.xxx);
			CHECK_NE(one.xxx, two.xyx);
			CHECK_EQ(two.xyx, three.xyx);
			CHECK_EQ(two.xyx, four.xyx);
			CHECK_NE(two.xyx, three.xyz);
			CHECK_EQ(three.xyz, four.xyz);
			CHECK_NE(three.xyz, four.xyw);
		}

		SUBCASE("4D indexed_vector comparison")
		{
			CHECK_EQ(one.xxxx, two.xxxx);
			CHECK_EQ(one.xxxx, three.xxxx);
			CHECK_EQ(one.xxxx, four.xxxx);
			CHECK_NE(one.xxxx, two.xyxy);
			CHECK_EQ(two.xyxy, three.xyxy);
			CHECK_EQ(two.xyxy, four.xyxy);
			CHECK_NE(two.xyxy, three.xyxz);
			CHECK_EQ(three.xyxz, four.xyxz);
			CHECK_NE(three.xyxz, four.xyzw);
		}
	}

	TEST_CASE("comparing basic_vector with indexed_vector")
	{
		SUBCASE("1D basic_vector with indexed_vector comparisons")
		{
			CHECK_EQ(one.x, iscal(0));
			CHECK_EQ(two.x, iscal(0));
			CHECK_EQ(two.y, iscal(1));
			CHECK_NE(two.x, iscal(-1));
			CHECK_EQ(three.x, iscal(0));
			CHECK_EQ(three.y, iscal(1));
			CHECK_EQ(three.z, iscal(2));
			CHECK_NE(three.y, iscal(-1));
			CHECK_NE(three.x, iscal(1));
			CHECK_NE(three.z, iscal(5));
			CHECK_EQ(four.x, iscal(0));
			CHECK_EQ(four.y, iscal(1));
			CHECK_EQ(four.z, iscal(2));
			CHECK_EQ(four.w, iscal(3));
			CHECK_NE(four.z, iscal(-2));
			CHECK_NE(four.y, iscal(-1));
			CHECK_NE(four.x, iscal(5));
		}

		SUBCASE("2D basic_vector with indexed_vector comparisons")
		{
			CHECK_EQ(one.xx, ivec2(0, 0));
			CHECK_EQ(two.xy, ivec2(0, 1));
			CHECK_EQ(three.yz, ivec2(1, 2));
			CHECK_EQ(four.zw, ivec2(2, 3));
			CHECK_NE(four.wz, ivec2(2, 3));
			CHECK_EQ(dvec4(9, 8, 7, 6).xz, ivec2(9, 7));
			CHECK_NE(dvec4(9, 8, 7, 6).xw, ivec2(9, 7));
			CHECK_EQ(ivec3(10, 20, 30).zx, ivec2(30, 10));
			CHECK_NE(ivec3(10, 20, 30).xz, ivec2(30, 10));
			CHECK_EQ(ivec2(44, 66).xy, ivec2(44, 66));
		}

		SUBCASE("3D basic_vector with indexed_vector comparisons")
		{
			CHECK_EQ(one.xxx, ivec3(0, 0, 0));
			CHECK_EQ(two.xyy, ivec3(0, 1, 1));
			CHECK_EQ(three.yzx, ivec3(1, 2, 0));
			CHECK_EQ(four.zwy, ivec3(2, 3, 1));
			CHECK_EQ(four.wxz, ivec3(3, 0, 2));
			CHECK_NE(four.wxz, ivec3(3, 0, 1));
			CHECK_EQ(dvec4(9, 8, 7, 6).wxz, ivec3(6, 9, 7));
			CHECK_NE(dvec4(9, 8, 7, 6).xwz, ivec3(6, 9, 7));
			CHECK_EQ(ivec3(10, 20, 30).zxy, ivec3(30, 10, 20));
			CHECK_NE(ivec3(10, 20, 30).xzy, ivec3(30, 10, 20));
		}

		SUBCASE("4D basic_vector with indexed_vector comparisons")
		{
			CHECK_EQ(one.xxxx, ivec4(0, 0, 0, 0));
			CHECK_EQ(two.xyyx, ivec4(0, 1, 1, 0));
			CHECK_EQ(three.yzyx, ivec4(1, 2, 1, 0));
			CHECK_EQ(four.zywy, ivec4(2, 1, 3, 1));
			CHECK_EQ(four.ywxz, ivec4(1, 3, 0, 2));
			CHECK_NE(four.ywxz, ivec4(1, 3, 0, 1));
			CHECK_EQ(dvec4(9, 8, 7, 6).wxzx, ivec4(6, 9, 7, 9));
			CHECK_NE(dvec4(9, 8, 7, 6).xwzx, ivec4(6, 9, 7, 9));
			CHECK_EQ(ivec3(10, 20, 30).zzxy, ivec4(30, 30, 10, 20));
			CHECK_NE(ivec3(10, 20, 30).xzzy, ivec4(30, 30, 10, 20));
			CHECK_EQ(ivec2(555, 888).xyxy, ivec4(555, 888, 555, 888));
			CHECK_NE(ivec2(555, 888).xyyx, ivec4(555, 888, 555, 888));
		}
	}
}
