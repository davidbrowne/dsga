
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
	constexpr ivec4	cx_four		(0, 1, 2, 3);
	constexpr ivec3	cx_three	(4, 5, 6);
	constexpr ivec2	cx_two		(7, 8);

	constexpr uvec2 utwo(0x207ef45f, 0xe518c41d);
	constexpr uvec3 uthree(0xae50d46b, 0x10712fd0, 0x47946919);
	constexpr uvec4 ufour(0x686e22e1, 0x4b79b211, 0x5f70e079, 0x5c30ee44);

	// all default assignment *is* implemented inside the class, so each dimension of basic_vector has its own implementation
	TEST_CASE("default assignment")
	{
		SUBCASE("1D default assignment =")
		{
			iscal v1(97);
			ivec4 v4(17);

			v4.y = 25;
			v1 = v4.w;

			CHECK_EQ(v1, iscal(17));
			CHECK_EQ(v4, ivec4(17, 25, 17, 17));
		}

		SUBCASE("2D default assignment =")
		{
			ivec2 v2(83);
			ivec4 v4(17);

			v4.yw = v2;
			v4.xz = ivec2(10, 20);
			v2 = v4.xy;

			CHECK_EQ(v2, ivec2(10, 83));
			CHECK_EQ(v4, ivec4(10, 83, 20, 83));
		}

		SUBCASE("3D default assignment =")
		{
			ivec3 v3(71);
			ivec4 v4(17);

			v4.wzx = v3;
			CHECK_EQ(v4, ivec4(71, 17, 71, 71));
			v3 = ivec3(33, 44, 55);
			v4.xyz = v3.yxy;
			CHECK_EQ(v4, ivec4(44, 33, 44, 71));
			v3 = v4.zyw;
			CHECK_EQ(v3, ivec3(44, 33, 71));
		}

		SUBCASE("4D default assignment =")
		{
			ivec2 v2(83, 29);
			ivec3 v3(71, 10, 4);
			ivec4 v4(17, 38, 56, 95);

			v4 = v4.wzyx;
			CHECK_EQ(v4, ivec4(95, 56, 38, 17));
			v4.ywxz = v3.xyzx;
			CHECK_EQ(v4, ivec4(4, 71, 71, 10));
			v4 = ivec4(v3, v2.y);
			CHECK_EQ(v4, ivec4(71, 10, 4, 29));
			v4.zwyx = v4;
			CHECK_EQ(v4, ivec4(29, 4, 71, 10));
		}
	}

	TEST_CASE("operator +=")
	{
		SUBCASE("basic_vector operator +=")
		{
			iscal v1(20);
			v1 += 10;
			CHECK_EQ(v1, 30);
			v1 += iscal(13);
			CHECK_EQ(v1, 43);

			ivec3 v3(1, 2, 3);
			v3 += cx_three;
			CHECK_EQ(v3, ivec3(5, 7, 9));
			v3 += 500;
			CHECK_EQ(v3, ivec3(505, 507, 509));
			v3 += v3.yzx;
			CHECK_EQ(v3, ivec3(1012, 1016, 1014));
			v3 += v3.z;
			CHECK_EQ(v3, ivec3(2026, 2030, 2028));
		}

		SUBCASE("indexed_vector operator +=")
		{
			ivec3 v3(1, 2, 3);
			v3.zx += 40;
			CHECK_EQ(v3, ivec3(41, 2, 43));
			v3.xy += cx_three.zy;
			CHECK_EQ(v3, ivec3(47, 7, 43));
			v3.yz += cx_two;
			CHECK_EQ(v3, ivec3(47, 14, 51));
			v3.xzy += cx_four.w;
			CHECK_EQ(v3, ivec3(50, 17, 54));
		}
	}

	TEST_CASE("operator -=")
	{
		SUBCASE("basic_vector operator -=")
		{
			iscal v1(200);
			v1 -= 10;
			CHECK_EQ(v1, 190);
			v1 -= iscal(13);
			CHECK_EQ(v1, 177);

			ivec3 v3(10, 20, 30);
			v3 -= cx_three;
			CHECK_EQ(v3, ivec3(6, 15, 24));
			v3 -= 500;
			CHECK_EQ(v3, ivec3(-494, -485, -476));
			v3 -= v3.yzx;
			CHECK_EQ(v3, ivec3(-9, -9, 18));
			v3 -= v3.z;
			CHECK_EQ(v3, ivec3(-27, -27, 0));
		}

		SUBCASE("indexed_vector operator -=")
		{
			ivec3 v3(100, 200, 300);
			v3.zx -= 40;
			CHECK_EQ(v3, ivec3(60, 200, 260));
			v3.xy -= cx_three.zy;
			CHECK_EQ(v3, ivec3(54, 195, 260));
			v3.yz -= cx_two;
			CHECK_EQ(v3, ivec3(54, 188, 252));
			v3.xzy -= cx_four.w;
			CHECK_EQ(v3, ivec3(51, 185, 249));
		}
	}

	TEST_CASE("operator *=")
	{
		SUBCASE("basic_vector operator *=")
		{
			iscal v1(20);
			v1 *= 10;
			CHECK_EQ(v1, 200);
			v1 *= iscal(13);
			CHECK_EQ(v1, 2600);

			ivec3 v3(1, 2, 3);
			v3 *= cx_three;
			CHECK_EQ(v3, ivec3(4, 10, 18));
			v3 *= 5;
			CHECK_EQ(v3, ivec3(20, 50, 90));
			v3 *= v3.yzx;
			CHECK_EQ(v3, ivec3(1000, 4500, 1800));
			v3 *= v3.x;
			CHECK_EQ(v3, ivec3(1000000, 4500000, 1800000));
		}

		SUBCASE("indexed_vector operator *=")
		{
			ivec3 v3(1, 2, 3);
			v3.zx *= 40;
			CHECK_EQ(v3, ivec3(40, 2, 120));
			v3.xy *= cx_three.zy;
			CHECK_EQ(v3, ivec3(240, 10, 120));
			v3.yz *= cx_two;
			CHECK_EQ(v3, ivec3(240, 70, 960));
			v3.xzy *= cx_four.w;
			CHECK_EQ(v3, ivec3(720, 210, 2880));
		}
	}

	TEST_CASE("operator /=")
	{
		SUBCASE("basic_vector operator /=")
		{
			iscal v1(200);
			v1 /= 10;
			CHECK_EQ(v1, 20);
			v1 /= iscal(5);
			CHECK_EQ(v1, 4);

			ivec3 v3(1000, 2000, 3000);
			v3 /= cx_three;
			CHECK_EQ(v3, ivec3(250, 400, 500));
			v3 /= 5;
			CHECK_EQ(v3, ivec3(50, 80, 100));
			v3 /= v3.xyx;
			CHECK_EQ(v3, ivec3(1, 1, 2));
			v3 /= v3.x;
			CHECK_EQ(v3, ivec3(1, 1, 2));
		}

		SUBCASE("indexed_vector operator /=")
		{
			ivec3 v3(1000, 2000, 3000);
			v3.zx /= 40;
			CHECK_EQ(v3, ivec3(25, 2000, 75));
			v3.xy /= cx_three.yx;
			CHECK_EQ(v3, ivec3(5, 500, 75));
			v3.yz /= ivec2(100, 3);
			CHECK_EQ(v3, ivec3(5, 5, 25));
			v3.xzy /= cx_three.y;
			CHECK_EQ(v3, ivec3(1, 1, 5));
		}
	}

	TEST_CASE("operator %=")
	{
		SUBCASE("basic_vector operator %=")
		{
			iscal v1(20);
			v1 %= 13;
			CHECK_EQ(v1, 7);
			v1 %= iscal(5);
			CHECK_EQ(v1, 2);

			ivec3 v3(1000, 2000, 3000);
			v3 %= ivec3(157, 295, 429);
			CHECK_EQ(v3, ivec3(58, 230, 426));
			v3 %= 39;
			CHECK_EQ(v3, ivec3(19, 35, 36));
			v3 %= v3.xxx;
			CHECK_EQ(v3, ivec3(0, 16, 17));
			v3 %= v3.y;
			CHECK_EQ(v3, ivec3(0, 0, 1));
		}

		SUBCASE("indexed_vector operator %=")
		{
			ivec3 v3(1000, 2000, 3000);
			v3.zx %= 473;
			CHECK_EQ(v3, ivec3(54, 2000, 162));
			v3.xy %= cx_two.yx;
			CHECK_EQ(v3, ivec3(6, 5, 162));
			v3.yz %= ivec2(2, 100);
			CHECK_EQ(v3, ivec3(6, 1, 62));
			v3.xzy %= cx_three.y;
			CHECK_EQ(v3, ivec3(1, 1, 2));
		}
	}

	TEST_CASE("operator <<=")
	{
		SUBCASE("basic_vector operator <<=")
		{
			iscal v1(20);
			v1 <<= 3;
			CHECK_EQ(v1, 160);
			v1 <<= iscal(5);
			CHECK_EQ(v1, 5120);

			ivec3 v3(5, 8, 13);
			v3 <<= ivec3(4, 2, 5);
			CHECK_EQ(v3, ivec3(80, 32, 416));
			v3 <<= 3;
			CHECK_EQ(v3, ivec3(640, 256, 3328));
			v3 <<= cx_three.xzy;
			CHECK_EQ(v3, ivec3(10240, 16384, 106496));
			v3 <<= cx_four.w;
			CHECK_EQ(v3, ivec3(81920, 131072, 851968));
		}

		SUBCASE("indexed_vector operator <<=")
		{
			ivec3 v3(20, 30, 40);
			v3.zx <<= 4;
			CHECK_EQ(v3, ivec3(320, 30, 640));
			v3.xy <<= cx_two.yx;
			CHECK_EQ(v3, ivec3(81920, 3840, 640));
			v3.yz <<= ivec2(2, 5);
			CHECK_EQ(v3, ivec3(81920, 15360, 20480));
			v3.xzy <<= cx_four.w;
			CHECK_EQ(v3, ivec3(655360, 122880, 163840));
		}
	}

	TEST_CASE("operator >>=")
	{
		SUBCASE("basic_vector operator >>=")
		{
			iscal v1(1234);
			v1 >>= 3;
			CHECK_EQ(v1, 154);
			v1 >>= iscal(4);
			CHECK_EQ(v1, 9);

			ivec3 v3(655360, 122880, 163840);
			v3 >>= ivec3(4, 2, 5);
			CHECK_EQ(v3, ivec3(40960, 30720, 5120));
			v3 >>= 3;
			CHECK_EQ(v3, ivec3(5120, 3840, 640));
			v3 >>= cx_three.xzy;
			CHECK_EQ(v3, ivec3(320, 60, 20));
			v3 >>= cx_four.w;
			CHECK_EQ(v3, ivec3(40, 7, 2));
		}

		SUBCASE("indexed_vector operator >>=")
		{
			ivec3 v3(655360, 122880, 163840);
			v3.zx >>= 4;
			CHECK_EQ(v3, ivec3(40960, 122880, 10240));
			v3.xy >>= cx_two.yx;
			CHECK_EQ(v3, ivec3(160, 960, 10240));
			v3.yz >>= ivec2(2, 5);
			CHECK_EQ(v3, ivec3(160, 240, 320));
			v3.xzy >>= cx_four.w;
			CHECK_EQ(v3, ivec3(20, 30, 40));
		}
	}
	TEST_CASE("operator &=")
	{
		SUBCASE("basic_vector operator &=")
		{
			uscal v1(0x276a9d76);
			v1 &= 0x3810fc6au;
			CHECK_EQ(v1, 0x20009c62);
			v1 &= uscal(0x4609cee8);
			CHECK_EQ(v1, 0x00008c60);
			// 450c1f3a145623765a4403ceec142e7b
			uvec3 v3(0x6047ac15, 0x817f512d, 0x6711e1df);
			v3 &= uvec3(0xe37f3941, 0xcb433621, 0x74886c4c);
			CHECK_EQ(v3, uvec3(0x60472801, 0x81431021, 0x6400604c));
			v3 &= 0xceec142e;
			CHECK_EQ(v3, uvec3(0x40440000, 0x80401020, 0x4400000c));
			v3 &= uthree.xxx;
			CHECK_EQ(v3, uvec3(0x00400000, 0x80401020, 0x04000008));
			v3 &= ufour.z;
			CHECK_EQ(v3, uvec3(0x00400000, 0x00400020, 0x04000008));
		}

		SUBCASE("indexed_vector operator &=")
		{
			uvec3 v3(0x328ad958, 0x817f512d, 0x961d14e0);
			v3.zx &= 0x6de37037u;
			CHECK_EQ(v3, uvec3(0x20825010, 0x817f512d, 0x04011020));
			v3.xy &= utwo.yx;
			CHECK_EQ(v3, uvec3(0x20004010, 0x007e500d, 0x04011020));
			v3.yz &= uvec2(0xe37f3941, 0x2099f910);
			CHECK_EQ(v3, uvec3(0x20004010, 0x007e1001, 0x00011000));
			v3.xzy &= ufour.w;
			CHECK_EQ(v3, uvec3(0x00004000, 0x00300000, 0x00000000));
		}
	}

	TEST_CASE("operator |=")
	{
		SUBCASE("basic_vector operator |=")
		{
			uscal v1(0x276a9d76);
			v1 |= 0x3810fc6au;
			CHECK_EQ(v1, 0x3f7afd7e);
			v1 |= uscal(0x4609cee8);
			CHECK_EQ(v1, 0x7f7bfffe);

			uvec3 v3(0x6047ac15, 0x235bee1b, 0x6711e1df);
			v3 |= uvec3(0x0a067d16, 0xcb433621, 0x74886c4c);
			CHECK_EQ(v3, uvec3(0x6a47fd17, 0xeb5bfe3b, 0x7799eddf));
			v3 |= 0x22e838c0u;
			CHECK_EQ(v3, uvec3(0x6aeffdd7, 0xebfbfefb, 0x77f9fddf));
			v3 |= uthree.xxx;
			CHECK_EQ(v3, uvec3(0xeefffdff, 0xeffbfefb, 0xfff9fdff));
			v3 |= ufour.w;
			CHECK_EQ(v3, uvec3(0xfeffffff, 0xfffbfeff, 0xfff9ffff));
		}

		SUBCASE("indexed_vector operator |=")
		{
			uvec3 v3(0x328ad958, 0x817f512d, 0x961d14e0);
			v3.zx |= 0x6de37037u;
			CHECK_EQ(v3, uvec3(0x7febf97f, 0x817f512d, 0xffff74f7));
			v3.xy |= utwo.yx;
			CHECK_EQ(v3, uvec3(0xfffbfd7f, 0xa17ff57f, 0xffff74f7));
			v3.yz |= uvec2(0xe37f3941, 0x2099f910);
			CHECK_EQ(v3, uvec3(0xfffbfd7f, 0xe37ffd7f, 0xfffffdf7));
			v3.xzy |= ufour.w;
			CHECK_EQ(v3, uvec3(0xfffbff7f, 0xff7fff7f, 0xfffffff7));
		}
	}

	TEST_CASE("operator ^=")
	{
		SUBCASE("basic_vector operator ^=")
		{
			uscal v1(0x276a9d76);
			v1 ^= 0x3810fc6au;
			CHECK_EQ(v1, 0x1f7a611c);
			v1 ^= uscal(0x4609cee8);
			CHECK_EQ(v1, 0x5973aff4);

			uvec3 v3(0x6047ac15, 0x235bee1b, 0x6711e1df);
			v3 ^= uvec3(0x0a067d16, 0xcb433621, 0x74886c4c);
			CHECK_EQ(v3, uvec3(0x6a41d103, 0xe818d83a, 0x13998d93));
			v3 ^= 0x22e838c0u;
			CHECK_EQ(v3, uvec3(0x48a9e9c3, 0xcaf0e0fa, 0x3171b553));
			v3 ^= uthree.xxx;
			CHECK_EQ(v3, uvec3(0xe6f93da8, 0x64a03491, 0x9f216138));
			v3 ^= ufour.w;
			CHECK_EQ(v3, uvec3(0xbac9d3ec, 0x3890dad5, 0xc3118f7c));
		}

		SUBCASE("indexed_vector operator ^=")
		{
			uvec3 v3(0x328ad958, 0x817f512d, 0x961d14e0);
			v3.zx ^= 0x6de37037u;
			CHECK_EQ(v3, uvec3(0x5f69a96f, 0x817f512d, 0xfbfe64d7));
			v3.xy ^= utwo.yx;
			CHECK_EQ(v3, uvec3(0xba716d72, 0xa101a572, 0xfbfe64d7));
			v3.yz ^= uvec2(0xe37f3941, 0x2099f910);
			CHECK_EQ(v3, uvec3(0xba716d72, 0x427e9c33, 0xdb679dc7));
			v3.xzy ^= ufour.w;
			CHECK_EQ(v3, uvec3(0xe6418336, 0x1e4e7277, 0x87577383));
		}
	}
}
