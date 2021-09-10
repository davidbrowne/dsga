
//          Copyright David Browne 2020-2021.
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

TEST_SUITE("test conversions")
{
	constexpr ivec4	cx_four		(0, 1, 2, 3);
	constexpr ivec3	cx_three	(4, 5, 6);
	constexpr ivec2	cx_two		(7, 8);
	constexpr iscal	cx_one		(9);

	TEST_CASE("external types conversion")
	{
		SUBCASE("std::array")
		{
			auto val1 = from_vec(cx_three);
			auto val2 = std::array<int, 3>{4, 5, 6};
			auto val3 = std::array<int, 3>{5, 6, 4};

			auto val4 = to_vec(val3);

			auto val5 = to_vec(from_vec(cx_two));
			auto val6 = from_vec(to_vec(val1));

			CHECK_EQ(val1, val2);
			CHECK_NE(val1, val3);
		
			CHECK_EQ(val4, ivec3(5, 6, 4));
			CHECK_NE(val4, ivec3(4, 5, 6));

			CHECK_EQ(val5, cx_two);
			CHECK_EQ(val6, val1);
		}

		SUBCASE("C array")
		{
			int val1[4];

			copy_from_vec(std::span(val1), cx_four);
			CHECK_EQ(val1[0], 0);
			CHECK_EQ(val1[1], 1);
			CHECK_EQ(val1[2], 2);
			CHECK_EQ(val1[3], 3);

			for (int i = 0; i < 4; ++i)
				val1[i] -= 10;

			ivec4 val2;
			copy_to_vec(val2, std::span(val1));
			CHECK_EQ(val2, ivec4(-10, -9, -8, -7));

			auto val3 = to_vec(val1);
			CHECK_EQ(val3, ivec4(-10, -9, -8, -7));
		}

		SUBCASE("ad hoc")
		{
			// roll your own:
			//    store.value -- best not to use this (internal rep)
			//    data()/operator []/size()
			//    begin()/end()

			// using data() and some helper functions
			ivec4 vec_arr[4];
			constexpr std::array<double, 16> lotsa_data = { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };

			for (std::size_t i = 0; i < 4; ++i)
			{
				copy_to_vec(vec_arr[i], std::span(lotsa_data.data() + (i * 4), 4));
			}

			std::array<int, 16> give_me_data;
			std::array<int, 16> expected_result { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			for (std::size_t i = 0; i < 4; ++i)
			{
				copy_from_vec(std::span(give_me_data.data() + (i * 4), 4), vec_arr[i]);
			}

			CHECK_EQ(give_me_data, expected_result);

			// using operator[] and size()
			std::array<double, 16> val1;
			for (std::size_t i = 0; i < 4; ++i)
				for (std::size_t j = 0; j < vec_arr[i].size(); ++j)
					val1[4 * i + j] = vec_arr[i][j];

			CHECK_EQ(val1, lotsa_data);

			// uses begin()/end() for range-for loop
			ivec4 val2;
			auto iter = val2.begin();
			for (auto i : vec_arr[3])
			{
				*iter = i;
				++iter;
			}

			CHECK_EQ(val2, vec_arr[3]);
		}
	}

	TEST_CASE("use primitives to convert")
	{
		[[ maybe_unused ]] constexpr std::array<double, 16> const_float_data = { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15. };
		std::array<int, 16> non_const_int_data { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

		SUBCASE("1D basic_vector primitive conversions")
		{
			iscal simple_data(999);
			iscal simple_dest;

			constexpr iscal all_elevens(11);
			[[ maybe_unused ]] constexpr iscal all_sixty_sixes(66);

			// non-const begin()/end()
			std::copy(simple_data.begin(), simple_data.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, iscal(999));

			// const begin()/end()
			std::copy(cx_one.begin(), cx_one.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, iscal(9));

			// non-const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = non_const_int_data[i];
			}
			CHECK_EQ(simple_dest, iscal(0));

			// range/for non-const begin()/end()
			for (auto &val : simple_dest)
			{
				val = 37;
			}
			CHECK_EQ(simple_dest, iscal(37));

			// range/for const begin()/end()
			std::size_t index = 0;
			for (auto val : all_elevens)
			{
				simple_dest[index] = val;
				++index;
			}
			CHECK_EQ(simple_dest, iscal(11));

			// const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = static_cast<int>(cx_one[i]);
			}
			CHECK_EQ(simple_dest, iscal(9));

			// tuple interface -- structured bindings and get<>
			auto [a] = simple_data;
			CHECK_EQ(a, 999);
			CHECK_EQ(a, get<0>(simple_data));

			const iscal get_tester(-10);

			CHECK_EQ(-10, get<0>(get_tester));
			CHECK_EQ(105, get<0>(iscal(105)));
			CHECK_EQ(-10, get<0>(std::move(get_tester)));
		}

		SUBCASE("2D basic_vector primitive conversions")
		{
			ivec2 simple_data(999, 9999);
			ivec2 simple_dest;

			constexpr ivec2 all_elevens(11);
			[[ maybe_unused ]] constexpr ivec2 all_sixty_sixes(66);

			// non-const begin()/end()
			std::copy(simple_data.begin(), simple_data.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, ivec2(999, 9999));

			// const begin()/end()
			std::copy(cx_two.begin(), cx_two.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, ivec2(7, 8));

			// non-const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = non_const_int_data[i];
			}
			CHECK_EQ(simple_dest, ivec2(0, 1));

			// range/for non-const begin()/end()
			for (auto &val : simple_dest)
			{
				val = 37;
			}
			CHECK_EQ(simple_dest, ivec2(37, 37));

			// range/for const begin()/end()
			std::size_t index = 0;
			for (auto val : all_elevens)
			{
				simple_dest[index] = val;
				++index;
			}
			CHECK_EQ(simple_dest, ivec2(11, 11));

			// const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = static_cast<int>(cx_two[i]);
			}
			CHECK_EQ(simple_dest, ivec2(7, 8));

			// tuple interface -- structured bindings and get<>
			auto [a, b] = simple_data;
			CHECK_EQ(a, 999);
			CHECK_EQ(b, 9999);
			CHECK_EQ(a, get<0>(simple_data));
			CHECK_EQ(b, get<1>(simple_data));

			const ivec2 get_tester(-10, -20);

			CHECK_EQ(-10, get<0>(get_tester));
			CHECK_EQ(104, get<1>(ivec2(105, 104)));
			CHECK_EQ(-20, get<1>(std::move(get_tester)));
		}

		SUBCASE("3D basic_vector primitive conversions")
		{
			ivec3 simple_data(999, 9999, 99999);
			ivec3 simple_dest;

			constexpr ivec3 all_elevens(11);
			[[ maybe_unused ]] constexpr ivec3 all_sixty_sixes(66);

			// non-const begin()/end()
			std::copy(simple_data.begin(), simple_data.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, ivec3(999, 9999, 99999));

			// const begin()/end()
			std::copy(cx_three.begin(), cx_three.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, ivec3(4, 5, 6));

			// non-const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = non_const_int_data[i];
			}
			CHECK_EQ(simple_dest, ivec3(0, 1, 2));

			// range/for non-const begin()/end()
			for (auto &val : simple_dest)
			{
				val = 37;
			}
			CHECK_EQ(simple_dest, ivec3(37, 37, 37));

			// range/for const begin()/end()
			std::size_t index = 0;
			for (auto val : all_elevens)
			{
				simple_dest[index] = val;
				++index;
			}
			CHECK_EQ(simple_dest, ivec3(11, 11, 11));

			// const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = static_cast<int>(cx_three[i]);
			}
			CHECK_EQ(simple_dest, ivec3(4, 5, 6));

			// tuple interface -- structured bindings and get<>
			auto [a, b, c] = simple_data;
			CHECK_EQ(a, 999);
			CHECK_EQ(b, 9999);
			CHECK_EQ(c, 99999);
			CHECK_EQ(a, get<0>(simple_data));
			CHECK_EQ(b, get<1>(simple_data));
			CHECK_EQ(c, get<2>(simple_data));

			const ivec3 get_tester(-10, -20, -30);

			CHECK_EQ(-10, get<0>(get_tester));
			CHECK_EQ(104, get<1>(ivec3(105, 104, 103)));
			CHECK_EQ(-30, get<2>(std::move(get_tester)));
		}

		SUBCASE("4D basic_vector primitive conversions")
		{
			ivec4 simple_data(999, 9999, 99999, 999999);
			ivec4 simple_dest;

			constexpr ivec4 all_elevens(11);
			[[ maybe_unused ]] constexpr ivec4 all_sixty_sixes(66);

			// non-const begin()/end()
			std::copy(simple_data.begin(), simple_data.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, ivec4(999, 9999, 99999, 999999));

			// const begin()/end()
			std::copy(cx_four.begin(), cx_four.end(), simple_dest.begin());
			CHECK_EQ(simple_dest, ivec4(0, 1, 2, 3));

			// non-const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = non_const_int_data[i];
			}
			CHECK_EQ(simple_dest, ivec4(0, 1, 2, 3));

			// range/for non-const begin()/end()
			for (auto &val : simple_dest)
			{
				val = 37;
			}
			CHECK_EQ(simple_dest, ivec4(37, 37, 37, 37));

			// range/for const begin()/end()
			std::size_t index = 0;
			for (auto val : all_elevens)
			{
				simple_dest[index] = val;
				++index;
			}
			CHECK_EQ(simple_dest, ivec4(11, 11, 11, 11));

			// const operator[]
			for (std::size_t i = 0; i < simple_dest.size(); ++i)
			{
				simple_dest[i] = static_cast<int>(cx_four[i]);
			}
			CHECK_EQ(simple_dest, ivec4(0, 1, 2, 3));

			// tuple interface -- structured bindings and get<>
			auto [a, b, c, d] = simple_data;
			CHECK_EQ(a, 999);
			CHECK_EQ(b, 9999);
			CHECK_EQ(c, 99999);
			CHECK_EQ(d, 999999);
			CHECK_EQ(a, get<0>(simple_data));
			CHECK_EQ(b, get<1>(simple_data));
			CHECK_EQ(c, get<2>(simple_data));
			CHECK_EQ(d, get<3>(simple_data));

			const ivec4 get_tester(-10, -20, -30, -40);

			CHECK_EQ(-20, get<1>(get_tester));
			CHECK_EQ(103, get<2>(ivec4(105, 104, 103, 102)));
			CHECK_EQ(-40, get<3>(std::move(get_tester)));
		}

		SUBCASE("1D indexed_vector primitive conversions")
		{
			iscal somevec(50);

			int increment = 3;
			for (auto &val : somevec.x)
			{
				val += increment;
				++increment;
			}

			CHECK_EQ(somevec, iscal(53));

			iscal anothervec;
			std::copy(somevec.x.cbegin(), somevec.x.cend(), anothervec.begin());
			CHECK_EQ(anothervec, iscal(53));

			auto [a] = anothervec.x;
			CHECK_EQ(a, 53);

			const iscal get_tester(-10);
			CHECK_EQ(53, get<0>(anothervec.x));
			CHECK_EQ(-10, get<0>(get_tester.x));
			CHECK_EQ(105, get<0>(iscal(105).x));
			CHECK_EQ(-10, get<0>(std::move(get_tester.x)));
		}

		SUBCASE("2D indexed_vector primitive conversions")
		{
			ivec2 somevec(50, 60);

			int increment = 3;
			for (auto &val : somevec.yx)
			{
				val += increment;
				++increment;
			}

			CHECK_EQ(somevec, ivec2(54, 63));

			ivec2 anothervec;
			std::copy(somevec.yx.cbegin(), somevec.yx.cend(), anothervec.begin());
			CHECK_EQ(anothervec, ivec2(63, 54));

			auto [a, b] = anothervec.xy;
			CHECK_EQ(a, 63);
			CHECK_EQ(b, 54);

			const ivec2 get_tester(-10, -20);
			CHECK_EQ(54, get<0>(anothervec.yx));
			CHECK_EQ(-10, get<1>(get_tester.yx));
			CHECK_EQ(104, get<1>(ivec2(105, 104).xy));
			CHECK_EQ(-20, get<0>(std::move(get_tester.yx)));
		}

		SUBCASE("3D indexed_vector primitive conversions")
		{
			ivec3 somevec(50, 60, 70);

			int increment = 3;
			for (auto &val : somevec.xzy)
			{
				val += increment;
				++increment;
			}

			CHECK_EQ(somevec, ivec3(53, 65, 74));

			ivec3 anothervec;
			std::copy(somevec.zxy.cbegin(), somevec.zxy.cend(), anothervec.begin());
			CHECK_EQ(anothervec, ivec3(74, 53, 65));

			auto [a, b, c] = anothervec.yxz;
			CHECK_EQ(a, 53);
			CHECK_EQ(b, 74);
			CHECK_EQ(c, 65);

			const ivec3 get_tester(-10, -20, -30);
			CHECK_EQ(74, get<0>(anothervec.xyz));
			CHECK_EQ(-10, get<1>(get_tester.yxz));
			CHECK_EQ(104, get<2>(ivec3(105, 104, 103).xzy));
			CHECK_EQ(-20, get<0>(std::move(get_tester.yzx)));
		}

		SUBCASE("4D indexed_vector primitive conversions")
		{
			ivec4 somevec(50, 60, 70, 80);

			int increment = 3;
			for (auto &val : somevec.wxzy)
			{
				val += increment;
				++increment;
			}

			CHECK_EQ(somevec, ivec4(54, 66, 75, 83));

			ivec4 anothervec;
			std::copy(somevec.zxwy.cbegin(), somevec.zxwy.cend(), anothervec.begin());
			CHECK_EQ(anothervec, ivec4(75, 54, 83, 66));

			auto [a, b, c, d] = anothervec.ywxz;
			CHECK_EQ(a, 54);
			CHECK_EQ(b, 66);
			CHECK_EQ(c, 75);
			CHECK_EQ(d, 83);

			const ivec4 get_tester(-10, -20, -30, -40);
			CHECK_EQ(75, get<0>(anothervec.xywz));
			CHECK_EQ(-40, get<1>(get_tester.ywxz));
			CHECK_EQ(104, get<2>(ivec4(105, 104, 103, 102).zzyw));
			CHECK_EQ(-30, get<3>(std::move(get_tester.wxyz)));
		}
	}

	TEST_CASE("indexed_vector conversions")
	{
		SUBCASE("1D indexed_vector conversions")
		{
			// the three conversion operators

			int val1 = cx_three.y;					// #1 to scalar type
			iscal val2 = cx_three.x;				// #2 implicit conversion operator
			fscal val3 = fscal(cx_three.z);			// #3 explicit cast needed to invoke conversion operator

			CHECK_EQ(val1, 5);
			CHECK_NE(val1, 4);

			CHECK_EQ(val2, iscal(4));
			CHECK_NE(val2, iscal(0));

			CHECK_EQ(val3, fscal(6.0f));
			CHECK_NE(val3, fscal(4.0f));
		}

		SUBCASE("2D indexed_vector conversions")
		{
			// the two conversion operators

			ivec2 val1 = cx_three.yz;				// #1 implicit conversion
			fvec2 val2 = fvec2(cx_three.xy);		// #2 explicit cast needed to invoke conversion operator

			CHECK_EQ(val1, ivec2(5, 6));
			CHECK_NE(val1, ivec2(4, 5));

			CHECK_EQ(val2, fvec2(4.0f, 5.0f));
			CHECK_NE(val2, fvec2(6.0f, 4.0f));
		}

		SUBCASE("3D indexed_vector conversions")
		{
			// the two conversion operators

			ivec3 val1 = cx_three.yzy;				// #1 implicit conversion
			fvec3 val2 = fvec3(cx_three.zzx);		// #2 explicit cast needed to invoke conversion operator

			CHECK_EQ(val1, ivec3(5, 6, 5));
			CHECK_NE(val1, ivec3(4, 5, 6));

			CHECK_EQ(val2, fvec3(6.0f, 6.0f, 4.0f));
			CHECK_NE(val2, fvec3(4.0f, 5.0f, 6.0f));
		}

		SUBCASE("4D indexed_vector conversions")
		{
			// the two conversion operators

			ivec4 val1 = cx_three.yzyx;				// #1 implicit conversion
			fvec4 val2 = fvec4(cx_three.zxyy);		// #2 explicit cast needed to invoke conversion operator

			CHECK_EQ(val1, ivec4(5, 6, 5, 4));
			CHECK_NE(val1, ivec4(4, 5, 6, 6));

			CHECK_EQ(val2, fvec4(6.0f, 4.0f, 5.0f, 5.0f));
			CHECK_NE(val2, fvec4(6.0f, 6.0f, 5.0f, 5.0f));
		}
	}

	TEST_CASE("conversion operators (1D only)")
	{
		// the only basic_vector with conversion operators is
		// for 1D, as we want them to mimic the scalar type.

		int val1 = cx_one;							// #1 implicit conversion
		[[maybe_unused]] float f2 = cx_one;							// #1 implicit conversion

		fscal fone = 9.9f;
		int val2 = static_cast<int>(fone);			// #2 explicit conversion

		CHECK_EQ(val1, 9);
		CHECK_NE(val1, 0);

		CHECK_EQ(val2, 9);
		CHECK_NE(val2, 0);
	}

	TEST_CASE("basic_vector conversion constructors")
	{
		// conversion constructors remove the need for conversion operators.
		// conversion operators can also cause ambiguities with the conversion constructors.

		SUBCASE("1D conversion constructor")
		{
			auto i1 = iscal(22);
			auto f1 = [](iscal &iv) -> fscal { return iv; } (i1);

			CHECK_EQ(f1, fscal(22.0f));
			CHECK_NE(f1, fscal(0.0f));
		}

		SUBCASE("2D conversion constructor")
		{
			auto i2 = ivec2(101, 202);
			auto f2 = [](ivec2 &iv) -> fvec2 { return iv; } (i2);

			CHECK_EQ(f2, fvec2(101.0f, 202.0f));
			CHECK_NE(f2, fvec2(101.0f));
		}

		SUBCASE("3D conversion constructor")
		{
			auto i3 = ivec3(47, 57, 67);
			auto f3 = [](ivec3 &iv) -> fvec3 { return iv; } (i3);

			CHECK_EQ(f3, fvec3(47.0f, 57.0f, 67.0f));
			CHECK_NE(f3, fvec3(47.0f));
		}

		SUBCASE("4D conversion constructor")
		{
			auto i4 = cx_four;
			auto f4 = [](ivec4 &iv) -> fvec4 { return iv; } (i4);

			CHECK_EQ(f4, fvec4(0.0f, 1.0f, 2.0f, 3.0f));
			CHECK_NE(f4, fvec4(0.0f));
		}
	}
}
