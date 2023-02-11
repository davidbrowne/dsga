
//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
using namespace dsga;

#include <span>

#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


// fill vectors from spans

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<U, T>
constexpr void copy_to_vector(dsga::basic_vector<T, S> &lhs, std::span<U, E> rhs)
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<U, T>
constexpr void copy_to_vector(dsga::basic_vector<T, S> &lhs, std::span<U, E> rhs)
{
	const std::size_t count = std::min(S, rhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

// fill spans from vectors

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<T, U>
constexpr void copy_from_vector(std::span<U, E> lhs, const dsga::basic_vector<T, S> &rhs)
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<T, U>
constexpr void copy_from_vector(std::span<U, E> lhs, const dsga::basic_vector<T, S> &rhs)
{
	const std::size_t count = std::min(S, lhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

// demonstration of ability to use dsga::vector_base::sequence() and dsga::vector_base::data() to
// get all the data out of the vector, no matter if vec is a dsga::basic_vector or dsga::indexed_vector.
// this is a manual pointer and offset way to access data as opposed to to using dsga::vector_base::operator [].
template <bool W, typename T, std::size_t C, typename D>
constexpr std::array<T, C> from_vector_by_data_sequence(const dsga::vector_base<W, T, C, D> &vec) noexcept
{
	return [ptr = vec.data()]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept -> std::array<T, C>
	{
		return { ptr[Is]... };		// equivalent to return {*(ptr + Is)...};
	}(vec.sequence());
}


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
			auto val1 = to_array(cx_three);
			auto val2 = std::array<int, 3>{4, 5, 6};
			auto val3 = std::array<int, 3>{5, 6, 4};

			auto val4 = to_vector(val3);

			auto val5 = to_vector(to_array(cx_two));
			auto val6 = to_array(to_vector(val1));

			CHECK_EQ(val1, val2);
			CHECK_NE(val1, val3);
		
			CHECK_EQ(val4, ivec3(5, 6, 4));
			CHECK_NE(val4, ivec3(4, 5, 6));

			CHECK_EQ(val5, cx_two);
			CHECK_EQ(val6, val1);

			// using data() and sequence()

			CHECK_EQ(from_vector_by_data_sequence(cx_one), cx_one.base.store);
			CHECK_EQ(from_vector_by_data_sequence(cx_two.y), std::array{8});

			CHECK_EQ(from_vector_by_data_sequence(cx_two), cx_two.base.store);
			CHECK_EQ(from_vector_by_data_sequence(cx_two.xx), std::array{7, 7});

			CHECK_EQ(from_vector_by_data_sequence(cx_three), cx_three.base.store);
			CHECK_EQ(from_vector_by_data_sequence(cx_two.yxx), std::array{8, 7, 7});

			CHECK_EQ(from_vector_by_data_sequence(cx_four), cx_four.base.store);
			CHECK_EQ(from_vector_by_data_sequence(cx_two.yxyx), std::array{8, 7, 8, 7});
		}

		SUBCASE("C array")
		{
			int val1[4];

			copy_from_vector(std::span(val1), cx_four);
			CHECK_EQ(val1[0], 0);
			CHECK_EQ(val1[1], 1);
			CHECK_EQ(val1[2], 2);
			CHECK_EQ(val1[3], 3);

			for (int &val : val1)
				val -= 10;

			ivec4 val2;
			copy_to_vector(val2, std::span(val1));
			CHECK_EQ(val2, ivec4(-10, -9, -8, -7));

			auto val3 = to_vector(val1);
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
				copy_to_vector(vec_arr[i], std::span(lotsa_data.data() + (i * 4), 4));
			}

			std::array<int, 16> give_me_data{};
			std::array<int, 16> expected_result { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			for (std::size_t i = 0; i < 4; ++i)
			{
				copy_from_vector(std::span(give_me_data.data() + (i * 4), 4), vec_arr[i]);
			}

			CHECK_EQ(give_me_data, expected_result);

			// using operator[] and size()
			std::array<double, 16> val1{};
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

			//
			// wrap a span around a basic_vector
			//

			ivec4 dest{};
			const ivec4 src(100, 200, 300, 400);
			auto dest_iter = dest.begin();
			auto span_src = std::span(src);
			auto src_iter = span_src.rbegin();
			while (src_iter != span_src.rend())
				*dest_iter++ = *src_iter++;

			CHECK_EQ(dest, ivec4(400, 300, 200, 100));
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
			CHECK_EQ(-10, get<0>(get_tester));
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
			CHECK_EQ(-20, get<1>(get_tester));
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
			CHECK_EQ(-30, get<2>(get_tester));
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
			CHECK_EQ(-40, get<3>(get_tester));
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
			CHECK_EQ(-10, get<0>(get_tester.x));
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
			CHECK_EQ(-20, get<0>(get_tester.yx));
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
			CHECK_EQ(-20, get<0>(get_tester.yzx));
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
			CHECK_EQ(-30, get<3>(get_tester.wxyz));
		}
	}

	TEST_CASE("indexed_vector conversions")
	{
		SUBCASE("1D indexed_vector conversions")
		{
			// the three conversion operators

			int val1 = cx_three.y;							// implicit conversion
			dscal val2{1};
			float val3 = static_cast<float>(cx_three.z);	// explicit conversion
			auto val4 = std::asin(val2.x);			// implicit conversion

			CHECK_EQ(val1, 5);
			CHECK_NE(val1, 4);

			CHECK_EQ(val2, 1.0);
			CHECK_NE(val2, iscal(0));

			CHECK_EQ(val3, fscal(6.0f));
			CHECK_NE(val3, fscal(4.0f));

			CHECK_EQ(val4, std::asin(1.0));
		}

		SUBCASE("2D indexed_vector conversions")
		{
			// the two conversion operators

			ivec2 val1 = cx_three.yz;						// #1 implicit conversion
			auto val2 = fvec2(cx_three.xy);		// #2 explicit cast needed to invoke conversion operator

			CHECK_EQ(val1, ivec2(5, 6));
			CHECK_NE(val1, ivec2(4, 5));

			CHECK_EQ(val2, fvec2(4.0f, 5.0f));
			CHECK_NE(val2, fvec2(6.0f, 4.0f));
		}

		SUBCASE("3D indexed_vector conversions")
		{
			// the two conversion operators

			ivec3 val1 = cx_three.yzy;						// #1 implicit conversion
			auto val2 = fvec3(cx_three.zzx);		// #2 explicit cast needed to invoke conversion operator

			CHECK_EQ(val1, ivec3(5, 6, 5));
			CHECK_NE(val1, ivec3(4, 5, 6));

			CHECK_EQ(val2, fvec3(6.0f, 6.0f, 4.0f));
			CHECK_NE(val2, fvec3(4.0f, 5.0f, 6.0f));
		}

		SUBCASE("4D indexed_vector conversions")
		{
			// the two conversion operators

			ivec4 val1 = cx_three.yzyx;						// #1 implicit conversion
			auto val2 = fvec4(cx_three.zxyy);		// #2 explicit cast needed to invoke conversion operator

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

		const int val1 = cx_one;										// implicit conversion
		[[maybe_unused]] float f2 = static_cast<float>(cx_one);			// explicit conversion

		const fscal fone = 9.9f;
		const int val2 = static_cast<int>(fone);						// explicit conversion

		dscal dval = 1.0;
		auto val3 = std::asin(dval);							// implicit conversion

		CHECK_EQ(val1, 9);
		CHECK_NE(val1, 0);

		CHECK_EQ(val2, 9);
		CHECK_NE(val2, 0);

		CHECK_EQ(val3, std::asin(1.0));
	}

	TEST_CASE("basic_vector conversion constructors")
	{
		// conversion constructors remove the need for conversion operators.
		// conversion operators can also cause ambiguities with the conversion constructors.

		SUBCASE("1D conversion constructor")
		{
			auto i1 = iscal(22);
			auto f1 = [](const iscal &iv) -> fscal { return iv; } (i1);

			CHECK_EQ(f1, fscal(22.0f));
			CHECK_NE(f1, fscal(0.0f));
		}

		SUBCASE("2D conversion constructor")
		{
			auto i2 = ivec2(101, 202);
			auto f2 = [](const ivec2 &iv) -> fvec2 { return iv; } (i2);

			CHECK_EQ(f2, fvec2(101.0f, 202.0f));
			CHECK_NE(f2, fvec2(101.0f));
		}

		SUBCASE("3D conversion constructor")
		{
			auto i3 = ivec3(47, 57, 67);
			auto f3 = [](const ivec3 &iv) -> fvec3 { return iv; } (i3);

			CHECK_EQ(f3, fvec3(47.0f, 57.0f, 67.0f));
			CHECK_NE(f3, fvec3(47.0f));
		}

		SUBCASE("4D conversion constructor")
		{
			auto i4 = cx_four;
			auto f4 = [](const ivec4 &iv) -> fvec4 { return iv; } (i4);

			CHECK_EQ(f4, fvec4(0.0f, 1.0f, 2.0f, 3.0f));
			CHECK_NE(f4, fvec4(0.0f));
		}
	}

	// this test case is more about compiling without warnings or errors
	TEST_CASE("Implicit Conversions, construction and assignment, Section 4.1.10")
	{
		constexpr iscal i1{-5};
		constexpr ivec2 i2{-4, 4};
		constexpr ivec3 i3{2, -3, 0};
		constexpr ivec4 i4{-10, -8, 16, 100};

		constexpr uscal u1{3};
		constexpr uvec2 u2{1, 12};
		constexpr uvec3 u3{30, 20, 10};
		constexpr uvec4 u4{9, 8, 7, 6};

		constexpr llscal ll1{-5};
		constexpr llvec2 ll2{-4, 4};
		constexpr llvec3 ll3{2, -3, 0};
		constexpr llvec4 ll4{-10, -8, 16, 100};

		constexpr ullscal ull1{3};
		constexpr ullvec2 ull2{1, 12};
		constexpr ullvec3 ull3{30, 20, 10};
		constexpr ullvec4 ull4{9, 8, 7, 6};

		constexpr fscal f1{1.5f};
		constexpr fvec2 f2{2.25f, -10.125f};
		constexpr fvec3 f3{-1234.5, 0.0, 45.75f};
		constexpr fvec4 f4{100.0f, 200.0f, 300.0f, 400.0f};

		constexpr mat2 m2{};
		constexpr mat3 m3{};
		constexpr mat4 m4{};
		constexpr mat2x3 m23{};
		constexpr mat2x4 m24{};
		constexpr mat3x2 m32{};
		constexpr mat3x4 m34{};
		constexpr mat4x2 m42{};
		constexpr mat4x3 m43{};

		SUBCASE("Implicit conversion, unsigned int")
		{
			uscal val1{i1};
			val1 = i1;

			uvec2 val2{i2};
			val2 = i2;

			uvec3 val3{i3};
			val3 = i3;

			uvec4 val4{i4};
			val4 = i4;
		}

		SUBCASE("Implicit conversion, long long")
		{
			llscal val1{i1};
			val1 = i1;

			llvec2 val2{i2};
			val2 = i2;

			llvec3 val3{i3};
			val3 = i3;

			llvec4 val4{i4};
			val4 = i4;

			llscal val5{u1};
			val5 = u1;

			llvec2 val6{u2};
			val6 = u2;

			llvec3 val7{u3};
			val7 = u3;

			llvec4 val8{u4};
			val8 = u4;
		}

		SUBCASE("Implicit conversion, unsigned long long")
		{
			ullscal val1{i1};
			val1 = i1;

			ullvec2 val2{i2};
			val2 = i2;

			ullvec3 val3{i3};
			val3 = i3;

			ullvec4 val4{i4};
			val4 = i4;

			ullscal val5{u1};
			val5 = u1;

			ullvec2 val6{u2};
			val6 = u2;

			ullvec3 val7{u3};
			val7 = u3;

			ullvec4 val8{u4};
			val8 = u4;

			ullscal val9{ll1};
			val9 = ll1;

			ullvec2 val10{ll2};
			val10 = ll2;

			ullvec3 val11{ll3};
			val11 = ll3;

			ullvec4 val12{ll4};
			val12 = ll4;
		}

		SUBCASE("Implicit conversion, float")
		{
			fscal val1{i1};
			val1 = i1;

			fvec2 val2{i2};
			val2 = i2;

			fvec3 val3{i3};
			val3 = i3;

			fvec4 val4{i4};
			val4 = i4;

			fscal val5{u1};
			val5 = u1;

			fvec2 val6{u2};
			val6 = u2;

			fvec3 val7{u3};
			val7 = u3;

			fvec4 val8{u4};
			val8 = u4;
		}

		SUBCASE("Implicit conversion, double")
		{
			dscal val1{i1};
			val1 = i1;

			dvec2 val2{i2};
			val2 = i2;

			dvec3 val3{i3};
			val3 = i3;

			dvec4 val4{i4};
			val4 = i4;

			dscal val5{u1};
			val5 = u1;

			dvec2 val6{u2};
			val6 = u2;

			dvec3 val7{u3};
			val7 = u3;

			dvec4 val8{u4};
			val8 = u4;

			dscal val9{f1};
			val9 = f1;

			dvec2 val10{f2};
			val10 = f2;

			dvec3 val11{f3};
			val11 = f3;

			dvec4 val12{f4};
			val12 = f4;

			dscal val13{ll1};
			val13 = ll1;

			dvec2 val14{ll2};
			val14 = ll2;

			dvec3 val15{ll3};
			val15 = ll3;

			dvec4 val16{ll4};
			val16 = ll4;

			dscal val17{ull1};
			val17 = ull1;

			dvec2 val18{ull2};
			val18 = ull2;

			dvec3 val19{ull3};
			val19 = ull3;

			dvec4 val20{ull4};
			val20 = ull4;
		}

		SUBCASE("Implicit conversion, matrices")
		{
			dmat2 dm2{m2};
			dm2 = m2;

			dmat3 dm3{m3};
			dm3 = m3;

			dmat4 dm4{m4};
			dm4 = m4;

			dmat2x3 dm23{m23};
			dm23 = m23;

			dmat2x4 dm24{m24};
			dm24 = m24;

			dmat3x2 dm32{m32};
			dm32 = m32;

			dmat3x4 dm34{m34};
			dm34 = m34;

			dmat4x2 dm42{m42};
			dm42 = m42;

			dmat4x3 dm43{m43};
			dm43 = m43;
		}
	}
}
