
//          Copyright David Browne 2020-2022.
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

TEST_SUITE("test swizzling access")
{
	static ivec4	four(0, 1, 2, 3);
	static ivec3	three(4, 5, 6);
	static ivec2	two(7, 8);
	static iscal	one(9);

	TEST_CASE("access through all swizzles for vecs sizes 1-2 (in xyzw space) - checking for typos and index matching")
	{
		SUBCASE("1D swizzling")
		{
			// 0				x				// legal lvalue
			// 1				y				// legal lvalue
			// 2				z				// legal lvalue
			// 3				w				// legal lvalue

			CHECK_EQ(four.x, 0);
			CHECK_EQ(three.x, 4);
			CHECK_EQ(two.x, 7);
			CHECK_EQ(one.x, 9);

			CHECK_EQ(four.y, 1);
			CHECK_EQ(three.y, 5);
			CHECK_EQ(two.y, 8);

			CHECK_EQ(four.z, 2);
			CHECK_EQ(three.z, 6);

			CHECK_EQ(four.w, 3);
		}

		SUBCASE("2D swizzling")
		{
			// 0 0				xx
			// 0 1				xy				// legal lvalue
			// 0 2				xz				// legal lvalue
			// 0 3				xw				// legal lvalue
			// 1 0				yx				// legal lvalue
			// 1 1				yy
			// 1 2				yz				// legal lvalue
			// 1 3				yw				// legal lvalue
			// 2 0				zx				// legal lvalue
			// 2 1				zy				// legal lvalue
			// 2 2				zz
			// 2 3				zw				// legal lvalue
			// 3 0				wx				// legal lvalue
			// 3 1				wy				// legal lvalue
			// 3 2				wz				// legal lvalue
			// 3 3				ww

			CHECK_EQ(four.xx, ivec2(0, 0));
			CHECK_EQ(three.xx, ivec2(4, 4));
			CHECK_EQ(two.xx, ivec2(7, 7));
			CHECK_EQ(one.xx, ivec2(9, 9));

			CHECK_EQ(four.xy, ivec2(0, 1));
			CHECK_EQ(three.xy, ivec2(4, 5));
			CHECK_EQ(two.xy, ivec2(7, 8));

			CHECK_EQ(four.xz, ivec2(0, 2));
			CHECK_EQ(three.xz, ivec2(4, 6));

			CHECK_EQ(four.xw, ivec2(0, 3));

			CHECK_EQ(four.yx, ivec2(1, 0));
			CHECK_EQ(three.yx, ivec2(5, 4));
			CHECK_EQ(two.yx, ivec2(8, 7));

			CHECK_EQ(four.yy, ivec2(1, 1));
			CHECK_EQ(three.yy, ivec2(5, 5));
			CHECK_EQ(two.yy, ivec2(8, 8));

			CHECK_EQ(four.yz, ivec2(1, 2));
			CHECK_EQ(three.yz, ivec2(5, 6));

			CHECK_EQ(four.yw, ivec2(1, 3));

			CHECK_EQ(four.zx, ivec2(2, 0));
			CHECK_EQ(three.zx, ivec2(6, 4));

			CHECK_EQ(four.zy, ivec2(2, 1));
			CHECK_EQ(three.zy, ivec2(6, 5));

			CHECK_EQ(four.zz, ivec2(2, 2));
			CHECK_EQ(three.zz, ivec2(6, 6));

			CHECK_EQ(four.zw, ivec2(2, 3));

			CHECK_EQ(four.wx, ivec2(3, 0));

			CHECK_EQ(four.wy, ivec2(3, 1));

			CHECK_EQ(four.wz, ivec2(3, 2));

			CHECK_EQ(four.ww, ivec2(3, 3));
		}
	}

	TEST_CASE("access through all swizzles for vecs length 3 (in xyzw space) - checking for typos and index matching")
	{
		SUBCASE("3D swizzling")
		{
			// 0 0 0			xxx
			// 0 0 1			xxy
			// 0 0 2			xxz
			// 0 0 3			xxw
			// 0 1 0			xyx
			// 0 1 1			xyy
			// 0 1 2			xyz				// legal lvalue
			// 0 1 3			xyw				// legal lvalue
			// 0 2 0			xzx
			// 0 2 1			xzy				// legal lvalue
			// 0 2 2			xzz
			// 0 2 3			xzw				// legal lvalue
			// 0 3 0			xwx
			// 0 3 1			xwy				// legal lvalue
			// 0 3 2			xwz				// legal lvalue
			// 0 3 3			xww
			// 1 0 0			yxx
			// 1 0 1			yxy
			// 1 0 2			yxz				// legal lvalue
			// 1 0 3			yxw				// legal lvalue
			// 1 1 0			yyx
			// 1 1 1			yyy
			// 1 1 2			yyz
			// 1 1 3			yyw
			// 1 2 0			yzx				// legal lvalue
			// 1 2 1			yzy
			// 1 2 2			yzz
			// 1 2 3			yzw				// legal lvalue
			// 1 3 0			ywx				// legal lvalue
			// 1 3 1			ywy
			// 1 3 2			ywz				// legal lvalue
			// 1 3 3			yww
			// 2 0 0			zxx
			// 2 0 1			zxy				// legal lvalue
			// 2 0 2			zxz
			// 2 0 3			zxw				// legal lvalue
			// 2 1 0			zyx				// legal lvalue
			// 2 1 1			zyy
			// 2 1 2			zyz
			// 2 1 3			zyw				// legal lvalue
			// 2 2 0			zzx
			// 2 2 1			zzy
			// 2 2 2			zzz
			// 2 2 3			zzw
			// 2 3 0			zwx				// legal lvalue
			// 2 3 1			zwy				// legal lvalue
			// 2 3 2			zwz
			// 2 3 3			zww
			// 3 0 0			wxx
			// 3 0 1			wxy				// legal lvalue
			// 3 0 2			wxz				// legal lvalue
			// 3 0 3			wxw
			// 3 1 0			wyx				// legal lvalue
			// 3 1 1			wyy
			// 3 1 2			wyz				// legal lvalue
			// 3 1 3			wyw
			// 3 2 0			wzx				// legal lvalue
			// 3 2 1			wzy				// legal lvalue
			// 3 2 2			wzz
			// 3 2 3			wzw
			// 3 3 0			wwx
			// 3 3 1			wwy
			// 3 3 2			wwz
			// 3 3 3			www

			CHECK_EQ(four.xxx, ivec3(0, 0, 0));
			CHECK_EQ(three.xxx, ivec3(4, 4, 4));
			CHECK_EQ(two.xxx, ivec3(7, 7, 7));
			CHECK_EQ(one.xxx, ivec3(9, 9, 9));

			CHECK_EQ(four.xxy, ivec3(0, 0, 1));
			CHECK_EQ(three.xxy, ivec3(4, 4, 5));
			CHECK_EQ(two.xxy, ivec3(7, 7, 8));

			CHECK_EQ(four.xxz, ivec3(0, 0, 2));
			CHECK_EQ(three.xxz, ivec3(4, 4, 6));

			CHECK_EQ(four.xxw, ivec3(0, 0, 3));

			CHECK_EQ(four.xyx, ivec3(0, 1, 0));
			CHECK_EQ(three.xyx, ivec3(4, 5, 4));
			CHECK_EQ(two.xyx, ivec3(7, 8, 7));

			CHECK_EQ(four.xyy, ivec3(0, 1, 1));
			CHECK_EQ(three.xyy, ivec3(4, 5, 5));
			CHECK_EQ(two.xyy, ivec3(7, 8, 8));

			CHECK_EQ(four.xyz, ivec3(0, 1, 2));
			CHECK_EQ(three.xyz, ivec3(4, 5, 6));

			CHECK_EQ(four.xyw, ivec3(0, 1, 3));

			CHECK_EQ(four.xzx, ivec3(0, 2, 0));
			CHECK_EQ(three.xzx, ivec3(4, 6, 4));

			CHECK_EQ(four.xzy, ivec3(0, 2, 1));
			CHECK_EQ(three.xzy, ivec3(4, 6, 5));

			CHECK_EQ(four.xzz, ivec3(0, 2, 2));
			CHECK_EQ(three.xzz, ivec3(4, 6, 6));

			CHECK_EQ(four.xzw, ivec3(0, 2, 3));

			CHECK_EQ(four.xwx, ivec3(0, 3, 0));

			CHECK_EQ(four.xwy, ivec3(0, 3, 1));

			CHECK_EQ(four.xwz, ivec3(0, 3, 2));

			CHECK_EQ(four.xww, ivec3(0, 3, 3));

			CHECK_EQ(four.yxx, ivec3(1, 0, 0));
			CHECK_EQ(three.yxx, ivec3(5, 4, 4));
			CHECK_EQ(two.yxx, ivec3(8, 7, 7));

			CHECK_EQ(four.yxy, ivec3(1, 0, 1));
			CHECK_EQ(three.yxy, ivec3(5, 4, 5));
			CHECK_EQ(two.yxy, ivec3(8, 7, 8));

			CHECK_EQ(four.yxz, ivec3(1, 0, 2));
			CHECK_EQ(three.yxz, ivec3(5, 4, 6));

			CHECK_EQ(four.yxw, ivec3(1, 0, 3));

			CHECK_EQ(four.yyx, ivec3(1, 1, 0));
			CHECK_EQ(three.yyx, ivec3(5, 5, 4));
			CHECK_EQ(two.yyx, ivec3(8, 8, 7));

			CHECK_EQ(four.yyy, ivec3(1, 1, 1));
			CHECK_EQ(three.yyy, ivec3(5, 5, 5));
			CHECK_EQ(two.yyy, ivec3(8, 8, 8));

			CHECK_EQ(four.yyz, ivec3(1, 1, 2));
			CHECK_EQ(three.yyz, ivec3(5, 5, 6));

			CHECK_EQ(four.yyw, ivec3(1, 1, 3));

			CHECK_EQ(four.yzx, ivec3(1, 2, 0));
			CHECK_EQ(three.yzx, ivec3(5, 6, 4));

			CHECK_EQ(four.yzy, ivec3(1, 2, 1));
			CHECK_EQ(three.yzy, ivec3(5, 6, 5));

			CHECK_EQ(four.yzz, ivec3(1, 2, 2));
			CHECK_EQ(three.yzz, ivec3(5, 6, 6));

			CHECK_EQ(four.yzw, ivec3(1, 2, 3));

			CHECK_EQ(four.ywx, ivec3(1, 3, 0));

			CHECK_EQ(four.ywy, ivec3(1, 3, 1));

			CHECK_EQ(four.ywz, ivec3(1, 3, 2));

			CHECK_EQ(four.yww, ivec3(1, 3, 3));

			CHECK_EQ(four.zxx, ivec3(2, 0, 0));
			CHECK_EQ(three.zxx, ivec3(6, 4, 4));

			CHECK_EQ(four.zxy, ivec3(2, 0, 1));
			CHECK_EQ(three.zxy, ivec3(6, 4, 5));

			CHECK_EQ(four.zxz, ivec3(2, 0, 2));
			CHECK_EQ(three.zxz, ivec3(6, 4, 6));

			CHECK_EQ(four.zxw, ivec3(2, 0, 3));

			CHECK_EQ(four.zyx, ivec3(2, 1, 0));
			CHECK_EQ(three.zyx, ivec3(6, 5, 4));

			CHECK_EQ(four.zyy, ivec3(2, 1, 1));
			CHECK_EQ(three.zyy, ivec3(6, 5, 5));

			CHECK_EQ(four.zyz, ivec3(2, 1, 2));
			CHECK_EQ(three.zyz, ivec3(6, 5, 6));

			CHECK_EQ(four.zyw, ivec3(2, 1, 3));

			CHECK_EQ(four.zzx, ivec3(2, 2, 0));
			CHECK_EQ(three.zzx, ivec3(6, 6, 4));

			CHECK_EQ(four.zzy, ivec3(2, 2, 1));
			CHECK_EQ(three.zzy, ivec3(6, 6, 5));

			CHECK_EQ(four.zzz, ivec3(2, 2, 2));
			CHECK_EQ(three.zzz, ivec3(6, 6, 6));

			CHECK_EQ(four.zzw, ivec3(2, 2, 3));

			CHECK_EQ(four.zwx, ivec3(2, 3, 0));

			CHECK_EQ(four.zwy, ivec3(2, 3, 1));

			CHECK_EQ(four.zwz, ivec3(2, 3, 2));

			CHECK_EQ(four.zww, ivec3(2, 3, 3));

			CHECK_EQ(four.wxx, ivec3(3, 0, 0));

			CHECK_EQ(four.wxy, ivec3(3, 0, 1));

			CHECK_EQ(four.wxz, ivec3(3, 0, 2));

			CHECK_EQ(four.wxw, ivec3(3, 0, 3));

			CHECK_EQ(four.wyx, ivec3(3, 1, 0));

			CHECK_EQ(four.wyy, ivec3(3, 1, 1));

			CHECK_EQ(four.wyz, ivec3(3, 1, 2));

			CHECK_EQ(four.wyw, ivec3(3, 1, 3));

			CHECK_EQ(four.wzx, ivec3(3, 2, 0));

			CHECK_EQ(four.wzy, ivec3(3, 2, 1));

			CHECK_EQ(four.wzz, ivec3(3, 2, 2));

			CHECK_EQ(four.wzw, ivec3(3, 2, 3));

			CHECK_EQ(four.wwx, ivec3(3, 3, 0));

			CHECK_EQ(four.wwy, ivec3(3, 3, 1));

			CHECK_EQ(four.wwz, ivec3(3, 3, 2));

			CHECK_EQ(four.www, ivec3(3, 3, 3));
		}
	}
}

TEST_SUITE("test 4D swizzling access through all swizzles for vecs length 4 (in xyzw space) - checking for typos and index matching")
{
	static ivec4	four(0, 1, 2, 3);
	static ivec3	three(4, 5, 6);
	static ivec2	two(7, 8);
	static iscal	one(9);

	TEST_CASE("first index is x")
	{
		// 0 0 0 0			xxxx
		// 0 0 0 1			xxxy
		// 0 0 0 2			xxxz
		// 0 0 0 3			xxxw
		// 0 0 1 0			xxyx
		// 0 0 1 1			xxyy
		// 0 0 1 2			xxyz
		// 0 0 1 3			xxyw
		// 0 0 2 0			xxzx
		// 0 0 2 1			xxzy
		// 0 0 2 2			xxzz
		// 0 0 2 3			xxzw
		// 0 0 3 0			xxwx
		// 0 0 3 1			xxwy
		// 0 0 3 2			xxwz
		// 0 0 3 3			xxww
		// 0 1 0 0			xyxx
		// 0 1 0 1			xyxy
		// 0 1 0 2			xyxz
		// 0 1 0 3			xyxw
		// 0 1 1 0			xyyx
		// 0 1 1 1			xyyy
		// 0 1 1 2			xyyz
		// 0 1 1 3			xyyw
		// 0 1 2 0			xyzx
		// 0 1 2 1			xyzy
		// 0 1 2 2			xyzz
		// 0 1 2 3			xyzw			// legal lvalue
		// 0 1 3 0			xywx
		// 0 1 3 1			xywy
		// 0 1 3 2			xywz			// legal lvalue
		// 0 1 3 3			xyww
		// 0 2 0 0			xzxx
		// 0 2 0 1			xzxy
		// 0 2 0 2			xzxz
		// 0 2 0 3			xzxw
		// 0 2 1 0			xzyx
		// 0 2 1 1			xzyy
		// 0 2 1 2			xzyz
		// 0 2 1 3			xzyw			// legal lvalue
		// 0 2 2 0			xzzx
		// 0 2 2 1			xzzy
		// 0 2 2 2			xzzz
		// 0 2 2 3			xzzw
		// 0 2 3 0			xzwx
		// 0 2 3 1			xzwy			// legal lvalue
		// 0 2 3 2			xzwz
		// 0 2 3 3			xzww
		// 0 3 0 0			xwxx
		// 0 3 0 1			xwxy
		// 0 3 0 2			xwxz
		// 0 3 0 3			xwxw
		// 0 3 1 0			xwyx
		// 0 3 1 1			xwyy
		// 0 3 1 2			xwyz			// legal lvalue
		// 0 3 1 3			xwyw
		// 0 3 2 0			xwzx
		// 0 3 2 1			xwzy			// legal lvalue
		// 0 3 2 2			xwzz
		// 0 3 2 3			xwzw
		// 0 3 3 0			xwwx
		// 0 3 3 1			xwwy
		// 0 3 3 2			xwwz
		// 0 3 3 3			xwww

		CHECK_EQ(four.xxxx, ivec4(0, 0, 0, 0));
		CHECK_EQ(three.xxxx, ivec4(4, 4, 4, 4));
		CHECK_EQ(two.xxxx, ivec4(7, 7, 7, 7));
		CHECK_EQ(one.xxxx, ivec4(9, 9, 9, 9));

		CHECK_EQ(four.xxxy, ivec4(0, 0, 0, 1));
		CHECK_EQ(three.xxxy, ivec4(4, 4, 4, 5));
		CHECK_EQ(two.xxxy, ivec4(7, 7, 7, 8));

		CHECK_EQ(four.xxxz, ivec4(0, 0, 0, 2));
		CHECK_EQ(three.xxxz, ivec4(4, 4, 4, 6));

		CHECK_EQ(four.xxxw, ivec4(0, 0, 0, 3));

		CHECK_EQ(four.xxyx, ivec4(0, 0, 1, 0));
		CHECK_EQ(three.xxyx, ivec4(4, 4, 5, 4));
		CHECK_EQ(two.xxyx, ivec4(7, 7, 8, 7));

		CHECK_EQ(four.xxyy, ivec4(0, 0, 1, 1));
		CHECK_EQ(three.xxyy, ivec4(4, 4, 5, 5));
		CHECK_EQ(two.xxyy, ivec4(7, 7, 8, 8));

		CHECK_EQ(four.xxyz, ivec4(0, 0, 1, 2));
		CHECK_EQ(three.xxyz, ivec4(4, 4, 5, 6));

		CHECK_EQ(four.xxyw, ivec4(0, 0, 1, 3));

		CHECK_EQ(four.xxzx, ivec4(0, 0, 2, 0));
		CHECK_EQ(three.xxzx, ivec4(4, 4, 6, 4));

		CHECK_EQ(four.xxzy, ivec4(0, 0, 2, 1));
		CHECK_EQ(three.xxzy, ivec4(4, 4, 6, 5));

		CHECK_EQ(four.xxzz, ivec4(0, 0, 2, 2));
		CHECK_EQ(three.xxzz, ivec4(4, 4, 6, 6));

		CHECK_EQ(four.xxzw, ivec4(0, 0, 2, 3));

		CHECK_EQ(four.xxwx, ivec4(0, 0, 3, 0));

		CHECK_EQ(four.xxwy, ivec4(0, 0, 3, 1));

		CHECK_EQ(four.xxwz, ivec4(0, 0, 3, 2));

		CHECK_EQ(four.xxww, ivec4(0, 0, 3, 3));

		CHECK_EQ(four.xyxx, ivec4(0, 1, 0, 0));
		CHECK_EQ(three.xyxx, ivec4(4, 5, 4, 4));
		CHECK_EQ(two.xyxx, ivec4(7, 8, 7, 7));

		CHECK_EQ(four.xyxy, ivec4(0, 1, 0, 1));
		CHECK_EQ(three.xyxy, ivec4(4, 5, 4, 5));
		CHECK_EQ(two.xyxy, ivec4(7, 8, 7, 8));

		CHECK_EQ(four.xyxz, ivec4(0, 1, 0, 2));
		CHECK_EQ(three.xyxz, ivec4(4, 5, 4, 6));

		CHECK_EQ(four.xyxw, ivec4(0, 1, 0, 3));

		CHECK_EQ(four.xyyx, ivec4(0, 1, 1, 0));
		CHECK_EQ(three.xyyx, ivec4(4, 5, 5, 4));
		CHECK_EQ(two.xyyx, ivec4(7, 8, 8, 7));

		CHECK_EQ(four.xyyy, ivec4(0, 1, 1, 1));
		CHECK_EQ(three.xyyy, ivec4(4, 5, 5, 5));
		CHECK_EQ(two.xyyy, ivec4(7, 8, 8, 8));

		CHECK_EQ(four.xyyz, ivec4(0, 1, 1, 2));
		CHECK_EQ(three.xyyz, ivec4(4, 5, 5, 6));

		CHECK_EQ(four.xyyw, ivec4(0, 1, 1, 3));

		CHECK_EQ(four.xyzx, ivec4(0, 1, 2, 0));
		CHECK_EQ(three.xyzx, ivec4(4, 5, 6, 4));

		CHECK_EQ(four.xyzy, ivec4(0, 1, 2, 1));
		CHECK_EQ(three.xyzy, ivec4(4, 5, 6, 5));

		CHECK_EQ(four.xyzz, ivec4(0, 1, 2, 2));
		CHECK_EQ(three.xyzz, ivec4(4, 5, 6, 6));

		CHECK_EQ(four.xyzw, ivec4(0, 1, 2, 3));

		CHECK_EQ(four.xywx, ivec4(0, 1, 3, 0));

		CHECK_EQ(four.xywy, ivec4(0, 1, 3, 1));

		CHECK_EQ(four.xywz, ivec4(0, 1, 3, 2));

		CHECK_EQ(four.xyww, ivec4(0, 1, 3, 3));

		CHECK_EQ(four.xzxx, ivec4(0, 2, 0, 0));
		CHECK_EQ(three.xzxx, ivec4(4, 6, 4, 4));

		CHECK_EQ(four.xzxy, ivec4(0, 2, 0, 1));
		CHECK_EQ(three.xzxy, ivec4(4, 6, 4, 5));

		CHECK_EQ(four.xzxz, ivec4(0, 2, 0, 2));
		CHECK_EQ(three.xzxz, ivec4(4, 6, 4, 6));

		CHECK_EQ(four.xzxw, ivec4(0, 2, 0, 3));

		CHECK_EQ(four.xzyx, ivec4(0, 2, 1, 0));
		CHECK_EQ(three.xzyx, ivec4(4, 6, 5, 4));

		CHECK_EQ(four.xzyy, ivec4(0, 2, 1, 1));
		CHECK_EQ(three.xzyy, ivec4(4, 6, 5, 5));

		CHECK_EQ(four.xzyz, ivec4(0, 2, 1, 2));
		CHECK_EQ(three.xzyz, ivec4(4, 6, 5, 6));

		CHECK_EQ(four.xzyw, ivec4(0, 2, 1, 3));

		CHECK_EQ(four.xzzx, ivec4(0, 2, 2, 0));
		CHECK_EQ(three.xzzx, ivec4(4, 6, 6, 4));

		CHECK_EQ(four.xzzy, ivec4(0, 2, 2, 1));
		CHECK_EQ(three.xzzy, ivec4(4, 6, 6, 5));

		CHECK_EQ(four.xzzz, ivec4(0, 2, 2, 2));
		CHECK_EQ(three.xzzz, ivec4(4, 6, 6, 6));

		CHECK_EQ(four.xzzw, ivec4(0, 2, 2, 3));

		CHECK_EQ(four.xzwx, ivec4(0, 2, 3, 0));

		CHECK_EQ(four.xzwy, ivec4(0, 2, 3, 1));

		CHECK_EQ(four.xzwz, ivec4(0, 2, 3, 2));

		CHECK_EQ(four.xzww, ivec4(0, 2, 3, 3));

		CHECK_EQ(four.xwxx, ivec4(0, 3, 0, 0));

		CHECK_EQ(four.xwxy, ivec4(0, 3, 0, 1));

		CHECK_EQ(four.xwxz, ivec4(0, 3, 0, 2));

		CHECK_EQ(four.xwxw, ivec4(0, 3, 0, 3));

		CHECK_EQ(four.xwyx, ivec4(0, 3, 1, 0));

		CHECK_EQ(four.xwyy, ivec4(0, 3, 1, 1));

		CHECK_EQ(four.xwyz, ivec4(0, 3, 1, 2));

		CHECK_EQ(four.xwyw, ivec4(0, 3, 1, 3));

		CHECK_EQ(four.xwzx, ivec4(0, 3, 2, 0));

		CHECK_EQ(four.xwzy, ivec4(0, 3, 2, 1));

		CHECK_EQ(four.xwzz, ivec4(0, 3, 2, 2));

		CHECK_EQ(four.xwzw, ivec4(0, 3, 2, 3));

		CHECK_EQ(four.xwwx, ivec4(0, 3, 3, 0));

		CHECK_EQ(four.xwwy, ivec4(0, 3, 3, 1));

		CHECK_EQ(four.xwwz, ivec4(0, 3, 3, 2));

		CHECK_EQ(four.xwww, ivec4(0, 3, 3, 3));
	}

	TEST_CASE("first index is y")
	{
		// 1 0 0 0			yxxx
		// 1 0 0 1			yxxy
		// 1 0 0 2			yxxz
		// 1 0 0 3			yxxw
		// 1 0 1 0			yxyx
		// 1 0 1 1			yxyy
		// 1 0 1 2			yxyz
		// 1 0 1 3			yxyw
		// 1 0 2 0			yxzx
		// 1 0 2 1			yxzy
		// 1 0 2 2			yxzz
		// 1 0 2 3			yxzw			// legal lvalue
		// 1 0 3 0			yxwx
		// 1 0 3 1			yxwy
		// 1 0 3 2			yxwz			// legal lvalue
		// 1 0 3 3			yxww
		// 1 1 0 0			yyxx
		// 1 1 0 1			yyxy
		// 1 1 0 2			yyxz
		// 1 1 0 3			yyxw
		// 1 1 1 0			yyyx
		// 1 1 1 1			yyyy
		// 1 1 1 2			yyyz
		// 1 1 1 3			yyyw
		// 1 1 2 0			yyzx
		// 1 1 2 1			yyzy
		// 1 1 2 2			yyzz
		// 1 1 2 3			yyzw
		// 1 1 3 0			yywx
		// 1 1 3 1			yywy
		// 1 1 3 2			yywz
		// 1 1 3 3			yyww
		// 1 2 0 0			yzxx
		// 1 2 0 1			yzxy
		// 1 2 0 2			yzxz
		// 1 2 0 3			yzxw			// legal lvalue
		// 1 2 1 0			yzyx
		// 1 2 1 1			yzyy
		// 1 2 1 2			yzyz
		// 1 2 1 3			yzyw
		// 1 2 2 0			yzzx
		// 1 2 2 1			yzzy
		// 1 2 2 2			yzzz
		// 1 2 2 3			yzzw
		// 1 2 3 0			yzwx			// legal lvalue
		// 1 2 3 1			yzwy
		// 1 2 3 2			yzwz
		// 1 2 3 3			yzww
		// 1 3 0 0			ywxx
		// 1 3 0 1			ywxy
		// 1 3 0 2			ywxz			// legal lvalue
		// 1 3 0 3			ywxw
		// 1 3 1 0			ywyx
		// 1 3 1 1			ywyy
		// 1 3 1 2			ywyz
		// 1 3 1 3			ywyw
		// 1 3 2 0			ywzx			// legal lvalue
		// 1 3 2 1			ywzy
		// 1 3 2 2			ywzz
		// 1 3 2 3			ywzw
		// 1 3 3 0			ywwx
		// 1 3 3 1			ywwy
		// 1 3 3 2			ywwz
		// 1 3 3 3			ywww

		CHECK_EQ(four.yxxx, ivec4(1, 0, 0, 0));
		CHECK_EQ(three.yxxx, ivec4(5, 4, 4, 4));
		CHECK_EQ(two.yxxx, ivec4(8, 7, 7, 7));

		CHECK_EQ(four.yxxy, ivec4(1, 0, 0, 1));
		CHECK_EQ(three.yxxy, ivec4(5, 4, 4, 5));
		CHECK_EQ(two.yxxy, ivec4(8, 7, 7, 8));

		CHECK_EQ(four.yxxz, ivec4(1, 0, 0, 2));
		CHECK_EQ(three.yxxz, ivec4(5, 4, 4, 6));

		CHECK_EQ(four.yxxw, ivec4(1, 0, 0, 3));

		CHECK_EQ(four.yxyx, ivec4(1, 0, 1, 0));
		CHECK_EQ(three.yxyx, ivec4(5, 4, 5, 4));
		CHECK_EQ(two.yxyx, ivec4(8, 7, 8, 7));

		CHECK_EQ(four.yxyy, ivec4(1, 0, 1, 1));
		CHECK_EQ(three.yxyy, ivec4(5, 4, 5, 5));
		CHECK_EQ(two.yxyy, ivec4(8, 7, 8, 8));

		CHECK_EQ(four.yxyz, ivec4(1, 0, 1, 2));
		CHECK_EQ(three.yxyz, ivec4(5, 4, 5, 6));

		CHECK_EQ(four.yxyw, ivec4(1, 0, 1, 3));

		CHECK_EQ(four.yxzx, ivec4(1, 0, 2, 0));
		CHECK_EQ(three.yxzx, ivec4(5, 4, 6, 4));

		CHECK_EQ(four.yxzy, ivec4(1, 0, 2, 1));
		CHECK_EQ(three.yxzy, ivec4(5, 4, 6, 5));

		CHECK_EQ(four.yxzz, ivec4(1, 0, 2, 2));
		CHECK_EQ(three.yxzz, ivec4(5, 4, 6, 6));

		CHECK_EQ(four.yxzw, ivec4(1, 0, 2, 3));

		CHECK_EQ(four.yxwx, ivec4(1, 0, 3, 0));

		CHECK_EQ(four.yxwy, ivec4(1, 0, 3, 1));

		CHECK_EQ(four.yxwz, ivec4(1, 0, 3, 2));

		CHECK_EQ(four.yxww, ivec4(1, 0, 3, 3));

		CHECK_EQ(four.yyxx, ivec4(1, 1, 0, 0));
		CHECK_EQ(three.yyxx, ivec4(5, 5, 4, 4));
		CHECK_EQ(two.yyxx, ivec4(8, 8, 7, 7));

		CHECK_EQ(four.yyxy, ivec4(1, 1, 0, 1));
		CHECK_EQ(three.yyxy, ivec4(5, 5, 4, 5));
		CHECK_EQ(two.yyxy, ivec4(8, 8, 7, 8));

		CHECK_EQ(four.yyxz, ivec4(1, 1, 0, 2));
		CHECK_EQ(three.yyxz, ivec4(5, 5, 4, 6));

		CHECK_EQ(four.yyxw, ivec4(1, 1, 0, 3));

		CHECK_EQ(four.yyyx, ivec4(1, 1, 1, 0));
		CHECK_EQ(three.yyyx, ivec4(5, 5, 5, 4));
		CHECK_EQ(two.yyyx, ivec4(8, 8, 8, 7));

		CHECK_EQ(four.yyyy, ivec4(1, 1, 1, 1));
		CHECK_EQ(three.yyyy, ivec4(5, 5, 5, 5));
		CHECK_EQ(two.yyyy, ivec4(8, 8, 8, 8));

		CHECK_EQ(four.yyyz, ivec4(1, 1, 1, 2));
		CHECK_EQ(three.yyyz, ivec4(5, 5, 5, 6));

		CHECK_EQ(four.yyyw, ivec4(1, 1, 1, 3));

		CHECK_EQ(four.yyzx, ivec4(1, 1, 2, 0));
		CHECK_EQ(three.yyzx, ivec4(5, 5, 6, 4));

		CHECK_EQ(four.yyzy, ivec4(1, 1, 2, 1));
		CHECK_EQ(three.yyzy, ivec4(5, 5, 6, 5));

		CHECK_EQ(four.yyzz, ivec4(1, 1, 2, 2));
		CHECK_EQ(three.yyzz, ivec4(5, 5, 6, 6));

		CHECK_EQ(four.yyzw, ivec4(1, 1, 2, 3));

		CHECK_EQ(four.yywx, ivec4(1, 1, 3, 0));

		CHECK_EQ(four.yywy, ivec4(1, 1, 3, 1));

		CHECK_EQ(four.yywz, ivec4(1, 1, 3, 2));

		CHECK_EQ(four.yyww, ivec4(1, 1, 3, 3));

		CHECK_EQ(four.yzxx, ivec4(1, 2, 0, 0));
		CHECK_EQ(three.yzxx, ivec4(5, 6, 4, 4));

		CHECK_EQ(four.yzxy, ivec4(1, 2, 0, 1));
		CHECK_EQ(three.yzxy, ivec4(5, 6, 4, 5));

		CHECK_EQ(four.yzxz, ivec4(1, 2, 0, 2));
		CHECK_EQ(three.yzxz, ivec4(5, 6, 4, 6));

		CHECK_EQ(four.yzxw, ivec4(1, 2, 0, 3));

		CHECK_EQ(four.yzyx, ivec4(1, 2, 1, 0));
		CHECK_EQ(three.yzyx, ivec4(5, 6, 5, 4));

		CHECK_EQ(four.yzyy, ivec4(1, 2, 1, 1));
		CHECK_EQ(three.yzyy, ivec4(5, 6, 5, 5));

		CHECK_EQ(four.yzyz, ivec4(1, 2, 1, 2));
		CHECK_EQ(three.yzyz, ivec4(5, 6, 5, 6));

		CHECK_EQ(four.yzyw, ivec4(1, 2, 1, 3));

		CHECK_EQ(four.yzzx, ivec4(1, 2, 2, 0));
		CHECK_EQ(three.yzzx, ivec4(5, 6, 6, 4));

		CHECK_EQ(four.yzzy, ivec4(1, 2, 2, 1));
		CHECK_EQ(three.yzzy, ivec4(5, 6, 6, 5));

		CHECK_EQ(four.yzzz, ivec4(1, 2, 2, 2));
		CHECK_EQ(three.yzzz, ivec4(5, 6, 6, 6));

		CHECK_EQ(four.yzzw, ivec4(1, 2, 2, 3));

		CHECK_EQ(four.yzwx, ivec4(1, 2, 3, 0));

		CHECK_EQ(four.yzwy, ivec4(1, 2, 3, 1));

		CHECK_EQ(four.yzwz, ivec4(1, 2, 3, 2));

		CHECK_EQ(four.yzww, ivec4(1, 2, 3, 3));

		CHECK_EQ(four.ywxx, ivec4(1, 3, 0, 0));

		CHECK_EQ(four.ywxy, ivec4(1, 3, 0, 1));

		CHECK_EQ(four.ywxz, ivec4(1, 3, 0, 2));

		CHECK_EQ(four.ywxw, ivec4(1, 3, 0, 3));

		CHECK_EQ(four.ywyx, ivec4(1, 3, 1, 0));

		CHECK_EQ(four.ywyy, ivec4(1, 3, 1, 1));

		CHECK_EQ(four.ywyz, ivec4(1, 3, 1, 2));

		CHECK_EQ(four.ywyw, ivec4(1, 3, 1, 3));

		CHECK_EQ(four.ywzx, ivec4(1, 3, 2, 0));

		CHECK_EQ(four.ywzy, ivec4(1, 3, 2, 1));

		CHECK_EQ(four.ywzz, ivec4(1, 3, 2, 2));

		CHECK_EQ(four.ywzw, ivec4(1, 3, 2, 3));

		CHECK_EQ(four.ywwx, ivec4(1, 3, 3, 0));

		CHECK_EQ(four.ywwy, ivec4(1, 3, 3, 1));

		CHECK_EQ(four.ywwz, ivec4(1, 3, 3, 2));

		CHECK_EQ(four.ywww, ivec4(1, 3, 3, 3));
	}

	TEST_CASE("first index is z")
	{
		// 2 0 0 0			zxxx
		// 2 0 0 1			zxxy
		// 2 0 0 2			zxxz
		// 2 0 0 3			zxxw
		// 2 0 1 0			zxyx
		// 2 0 1 1			zxyy
		// 2 0 1 2			zxyz
		// 2 0 1 3			zxyw			// legal lvalue
		// 2 0 2 0			zxzx
		// 2 0 2 1			zxzy
		// 2 0 2 2			zxzz
		// 2 0 2 3			zxzw
		// 2 0 3 0			zxwx
		// 2 0 3 1			zxwy			// legal lvalue
		// 2 0 3 2			zxwz
		// 2 0 3 3			zxww
		// 2 1 0 0			zyxx
		// 2 1 0 1			zyxy
		// 2 1 0 2			zyxz
		// 2 1 0 3			zyxw			// legal lvalue
		// 2 1 1 0			zyyx
		// 2 1 1 1			zyyy
		// 2 1 1 2			zyyz
		// 2 1 1 3			zyyw
		// 2 1 2 0			zyzx
		// 2 1 2 1			zyzy
		// 2 1 2 2			zyzz
		// 2 1 2 3			zyzw
		// 2 1 3 0			zywx			// legal lvalue
		// 2 1 3 1			zywy
		// 2 1 3 2			zywz
		// 2 1 3 3			zyww
		// 2 2 0 0			zzxx
		// 2 2 0 1			zzxy
		// 2 2 0 2			zzxz
		// 2 2 0 3			zzxw
		// 2 2 1 0			zzyx
		// 2 2 1 1			zzyy
		// 2 2 1 2			zzyz
		// 2 2 1 3			zzyw
		// 2 2 2 0			zzzx
		// 2 2 2 1			zzzy
		// 2 2 2 2			zzzz
		// 2 2 2 3			zzzw
		// 2 2 3 0			zzwx
		// 2 2 3 1			zzwy
		// 2 2 3 2			zzwz
		// 2 2 3 3			zzww
		// 2 3 0 0			zwxx
		// 2 3 0 1			zwxy			// legal lvalue
		// 2 3 0 2			zwxz
		// 2 3 0 3			zwxw
		// 2 3 1 0			zwyx			// legal lvalue
		// 2 3 1 1			zwyy
		// 2 3 1 2			zwyz
		// 2 3 1 3			zwyw
		// 2 3 2 0			zwzx
		// 2 3 2 1			zwzy
		// 2 3 2 2			zwzz
		// 2 3 2 3			zwzw
		// 2 3 3 0			zwwx
		// 2 3 3 1			zwwy
		// 2 3 3 2			zwwz
		// 2 3 3 3			zwww

		CHECK_EQ(four.zxxx, ivec4(2, 0, 0, 0));
		CHECK_EQ(three.zxxx, ivec4(6, 4, 4, 4));

		CHECK_EQ(four.zxxy, ivec4(2, 0, 0, 1));
		CHECK_EQ(three.zxxy, ivec4(6, 4, 4, 5));

		CHECK_EQ(four.zxxz, ivec4(2, 0, 0, 2));
		CHECK_EQ(three.zxxz, ivec4(6, 4, 4, 6));

		CHECK_EQ(four.zxxw, ivec4(2, 0, 0, 3));

		CHECK_EQ(four.zxyx, ivec4(2, 0, 1, 0));
		CHECK_EQ(three.zxyx, ivec4(6, 4, 5, 4));

		CHECK_EQ(four.zxyy, ivec4(2, 0, 1, 1));
		CHECK_EQ(three.zxyy, ivec4(6, 4, 5, 5));

		CHECK_EQ(four.zxyz, ivec4(2, 0, 1, 2));
		CHECK_EQ(three.zxyz, ivec4(6, 4, 5, 6));

		CHECK_EQ(four.zxyw, ivec4(2, 0, 1, 3));

		CHECK_EQ(four.zxzx, ivec4(2, 0, 2, 0));
		CHECK_EQ(three.zxzx, ivec4(6, 4, 6, 4));

		CHECK_EQ(four.zxzy, ivec4(2, 0, 2, 1));
		CHECK_EQ(three.zxzy, ivec4(6, 4, 6, 5));

		CHECK_EQ(four.zxzz, ivec4(2, 0, 2, 2));
		CHECK_EQ(three.zxzz, ivec4(6, 4, 6, 6));

		CHECK_EQ(four.zxzw, ivec4(2, 0, 2, 3));

		CHECK_EQ(four.zxwx, ivec4(2, 0, 3, 0));

		CHECK_EQ(four.zxwy, ivec4(2, 0, 3, 1));

		CHECK_EQ(four.zxwz, ivec4(2, 0, 3, 2));

		CHECK_EQ(four.zxww, ivec4(2, 0, 3, 3));

		CHECK_EQ(four.zyxx, ivec4(2, 1, 0, 0));
		CHECK_EQ(three.zyxx, ivec4(6, 5, 4, 4));

		CHECK_EQ(four.zyxy, ivec4(2, 1, 0, 1));
		CHECK_EQ(three.zyxy, ivec4(6, 5, 4, 5));

		CHECK_EQ(four.zyxz, ivec4(2, 1, 0, 2));
		CHECK_EQ(three.zyxz, ivec4(6, 5, 4, 6));

		CHECK_EQ(four.zyxw, ivec4(2, 1, 0, 3));

		CHECK_EQ(four.zyyx, ivec4(2, 1, 1, 0));
		CHECK_EQ(three.zyyx, ivec4(6, 5, 5, 4));

		CHECK_EQ(four.zyyy, ivec4(2, 1, 1, 1));
		CHECK_EQ(three.zyyy, ivec4(6, 5, 5, 5));

		CHECK_EQ(four.zyyz, ivec4(2, 1, 1, 2));
		CHECK_EQ(three.zyyz, ivec4(6, 5, 5, 6));

		CHECK_EQ(four.zyyw, ivec4(2, 1, 1, 3));

		CHECK_EQ(four.zyzx, ivec4(2, 1, 2, 0));
		CHECK_EQ(three.zyzx, ivec4(6, 5, 6, 4));

		CHECK_EQ(four.zyzy, ivec4(2, 1, 2, 1));
		CHECK_EQ(three.zyzy, ivec4(6, 5, 6, 5));

		CHECK_EQ(four.zyzz, ivec4(2, 1, 2, 2));
		CHECK_EQ(three.zyzz, ivec4(6, 5, 6, 6));

		CHECK_EQ(four.zyzw, ivec4(2, 1, 2, 3));

		CHECK_EQ(four.zywx, ivec4(2, 1, 3, 0));

		CHECK_EQ(four.zywy, ivec4(2, 1, 3, 1));

		CHECK_EQ(four.zywz, ivec4(2, 1, 3, 2));

		CHECK_EQ(four.zyww, ivec4(2, 1, 3, 3));

		CHECK_EQ(four.zzxx, ivec4(2, 2, 0, 0));
		CHECK_EQ(three.zzxx, ivec4(6, 6, 4, 4));

		CHECK_EQ(four.zzxy, ivec4(2, 2, 0, 1));
		CHECK_EQ(three.zzxy, ivec4(6, 6, 4, 5));

		CHECK_EQ(four.zzxz, ivec4(2, 2, 0, 2));
		CHECK_EQ(three.zzxz, ivec4(6, 6, 4, 6));

		CHECK_EQ(four.zzxw, ivec4(2, 2, 0, 3));

		CHECK_EQ(four.zzyx, ivec4(2, 2, 1, 0));
		CHECK_EQ(three.zzyx, ivec4(6, 6, 5, 4));

		CHECK_EQ(four.zzyy, ivec4(2, 2, 1, 1));
		CHECK_EQ(three.zzyy, ivec4(6, 6, 5, 5));

		CHECK_EQ(four.zzyz, ivec4(2, 2, 1, 2));
		CHECK_EQ(three.zzyz, ivec4(6, 6, 5, 6));

		CHECK_EQ(four.zzyw, ivec4(2, 2, 1, 3));

		CHECK_EQ(four.zzzx, ivec4(2, 2, 2, 0));
		CHECK_EQ(three.zzzx, ivec4(6, 6, 6, 4));

		CHECK_EQ(four.zzzy, ivec4(2, 2, 2, 1));
		CHECK_EQ(three.zzzy, ivec4(6, 6, 6, 5));

		CHECK_EQ(four.zzzz, ivec4(2, 2, 2, 2));
		CHECK_EQ(three.zzzz, ivec4(6, 6, 6, 6));

		CHECK_EQ(four.zzzw, ivec4(2, 2, 2, 3));

		CHECK_EQ(four.zzwx, ivec4(2, 2, 3, 0));

		CHECK_EQ(four.zzwy, ivec4(2, 2, 3, 1));

		CHECK_EQ(four.zzwz, ivec4(2, 2, 3, 2));

		CHECK_EQ(four.zzww, ivec4(2, 2, 3, 3));

		CHECK_EQ(four.zwxx, ivec4(2, 3, 0, 0));

		CHECK_EQ(four.zwxy, ivec4(2, 3, 0, 1));

		CHECK_EQ(four.zwxz, ivec4(2, 3, 0, 2));

		CHECK_EQ(four.zwxw, ivec4(2, 3, 0, 3));

		CHECK_EQ(four.zwyx, ivec4(2, 3, 1, 0));

		CHECK_EQ(four.zwyy, ivec4(2, 3, 1, 1));

		CHECK_EQ(four.zwyz, ivec4(2, 3, 1, 2));

		CHECK_EQ(four.zwyw, ivec4(2, 3, 1, 3));

		CHECK_EQ(four.zwzx, ivec4(2, 3, 2, 0));

		CHECK_EQ(four.zwzy, ivec4(2, 3, 2, 1));

		CHECK_EQ(four.zwzz, ivec4(2, 3, 2, 2));

		CHECK_EQ(four.zwzw, ivec4(2, 3, 2, 3));

		CHECK_EQ(four.zwwx, ivec4(2, 3, 3, 0));

		CHECK_EQ(four.zwwy, ivec4(2, 3, 3, 1));

		CHECK_EQ(four.zwwz, ivec4(2, 3, 3, 2));

		CHECK_EQ(four.zwww, ivec4(2, 3, 3, 3));
	}

	TEST_CASE("first index is w")
	{
		// 3 0 0 0			wxxx
		// 3 0 0 1			wxxy
		// 3 0 0 2			wxxz
		// 3 0 0 3			wxxw
		// 3 0 1 0			wxyx
		// 3 0 1 1			wxyy
		// 3 0 1 2			wxyz			// legal lvalue
		// 3 0 1 3			wxyw
		// 3 0 2 0			wxzx
		// 3 0 2 1			wxzy			// legal lvalue
		// 3 0 2 2			wxzz
		// 3 0 2 3			wxzw
		// 3 0 3 0			wxwx
		// 3 0 3 1			wxwy
		// 3 0 3 2			wxwz
		// 3 0 3 3			wxww
		// 3 1 0 0			wyxx
		// 3 1 0 1			wyxy
		// 3 1 0 2			wyxz			// legal lvalue
		// 3 1 0 3			wyxw
		// 3 1 1 0			wyyx
		// 3 1 1 1			wyyy
		// 3 1 1 2			wyyz
		// 3 1 1 3			wyyw
		// 3 1 2 0			wyzx			// legal lvalue
		// 3 1 2 1			wyzy
		// 3 1 2 2			wyzz
		// 3 1 2 3			wyzw
		// 3 1 3 0			wywx
		// 3 1 3 1			wywy
		// 3 1 3 2			wywz
		// 3 1 3 3			wyww
		// 3 2 0 0			wzxx
		// 3 2 0 1			wzxy			// legal lvalue
		// 3 2 0 2			wzxz
		// 3 2 0 3			wzxw
		// 3 2 1 0			wzyx			// legal lvalue
		// 3 2 1 1			wzyy
		// 3 2 1 2			wzyz
		// 3 2 1 3			wzyw
		// 3 2 2 0			wzzx
		// 3 2 2 1			wzzy
		// 3 2 2 2			wzzz
		// 3 2 2 3			wzzw
		// 3 2 3 0			wzwx
		// 3 2 3 1			wzwy
		// 3 2 3 2			wzwz
		// 3 2 3 3			wzww
		// 3 3 0 0			wwxx
		// 3 3 0 1			wwxy
		// 3 3 0 2			wwxz
		// 3 3 0 3			wwxw
		// 3 3 1 0			wwyx
		// 3 3 1 1			wwyy
		// 3 3 1 2			wwyz
		// 3 3 1 3			wwyw
		// 3 3 2 0			wwzx
		// 3 3 2 1			wwzy
		// 3 3 2 2			wwzz
		// 3 3 2 3			wwzw
		// 3 3 3 0			wwwx
		// 3 3 3 1			wwwy
		// 3 3 3 2			wwwz
		// 3 3 3 3			wwww

		CHECK_EQ(four.wxxx, ivec4(3, 0, 0, 0));

		CHECK_EQ(four.wxxy, ivec4(3, 0, 0, 1));

		CHECK_EQ(four.wxxz, ivec4(3, 0, 0, 2));

		CHECK_EQ(four.wxxw, ivec4(3, 0, 0, 3));

		CHECK_EQ(four.wxyx, ivec4(3, 0, 1, 0));

		CHECK_EQ(four.wxyy, ivec4(3, 0, 1, 1));

		CHECK_EQ(four.wxyz, ivec4(3, 0, 1, 2));

		CHECK_EQ(four.wxyw, ivec4(3, 0, 1, 3));

		CHECK_EQ(four.wxzx, ivec4(3, 0, 2, 0));

		CHECK_EQ(four.wxzy, ivec4(3, 0, 2, 1));

		CHECK_EQ(four.wxzz, ivec4(3, 0, 2, 2));

		CHECK_EQ(four.wxzw, ivec4(3, 0, 2, 3));

		CHECK_EQ(four.wxwx, ivec4(3, 0, 3, 0));

		CHECK_EQ(four.wxwy, ivec4(3, 0, 3, 1));

		CHECK_EQ(four.wxwz, ivec4(3, 0, 3, 2));

		CHECK_EQ(four.wxww, ivec4(3, 0, 3, 3));

		CHECK_EQ(four.wyxx, ivec4(3, 1, 0, 0));

		CHECK_EQ(four.wyxy, ivec4(3, 1, 0, 1));

		CHECK_EQ(four.wyxz, ivec4(3, 1, 0, 2));

		CHECK_EQ(four.wyxw, ivec4(3, 1, 0, 3));

		CHECK_EQ(four.wyyx, ivec4(3, 1, 1, 0));

		CHECK_EQ(four.wyyy, ivec4(3, 1, 1, 1));

		CHECK_EQ(four.wyyz, ivec4(3, 1, 1, 2));

		CHECK_EQ(four.wyyw, ivec4(3, 1, 1, 3));

		CHECK_EQ(four.wyzx, ivec4(3, 1, 2, 0));

		CHECK_EQ(four.wyzy, ivec4(3, 1, 2, 1));

		CHECK_EQ(four.wyzz, ivec4(3, 1, 2, 2));

		CHECK_EQ(four.wyzw, ivec4(3, 1, 2, 3));

		CHECK_EQ(four.wywx, ivec4(3, 1, 3, 0));

		CHECK_EQ(four.wywy, ivec4(3, 1, 3, 1));

		CHECK_EQ(four.wywz, ivec4(3, 1, 3, 2));

		CHECK_EQ(four.wyww, ivec4(3, 1, 3, 3));

		CHECK_EQ(four.wzxx, ivec4(3, 2, 0, 0));

		CHECK_EQ(four.wzxy, ivec4(3, 2, 0, 1));

		CHECK_EQ(four.wzxz, ivec4(3, 2, 0, 2));

		CHECK_EQ(four.wzxw, ivec4(3, 2, 0, 3));

		CHECK_EQ(four.wzyx, ivec4(3, 2, 1, 0));

		CHECK_EQ(four.wzyy, ivec4(3, 2, 1, 1));

		CHECK_EQ(four.wzyz, ivec4(3, 2, 1, 2));

		CHECK_EQ(four.wzyw, ivec4(3, 2, 1, 3));

		CHECK_EQ(four.wzzx, ivec4(3, 2, 2, 0));

		CHECK_EQ(four.wzzy, ivec4(3, 2, 2, 1));

		CHECK_EQ(four.wzzz, ivec4(3, 2, 2, 2));

		CHECK_EQ(four.wzzw, ivec4(3, 2, 2, 3));

		CHECK_EQ(four.wzwx, ivec4(3, 2, 3, 0));

		CHECK_EQ(four.wzwy, ivec4(3, 2, 3, 1));

		CHECK_EQ(four.wzwz, ivec4(3, 2, 3, 2));

		CHECK_EQ(four.wzww, ivec4(3, 2, 3, 3));

		CHECK_EQ(four.wwxx, ivec4(3, 3, 0, 0));

		CHECK_EQ(four.wwxy, ivec4(3, 3, 0, 1));

		CHECK_EQ(four.wwxz, ivec4(3, 3, 0, 2));

		CHECK_EQ(four.wwxw, ivec4(3, 3, 0, 3));

		CHECK_EQ(four.wwyx, ivec4(3, 3, 1, 0));

		CHECK_EQ(four.wwyy, ivec4(3, 3, 1, 1));

		CHECK_EQ(four.wwyz, ivec4(3, 3, 1, 2));

		CHECK_EQ(four.wwyw, ivec4(3, 3, 1, 3));

		CHECK_EQ(four.wwzx, ivec4(3, 3, 2, 0));

		CHECK_EQ(four.wwzy, ivec4(3, 3, 2, 1));

		CHECK_EQ(four.wwzz, ivec4(3, 3, 2, 2));

		CHECK_EQ(four.wwzw, ivec4(3, 3, 2, 3));

		CHECK_EQ(four.wwwx, ivec4(3, 3, 3, 0));

		CHECK_EQ(four.wwwy, ivec4(3, 3, 3, 1));

		CHECK_EQ(four.wwwz, ivec4(3, 3, 3, 2));

		CHECK_EQ(four.wwww, ivec4(3, 3, 3, 3));
	}
}

TEST_SUITE("lvalue swizzle copy assignment")
{
	TEST_CASE("lvalue swizzle copy assignment for 1D and 2D")
	{
		// make sure we can assign to these swizzle cases as lvalues

		SUBCASE("1D swizzling lvalues")
		{
			// all 1D swizzles can be lvalues, so there is no concept necessary to check for it

			// 0				x
			// 1				y
			// 2				z
			// 3				w
		}

		SUBCASE("2D swizzling lvalues")
		{
			// swizzles that can be lvalues

			// 0 1				xy
			// 0 2				xz
			// 0 3				xw
			// 1 0				yx
			// 1 2				yz
			// 1 3				yw
			// 2 0				zx
			// 2 1				zy
			// 2 3				zw
			// 3 0				wx
			// 3 1				wy
			// 3 2				wz

			CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0>);
			CHECK_UNARY(dsga::unique_indexes<0, 1>);
			CHECK_UNARY(dsga::unique_indexes<0, 2>);
			CHECK_UNARY(dsga::unique_indexes<0, 3>);
			CHECK_UNARY(dsga::unique_indexes<1, 0>);
			CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1>);
			CHECK_UNARY(dsga::unique_indexes<1, 2>);
			CHECK_UNARY(dsga::unique_indexes<1, 3>);
			CHECK_UNARY(dsga::unique_indexes<2, 0>);
			CHECK_UNARY(dsga::unique_indexes<2, 1>);
			CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2>);
			CHECK_UNARY(dsga::unique_indexes<2, 3>);
			CHECK_UNARY(dsga::unique_indexes<3, 0>);
			CHECK_UNARY(dsga::unique_indexes<3, 1>);
			CHECK_UNARY(dsga::unique_indexes<3, 2>);
			CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3>);
		}
	}

	TEST_CASE("lvalue swizzle copy assignment for 3D")
	{
		// swizzles that can be lvalues

		// 0 1 2			xyz
		// 0 1 3			xyw
		// 0 2 1			xzy
		// 0 2 3			xzw
		// 0 3 1			xwy
		// 0 3 2			xwz
		// 1 0 2			yxz
		// 1 0 3			yxw
		// 1 2 0			yzx
		// 1 2 3			yzw
		// 1 3 0			ywx
		// 1 3 2			ywz
		// 2 0 1			zxy
		// 2 0 3			zxw
		// 2 1 0			zyx
		// 2 1 3			zyw
		// 2 3 0			zwx
		// 2 3 1			zwy
		// 3 0 1			wxy
		// 3 0 2			wxz
		// 3 1 0			wyx
		// 3 1 2			wyz
		// 3 2 0			wzx
		// 3 2 1			wzy

		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 1>);
		CHECK_UNARY(dsga::unique_indexes<0, 1, 2>);
		CHECK_UNARY(dsga::unique_indexes<0, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 0>);
		CHECK_UNARY(dsga::unique_indexes<0, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 2>);
		CHECK_UNARY(dsga::unique_indexes<0, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 0>);
		CHECK_UNARY(dsga::unique_indexes<0, 3, 1>);
		CHECK_UNARY(dsga::unique_indexes<0, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 1>);
		CHECK_UNARY(dsga::unique_indexes<1, 0, 2>);
		CHECK_UNARY(dsga::unique_indexes<1, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 3>);
		CHECK_UNARY(dsga::unique_indexes<1, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 2>);
		CHECK_UNARY(dsga::unique_indexes<1, 2, 3>);
		CHECK_UNARY(dsga::unique_indexes<1, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 1>);
		CHECK_UNARY(dsga::unique_indexes<1, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 0>);
		CHECK_UNARY(dsga::unique_indexes<2, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 2>);
		CHECK_UNARY(dsga::unique_indexes<2, 0, 3>);
		CHECK_UNARY(dsga::unique_indexes<2, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 2>);
		CHECK_UNARY(dsga::unique_indexes<2, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 3>);
		CHECK_UNARY(dsga::unique_indexes<2, 3, 0>);
		CHECK_UNARY(dsga::unique_indexes<2, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 0>);
		CHECK_UNARY(dsga::unique_indexes<3, 0, 1>);
		CHECK_UNARY(dsga::unique_indexes<3, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 3>);
		CHECK_UNARY(dsga::unique_indexes<3, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 1>);
		CHECK_UNARY(dsga::unique_indexes<3, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 3>);
		CHECK_UNARY(dsga::unique_indexes<3, 2, 0>);
		CHECK_UNARY(dsga::unique_indexes<3, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 3>);

		CHECK_UNARY(dsga::writable_swizzle<4, 3, 3, 2, 1>);
		CHECK_UNARY_FALSE(dsga::writable_swizzle<4, 3, 3, 2, 2>);
	}


	TEST_CASE("lvalue swizzle copy assignment for 4D - starting index x or y")
	{
		// swizzles that can be lvalues

		// 0 1 2 3			xyzw
		// 0 1 3 2			xywz
		// 0 2 1 3			xzyw
		// 0 2 3 1			xzwy
		// 0 3 1 2			xwyz
		// 0 3 2 1			xwzy
		// 1 0 2 3			yxzw
		// 1 0 3 2			yxwz
		// 1 2 0 3			yzxw
		// 1 2 3 0			yzwx
		// 1 3 0 2			ywxz
		// 1 3 2 0			ywzx

		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 0, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 2, 2>);
		CHECK_UNARY(dsga::unique_indexes<0, 1, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 3, 1>);
		CHECK_UNARY(dsga::unique_indexes<0, 1, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 1, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 1, 2>);
		CHECK_UNARY(dsga::unique_indexes<0, 2, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 3, 0>);
		CHECK_UNARY(dsga::unique_indexes<0, 2, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 2, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 1, 1>);
		CHECK_UNARY(dsga::unique_indexes<0, 3, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 2, 0>);
		CHECK_UNARY(dsga::unique_indexes<0, 3, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<0, 3, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 2, 2>);
		CHECK_UNARY(dsga::unique_indexes<1, 0, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 3, 1>);
		CHECK_UNARY(dsga::unique_indexes<1, 0, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 0, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 1, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 0, 2>);
		CHECK_UNARY(dsga::unique_indexes<1, 2, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 2, 3>);
		CHECK_UNARY(dsga::unique_indexes<1, 2, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 2, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 0, 1>);
		CHECK_UNARY(dsga::unique_indexes<1, 3, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 1, 3>);
		CHECK_UNARY(dsga::unique_indexes<1, 3, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<1, 3, 3, 3>);
	}

	TEST_CASE("lvalue swizzle copy assignment for 4D - starting index z or w")
	{
		// swizzles that can be lvalues

		// 2 0 1 3			zxyw
		// 2 0 3 1			zxwy
		// 2 1 0 3			zyxw
		// 2 1 3 0			zywx
		// 2 3 0 1			zwxy
		// 2 3 1 0			zwyx
		// 3 0 1 2			wxyz
		// 3 0 2 1			wxzy
		// 3 1 0 2			wyxz
		// 3 1 2 0			wyzx
		// 3 2 0 1			wzxy
		// 3 2 1 0			wzyx

		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 1, 2>);
		CHECK_UNARY(dsga::unique_indexes<2, 0, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 3, 0>);
		CHECK_UNARY(dsga::unique_indexes<2, 0, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 0, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 0, 2>);
		CHECK_UNARY(dsga::unique_indexes<2, 1, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 2, 3>);
		CHECK_UNARY(dsga::unique_indexes<2, 1, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 1, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 2, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 0, 0>);
		CHECK_UNARY(dsga::unique_indexes<2, 3, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 0, 3>);
		CHECK_UNARY(dsga::unique_indexes<2, 3, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<2, 3, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 1, 1>);
		CHECK_UNARY(dsga::unique_indexes<3, 0, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 2, 0>);
		CHECK_UNARY(dsga::unique_indexes<3, 0, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 0, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 0, 1>);
		CHECK_UNARY(dsga::unique_indexes<3, 1, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 1, 3>);
		CHECK_UNARY(dsga::unique_indexes<3, 1, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 1, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 0, 0>);
		CHECK_UNARY(dsga::unique_indexes<3, 2, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 0, 3>);
		CHECK_UNARY(dsga::unique_indexes<3, 2, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 2, 3, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 0, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 0, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 0, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 0, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 1, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 1, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 1, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 1, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 2, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 2, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 2, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 2, 3>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 3, 0>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 3, 1>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 3, 2>);
		CHECK_UNARY_FALSE(dsga::unique_indexes<3, 3, 3, 3>);
	}
}

// structs for demonstrating common initial sequence
struct A
{
	std::array<double, 4> i;
};

struct B
{
	std::array<double, 4> j;
};

struct faux_vector
{
	B k;
};

struct faux_wrapper
{
	A l;
};

TEST_SUITE("test swizzling applications")
{
	TEST_CASE("type traits tests")
	{
		SUBCASE("type traits for storage_wrapper")
		{
			using dwrap4 = dsga::storage_wrapper<double, 4u>;

			CHECK_UNARY(std::is_standard_layout_v<dwrap4>);
			CHECK_UNARY(std::is_default_constructible_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_constructible_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_default_constructible_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_copy_constructible_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_move_constructible_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_copyable_v<dwrap4>);
			CHECK_UNARY(std::is_trivial_v<dwrap4>);
			CHECK_UNARY(std::is_copy_assignable_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_copy_assignable_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_move_assignable_v<dwrap4>);
			CHECK_UNARY(std::is_trivially_destructible_v<dwrap4>);

			CHECK_UNARY(std::is_aggregate_v<dwrap4>);
		}

		SUBCASE("type traits for basic_vector")
		{
			using dvec4 = dsga::basic_vector<double, 4u>;

			CHECK_UNARY(std::is_standard_layout_v<dvec4>);
			CHECK_UNARY(std::is_default_constructible_v<dvec4>);
			CHECK_UNARY(std::is_trivially_constructible_v<dvec4>);
			CHECK_UNARY(std::is_trivially_default_constructible_v<dvec4>);
			CHECK_UNARY(std::is_trivially_copy_constructible_v<dvec4>);
			CHECK_UNARY(std::is_trivially_move_constructible_v<dvec4>);
			CHECK_UNARY(std::is_trivially_copyable_v<dvec4>);
			CHECK_UNARY(std::is_trivial_v<dvec4>);
			CHECK_UNARY(std::is_copy_assignable_v<dvec4>);
			CHECK_UNARY(std::is_trivially_copy_assignable_v<dvec4>);
			CHECK_UNARY(std::is_trivially_move_assignable_v<dvec4>);
			CHECK_UNARY(std::is_trivially_destructible_v<dvec4>);

			CHECK_UNARY_FALSE(std::is_aggregate_v<dvec4>);
		}

		SUBCASE("type traits for indexed_vector")
		{
			using dswizzle1 = dsga::indexed_vector<double, 1u, 1u, 0u>;
			using dswizzle4 = dsga::indexed_vector<double, 4u, 4u, 0u, 1u, 2u, 3u>;

			CHECK_UNARY(std::is_standard_layout_v<dswizzle4>);
			CHECK_UNARY(std::is_default_constructible_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_constructible_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_default_constructible_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_copy_constructible_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_move_constructible_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_copyable_v<dswizzle4>);
			CHECK_UNARY(std::is_trivial_v<dswizzle4>);
			CHECK_UNARY(std::is_copy_assignable_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_copy_assignable_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_move_assignable_v<dswizzle4>);
			CHECK_UNARY(std::is_trivially_destructible_v<dswizzle4>);

			CHECK_UNARY(std::is_aggregate_v<dswizzle1>);
			CHECK_UNARY(std::is_aggregate_v<dswizzle4>);

			// indexed_vector iterators are forward iterators
			CHECK_UNARY(std::forward_iterator<indexed_vector_iterator<double, 4, 2, 0, 1>>);
			CHECK_UNARY(std::forward_iterator<indexed_vector_const_iterator<double, 4, 2, 0, 1>>);
		}

		SUBCASE("type traits for common initial sequence for anonymous union")
		{
			// for our vector and swizzling, we need to rely on union and the common initial sequence.
			// anything written in the union via a union member that shares a common initial sequence with
			// another union member can be referenced via any of the shared common initial sequence union
			// members, regardless of whether the member last wrote to the union or not. this is the exception
			// to the normal rule, where you can't read from a union member unless that was the last one that
			// wrote through to the union.
			//
			// for our vector and swizzling, we want all union data members to share the same common intial
			// sequence. it was unclear to me if we just want something there representing the type that isn't
			// really used for normal reference, but after having implemented this, it is essential to have this
			// or at least one member of the union to be the standard goto member. this is important if you want
			// to have constexpr instances of the class: you have to initialize one and only one member, and so
			// this first placeholder is a good choice. it is also how the class can internally modify the data.
			//
			// I have seen many unions that want to have the common intial sequence benefit and have the first
			// union member be a dummy member of the common type. the problem lies in that the other union
			// members use that type inside them, as they are often a struct with the data member in it. so when
			// this first thing is not in a struct, it should not be considered a part of the shared common
			// initial sequence. to make it be a part of it, then it too needs to be in a struct.
			//
			// this information comes from what I have gathered from these links:
			//
			// https://www.reddit.com/r/cpp_questions/comments/7ktrrj/language_lawyers_unions_and_common_initial/
			// https://stackoverflow.com/questions/43655657/union-common-initial-sequence-with-primitive
			// https://stackoverflow.com/questions/48209179/do-scalar-members-in-a-union-count-towards-the-common-initial-sequence
			// https://stackoverflow.com/questions/48058545/are-there-any-guarantees-for-unions-that-contain-a-wrapped-type-and-the-type-its

#if defined(__cpp_lib_is_layout_compatible)

			// proof that we are using the common initial sequence ***properly*** by introducing
			// dsga::storage_wrapper<> for the anonymous union instead of just adding a std::array<>:

			CHECK_UNARY(std::is_corresponding_member(&A::i, &B::j));						// using two structs of the same type form
			CHECK_UNARY_FALSE(std::is_corresponding_member(&A::i, &faux_vector::k));		// analogous to using std::array<> and dsga::indexed_vector<> at same level of anonymous union
			CHECK_UNARY(std::is_corresponding_member(&faux_wrapper::l, &faux_vector::k));	// analogous to using dsga::storage_wrapper<> and dsga::indexed_vector<> at same level of anonymous union

			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 1>::store, &dsga::indexed_vector<int, 1, 1, 0>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 2>::store, &dsga::indexed_vector<int, 2, 2, 1, 0>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 3>::store, &dsga::indexed_vector<int, 3, 3, 2, 0, 1>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 4>::store, &dsga::indexed_vector<int, 4, 1, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 4>::store, &dsga::indexed_vector<int, 4, 2, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 4>::store, &dsga::indexed_vector<int, 4, 3, 3, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<int, 4>::store, &dsga::indexed_vector<int, 4, 4, 3, 3, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<int, 4, 1, 3>::base, &dsga::indexed_vector<int, 4, 2, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<int, 4, 1, 3>::base, &dsga::indexed_vector<int, 4, 3, 3, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<int, 4, 1, 3>::base, &dsga::indexed_vector<int, 4, 4, 3, 3, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<int, 4, 2, 3, 3>::base, &dsga::indexed_vector<int, 4, 3, 3, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<int, 4, 2, 3, 3>::base, &dsga::indexed_vector<int, 4, 4, 3, 3, 3, 3>::base));
			CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<int, 4, 3, 3, 3, 3>::base, &dsga::indexed_vector<int, 4, 4, 3, 3, 3, 3>::base));

#endif
		}

		SUBCASE("type traits for basic_matrix")
		{
			using dmat4 = dsga::basic_matrix<double, 4u, 4u>;

			CHECK_UNARY(std::is_standard_layout_v<dmat4>);
			CHECK_UNARY(std::is_default_constructible_v<dmat4>);
			CHECK_UNARY(std::is_trivially_constructible_v<dmat4>);
			CHECK_UNARY(std::is_trivially_default_constructible_v<dmat4>);
			CHECK_UNARY(std::is_trivially_copy_constructible_v<dmat4>);
			CHECK_UNARY(std::is_trivially_move_constructible_v<dmat4>);
			CHECK_UNARY(std::is_trivially_copyable_v<dmat4>);
			CHECK_UNARY(std::is_trivial_v<dmat4>);
			CHECK_UNARY(std::is_copy_assignable_v<dmat4>);
			CHECK_UNARY(std::is_trivially_copy_assignable_v<dmat4>);
			CHECK_UNARY(std::is_trivially_move_assignable_v<dmat4>);
			CHECK_UNARY(std::is_trivially_destructible_v<dmat4>);

			CHECK_UNARY_FALSE(std::is_aggregate_v<dmat4>);
		}

	}

	TEST_CASE("structured binding (which requires) tuple interface (tuple_size, tuple_element, get)")
	{
		ivec4	four	(0, 1, 2, 3);
		ivec3	three	(4, 5, 6);
		ivec2	two		(7, 8);
		iscal	one		(9);

		SUBCASE("structured binding")
		{
			auto [one_x] = one;
			auto [two_x, two_y] = two;
			auto [three_x, three_y, three_z] = three;
			auto [four_x, four_y, four_z, four_w] = four;

			// dimension_data
			CHECK_EQ(one,	iscal(one_x));
			CHECK_EQ(two,	ivec2(two_x, two_y));
			CHECK_EQ(three,	ivec3(three_x, three_y, three_z));
			CHECK_EQ(four,	ivec4(four_x, four_y, four_z, four_w));

			const auto [one_x1, one_x2, one_x3] = one.xxx;
			const auto [two_y1, two_y2, two_x3] = two.yyx;
			auto [three_z1, three_x2] = three.zx;
			auto [four_y1, four_w2, four_z3, four_x4] = four.ywzx;

			// indexed_vector
			CHECK_EQ(one_x1,	one.x);
			CHECK_EQ(one_x2,	one.x);
			CHECK_EQ(one_x3,	one.x);
			CHECK_EQ(two_y1,	two.y);
			CHECK_EQ(two_y2,	two.y);
			CHECK_EQ(two_x3,	two.x);
			CHECK_EQ(three_z1,	three.z);
			CHECK_EQ(three_x2,	three.x);
			CHECK_EQ(four_y1,	four.y);
			CHECK_EQ(four_w2,	four.w);
			CHECK_EQ(four_z3,	four.z);
			CHECK_EQ(four_x4,	four.x);

			//
			// structured binding with lvalue references
			//

			ivec4 four_copy{four};

			auto &[fcz1, fcz2, fcw3, fcy4] = four_copy.zxwy;

			CHECK_EQ(fcz1,	four.z);
			CHECK_EQ(fcz2,	four.x);
			CHECK_EQ(fcw3,	four.w);
			CHECK_EQ(fcy4,	four.y);

			// changing structured binding references changes object destructured
			const int old_four_copy_z = four_copy.z;
			constexpr int new_four_copy_z = 12345;

			CHECK_NE(new_four_copy_z, old_four_copy_z);
			CHECK_EQ(four_copy.z, old_four_copy_z);

			fcz1 = new_four_copy_z;

			CHECK_EQ(four_copy.z, new_four_copy_z);
		}
	}

	TEST_CASE("range-for loop (which requires) begin/end/deref(*)/prefix ++ interface")
	{
		ivec4	four(0, 1, 2, 3);
		[[ maybe_unused ]] ivec3	three(4, 5, 6);
		[[ maybe_unused ]] ivec2	two(7, 8);
		[[ maybe_unused ]] iscal	one(9);

		SUBCASE("range-for non-const dimension_data")
		{
			ivec4 four_dest(0);

			// for basic_vector

			// recreate input one at a time
			// "int &" deduced for "auto &"
			for (unsigned dest_indx = 0; auto & loop_var : four)
			{
				four_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(four, four_dest);
		}

		SUBCASE("range-for const dimension_data")
		{
			const ivec4 const_data(19, 28, 37, 46);
			ivec4 data_dest(0);

			// for basic_vector

			// recreate input one at a time
			// "const int &" deduced for "auto &"
			for (unsigned dest_indx = 0; auto & loop_var : const_data)
			{
				data_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(const_data, data_dest);
		}

		SUBCASE("range-for dimension_data allows modifying")
		{
			ivec4 mutable_data(0, 0, 0, 0);

			// for indexed_vector

			// add index squared to whatever is accessed from the swizzle
			for (int dest_indx = 0; auto &loop_var : mutable_data)
			{
				loop_var += dest_indx * dest_indx;
				++dest_indx;
			}

			CHECK_EQ(mutable_data, ivec4(0, 1, 4, 9));
		}

		SUBCASE("range-for non-const indexed_vector")
		{
			ivec4 non_const_data(55, 64, 73, 82);
			ivec4 data_dest(0);

			// for indexed_vector

			// recreate input one at a time
			// "int &" deduced for "auto &"
			for (unsigned dest_indx = 0; auto & loop_var : non_const_data.zwxy)
			{
				data_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(data_dest, ivec4(73, 82, 55, 64));
		}

		SUBCASE("range-for const indexed_vector")
		{
			const ivec4 const_data(19, 28, 37, 46);
			ivec4 data_dest(0);

			// for indexed_vector

			// recreate input one at a time
			// "const int &" deduced for "auto &"
			for (unsigned dest_indx = 0; auto & loop_var : const_data.zwxy)
			{
				data_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(data_dest, ivec4(37, 46, 19, 28));
		}

		SUBCASE("range-for indexed_vector allows aliasing")
		{
			ivec4 mutable_data(0, 0, 0, 0);

			// for indexed_vector

			// add index to whatever is accessed from the swizzle
			for (int dest_indx = 0; auto &loop_var : mutable_data.wzyx)
			{
				loop_var += dest_indx;
				++dest_indx;
			}

			CHECK_EQ(mutable_data, ivec4(3, 2, 1, 0));
		}
	}

	TEST_CASE("typical usage but without math")
	{
		SUBCASE("first case")
		{
			// vector declarations
			ivec4 somethingoranother(0, 1, 2, 3);

			// test temporary rvalue with a swizzle to see if they compare equal
			CHECK_EQ(ivec3(1, 3, 0), somethingoranother.ywx);

			// testing full construction of different types from components (OtherScalar types)
			fvec3 foo(4, 3, 2);
			fvec2 pair(5, 6);

			// testing default construction
			fvec4 bar(0);

			// testing rvalue converted with its Scalar operator
			[[ maybe_unused ]] fvec2 asdf(fscal(33), 55);

			// testing 1, 2, 1 constructor for 4 element vector type
			[[ maybe_unused ]] fvec4 quux(3, pair, foo.z);

			// checking if different swizzles from different types (different sizes, Scalars) can compare equal
			CHECK_EQ(foo.zy, somethingoranother.zw);

			// check things that should obviously have values that will not compare equal
			CHECK_NE(somethingoranother.ww, pair);

			// vector type instance from lambda
			auto pairgen = []() -> fvec2 { return fvec2(5, 6); };

			// compare vector type from lambda with a vector type with same data
			CHECK_EQ(pair, pairgen());

			// create a 4 swizzle from a size 3 vectory type
			bar = foo.yyzx;

			//
			// test that former aliasing problem for swizzle self assignment is now working properly
			//

			// test swizzle to vector type conversion operator for vector type copy constructor
			fvec3 temp(foo.zyx);

			// using swizzle to vector type conversion operator for vector type assignment operator (does it do a move assign?)
			foo = foo.zyx;

			// show that the aliasing problem is fixed
			CHECK_EQ(temp, foo);

			//
			// lvalue swizzle examples
			//

			pair = foo.xz;

			bar.xy = pair;
		//	bar.zw = foo.xz + bar.zw;	// <- don't currently have arithmetic operators
			bar.zw = foo.xz;
			pair.y = 8;

			fvec2 lastpair(7, 10);
			lastpair.xy = pairgen();
		}
	}
}

TEST_SUITE("test self-assignment mutate")
{
	ivec4 fwd{0, 1, 2, 3};
	ivec4 rev{3, 2, 1, 0};

	TEST_CASE("multiple ways to accomplish same swizzling mutation")
	{
		auto val1 = fwd;
		val1.xyzw = val1.wzyx;
		CHECK_UNARY(all(equal(val1, rev)));

		auto val2 = fwd;
		val2.xywz = val2.wzxy;
		CHECK_UNARY(all(equal(val2, rev)));

		auto val3 = fwd;
		val3.xzyw = val3.wyzx;
		CHECK_UNARY(all(equal(val3, rev)));

		auto val4 = fwd;
		val4.xzwy = val4.wyxz;
		CHECK_UNARY(all(equal(val4, rev)));

		auto val5 = fwd;
		val5.xwzy = val5.wxyz;
		CHECK_UNARY(all(equal(val5, rev)));

		auto val6 = fwd;
		val6.xwyz = val6.wxzy;
		CHECK_UNARY(all(equal(val6, rev)));

		auto val7 = fwd;
		val7.yxzw = val7.zwyx;
		CHECK_UNARY(all(equal(val7, rev)));

		auto val8 = fwd;
		val8.yxwz = val8.zwxy;
		CHECK_UNARY(all(equal(val8, rev)));

		auto val9 = fwd;
		val9.yzxw = val9.zywx;
		CHECK_UNARY(all(equal(val9, rev)));

		auto val10 = fwd;
		val10.yzwx = val10.zyxw;
		CHECK_UNARY(all(equal(val10, rev)));

		auto val11 = fwd;
		val11.ywzx = val11.zxyw;
		CHECK_UNARY(all(equal(val11, rev)));

		auto val12 = fwd;
		val12.ywxz = val12.zxwy;
		CHECK_UNARY(all(equal(val12, rev)));

		auto val13 = fwd;
		val13.zxyw = val13.ywzx;
		CHECK_UNARY(all(equal(val13, rev)));

		auto val14 = fwd;
		val14.zxwy = val14.ywxz;
		CHECK_UNARY(all(equal(val14, rev)));

		auto val15 = fwd;
		val15.zyxw = val15.yzwx;
		CHECK_UNARY(all(equal(val15, rev)));

		auto val16 = fwd;
		val16.zywx = val16.yzxw;
		CHECK_UNARY(all(equal(val16, rev)));

		auto val17 = fwd;
		val17.zwxy = val17.yxwz;
		CHECK_UNARY(all(equal(val17, rev)));

		auto val18 = fwd;
		val18.zwyx = val18.yxzw;
		CHECK_UNARY(all(equal(val18, rev)));

		auto val19 = fwd;
		val19.wxyz = val19.xwzy;
		CHECK_UNARY(all(equal(val19, rev)));

		auto val20 = fwd;
		val20.wxzy = val20.xwyz;
		CHECK_UNARY(all(equal(val20, rev)));

		auto val21 = fwd;
		val21.wyzx = val21.xzyw;
		CHECK_UNARY(all(equal(val21, rev)));

		auto val22 = fwd;
		val22.wyxz = val22.xzwy;
		CHECK_UNARY(all(equal(val22, rev)));

		auto val23 = fwd;
		val23.wzxy = val23.xywz;
		CHECK_UNARY(all(equal(val23, rev)));

		auto val24 = fwd;
		val24.wzyx = val24.xyzw;
		CHECK_UNARY(all(equal(val24, rev)));
	}
}
