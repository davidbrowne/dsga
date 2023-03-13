
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

//
// make sure that exact equality comparisons (==, !=) work so we can trust the rest of the tests.
// these tests DO NOT perform floating point epsilon types of checks.
//

TEST_SUITE("test equality comparisons")
{
	static constexpr ivec4	four	(0, 1, 2, 3);
	static constexpr ivec3	three	(0, 1, 2);
	static constexpr ivec2	two		(0, 1);
	static constexpr iscal	one		(0);

	TEST_CASE("basic_vector equality comparisons")
	{
		SUBCASE("1D basic_vector equality comparisons")
		{
			CHECK_EQ(one, iscal(0));
			CHECK_EQ(iscal(0), iscal(0));
			CHECK_NE(iscal(1), iscal(-1));
			CHECK_NE(one, iscal(1));
			CHECK_NE(one, iscal(-1));
		}

		SUBCASE("2D basic_vector equality comparisons")
		{
			CHECK_EQ(two, ivec2(0, 1));
			CHECK_EQ(ivec2(0, 1), ivec2(0, 1));
			CHECK_NE(ivec2(1, -1), ivec2(-1, 1));
			CHECK_NE(two, ivec2(1, 0));
			CHECK_NE(two, ivec2(0, -1));
		}

		SUBCASE("3D basic_vector equality comparisons")
		{
			CHECK_EQ(three, ivec3(0, 1, 2));
			CHECK_EQ(ivec3(0, 1, 2), ivec3(0, 1, 2));
			CHECK_NE(ivec3(0, 1, -1), ivec3(0, -1, 1));
			CHECK_NE(three, ivec3(1, 0, 2));
			CHECK_NE(three, ivec3(0, -1, 2));
		}

		SUBCASE("4D basic_vector equality comparisons")
		{
			CHECK_EQ(four, ivec4(0, 1, 2, 3));
			CHECK_EQ(ivec4(0, 1, 2, 3), ivec4(0, 1, 2, 3));
			CHECK_NE(ivec4(0, 1, -1, 3), ivec4(0, -1, 1, 3));
			CHECK_NE(four, ivec4(1, 0, 2, 3));
			CHECK_NE(four, ivec4(0, -1, 2, 3));
		}
	}

	TEST_CASE("indexed_vector equality comparisons")
	{
		SUBCASE("1D indexed_vector equality comparison")
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

		SUBCASE("2D indexed_vector equality comparison")
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

		SUBCASE("3D indexed_vector equality comparison")
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

		SUBCASE("4D indexed_vector equality comparison")
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

	TEST_CASE("equality comparing basic_vector with indexed_vector")
	{
		SUBCASE("1D basic_vector with indexed_vector equality comparisons")
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

		SUBCASE("2D basic_vector with indexed_vector equality comparisons")
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

		SUBCASE("3D basic_vector with indexed_vector equality comparisons")
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

		SUBCASE("4D basic_vector with indexed_vector equality comparisons")
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

	TEST_CASE("matrix equality comparisons")
	{
		const auto m2x2 = mat2x2(fvec2(1, 1), fvec2(1, 1));
		const auto m2x3 = mat2x3(fvec2(2, 2), fvec2(2, 2), fvec2(2, 2));
		const auto m2x4 = mat2x4(fvec2(3, 3), fvec2(3, 3), fvec2(3, 3), fvec2(3, 3));
		const auto m3x2 = mat3x2(fvec3(4, 4, 4), fvec3(4, 4, 4));
		const auto m3x3 = mat3x3(fvec3(5, 5, 5), fvec3(5, 5, 5), fvec3(5, 5, 5));
		const auto m3x4 = mat3x4(fvec3(6, 6, 6), fvec3(6, 6, 6), fvec3(6, 6, 6), fvec3(6, 6, 6));
		const auto m4x2 = mat4x2(fvec4(7, 7, 7, 7), fvec4(7, 7, 7, 7));
		const auto m4x3 = mat4x3(fvec4(8, 8, 8, 8), fvec4(8, 8, 8, 8), fvec4(8, 8, 8, 8));
		const auto m4x4 = mat4x4(fvec4(9, 9, 9, 9), fvec4(9, 9, 9, 9), fvec4(9, 9, 9, 9), fvec4(9, 9, 9, 9));

		SUBCASE("mat2x2 equality comparisons")
		{
			auto mat = mat2x2(m4x4);
			CHECK_NE(mat, m2x2);
			mat = m2x2;
			CHECK_EQ(mat, m2x2);
		}

		SUBCASE("mat2x3 equality comparisons")
		{
			auto mat = mat2x3(m4x4);
			CHECK_NE(mat, m2x3);
			mat = m2x3;
			CHECK_EQ(mat, m2x3);
		}

		SUBCASE("mat2x4 equality comparisons")
		{
			auto mat = mat2x4(m4x4);
			CHECK_NE(mat, m2x4);
			mat = m2x4;
			CHECK_EQ(mat, m2x4);
		}

		SUBCASE("mat3x2 equality comparisons")
		{
			auto mat = mat3x2(m4x4);
			CHECK_NE(mat, m3x2);
			mat = m3x2;
			CHECK_EQ(mat, m3x2);
		}

		SUBCASE("mat3x3 equality comparisons")
		{
			auto mat = mat3x3(m4x4);
			CHECK_NE(mat, m3x3);
			mat = m3x3;
			CHECK_EQ(mat, m3x3);
		}

		SUBCASE("mat3x4 equality comparisons")
		{
			auto mat = mat3x4(m4x4);
			CHECK_NE(mat, m3x4);
			mat = m3x4;
			CHECK_EQ(mat, m3x4);
		}

		SUBCASE("mat4x2 equality comparisons")
		{
			auto mat = mat4x2(m4x4);
			CHECK_NE(mat, m4x2);
			mat = m4x2;
			CHECK_EQ(mat, m4x2);
		}

		SUBCASE("mat4x3 equality comparisons")
		{
			auto mat = mat4x3(m4x4);
			CHECK_NE(mat, m4x3);
			mat = m4x3;
			CHECK_EQ(mat, m4x3);
		}

		SUBCASE("mat4x4 equality comparisons")
		{
			auto mat = mat4x4(m2x2);
			CHECK_NE(mat, m4x4);
			mat = m4x4;
			CHECK_EQ(mat, m4x4);
		}
	}
}

TEST_SUITE("test <=> comparisons")
{
	TEST_CASE("basic_vector <=> comparisons")
	{
		SUBCASE("1D basic_vector <=> comparisons")
		{
			constexpr iscal	is1(0);
			constexpr iscal	is2(-1);
			constexpr iscal	is3(1);
			constexpr uscal	us1(0);
			constexpr uscal	us2(1);
			constexpr fscal	fs1(0);
			constexpr fscal	fs2(-1);
			constexpr fscal	fs3(1);
			constexpr bscal	bs1(false);
			constexpr bscal	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1, is3);
			CHECK_LE(is1, is3);
			CHECK_GT(is1, is2);
			CHECK_GE(is1, is2);
			CHECK_LE(is1, is1);
			CHECK_GE(is1, is1);

			// equality across different vector type classifications
			CHECK_EQ(is1, us1);
			CHECK_EQ(is1, fs1);
			CHECK_EQ(is1, bs1);
			CHECK_EQ(bs1, fs1);

			// <=> across different vector type classifications
			CHECK_LT(is1, us2);
			CHECK_LE(is1, us2);
			CHECK_GT(us2, is1);
			CHECK_GE(is3, us1);
			CHECK_LE(is1, us1);

			CHECK_LT(is1, fs3);
			CHECK_LE(is1, fs3);
			CHECK_GT(is1, fs2);
			CHECK_GE(is1, fs2);

			CHECK_LT(is1, bs2);
			CHECK_LE(is1, bs2);
			CHECK_GT(bs2, is1);
			CHECK_GE(is3, bs1);
			CHECK_LE(is1, bs1);
		}

		SUBCASE("2D basic_vector <=> comparisons")
		{
			constexpr ivec2	iv1(0);
			constexpr ivec2	iv2(-1);
			constexpr ivec2	iv3(1);
			constexpr uvec2	uv1(0);
			constexpr uvec2	uv2(1);
			constexpr vec2	fv1(0);
			constexpr vec2	fv2(-1);
			constexpr vec2	fv3(1);
			constexpr bvec2	bv1(false);
			constexpr bvec2	bv2(true);

			// <=> within a vector type classification
			CHECK_LT(iv1, iv3);
			CHECK_LE(iv1, iv3);
			CHECK_GT(iv1, iv2);
			CHECK_GE(iv1, iv2);
			CHECK_LE(iv1, iv1);
			CHECK_GE(iv1, iv1);

			// equality across different vector type classifications
			CHECK_EQ(iv1, uv1);
			CHECK_EQ(iv1, fv1);
			CHECK_EQ(iv1, bv1);
			CHECK_EQ(bv1, fv1);

			// <=> across different vector type classifications
			CHECK_LT(iv1, uv2);
			CHECK_LE(iv1, uv2);
			CHECK_GT(uv2, iv1);
			CHECK_GE(iv3, uv1);
			CHECK_LE(iv1, uv1);

			CHECK_LT(iv1, fv3);
			CHECK_LE(iv1, fv3);
			CHECK_GT(iv1, fv2);
			CHECK_GE(iv1, fv2);

			CHECK_LT(iv1, bv2);
			CHECK_LE(iv1, bv2);
			CHECK_GT(bv2, iv1);
			CHECK_GE(iv3, bv1);
			CHECK_LE(iv1, bv1);
		}

		SUBCASE("3D basic_vector <=> comparisons")
		{
			constexpr ivec3	iv1(0);
			constexpr ivec3	iv2(-1);
			constexpr ivec3	iv3(1);
			constexpr uvec3	uv1(0);
			constexpr uvec3	uv2(1);
			constexpr vec3	fv1(0);
			constexpr vec3	fv2(-1);
			constexpr vec3	fv3(1);
			constexpr bvec3	bv1(false);
			constexpr bvec3	bv2(true);

			// <=> within a vector type classification
			CHECK_LT(iv1, iv3);
			CHECK_LE(iv1, iv3);
			CHECK_GT(iv1, iv2);
			CHECK_GE(iv1, iv2);
			CHECK_LE(iv1, iv1);
			CHECK_GE(iv1, iv1);

			// equality across different vector type classifications
			CHECK_EQ(iv1, uv1);
			CHECK_EQ(iv1, fv1);
			CHECK_EQ(iv1, bv1);
			CHECK_EQ(bv1, fv1);

			// <=> across different vector type classifications
			CHECK_LT(iv1, uv2);
			CHECK_LE(iv1, uv2);
			CHECK_GT(uv2, iv1);
			CHECK_GE(iv3, uv1);
			CHECK_LE(iv1, uv1);

			CHECK_LT(iv1, fv3);
			CHECK_LE(iv1, fv3);
			CHECK_GT(iv1, fv2);
			CHECK_GE(iv1, fv2);

			CHECK_LT(iv1, bv2);
			CHECK_LE(iv1, bv2);
			CHECK_GT(bv2, iv1);
			CHECK_GE(iv3, bv1);
			CHECK_LE(iv1, bv1);
		}

		SUBCASE("4D basic_vector <=> comparisons")
		{
			constexpr ivec4	iv1(0);
			constexpr ivec4	iv2(-1);
			constexpr ivec4	iv3(1);
			constexpr uvec4	uv1(0);
			constexpr uvec4	uv2(1);
			constexpr vec4	fv1(0);
			constexpr vec4	fv2(-1);
			constexpr vec4	fv3(1);
			constexpr bvec4	bv1(false);
			constexpr bvec4	bv2(true);

			// <=> within a vector type classification
			CHECK_LT(iv1, iv3);
			CHECK_LE(iv1, iv3);
			CHECK_GT(iv1, iv2);
			CHECK_GE(iv1, iv2);
			CHECK_LE(iv1, iv1);
			CHECK_GE(iv1, iv1);

			// equality across different vector type classifications
			CHECK_EQ(iv1, uv1);
			CHECK_EQ(iv1, fv1);
			CHECK_EQ(iv1, bv1);
			CHECK_EQ(bv1, fv1);

			// <=> across different vector type classifications
			CHECK_LT(iv1, uv2);
			CHECK_LE(iv1, uv2);
			CHECK_GT(uv2, iv1);
			CHECK_GE(iv3, uv1);
			CHECK_LE(iv1, uv1);

			CHECK_LT(iv1, fv3);
			CHECK_LE(iv1, fv3);
			CHECK_GT(iv1, fv2);
			CHECK_GE(iv1, fv2);

			CHECK_LT(iv1, bv2);
			CHECK_LE(iv1, bv2);
			CHECK_GT(bv2, iv1);
			CHECK_GE(iv3, bv1);
			CHECK_LE(iv1, bv1);
		}
	}

	TEST_CASE("indexed_vector <=> comparisons")
	{
		SUBCASE("1D indexed_vector <=> comparison")
		{
			constexpr iscal	is1(0);
			constexpr iscal	is2(-1);
			constexpr iscal	is3(1);
			constexpr uscal	us1(0);
			constexpr uscal	us2(1);
			constexpr fscal	fs1(0);
			constexpr fscal	fs2(-1);
			constexpr fscal	fs3(1);
			constexpr bscal	bs1(false);
			constexpr bscal	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.x, is3.x);
			CHECK_LE(is1.x, is3.x);
			CHECK_GT(is1.x, is2.x);
			CHECK_GE(is1.x, is2.x);
			CHECK_LE(is1.x, is1.x);
			CHECK_GE(is1.x, is1.x);

			// equality across different vector type classifications
			CHECK_EQ(is1.x, us1.x);
			CHECK_EQ(is1.x, fs1.x);
			CHECK_EQ(is1.x, bs1.x);
			CHECK_EQ(bs1.x, fs1.x);

			// <=> across different vector type classifications
			CHECK_LT(is1.x, us2.x);
			CHECK_LE(is1.x, us2.x);
			CHECK_GT(us2.x, is1.x);
			CHECK_GE(is3.x, us1.x);
			CHECK_LE(is1.x, us1.x);

			CHECK_LT(is1.x, fs3.x);
			CHECK_LE(is1.x, fs3.x);
			CHECK_GT(is1.x, fs2.x);
			CHECK_GE(is1.x, fs2.x);

			CHECK_LT(is1.x, bs2.x);
			CHECK_LE(is1.x, bs2.x);
			CHECK_GT(bs2.x, is1.x);
			CHECK_GE(is3.x, bs1.x);
			CHECK_LE(is1.x, bs1.x);
		}

		SUBCASE("2D indexed_vector <=> comparison")
		{
			constexpr iscal	is1(0);
			constexpr iscal	is2(-1);
			constexpr iscal	is3(1);
			constexpr uscal	us1(0);
			constexpr uscal	us2(1);
			constexpr fscal	fs1(0);
			constexpr fscal	fs2(-1);
			constexpr fscal	fs3(1);
			constexpr bscal	bs1(false);
			constexpr bscal	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.xx, is3.xx);
			CHECK_LE(is1.xx, is3.xx);
			CHECK_GT(is1.xx, is2.xx);
			CHECK_GE(is1.xx, is2.xx);
			CHECK_LE(is1.xx, is1.xx);
			CHECK_GE(is1.xx, is1.xx);

			// equality across different vector type classifications
			CHECK_EQ(is1.xx, us1.xx);
			CHECK_EQ(is1.xx, fs1.xx);
			CHECK_EQ(is1.xx, bs1.xx);
			CHECK_EQ(bs1.xx, fs1.xx);

			// <=> across different vector type classifications
			CHECK_LT(is1.xx, us2.xx);
			CHECK_LE(is1.xx, us2.xx);
			CHECK_GT(us2.xx, is1.xx);
			CHECK_GE(is3.xx, us1.xx);
			CHECK_LE(is1.xx, us1.xx);

			CHECK_LT(is1.xx, fs3.xx);
			CHECK_LE(is1.xx, fs3.xx);
			CHECK_GT(is1.xx, fs2.xx);
			CHECK_GE(is1.xx, fs2.xx);

			CHECK_LT(is1.xx, bs2.xx);
			CHECK_LE(is1.xx, bs2.xx);
			CHECK_GT(bs2.xx, is1.xx);
			CHECK_GE(is3.xx, bs1.xx);
			CHECK_LE(is1.xx, bs1.xx);
		}

		SUBCASE("3D indexed_vector <=> comparison")
		{
			constexpr iscal	is1(0);
			constexpr iscal	is2(-1);
			constexpr iscal	is3(1);
			constexpr uscal	us1(0);
			constexpr uscal	us2(1);
			constexpr fscal	fs1(0);
			constexpr fscal	fs2(-1);
			constexpr fscal	fs3(1);
			constexpr bscal	bs1(false);
			constexpr bscal	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.xxx, is3.xxx);
			CHECK_LE(is1.xxx, is3.xxx);
			CHECK_GT(is1.xxx, is2.xxx);
			CHECK_GE(is1.xxx, is2.xxx);
			CHECK_LE(is1.xxx, is1.xxx);
			CHECK_GE(is1.xxx, is1.xxx);

			// equality across different vector type classifications
			CHECK_EQ(is1.xxx, us1.xxx);
			CHECK_EQ(is1.xxx, fs1.xxx);
			CHECK_EQ(is1.xxx, bs1.xxx);
			CHECK_EQ(bs1.xxx, fs1.xxx);

			// <=> across different vector type classifications
			CHECK_LT(is1.xxx, us2.xxx);
			CHECK_LE(is1.xxx, us2.xxx);
			CHECK_GT(us2.xxx, is1.xxx);
			CHECK_GE(is3.xxx, us1.xxx);
			CHECK_LE(is1.xxx, us1.xxx);

			CHECK_LT(is1.xxx, fs3.xxx);
			CHECK_LE(is1.xxx, fs3.xxx);
			CHECK_GT(is1.xxx, fs2.xxx);
			CHECK_GE(is1.xxx, fs2.xxx);

			CHECK_LT(is1.xxx, bs2.xxx);
			CHECK_LE(is1.xxx, bs2.xxx);
			CHECK_GT(bs2.xxx, is1.xxx);
			CHECK_GE(is3.xxx, bs1.xxx);
			CHECK_LE(is1.xxx, bs1.xxx);
		}

		SUBCASE("4D indexed_vector <=> comparison")
		{
			constexpr iscal	is1(0);
			constexpr iscal	is2(-1);
			constexpr iscal	is3(1);
			constexpr uscal	us1(0);
			constexpr uscal	us2(1);
			constexpr fscal	fs1(0);
			constexpr fscal	fs2(-1);
			constexpr fscal	fs3(1);
			constexpr bscal	bs1(false);
			constexpr bscal	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.xxxx, is3.xxxx);
			CHECK_LE(is1.xxxx, is3.xxxx);
			CHECK_GT(is1.xxxx, is2.xxxx);
			CHECK_GE(is1.xxxx, is2.xxxx);
			CHECK_LE(is1.xxxx, is1.xxxx);
			CHECK_GE(is1.xxxx, is1.xxxx);

			// equality across different vector type classifications
			CHECK_EQ(is1.xxxx, us1.xxxx);
			CHECK_EQ(is1.xxxx, fs1.xxxx);
			CHECK_EQ(is1.xxxx, bs1.xxxx);
			CHECK_EQ(bs1.xxxx, fs1.xxxx);

			// <=> across different vector type classifications
			CHECK_LT(is1.xxxx, us2.xxxx);
			CHECK_LE(is1.xxxx, us2.xxxx);
			CHECK_GT(us2.xxxx, is1.xxxx);
			CHECK_GE(is3.xxxx, us1.xxxx);
			CHECK_LE(is1.xxxx, us1.xxxx);

			CHECK_LT(is1.xxxx, fs3.xxxx);
			CHECK_LE(is1.xxxx, fs3.xxxx);
			CHECK_GT(is1.xxxx, fs2.xxxx);
			CHECK_GE(is1.xxxx, fs2.xxxx);

			CHECK_LT(is1.xxxx, bs2.xxxx);
			CHECK_LE(is1.xxxx, bs2.xxxx);
			CHECK_GT(bs2.xxxx, is1.xxxx);
			CHECK_GE(is3.xxxx, bs1.xxxx);
			CHECK_LE(is1.xxxx, bs1.xxxx);
		}
	}

	TEST_CASE("<=> comparing basic_vector with indexed_vector")
	{
		SUBCASE("1D basic_vector with indexed_vector <=> comparisons")
		{
			constexpr iscal	is1(0);
			constexpr iscal	is2(-1);
			constexpr iscal	is3(1);
			constexpr uscal	us1(0);
			constexpr uscal	us2(1);
			constexpr fscal	fs1(0);
			constexpr fscal	fs2(-1);
			constexpr fscal	fs3(1);
			constexpr bscal	bs1(false);
			constexpr bscal	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.x, is3);
			CHECK_LE(is1.x, is3);
			CHECK_GT(is1.x, is2);
			CHECK_GE(is1.x, is2);
			CHECK_LE(is1.x, is1);
			CHECK_GE(is1.x, is1);

			// equality across different vector type classifications
			CHECK_EQ(is1.x, us1);
			CHECK_EQ(is1.x, fs1);
			CHECK_EQ(is1.x, bs1);
			CHECK_EQ(bs1.x, fs1);

			// <=> across different vector type classifications
			CHECK_LT(is1.x, us2);
			CHECK_LE(is1.x, us2);
			CHECK_GT(us2.x, is1);
			CHECK_GE(is3.x, us1);
			CHECK_LE(is1.x, us1);

			CHECK_LT(is1.x, fs3);
			CHECK_LE(is1.x, fs3);
			CHECK_GT(is1.x, fs2);
			CHECK_GE(is1.x, fs2);

			CHECK_LT(is1.x, bs2);
			CHECK_LE(is1.x, bs2);
			CHECK_GT(bs2.x, is1);
			CHECK_GE(is3.x, bs1);
			CHECK_LE(is1.x, bs1);
		}

		SUBCASE("2D basic_vector with indexed_vector <=> comparisons")
		{
			constexpr ivec2	is1(0);
			constexpr ivec2	is2(-1);
			constexpr ivec2	is3(1);
			constexpr uvec2	us1(0);
			constexpr uvec2	us2(1);
			constexpr fvec2	fs1(0);
			constexpr fvec2	fs2(-1);
			constexpr fvec2	fs3(1);
			constexpr bvec2	bs1(false);
			constexpr bvec2	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.xx, is3);
			CHECK_LE(is1.xx, is3);
			CHECK_GT(is1.xx, is2);
			CHECK_GE(is1.xx, is2);
			CHECK_LE(is1.xx, is1);
			CHECK_GE(is1.xx, is1);

			// equality across different vector type classifications
			CHECK_EQ(is1.xx, us1);
			CHECK_EQ(is1.xx, fs1);
			CHECK_EQ(is1.xx, bs1);
			CHECK_EQ(bs1.xx, fs1);

			// <=> across different vector type classifications
			CHECK_LT(is1.xx, us2);
			CHECK_LE(is1.xx, us2);
			CHECK_GT(us2.xx, is1);
			CHECK_GE(is3.xx, us1);
			CHECK_LE(is1.xx, us1);

			CHECK_LT(is1.xx, fs3);
			CHECK_LE(is1.xx, fs3);
			CHECK_GT(is1.xx, fs2);
			CHECK_GE(is1.xx, fs2);

			CHECK_LT(is1.xx, bs2);
			CHECK_LE(is1.xx, bs2);
			CHECK_GT(bs2.xx, is1);
			CHECK_GE(is3.xx, bs1);
			CHECK_LE(is1.xx, bs1);
		}

		SUBCASE("3D basic_vector with indexed_vector <=> comparisons")
		{
			constexpr ivec3	is1(0);
			constexpr ivec3	is2(-1);
			constexpr ivec3	is3(1);
			constexpr uvec3	us1(0);
			constexpr uvec3	us2(1);
			constexpr fvec3	fs1(0);
			constexpr fvec3	fs2(-1);
			constexpr fvec3	fs3(1);
			constexpr bvec3	bs1(false);
			constexpr bvec3	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.xxx, is3);
			CHECK_LE(is1.xxx, is3);
			CHECK_GT(is1.xxx, is2);
			CHECK_GE(is1.xxx, is2);
			CHECK_LE(is1.xxx, is1);
			CHECK_GE(is1.xxx, is1);

			// equality across different vector type classifications
			CHECK_EQ(is1.xxx, us1);
			CHECK_EQ(is1.xxx, fs1);
			CHECK_EQ(is1.xxx, bs1);
			CHECK_EQ(bs1.xxx, fs1);

			// <=> across different vector type classifications
			CHECK_LT(is1.xxx, us2);
			CHECK_LE(is1.xxx, us2);
			CHECK_GT(us2.xxx, is1);
			CHECK_GE(is3.xxx, us1);
			CHECK_LE(is1.xxx, us1);

			CHECK_LT(is1.xxx, fs3);
			CHECK_LE(is1.xxx, fs3);
			CHECK_GT(is1.xxx, fs2);
			CHECK_GE(is1.xxx, fs2);

			CHECK_LT(is1.xxx, bs2);
			CHECK_LE(is1.xxx, bs2);
			CHECK_GT(bs2.xxx, is1);
			CHECK_GE(is3.xxx, bs1);
			CHECK_LE(is1.xxx, bs1);
		}

		SUBCASE("4D basic_vector with indexed_vector <=> comparisons")
		{
			constexpr ivec4	is1(0);
			constexpr ivec4	is2(-1);
			constexpr ivec4	is3(1);
			constexpr uvec4	us1(0);
			constexpr uvec4	us2(1);
			constexpr fvec4	fs1(0);
			constexpr fvec4	fs2(-1);
			constexpr fvec4	fs3(1);
			constexpr bvec4	bs1(false);
			constexpr bvec4	bs2(true);

			// <=> within a vector type classification
			CHECK_LT(is1.xxxx, is3);
			CHECK_LE(is1.xxxx, is3);
			CHECK_GT(is1.xxxx, is2);
			CHECK_GE(is1.xxxx, is2);
			CHECK_LE(is1.xxxx, is1);
			CHECK_GE(is1.xxxx, is1);

			// equality across different vector type classifications
			CHECK_EQ(is1.xxxx, us1);
			CHECK_EQ(is1.xxxx, fs1);
			CHECK_EQ(is1.xxxx, bs1);
			CHECK_EQ(bs1.xxxx, fs1);

			// <=> across different vector type classifications
			CHECK_LT(is1.xxxx, us2);
			CHECK_LE(is1.xxxx, us2);
			CHECK_GT(us2.xxxx, is1);
			CHECK_GE(is3.xxxx, us1);
			CHECK_LE(is1.xxxx, us1);

			CHECK_LT(is1.xxxx, fs3);
			CHECK_LE(is1.xxxx, fs3);
			CHECK_GT(is1.xxxx, fs2);
			CHECK_GE(is1.xxxx, fs2);

			CHECK_LT(is1.xxxx, bs2);
			CHECK_LE(is1.xxxx, bs2);
			CHECK_GT(bs2.xxxx, is1);
			CHECK_GE(is3.xxxx, bs1);
			CHECK_LE(is1.xxxx, bs1);
		}
	}

	TEST_CASE("matrix <=> comparisons")
	{
		constexpr auto m2x2 = mat2x2(fvec2(1, 1), fvec2(1, 1));
		constexpr auto m2x3 = mat2x3(fvec2(2, 2), fvec2(2, 2), fvec2(2, 2));
		constexpr auto m2x4 = mat2x4(fvec2(3, 3), fvec2(3, 3), fvec2(3, 3), fvec2(3, 3));
		constexpr auto m3x2 = mat3x2(fvec3(4, 4, 4), fvec3(4, 4, 4));
		constexpr auto m3x3 = mat3x3(fvec3(5, 5, 5), fvec3(5, 5, 5), fvec3(5, 5, 5));
		constexpr auto m3x4 = mat3x4(fvec3(6, 6, 6), fvec3(6, 6, 6), fvec3(6, 6, 6), fvec3(6, 6, 6));
		constexpr auto m4x2 = mat4x2(fvec4(7, 7, 7, 7), fvec4(7, 7, 7, 7));
		constexpr auto m4x3 = mat4x3(fvec4(8, 8, 8, 8), fvec4(8, 8, 8, 8), fvec4(8, 8, 8, 8));
		constexpr auto m4x4 = mat4x4(fvec4(9, 9, 9, 9), fvec4(9, 9, 9, 9), fvec4(9, 9, 9, 9), fvec4(9, 9, 9, 9));

		SUBCASE("mat2x2 <=> comparisons")
		{
			auto mat = mat2x2(m4x4);
			CHECK_LT(m2x2, mat);
			CHECK_LE(m2x2, mat);
			CHECK_GT(mat, m2x2);
			CHECK_GE(mat, m2x2);
		}

		SUBCASE("mat2x3 <=> comparisons")
		{
			auto mat = mat2x3(m4x4);
			CHECK_LT(m2x3, mat);
			CHECK_LE(m2x3, mat);
			CHECK_GT(mat, m2x3);
			CHECK_GE(mat, m2x3);
		}

		SUBCASE("mat2x4 <=> comparisons")
		{
			auto mat = mat2x4(m4x4);
			CHECK_LT(m2x4, mat);
			CHECK_LE(m2x4, mat);
			CHECK_GT(mat, m2x4);
			CHECK_GE(mat, m2x4);
		}

		SUBCASE("mat3x2 <=> comparisons")
		{
			auto mat = mat3x2(m4x4);
			CHECK_LT(m3x2, mat);
			CHECK_LE(m3x2, mat);
			CHECK_GT(mat, m3x2);
			CHECK_GE(mat, m3x2);
		}

		SUBCASE("mat3x3 <=> comparisons")
		{
			auto mat = mat3x3(m4x4);
			CHECK_LT(m3x3, mat);
			CHECK_LE(m3x3, mat);
			CHECK_GT(mat, m3x3);
			CHECK_GE(mat, m3x3);
		}

		SUBCASE("mat3x4 <=> comparisons")
		{
			auto mat = mat3x4(m4x4);
			CHECK_LT(m3x4, mat);
			CHECK_LE(m3x4, mat);
			CHECK_GT(mat, m3x4);
			CHECK_GE(mat, m3x4);
		}

		SUBCASE("mat4x2 <=> comparisons")
		{
			auto mat = mat4x2(m4x4);
			CHECK_LT(m4x2, mat);
			CHECK_LE(m4x2, mat);
			CHECK_GT(mat, m4x2);
			CHECK_GE(mat, m4x2);
		}

		SUBCASE("mat4x3 <=> comparisons")
		{
			auto mat = mat4x3(m4x4);
			CHECK_LT(m4x3, mat);
			CHECK_LE(m4x3, mat);
			CHECK_GT(mat, m4x3);
			CHECK_GE(mat, m4x3);
		}

		SUBCASE("mat4x4 <=> comparisons")
		{
			auto mat = mat4x4(m2x2);
			CHECK_LT(mat, m4x4);
			CHECK_LE(mat, m4x4);
			CHECK_GT(m4x4, mat);
			CHECK_GE(m4x4, mat);
		}
	}
}

TEST_SUITE("weighted_compare() comparisons")
{
	TEST_CASE("weighted_compare() with basic_vector and indexed_vector")
	{
		constexpr auto d1 = dsga::dvec4(12, 0, 14, 2);
		constexpr auto d2 = dsga::dvec4(9, 13, 7, 15);

		SUBCASE("1D weighted_compare()")
		{
			constexpr auto weights = default_comparison_weights<1>();
			constexpr auto s1 = dscal(1);
			constexpr auto s2 = dscal(-1);

			// indexed_vector
			CHECK_EQ(weighted_compare(d1.x, d2.x, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.x, d2.x, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.x, d2.x, iscal(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.x, d2.x, weights.x), std::partial_ordering::greater);

			// basic_vector
			CHECK_EQ(weighted_compare(s1, s2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(s1, s2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(s1, s2, iscal(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(s1, s2, weights.x), std::partial_ordering::greater);

			// mix of the two
			CHECK_EQ(weighted_compare(d1.x, s2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.x, s2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.x, s2, iscal(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.x, s2, weights.x), std::partial_ordering::greater);
		}

		SUBCASE("2D weighted_compare()")
		{
			constexpr auto weights = default_comparison_weights<2>();
			constexpr auto v1 = dvec2(1);
			constexpr auto v2 = dvec2(-1);

			// indexed_vector
			CHECK_EQ(weighted_compare(d1.xy, d2.xy, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.xy, d2.xy, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.xy, d2.xy, ivec2(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.xy, d2.xy, weights.xy), std::partial_ordering::greater);

			// basic_vector
			CHECK_EQ(weighted_compare(v1, v2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(v1, v2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(v1, v2, ivec2(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(v1, v2, weights.xy), std::partial_ordering::greater);

			// mix of the two
			CHECK_EQ(weighted_compare(d1.xy, v2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.xy, v2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.xy, v2, ivec2(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.xy, v2, weights.xy), std::partial_ordering::greater);
		}

		SUBCASE("3D weighted_compare()")
		{
			constexpr auto weights = default_comparison_weights<3>();
			constexpr auto v1 = dvec3(1);
			constexpr auto v2 = dvec3(-1);

			// indexed_vector
			CHECK_EQ(weighted_compare(d1.xyz, d2.xyz, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.xyz, d2.xyz, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.xyz, d2.xyz, ivec3(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.xyz, d2.xyz, weights.xyz), std::partial_ordering::greater);

			// basic_vector
			CHECK_EQ(weighted_compare(v1, v2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(v1, v2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(v1, v2, ivec3(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(v1, v2, weights.xyz), std::partial_ordering::greater);

			// mix of the two
			CHECK_EQ(weighted_compare(d1.xyz, v2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.xyz, v2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.xyz, v2, ivec3(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.xyz, v2, weights.xyz), std::partial_ordering::greater);
		}

		SUBCASE("4D weighted_compare()")
		{
			constexpr auto weights = default_comparison_weights<4>();
			constexpr auto v1 = dvec4(1);
			constexpr auto v2 = dvec4(-1);

			// indexed_vector
			CHECK_EQ(weighted_compare(d1.xyzw, d2.xyzw, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.xyzw, d2.xyzw, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.xyzw, d2.xyzw, ivec4(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.xyzw, d2.xyzw, weights.xyzw), std::partial_ordering::greater);

			// basic_vector
			CHECK_EQ(weighted_compare(v1, v2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(v1, v2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(v1, v2, ivec4(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(v1, v2, weights.xyzw), std::partial_ordering::greater);

			// mix of the two
			CHECK_EQ(weighted_compare(d1.xyzw, v2, weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1.xyzw, v2, -weights), std::partial_ordering::less);
			CHECK_EQ(weighted_compare(d1.xyzw, v2, ivec4(0)), std::partial_ordering::equivalent);
			CHECK_EQ(weighted_compare(d1.xyzw, v2, weights.xyzw), std::partial_ordering::greater);
		}

		SUBCASE("weighted_compare exploration")
		{
			// ascending x, ascending y, ascending z, ascending w
			auto lexicographical_weights = default_comparison_weights<4>();

			// ascending y, ascending w, ascending x, ascending z
			//
			// it might look unusual for that variable name, but we are swizzling so that the
			// default z weight is used for x, the default x weight is used for y, the default
			// w weight is used for z, and the default y weight is used for w. that gives the
			// dimensional ordering of ascending y, ascending w, ascending x, ascending z.
			auto ywxz_weights = lexicographical_weights.zxwy;

			CHECK_EQ(weighted_compare(d1, d2, lexicographical_weights), std::partial_ordering::greater);
			CHECK_EQ(weighted_compare(d1, d2, ywxz_weights), std::partial_ordering::less);
		}
	}
}
