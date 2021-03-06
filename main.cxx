
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

//#include "dev_3rd/nanobench.h"
#include "dsga.hxx"


auto playing()
{
	auto x = [] <std::size_t ...Indexes>(std::index_sequence<Indexes...> /* dummy */)
	{
		return sizeof...(Indexes);
	};

	return x(std::make_index_sequence<3>{});
}

fvec4 sometest()
{
	iscal first = 9;
	iscal first2 = 100;
	ivec2 second{ 20, 30 };

[[ maybe_unused]] auto third = first + second;
[[ maybe_unused]] auto third2 = second + first;
[[ maybe_unused]] auto third3 = first + first2;

	// vector declarations
	ivec4 somethingoranother{ 0, 1, 2, 3 };

	bool val = false;
	if (ivec3{1, 3, 0} == somethingoranother.ywx)
		val = true;

	fvec3 foo(4, 3, 2);
	fvec4 bar(0);
	fvec2 pair(5, 6);

	fvec2 asdf(fscal(33), 55);
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

	auto added = pair + fvec2(foo.xz);
	auto added2 = fvec2(foo.xz) + pair;
	auto added3 = fvec2(foo.xz) + fvec2(bar.zw);
	auto added4 = 4.f + fvec2(foo.xz);
	auto added5 = fvec2(foo.xz) + 4.f;


// the following is illegal because you can't do a structure binding to an anonymous union,
// but becomes legal when you have a tuple interface
	auto &[a, b, c, d] = bar;
	const auto &[e, f, g, h] = somethingoranother.zzwy;

	[[ maybe_unused ]] auto ieils = playing();

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
	fvec2 newtrick = (fvec4)quux.yzwx;
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
	bvec4 booooools{ 0, 3, 6.0f, 0.0f };

	constexpr double arr[] = { 64., 63., 62., 61. };
	constexpr auto arr_vec = to_vec(arr);
	
	constexpr std::array<double, 4> stdarr = { 884., 883., 882., 881. };
	constexpr auto stdarr_vec = to_vec(stdarr);

	constexpr auto some_sum = arr_vec + stdarr_vec;

	constexpr auto some_stdarr_sum = from_vec(some_sum);

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

	ivec4 next_src{ 0, 0, 0, 0 };
//	ivec4 next_dest;

	fvec2 lastpair(7, 10);
	lastpair.xy = pairgen();
//	lastpair.xx = pairgen();
	return bar;
}


// Microsoft bug here
// https://developercommunity.visualstudio.com/content/problem/1259625/constexpr-and-anonymous-union-intialization-error.html
// Fixed in v16.9 Preview 1

/*
struct A
{
	int store;
};

struct C
{
	union
	{
		A a;
	};

	constexpr C(int val)
	{
		a.store = val;
	}
};

constexpr int c_val(const C &c)
{
	return c.a.store;
}

constexpr int some_c_val = c_val(C(7));
*/

#define CXCM_DISABLE_RUNTIME_OPTIMIZATIONS
#include "cxcm.hxx"

void inv_sqrt_all_floats_test()
{
	long long num_same = 0;
	long long num_diff = 0;

	float input = 0.0f;
	//double input = 0x1.0p+52;
	//double input_max = input + 0x1.0p+31 - 1;
	//while (input < input_max)
	while (input < std::numeric_limits<float>::max())
	{
		//input = std::nextafter(input, input_max);
		//double contender = cxcm::rsqrt(input);
		//double official = 1.0 / std::sqrt(input);

		input = std::nextafter(input, std::numeric_limits<float>::max());
		float contender = cxcm::rsqrt(input);
		float official = 1.0f / std::sqrt(input);

		if (contender == official)
		{
			++num_same;
		}
		else
		{
			++num_diff;
//			std::printf("difference -- official = %0.6a  contender = %0.6a\n", official, contender);
//			std::printf("difference -- official = %0.13a  contender = %0.13a\n", official, contender);
		}
	}

	std::printf("floats: num_same = %lld\nnum_diff = %lld\n", num_same, num_diff);
}

void inv_sqrt_doubles_test()
{
	long long num_same = 0;
	long long num_diff = 0;

	double input = 0x1.0p+52;
//	double input = 1.25;
//	double input = 0.0;
//	double input = 123456.789;
//	double input = std::numeric_limits<double>::max();

	for (long long i = 0; i < std::numeric_limits<int>::max(); ++i)
	{
		input = std::nextafter(input, std::numeric_limits<double>::max());
//		input = std::nextafter(input, 0.0);
		double contender = cxcm::rsqrt(input);
		double official = 1.0 / std::sqrt(input);

		if (contender == official)
		{
			++num_same;
		}
		else
		{
			++num_diff;
//			std::printf("difference -- official = %0.13a  contender = %0.13a\n", official, contender);
		}

		if (((i+1) % 1000000) == 0)
			std::printf("#");
	}
	std::printf("\n");

	std::printf("doubles: num_same = %lld\nnum_diff = %lld\n", num_same, num_diff);
}



void test_bin_op()
{
	constexpr dvec3 dv(41.5, 42.5, 43.5);
	ivec3 iv(40, 50, 60);

	auto vec_vec1 = dv + iv;
	auto vec_vec2 = iv + dv;

	auto not_vec = ~iv;
	auto neg_vec = -dv;

	constexpr int some_int = static_cast<int>(98.6);

	auto vec_scal = dv + 300;
	auto scal_vec = -400 + iv;

	const volatile int nine_k = 9000;
	const volatile int &nine_k_ref = nine_k;
	vec_scal.y = nine_k_ref;

	ivec2 vec_from_cvref(nine_k_ref, nine_k_ref);
}

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char *argv[])
{
//	inv_sqrt_all_floats_test();
//	inv_sqrt_doubles_test();
	
	//	test_bin_op();
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
