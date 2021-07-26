
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

//#include "dev_3rd/nanobench.h"
#include "dsga.hxx"

// see https://compiler-explorer.com/z/9jjh5oj5o for an example
// of this but using std::array<> instead of dsga vectors

void mat_box()
{
	dscal one(1);
	dvec2 two(2);
	dvec3 three(3);
	dvec4 four(4);

	double rando = 5;

	auto val = dmat4(four, rando, three, rando, rando, one, two, rando, three);

	for (std::size_t i = 0; i < 4; ++i)
	{
		for (std::size_t j = 0; j < 4; ++j)
			std::printf("value[%zu][%zu] = %g\n", i, j, val.value[i][j]);
	}
}

#include "../examples/bezier.hxx"
fvec4 sometest()
{
	[[ maybe_unused ]]auto qbezval = quadratic_bezier_eval(vec2(2, 4), vec2(4, 5), vec2(8, 3), 0.25f);

	iscal first = 9;
	iscal first2 = 100;
	ivec2 second{ 20, 30 };

	[[ maybe_unused]] auto third = first + second;
	[[ maybe_unused]] auto third2 = second + first;
	[[ maybe_unused]] auto third3 = first + first2;


	[[ maybe_unused]] bool b1 = within_box(ivec2(3, 5), ivec2(4, 4), iscal(2));


	auto m = dmat2x3(3, 5., 7., 9, 11., 13.);
	auto n = dmat4x2(2., 4., 6, 8., 10., 12., 14., 16.);
	[[ maybe_unused]] auto r = m * n;

	[[ maybe_unused]] auto op = outerProduct(dvec3(3, 5, 7), dvec3(2, 4, 6));

	auto some3x3 = mat3(vec3(1, 2, 3), vec3(-3, 4, -2), vec3(2, -2, 1));
	auto iverse = inverse(some3x3);
	[[ maybe_unused]] auto ident1 = some3x3 * iverse;
	[[ maybe_unused]] auto ident2 = iverse * some3x3;

//	auto some4x4 = dsga::basic_matrix<double, 4u, 4u>(dvec4(1, 2, 1, -2), dvec4(-2, 1, -2, 1), dvec4(0, 2, -2, 1), dvec4(-1, -1, 2, 2));
	auto some4x4 = dmat4(dvec4(1, 0, 2, 2), dvec4(0, 2, 1, 0), dvec4(0, 1, 0, 1), dvec4(1, 2, 1, 4));
	auto iverse4 = inverse(some4x4);
	[[ maybe_unused]] auto ident4_1 = some4x4 * iverse4;
	[[ maybe_unused]] auto ident4_2 = iverse4 * some4x4;

	dsga::storage_wrapper<int, 4> sw{ 999, 9999, 99999, 999999 };
	sw.set(1, 2, 3, 4);

	// vector declarations
	ivec4 somethingoranother{ 0, 1, 2, 3 };

	[[ maybe_unused ]] bool val = false;
	if (ivec3{1, 3, 0} == somethingoranother.ywx)
		val = true;

	fvec3 foo(4, 3, 2);
	fvec4 bar(0);
	fvec2 pair(5, 6);

	[[ maybe_unused ]] fvec2 asdf(fscal(33), 55);
	fvec4 quux(3, pair, foo.z);

	if (foo.zy == somethingoranother.zw)
		val = true;

	if (somethingoranother.ww != pair)
		val = true;

	auto pairgen = []() -> fvec2 { return fvec2{5, 6}; };
	if (pair == pairgen())
		val = true;

	// swizzle examples
	bar = foo.yyzx;
	foo = foo.zyx;			// former aliasing issue for our implementation
	pair = foo.xz;

	// writemask examples
	bar.xy = pair;
//	bar.zw = foo.xz + bar.zw;	// <- don't currently have arithmetic operators
	bar.zw = foo.xz;
	pair.y = 8;

	[[ maybe_unused ]] auto added = pair + fvec2(foo.xz);
	[[ maybe_unused ]] auto added2 = fvec2(foo.xz) + pair;
	[[ maybe_unused ]] auto added3 = fvec2(foo.xz) + fvec2(bar.zw);
	[[ maybe_unused ]] auto added4 = 4.f + fvec2(foo.xz);
	[[ maybe_unused ]] auto added5 = fvec2(foo.xz) + 4.f;


// the following is illegal because you can't do a structure binding to an anonymous union,
// but becomes legal when you have a tuple interface
	[[ maybe_unused ]] auto &[a, b, c, d] = bar;
	[[ maybe_unused ]] const auto &[e, f, g, h] = somethingoranother.zzwy;

//	our_type_helper<decltype(somethingoranother.zzwy)> stupid_thing;
	bvec2 bool_vec(true, false);

	bool_vec = bvec2(false, true);
	bool_vec.yx = bool_vec.xy;
	[[ maybe_unused ]] auto is_eq1 = (bool_vec.yx == bool_vec.xy);
	[[ maybe_unused ]] auto is_eq2 = (bool_vec == bool_vec.xy);
	[[ maybe_unused ]] auto is_eq3 = (bool_vec.xy == bool_vec);
	[[ maybe_unused ]] auto is_eq4 = (bool_vec == bvec2(true, true));
	fscal some_fscal = foo;
	fvec2 blahblah(77, quux);
	[[ maybe_unused ]] fvec2 newtrick = (fvec4)quux.yzwx;
	fvec3 sametrick = (fvec4)quux.yxwz;
	fvec3 blah3(77, quux);
	fvec3 blah3_take2(11, fscal(373), quux);
	fvec3 blah3_take2222(pair, quux);
	fvec4 blah4(77, quux);
	fvec4 blah4_two_plus(pair, quux);
	fvec4 blah4_take2(11, fscal(373), quux);
	fvec4 blah4_taked_infinity(-98758, 511, fscal(4430), quux);
	fvec4 going_bigger(blah3, sametrick);
	fvec4 yeehaw(pair, 909, quux);
	fvec4 yeehaw_and_haw(38783, pair, sametrick);

	[[ maybe_unused ]] float fllllloooat = some_fscal;
	iscal intything = 7;
	[[ maybe_unused ]] int j = intything;
//	intything = fllllloooat;								// this shouldn't compile
//	intything = some_fscal;									// this shouldn't compile
	intything = static_cast<iscal>(some_fscal);
	some_fscal = 8;

	// bool vectors from non-bool components is ok since it is explicit.
	// bool vectors are NOT implicitly convertible to.
	[[ maybe_unused ]] bvec4 booooools{ 0, 3, 6.0f, 0.0f };

	constexpr double arr[] = { 64., 63., 62., 61. };
	constexpr auto arr_vec = to_vec(arr);
	
	constexpr std::array<double, 4> stdarr = { 884., 883., 882., 881. };
	constexpr auto stdarr_vec = to_vec(stdarr);

	constexpr auto some_sum = arr_vec + stdarr_vec;

	[[ maybe_unused ]] constexpr auto some_stdarr_sum = from_vec(some_sum);
	[[ maybe_unused ]] auto some_stdarr_sum_swizzle = from_vec(some_sum.yzwx);

	auto op_assign = stdarr_vec;
	op_assign += arr_vec;

	[[ maybe_unused ]] auto &pair_second_copy = get<1>(pair);

	ivec4 dest(0);
	unsigned dest_indx = 0;
	for (auto &yyz : somethingoranother)
	{
		dest[dest_indx] = yyz;
		++dest_indx;
	}

	const ivec4 new_something{ 19, 28, 37, 46 };
	ivec4 new_dest(0);
	unsigned new_dest_indx = 0;
	for (auto &yyz : new_something)
	{
		new_dest[new_dest_indx] = yyz;
		++new_dest_indx;
	}

	// compound operators for data_mappers
	ivec4 frank(1, 2, 3, 4);
	frank.wzyx += new_something;
	frank = new_something.xyzz;
	frank += new_something;
	frank += frank.xxxx;
	frank.xz += 10;
	frank.zx += frank.yz;
	frank = 12 + frank.wxyz;

	// swizzle range-for

	ivec4 new_unconst{ 55, 64, 73, 82 };
	ivec4 foo_dest(0);
	unsigned foo_dest_indx = 0;
	for (auto &sss : new_unconst.zwxy)
	{
		foo_dest[foo_dest_indx] = sss;
		++foo_dest_indx;
	}

	ivec4 shuff_dest(0);
	unsigned shuff_dest_indx = 0;
	for (auto &shufffff : new_something.zwxy)
	{
		shuff_dest[shuff_dest_indx] = shufffff;
		++shuff_dest_indx;
	}

	auto ttt = radians(fvec3(123, 55, 40));
	[[maybe_unused]] auto uuu = degrees(ttt);
	[[maybe_unused]] auto vvv = sin(ttt);


	[[ maybe_unused ]] ivec4 next_src{ 0, 0, 0, 0 };
//	ivec4 next_dest;

	fvec2 lastpair(7, 10);
	lastpair.xy = pairgen();
//	lastpair.xx = pairgen();
	return bar;
}

// https://twitter.com/the_whole_daisy/status/1379580525078147072
// https://godbolt.org/z/h5P1Mxsrz
// TLDR -- operator precedence ('=' is right to left, ',' is left to right) matters with binary fold expressions.
//
//#include <iostream>
//
//template <class... Ts>
//void print_args_backwards(Ts... ts)
//{
//	auto print_one = [](auto t)
//	{
//		std::cout << t << std::endl;
//
//		// Anything with a reasonable assignment operator will work here
//		return std::type_identity<void>{};
//	};
//
//	// Backwards:
//	(print_one(ts) = ...);
//
//	// Forwards:
//	(print_one(ts), ...);
//}
//
//int main()
//{
//	print_args_backwards(1, 2, "hello", 3, 4, "world");
//}


// https://stackoverflow.com/questions/51408771/c-reversed-integer-sequence-implementation
//
//#include <utility>
//#include <type_traits>
//
//template <std::size_t ... Is>
//constexpr auto indexSequenceReverse (std::index_sequence<Is...> const &)
//-> decltype( std::index_sequence<sizeof...(Is)-1U-Is...>{} );
//
//template <std::size_t N>
//using makeIndexSequenceReverse = decltype(indexSequenceReverse(std::make_index_sequence<N>{}));
//
//int main()
//{
//	static_assert( std::is_same<std::index_sequence<4U, 3U, 2U, 1U, 0U>,
//				   makeIndexSequenceReverse<5U>>::value, "!" );
//}


// https://stackoverflow.com/questions/47303466/compile-time-reversal-of-parameter-pack-expansion
//
//template<typename T>
//struct Test
//{
//	template <std::size_t...Is, typename... B>
//	Test(std::index_sequence<Is...>, B&&... vv) :
//		b{std::get<sizeof...(Is) - 1 - Is>(std::tie(vv...))...}
//	{}
//
//	public:
//		template<typename... B>
//		explicit Test(B... vv) : Test(std::index_sequence_for<B...>{}, vv...) {}
//	private:
//		std::byte b[sizeof(T)];
//};


template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
	bool W2, dsga::dimensional_scalar T2, typename D2>
requires dsga::implicitly_convertible_to<T2, T1> || dsga::implicitly_convertible_to<T1, T2>
constexpr auto sum1(const dsga::vector_base<W1, T1, C, D1> &lhs,
					const dsga::vector_base<W2, T2, C, D2> &rhs) noexcept
{
	return dsga::detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, dsga::plus_op);
}

template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
	bool W2, dsga::dimensional_scalar T2, typename D2>
requires dsga::implicitly_convertible_to<T2, T1> || dsga::implicitly_convertible_to<T1, T2>
constexpr auto sum2(const dsga::vector_base<W1, T1, C, D1> &lhs,
					const dsga::vector_base<W2, T2, C, D2> &rhs) noexcept
{
	return
	[&] <std::size_t ...Is, std::size_t ...Js, typename BinOp>(std::index_sequence<Is ...> /* dummy */, const T1 *lhs_ptr,
															   std::index_sequence<Js ...> /* dummy */, const T2 *rhs_ptr, BinOp lambda) noexcept
	{
		return dsga::basic_vector<dsga::detail::binary_op_return_t<BinOp, T1, T2>, C>(lambda(lhs_ptr[Is], rhs_ptr[Js])...);
	}(lhs.sequence(), lhs.data(), rhs.sequence(), rhs.data(), dsga::plus_op);
}

template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
	bool W2, dsga::dimensional_scalar T2, typename D2>
requires dsga::implicitly_convertible_to<T2, T1> || dsga::implicitly_convertible_to<T1, T2>
constexpr auto sum3(const dsga::vector_base<W1, T1, C, D1> &lhs,
					const dsga::vector_base<W2, T2, C, D2> &rhs) noexcept
{
	return
	[&]<typename BinOp>(BinOp lambda) noexcept
	{
		dsga::basic_vector<dsga::detail::binary_op_return_t<BinOp, T1, T2>, C> v(0);

		for (std::size_t i = 0; i < C; ++i)
			v[i] = lambda(lhs[i], rhs[i]);

		return v;
	}(dsga::plus_op);
}

template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
	bool W2, dsga::dimensional_scalar T2, typename D2>
requires dsga::implicitly_convertible_to<T2, T1> || dsga::implicitly_convertible_to<T1, T2>
constexpr auto sum4(const dsga::vector_base<W1, T1, C, D1> &lhs,
					const dsga::vector_base<W2, T2, C, D2> &rhs) noexcept
{
	dsga::basic_vector<dsga::detail::binary_op_return_t<decltype(dsga::plus_op), T1, T2>, C> v(0);

	for (std::size_t i = 0; i < C; ++i)
		v[i] = lhs[i] + rhs[i];

	return v;
}

#if 0
void bench()
{
	dvec4 v1(1, 2, 3, 4);
	dvec4 v2(10, 20, 30, 40);

	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum1 basic basic",
								   [&] { auto v = sum1(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum2 basic basic",
								   [&] { auto v = sum2(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum3 basic basic",
	//							   [&] { auto v = sum3(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum4 basic basic",
	//							   [&] { auto v = sum4(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum1 basic basic",
								   [&] { auto v = sum1(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum2 basic basic",
								   [&] { auto v = sum2(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum3 basic basic",
	//							   [&] { auto v = sum3(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum4 basic basic",
	//							   [&] { auto v = sum4(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum1 basic basic",
								   [&] { auto v = sum1(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum2 basic basic",
								   [&] { auto v = sum2(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum3 basic basic",
	//							   [&] { auto v = sum3(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum4 basic basic",
	//							   [&] { auto v = sum4(v1, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum1 indexed indexed",
								   [&] { auto v = sum1(v1.wzyx, v2.xxxx); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum2 indexed indexed",
								   [&] { auto v = sum2(v1.wzyx, v2.xxxx); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum3 indexed indexed",
	//							   [&] { auto v = sum3(v1.wzyx, v2.xxxx); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum4 indexed indexed",
	//							   [&] { auto v = sum4(v1.wzyx, v2.xxxx); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum1 indexed basic",
								   [&] { auto v = sum1(v1.wzyx, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum2 indexed basic",
								   [&] { auto v = sum2(v1.wzyx, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum3 indexed basic",
	//							   [&] { auto v = sum3(v1.wzyx, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum4 indexed basic",
	//							   [&] { auto v = sum4(v1.wzyx, v2); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum1 basic indexed",
								   [&] { auto v = sum1(v1, v2.wzyx); ankerl::nanobench::doNotOptimizeAway(v); });
	ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum2 basic indexed",
								   [&] { auto v = sum2(v1, v2.wzyx); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum3 basic indexed",
	//							   [&] { auto v = sum3(v1, v2.wzyx); ankerl::nanobench::doNotOptimizeAway(v); });
	//ankerl::nanobench::Bench().minEpochIterations(1000000).run("sum4 basic indexed",
	//							   [&] { auto v = sum4(v1, v2.wzyx); ankerl::nanobench::doNotOptimizeAway(v); });
}
#endif

#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char *argv[])
{
//	bench();

//	inv_sqrt_all_floats_test();
//	inv_sqrt_doubles_test();
	
//	mat_box();
	sometest();


	//
	// doctest
	//

	int doctest_result = 0;

// comment out if we define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN somewhere to get the main() from doctest
#define DONT_USE_DOCTEST_MAIN

#if defined(DONT_USE_DOCTEST_MAIN)

	doctest::Context context;
	context.applyCommandLine(argc, argv);

	doctest_result = context.run();				// run

	if (context.shouldExit())					// important - query flags (and --exit) rely on the user doing this
		return doctest_result;					// propagate the result of the tests

#endif



	return EXIT_SUCCESS + doctest_result;
}
