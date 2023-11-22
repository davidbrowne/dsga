
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


template <bool W1, dsga::dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
requires W1
constexpr void pointer_interface_copy(dsga::vector_base<W1, T, C, D1> &dest, const dsga::vector_base<W2, T, C, D2> &src) noexcept
{
	[&] <std::size_t ...Is, std::size_t ...Js>(std::index_sequence<Is ...>, std::index_sequence<Js ...> /* dummy */) noexcept
	{
		((dest.data()[Is] = src.data()[Js]), ...);
	}(D1::sequence(), D2::sequence());
}

template <bool W1, dsga::dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
requires W1
constexpr void iterator_interface_copy(dsga::vector_base<W1, T, C, D1> &dest, const dsga::vector_base<W2, T, C, D2> &src) noexcept
{
	auto dest_iter = dest.begin();
	auto src_iter = src.cbegin();
	while (src_iter != src.cend())
		*dest_iter++ = *src_iter++;
}

// test swap() for basic_vector. basic_Vector swap() uses storage_wrapper.swap() under the hood
// swap() doesn't work as well conceptually for indexed_vector
TEST_SUITE("vector swap")
{
	TEST_CASE("1d swap")
	{
		auto i1 = dsga::iscal{11};
		auto i2 = dsga::iscal{100};

		i1.swap(i2);
		CHECK_EQ(i1, dsga::iscal{100});
		CHECK_EQ(i2, dsga::iscal{11});

		swap(i1, i2);
		CHECK_EQ(i1, dsga::iscal{11});
		CHECK_EQ(i2, dsga::iscal{100});

		std::ranges::swap(i1, i2);
		CHECK_EQ(i1, dsga::iscal{100});
		CHECK_EQ(i2, dsga::iscal{11});

		std::swap(i1, i2);
		CHECK_EQ(i1, dsga::iscal{11});
		CHECK_EQ(i2, dsga::iscal{100});
	}

	TEST_CASE("2d swap")
	{
		auto i1 = dsga::ivec2{11, 22};
		auto i2 = dsga::ivec2{100, 200};

		i1.swap(i2);
		CHECK_EQ(i1, dsga::ivec2{100, 200});
		CHECK_EQ(i2, dsga::ivec2{11, 22});

		swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec2{11, 22});
		CHECK_EQ(i2, dsga::ivec2{100, 200});

		std::ranges::swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec2{100, 200});
		CHECK_EQ(i2, dsga::ivec2{11, 22});

		std::swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec2{11, 22});
		CHECK_EQ(i2, dsga::ivec2{100, 200});
	}

	TEST_CASE("3d swap")
	{
		auto i1 = dsga::ivec3{11, 22, 33};
		auto i2 = dsga::ivec3{100, 200, 300};

		i1.swap(i2);
		CHECK_EQ(i1, dsga::ivec3{100, 200, 300});
		CHECK_EQ(i2, dsga::ivec3{11, 22, 33});

		swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec3{11, 22, 33});
		CHECK_EQ(i2, dsga::ivec3{100, 200, 300});

		std::ranges::swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec3{100, 200, 300});
		CHECK_EQ(i2, dsga::ivec3{11, 22, 33});

		std::swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec3{11, 22, 33});
		CHECK_EQ(i2, dsga::ivec3{100, 200, 300});
	}

	TEST_CASE("4d swap")
	{
		auto i1 = dsga::ivec4{11, 22, 33, 44};
		auto i2 = dsga::ivec4{100, 200, 300, 400};

		i1.swap(i2);
		CHECK_EQ(i1, dsga::ivec4{100, 200, 300, 400});
		CHECK_EQ(i2, dsga::ivec4{11, 22, 33, 44});

		swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec4{11, 22, 33, 44});
		CHECK_EQ(i2, dsga::ivec4{100, 200, 300, 400});

		std::ranges::swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec4{100, 200, 300, 400});
		CHECK_EQ(i2, dsga::ivec4{11, 22, 33, 44});

		std::swap(i1, i2);
		CHECK_EQ(i1, dsga::ivec4{11, 22, 33, 44});
		CHECK_EQ(i2, dsga::ivec4{100, 200, 300, 400});
	}
}


// compund assignments are implemented without regard to any specific dimension, so we can test generically
TEST_SUITE("test assignment")
{
	constexpr ivec4	cx_four		(0, 1, 2, 3);
	constexpr ivec3	cx_three	(4, 5, 6);
	constexpr ivec2	cx_two		(7, 8);

	constexpr uvec2 utwo(0x207ef45f, 0xe518c41d);
	constexpr uvec3 uthree(0xae50d46b, 0x10712fd0, 0x47946919);
	constexpr uvec4 ufour(0x686e22e1, 0x4b79b211, 0x5f70e079, 0x5c30ee44);

	// use vector_base functions sequence() and data() to perform assignment
	TEST_CASE("pointer interface assignment")
	{
		const auto src = vec4(100, 200, 300, 400);
		auto dest = vec4(0);

		pointer_interface_copy(dest, src);
		CHECK_EQ(dest, src);

		pointer_interface_copy(dest.wzyx, src);
		CHECK_EQ(dest, vec4(400, 300, 200, 100));

		pointer_interface_copy(dest, src.wzyx);
		CHECK_EQ(dest, vec4(400, 300, 200, 100));

		pointer_interface_copy(dest.xzwy, src.wyxz);
		CHECK_EQ(dest, vec4(400, 300, 200, 100));
	}

	// use iterators from vector_base to perform assignment
	TEST_CASE("iterator interface assignment")
	{
		const auto src = vec4(100, 200, 300, 400);
		auto dest = vec4(0);

		iterator_interface_copy(dest, src);
		CHECK_EQ(dest, src);

		iterator_interface_copy(dest.wzyx, src);
		CHECK_EQ(dest, vec4(400, 300, 200, 100));

		iterator_interface_copy(dest, src.wzyx);
		CHECK_EQ(dest, vec4(400, 300, 200, 100));

		iterator_interface_copy(dest.xzwy, src.wyxz);
		CHECK_EQ(dest, vec4(400, 300, 200, 100));
	}

	// all standard assignment *is* implemented inside the class, so each dimension of basic_vector has its own implementation
	TEST_CASE("vector standard assignment")
	{
		SUBCASE("1D standard assignment =")
		{
			iscal v1(97);
			ivec4 v4(17);
			ivec3 v3(44);
			dscal d1(88);
			dvec3 d3(22);
			int i = 30;

			v4.y = 25;
			v1 = v4.w;
			d1 = 909;
			i = static_cast<int>(d1.x);
			d3.y = v3.z;
			d3.x = d1.x;

			CHECK_EQ(v1, iscal(17));
			CHECK_EQ(v4, ivec4(17, 25, 17, 17));
			CHECK_EQ(d1, 909);
			CHECK_EQ(i, 909);
			CHECK_EQ(d3, dvec3(909, 44, 22));
		}

		SUBCASE("2D standard assignment =")
		{
			ivec2 v2(83);
			ivec4 v4(17);
			dvec3 d3(22);

			v4.yw = v2;
			v4.xz = ivec2(10, 20);
			v2 = v4.xy;
			d3.yz = v2.yx;

			CHECK_EQ(v2, ivec2(10, 83));
			CHECK_EQ(v4, ivec4(10, 83, 20, 83));
			CHECK_EQ(d3, dvec3(22, 83, 10));
		}

		SUBCASE("3D standard assignment =")
		{
			ivec3 v3(71);
			ivec4 v4(17);
			dvec4 d4(44);

			d4.xwy = v4.zzy;

			v4.wzx = v3;
			CHECK_EQ(v4, ivec4(71, 17, 71, 71));
			v3 = ivec3(33, 44, 55);
			v4.xyz = v3.yxy;
			CHECK_EQ(v4, ivec4(44, 33, 44, 71));
			v3 = v4.zyw;
			CHECK_EQ(v3, ivec3(44, 33, 71));
			CHECK_EQ(d4, dvec4(17, 17, 44, 17));
		}

		SUBCASE("4D standard assignment =")
		{
			ivec2 v2(83, 29);
			ivec3 v3(71, 10, 4);
			ivec4 v4(17, 38, 56, 95);
			dvec4 d4(44);

			d4.xyzw = v4.yzwx;

			v4 = v4.wzyx;
			CHECK_EQ(v4, ivec4(95, 56, 38, 17));
			v4.ywxz = v3.xyzx;
			CHECK_EQ(v4, ivec4(4, 71, 71, 10));
			v4 = ivec4(v3, v2.y);
			CHECK_EQ(v4, ivec4(71, 10, 4, 29));
			v4.zwyx = v4;
			CHECK_EQ(v4, ivec4(29, 4, 71, 10));
			CHECK_EQ(d4, dvec4(38, 56, 95, 17));
		}
	}

	TEST_CASE("vector operator +=")
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

	TEST_CASE("vector operator -=")
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

	TEST_CASE("vector operator *=")
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

	TEST_CASE("vector operator /=")
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

	TEST_CASE("vector operator %=")
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

	TEST_CASE("vector operator <<=")
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

	TEST_CASE("vector operator >>=")
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
	TEST_CASE("vector operator &=")
	{
		SUBCASE("basic_vector operator &=")
		{
			uscal v1(0x276a9d76);
			v1 &= 0x3810fc6au;
			CHECK_EQ(v1, 0x20009c62);
			v1 &= uscal(0x4609cee8);
			CHECK_EQ(v1, 0x00008c60);

			uvec3 v3(0x6047ac15, 0x817f512d, 0x6711e1df);
			v3 &= uvec3(0xe37f3941, 0xcb433621, 0x74886c4c);
			CHECK_EQ(v3, uvec3(0x60472801, 0x81431021, 0x6400604c));
			v3 &= 0xceec142eu;
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

	TEST_CASE("vector operator |=")
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

	TEST_CASE("vector operator ^=")
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

	TEST_CASE("matrix standard assignment =")
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

		SUBCASE("mat2x2 standard assignment =")
		{
			auto mat = mat2x2(m4x4);
			mat = m2x2;
			CHECK_EQ(mat, m2x2);
		}

		SUBCASE("mat2x3 standard assignment =")
		{
			auto mat = mat2x3(m4x4);
			mat = m2x3;
			CHECK_EQ(mat, m2x3);
		}

		SUBCASE("mat2x4 standard assignment =")
		{
			auto mat = mat2x4(m4x4);
			mat = m2x4;
			CHECK_EQ(mat, m2x4);
		}

		SUBCASE("mat3x2 standard assignment =")
		{
			auto mat = mat3x2(m4x4);
			mat = m3x2;
			CHECK_EQ(mat, m3x2);
		}

		SUBCASE("mat3x3 standard assignment =")
		{
			auto mat = mat3x3(m4x4);
			mat = m3x3;
			CHECK_EQ(mat, m3x3);
		}

		SUBCASE("mat3x4 standard assignment =")
		{
			auto mat = mat3x4(m4x4);
			mat = m3x4;
			CHECK_EQ(mat, m3x4);
		}

		SUBCASE("mat4x2 standard assignment =")
		{
			auto mat = mat4x2(m4x4);
			mat = m4x2;
			CHECK_EQ(mat, m4x2);
		}

		SUBCASE("mat4x3 standard assignment =")
		{
			auto mat = mat4x3(m4x4);
			mat = m4x3;
			CHECK_EQ(mat, m4x3);
		}

		SUBCASE("mat4x4 standard assignment =")
		{
			auto mat = mat4x4(m2x2);
			mat = m4x4;
			CHECK_EQ(mat, m4x4);
		}
	}
}
