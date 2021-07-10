
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
	TEST_CASE("vector unary operator +")
	{
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		auto res1 = +i1;
		auto res2 = +i2;
		auto res3 = +i3;

		CHECK_EQ(i1, res1);
		CHECK_EQ(i2, res2);
		CHECK_EQ(i3, res3);

	}

	TEST_CASE("vector unary operator -")
	{
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		auto res1 = -i1;
		auto res2 = -i2;
		auto res3 = -i3;

		CHECK_EQ(-2, res1);
		CHECK_EQ(ivec2(-3, -4), res2);
		CHECK_EQ(ivec3(-5, -6, -7), res3);
	}

	TEST_CASE("vector unary pre-increment operator ++")
	{
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		auto v1 = ++i1;
		auto v2 = ++i2;
		auto v3 = ++i3;

		CHECK_EQ(3, v1);
		CHECK_EQ(ivec2(4, 5), v2);
		CHECK_EQ(ivec3(6, 7, 8), v3);

		CHECK_EQ(i1, 3);
		CHECK_EQ(i2, ivec2(4, 5));
		CHECK_EQ(i3, ivec3(6, 7, 8));
	}

	TEST_CASE("vector unary post-increment operator ++")
	{
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		auto v1 = i1++;
		auto v2 = i2++;
		auto v3 = i3++;

		CHECK_EQ(2, v1);
		CHECK_EQ(ivec2(3, 4), v2);
		CHECK_EQ(ivec3(5, 6, 7), v3);

		CHECK_EQ(i1, 3);
		CHECK_EQ(i2, ivec2(4, 5));
		CHECK_EQ(i3, ivec3(6, 7, 8));
	}

	TEST_CASE("vector unary pre-decrement operator --")
	{
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		auto v1 = --i1;
		auto v2 = --i2;
		auto v3 = --i3;

		CHECK_EQ(1, v1);
		CHECK_EQ(ivec2(2, 3), v2);
		CHECK_EQ(ivec3(4, 5, 6), v3);

		CHECK_EQ(i1, 1);
		CHECK_EQ(i2, ivec2(2, 3));
		CHECK_EQ(i3, ivec3(4, 5, 6));
	}

	TEST_CASE("vector unary post-decrement operator --")
	{
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		auto v1 = i1--;
		auto v2 = i2--;
		auto v3 = i3--;

		CHECK_EQ(2, v1);
		CHECK_EQ(ivec2(3, 4), v2);
		CHECK_EQ(ivec3(5, 6, 7), v3);

		CHECK_EQ(i1, 1);
		CHECK_EQ(i2, ivec2(2, 3));
		CHECK_EQ(i3, ivec3(4, 5, 6));
	}

	TEST_CASE("vector binary operator +")
	{
		// commutative
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		CHECK_EQ(i2 + ivec2(10, 20), ivec2(13, 24));
		CHECK_EQ(i2 + i1, ivec2(5, 6));
		CHECK_EQ(i1 + i3, ivec3(7, 8, 9));
		CHECK_EQ(2 + i2, ivec2(5, 6));
		CHECK_EQ(i3 + 8, ivec3(13, 14, 15));

		CHECK_EQ(i2.xy + ivec2(10, 20), ivec2(13, 24));
		CHECK_EQ(i2.xy + i1, ivec2(5, 6));
		CHECK_EQ(i1 + i3.xyz, ivec3(7, 8, 9));
		CHECK_EQ(2 + i2.xy, ivec2(5, 6));
		CHECK_EQ(i3.xyz + 8, ivec3(13, 14, 15));

		CHECK_EQ(i2.yx + i1, ivec2(6, 5));
		CHECK_EQ(i2.y + i1, 6);
		CHECK_EQ(i2.y + 2, 6);
		CHECK_EQ(4 + i1, 6);
	}

	TEST_CASE("vector binary operator -")
	{
		// non-commutative
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		CHECK_EQ(i2 - ivec2(10, 20), ivec2(-7, -16));
		CHECK_EQ(i2 - i1, ivec2(1, 2));
		CHECK_EQ(i1 - i3, ivec3(-3, -4, -5));
		CHECK_EQ(2 - i2, ivec2(-1, -2));
		CHECK_EQ(i3 - 8, ivec3(-3, -2, -1));

		CHECK_EQ(i2.xy - ivec2(10, 20), ivec2(-7, -16));
		CHECK_EQ(i2.xy - i1, ivec2(1, 2));
		CHECK_EQ(i1 - i3.xyz, ivec3(-3, -4, -5));
		CHECK_EQ(2 - i2.xy, ivec2(-1, -2));
		CHECK_EQ(i3.xyz - 8, ivec3(-3, -2, -1));

		CHECK_EQ(i2.yx - i1, ivec2(2, 1));
		CHECK_EQ(i2.y - i1, 2);
		CHECK_EQ(i2.y - 2, 2);
		CHECK_EQ(4 - i1, 2);
	}

	TEST_CASE("vector binary operator *")
	{
		// commutative
		iscal i1(2);
		ivec2 i2(3, 4);
		ivec3 i3(5, 6, 7);

		CHECK_EQ(i2 * ivec2(10, 20), ivec2(30, 80));
		CHECK_EQ(i2 * i1, ivec2(6, 8));
		CHECK_EQ(i1 * i3, ivec3(10, 12, 14));
		CHECK_EQ(2 * i2, ivec2(6, 8));
		CHECK_EQ(i3 * 8, ivec3(40, 48, 56));

		CHECK_EQ(i2.xy * ivec2(10, 20), ivec2(30, 80));
		CHECK_EQ(i2.xy * i1, ivec2(6, 8));
		CHECK_EQ(i1 * i3.xyz, ivec3(10, 12, 14));
		CHECK_EQ(2 * i2.xy, ivec2(6, 8));
		CHECK_EQ(i3.xyz * 8, ivec3(40, 48, 56));

		CHECK_EQ(i2.yx * i1, ivec2(8, 6));
		CHECK_EQ(i2.y * i1, 8);
		CHECK_EQ(i2.y * 2, 8);
		CHECK_EQ(4 * i1, 8);
	}

	TEST_CASE("vector binary operator /")
	{
		// non-commutative
		iscal i1(1020);
		ivec2 i2(32, 40);
		ivec3 i3(5, 30, 85);

		CHECK_EQ(i2 / ivec2(16, 5), ivec2(2, 8));
		CHECK_EQ(i2 / iscal(8), ivec2(4, 5));
		CHECK_EQ(i1 / i3, ivec3(204, 34, 12));
		CHECK_EQ(160 / i2, ivec2(5, 4));
		CHECK_EQ(i3 / 5, ivec3(1, 6, 17));

		CHECK_EQ(i2.xy / ivec2(16, 5), ivec2(2, 8));
		CHECK_EQ(i2.xy / iscal(8), ivec2(4, 5));
		CHECK_EQ(i1 / i3.xyz, ivec3(204, 34, 12));
		CHECK_EQ(160 / i2.xy, ivec2(5, 4));
		CHECK_EQ(i3.xyz / 5, ivec3(1, 6, 17));

		CHECK_EQ(i2.yx / iscal(8), ivec2(5, 4));
		CHECK_EQ(i2.y / iscal(8), 5);
		CHECK_EQ(i2.y / 8, 5);
		CHECK_EQ(40 / iscal(8), 5);
	}

	TEST_CASE("vector binary operator %")
	{
		// non-commutative
		iscal i1(1023);
		ivec2 i2(37, 41);
		ivec3 i3(5, 18, 75);

		CHECK_EQ(i2 % ivec2(16, 5), ivec2(5, 1));
		CHECK_EQ(i2 % iscal(7), ivec2(2, 6));
		CHECK_EQ(i1 % i3, ivec3(3, 15, 48));
		CHECK_EQ(160 % i2, ivec2(12, 37));
		CHECK_EQ(i3 % 5, ivec3(0, 3, 0));

		CHECK_EQ(i2.xy % ivec2(16, 5), ivec2(5, 1));
		CHECK_EQ(i2.xy % iscal(7), ivec2(2, 6));
		CHECK_EQ(i1 % i3.xyz, ivec3(3, 15, 48));
		CHECK_EQ(160 % i2.xy, ivec2(12, 37));
		CHECK_EQ(i3.xyz % 5, ivec3(0, 3, 0));

		CHECK_EQ(i2.yx % iscal(7), ivec2(6, 2));
		CHECK_EQ(i2.y % iscal(7), 6);
		CHECK_EQ(i2.y % 7, 6);
		CHECK_EQ(41 % iscal(7), 6);
	}

	TEST_CASE("vector unary operator ~")
	{
		iscal i1(0x00ff00cc);
		ivec2 i2(0xab00ff00, 0x00de0048);

		CHECK_EQ(~i1, 0xff00ff33);
		CHECK_EQ(~i2, ivec2(0x54ff00ff, 0xff21ffb7));

		CHECK_EQ(~ivec2(0x00000000, 0xffffffff), ivec2(0xffffffff, 0x00000000));
		CHECK_EQ(~ivec2(0xffffffff, 0x00000000), ivec2(0x00000000, 0xffffffff));

		CHECK_EQ(~i1.x, 0xff00ff33);
		CHECK_EQ(~i2.xy, ivec2(0x54ff00ff, 0xff21ffb7));

		CHECK_EQ(~i2.yx, ivec2(0xff21ffb7, 0x54ff00ff));
		CHECK_EQ(~i2.y, 0xff21ffb7);
	}

	TEST_CASE("vector binary operator <<")
	{
		// non-commutative
		iscal i1(1023);
		ivec2 i2(3, 2);
		ivec3 i3(4, 5, 6);

		CHECK_EQ(i2 << ivec2(16, 5), ivec2(196608, 64));
		CHECK_EQ(i2 << iscal(7), ivec2(384, 256));
		CHECK_EQ(i1 << i3, ivec3(16368, 32736, 65472));
		CHECK_EQ(160 << i2, ivec2(1280, 640));
		CHECK_EQ(i3 << 5, ivec3(128, 160, 192));

		CHECK_EQ(i2.xy << ivec2(16, 5), ivec2(196608, 64));
		CHECK_EQ(i2.xy << iscal(7), ivec2(384, 256));
		CHECK_EQ(i1 << i3.xyz, ivec3(16368, 32736, 65472));
		CHECK_EQ(160 << i2.xy, ivec2(1280, 640));
		CHECK_EQ(i3.xyz << 5, ivec3(128, 160, 192));

		CHECK_EQ(i2.yx << iscal(7), ivec2(256, 384));
		CHECK_EQ(i2.y << iscal(7), 256);
		CHECK_EQ(i2.y << 7, 256);
		CHECK_EQ(2 << iscal(7), 256);
	}

	TEST_CASE("vector binary operator >>")
	{
		// non-commutative
		iscal i1(123456);
		ivec2 i2(3, 2);
		ivec3 i3(4, 5, 6);
		ivec4 i4(56723, 904, 1023, 121);

		CHECK_EQ(i4 >> ivec4(14, 5, 8, 3), ivec4(3, 28, 3, 15));
		CHECK_EQ(i4 >> iscal(7), ivec4(443, 7, 7, 0));
		CHECK_EQ(i1 >> i3, ivec3(7716, 3858, 1929));
		CHECK_EQ(160 >> i2, ivec2(20, 40));
		CHECK_EQ(i3 >> 2, ivec3(1, 1, 1));

		CHECK_EQ(i4.xyzw >> ivec4(14, 5, 8, 3), ivec4(3, 28, 3, 15));
		CHECK_EQ(i4.xyzw >> iscal(7), ivec4(443, 7, 7, 0));
		CHECK_EQ(i1 >> i3.xyz, ivec3(7716, 3858, 1929));
		CHECK_EQ(160 >> i2.xy, ivec2(20, 40));
		CHECK_EQ(i3.xyz >> 2, ivec3(1, 1, 1));

		CHECK_EQ(i4.wzyx >> iscal(7), ivec4(0, 7, 7, 443));
		CHECK_EQ(i4.x >> iscal(7), 443);
		CHECK_EQ(i4.x >> 7, 443);
		CHECK_EQ(56723 >> iscal(7), 443);
	}

	TEST_CASE("vector binary operator &")
	{
		// commutative
		uscal u1(0x638d9e07);
		uvec2 u2(0x78ab0d16, 0x2103d5f3);
		uvec3 u3(0x2468ace0, 0x7531f9db, 0xc8f54716);

		CHECK_EQ(u2 & uvec2(0xd4026639, 0x32417869), uvec2(0x50020410, 0x20015061));
		CHECK_EQ(u2 & uscal(0x324a77e1), uvec2(0x300a0500, 0x200255e1));
		CHECK_EQ(u1 & u3, uvec3(0x20088c00, 0x61019803, 0x40850606));
		CHECK_EQ(0x9696a5a5u & u2, uvec2(0x10820504, 0x000285a1));
		CHECK_EQ(u3 & 0x3c3c1248u, uvec3(0x24280040, 0x34301048, 0x08340200));

		CHECK_EQ(u2.xy & uvec2(0xd4026639, 0x32417869), uvec2(0x50020410, 0x20015061));
		CHECK_EQ(u2.xy & uscal(0x324a77e1), uvec2(0x300a0500, 0x200255e1));
		CHECK_EQ(u1 & u3.xyz, uvec3(0x20088c00, 0x61019803, 0x40850606));
		CHECK_EQ(0x9696a5a5u & u2.xy, uvec2(0x10820504, 0x000285a1));
		CHECK_EQ(u3.xyz & 0x3c3c1248u, uvec3(0x24280040, 0x34301048, 0x08340200));

		CHECK_EQ(u2.yx & uscal(0x324a77e1), uvec2(0x200255e1, 0x300a0500));
		CHECK_EQ(u2.y & uscal(0x324a77e1), 0x200255e1u);
		CHECK_EQ(u2.y & 0x324a77e1u, 0x200255e1u);
		CHECK_EQ(0x324a77e1u & u2.y, 0x200255e1u);
	}

	TEST_CASE("vector binary operator |")
	{
		// commutative
		uscal u1(0x638d9e07);
		uvec2 u2(0x78ab0d16, 0x2103d5f3);
		uvec3 u3(0x2468ace0, 0x7531f9db, 0xc8f54716);

		CHECK_EQ(u2 | uvec2(0xd4026639, 0x32417869), uvec2(0xfcab6f3f, 0x3343fdfb));
		CHECK_EQ(u2 | uscal(0x324a77e1), uvec2(0x7aeb7ff7, 0x334bf7f3));
		CHECK_EQ(u1 | u3, uvec3(0x67edbee7, 0x77bdffdf, 0xebfddf17));
		CHECK_EQ(0x9696a5a5u | u2, uvec2(0xfebfadb7, 0xb797f5f7));
		CHECK_EQ(u3 | 0x3c3c1248u, uvec3(0x3c7cbee8, 0x7d3dfbdb, 0xfcfd575e));

		CHECK_EQ(u2.xy | uvec2(0xd4026639, 0x32417869), uvec2(0xfcab6f3f, 0x3343fdfb));
		CHECK_EQ(u2.xy | uscal(0x324a77e1), uvec2(0x7aeb7ff7, 0x334bf7f3));
		CHECK_EQ(u1 | u3.xyz, uvec3(0x67edbee7, 0x77bdffdf, 0xebfddf17));
		CHECK_EQ(0x9696a5a5u | u2.xy, uvec2(0xfebfadb7, 0xb797f5f7));
		CHECK_EQ(u3.xyz | 0x3c3c1248u, uvec3(0x3c7cbee8, 0x7d3dfbdb, 0xfcfd575e));

		CHECK_EQ(u2.yx | uscal(0x324a77e1), uvec2(0x334bf7f3, 0x7aeb7ff7));
		CHECK_EQ(u2.y | uscal(0x324a77e1), 0x334bf7f3u);
		CHECK_EQ(u2.y | 0x324a77e1u, 0x334bf7f3u);
		CHECK_EQ(0x324a77e1u | u2.y, 0x334bf7f3u);
	}

	TEST_CASE("vector binary operator ^")
	{
		// commutative
		uscal u1(0x638d9e07);
		uvec2 u2(0x78ab0d16, 0x2103d5f3);
		uvec3 u3(0x2468ace0, 0x7531f9db, 0xc8f54716);

		CHECK_EQ(u2 ^ uvec2(0xd4026639, 0x32417869), uvec2(0xaca96b2f, 0x1342ad9a));
		CHECK_EQ(u2 ^ uscal(0x324a77e1), uvec2(0x4ae17af7, 0x1349a212));
		CHECK_EQ(u1 ^ u3, uvec3(0x47e532e7, 0x16bc67dc, 0xab78d911));
		CHECK_EQ(0x9696a5a5u ^ u2, uvec2(0xee3da8b3, 0xb7957056));
		CHECK_EQ(u3 ^ 0x3c3c1248u, uvec3(0x1854bea8, 0x490deb93, 0xf4c9555e));

		CHECK_EQ(u2.xy ^ uvec2(0xd4026639, 0x32417869), uvec2(0xaca96b2f, 0x1342ad9a));
		CHECK_EQ(u2.xy ^ uscal(0x324a77e1), uvec2(0x4ae17af7, 0x1349a212));
		CHECK_EQ(u1 ^ u3.xyz, uvec3(0x47e532e7, 0x16bc67dc, 0xab78d911));
		CHECK_EQ(0x9696a5a5u ^ u2.xy, uvec2(0xee3da8b3, 0xb7957056));
		CHECK_EQ(u3.xyz ^ 0x3c3c1248u, uvec3(0x1854bea8, 0x490deb93, 0xf4c9555e));

		CHECK_EQ(u2.yx ^ uscal(0x324a77e1), uvec2(0x1349a212, 0x4ae17af7));
		CHECK_EQ(u2.y ^ uscal(0x324a77e1), 0x1349a212u);
		CHECK_EQ(u2.y ^ 0x324a77e1u, 0x1349a212u);
		CHECK_EQ(0x324a77e1u ^ u2.y, 0x1349a212u);
	}

	TEST_CASE("vector length and size")
	{
		// length()

		// size()

	}

	TEST_CASE("matrix unary operator +")
	{
		//  auto B = +A;
	}

	TEST_CASE("matrix unary operator -")
	{
		//  auto B = -A;
	}

	TEST_CASE("matrix unary pre-increment operator ++")
	{
		// ++A;
	}

	TEST_CASE("matrix unary post-increment operator ++")
	{
		// auto B = A++;
	}

	TEST_CASE("matrix unary pre-decrement operator --")
	{
		// --A;
	}

	TEST_CASE("matrix unary post-decrement operator --")
	{
		// auto B = A--;
	}

	TEST_CASE("matrix binary operator +")
	{
		// dmat2 A, B;
		// double x;
		// ...
		// auto foo = A + x;
		// auto bar = x + A;
		// auto baz = A + B;
	}

	TEST_CASE("matrix binary operator -")
	{
		// dmat2 A, B;
		// double x;
		// ...
		// auto foo = A - x;
		// auto bar = x - A;
		// auto baz = A - B;
	}

	TEST_CASE("matrix binary operator *")
	{
		// dmat2 A;
		// double x;
		// ...
		// auto foo = A * x;
		// auto bar = x * A;
	}

	TEST_CASE("matrix binary operator /")
	{
		// dmat2 A, B;
		// double x;
		// ...
		// auto foo = A / x;
		// auto bar = x / A;
		// auto baz = A / B;
	}

	TEST_CASE("matrix linear-algebraic *")
	{
		// dmat3 A, B;
		// dvec3 v;
		// ...
		// auto foo = A * v;
		// auto bar = v * A;
		// auto baz = A * B;
	}

	TEST_CASE("matrix row and column access")
	{
		// dmat3 A;
		// 
		// auto col = A[0u] ;
		// auto row = A.template row<1u>();
	}

	TEST_CASE("matrix length and size")
	{
		// length()

		// size()

	}

}
