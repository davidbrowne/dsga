
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

//#include "nanobench.h"
#include "dsga.hxx"
#include <iostream>
#include <utility>
#include <format>
#include <random>
#include <map>
#include "nanobench.h"
#include <ranges>
#include <numbers>
#include <thread>
#include <valarray>
#include <string>
#include <vector>
#include <cassert>
#include "../examples/hash.hxx"
#include "../examples/angle.hxx"
#include "../examples/bezier.hxx"
#include "../examples/basic.hxx"
#include "../examples/compare.hxx"
#include "../examples/ostream_output.hxx"
#include "../examples/format_output.hxx"
#include "../examples/invoke.hxx"
#include "../examples/span_convert.hxx"
#include "../examples/stl.hxx"
#include "../examples/valarray_convert.hxx"
#include "../examples/tolerance.hxx"
#include "../examples/transform.hxx"
#include <source_location>
#include <cstdint>

//
//
// This file contains main(), and it is used for both running the unit tests and
// for playing around with some example and ideas. This file doesn't have anything
// to demonstrate beyond that.
//
//

// print current version number
namespace
{
	void print_dsga_version()
	{
		std::cout << "\ndsga version: v"
				  << dsga::DSGA_MAJOR_VERSION << "." << dsga::DSGA_MINOR_VERSION << "." << dsga::DSGA_PATCH_VERSION << "\n\n";
	}
}

struct xoshiro256p
{
	// state
	std::array<uint64_t, 4> s{};

	void print_state() const
	{
		for (auto val : s)
			std::cout << std::format("{:#018x}\n", val);

		std::cout << "\n";
	}

	// xoshiro256+ prng
	explicit xoshiro256p(uint64_t seed) noexcept
	{
		// SplitMix64
		auto split_mix_64 = [&seed]()
		{
			uint64_t z = (seed += 0x9e3779b97f4a7c15);
			z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
			z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
			return z ^ (z >> 31);
		};

		// initialize state from seed
		for (auto &val : s)
			val = split_mix_64();

//		print_state();
	}

	[[ nodiscard ]] constexpr uint64_t rotl(const uint64_t x, int k) const noexcept
	{
		return (x << k) | (x >> (64 - k));
	}

	[[ nodiscard ]] inline uint64_t next_uint64_t() noexcept
	{
		const uint64_t result = s[0] + s[3];

		const uint64_t t = s[1] << 17;

		s[2] ^= s[0];
		s[3] ^= s[1];
		s[1] ^= s[2];
		s[0] ^= s[3];

		s[2] ^= t;

		s[3] = rotl(s[3], 45);

		return result;
	}

	[[ nodiscard ]] inline int64_t next_int64_t() noexcept
	{
		return std::bit_cast<int64_t>(next_uint64_t());
	}

	[[ nodiscard ]] inline uint32_t next_uint32_t() noexcept
	{
		// right shift to get high bits
		return static_cast<uint32_t>((next_uint64_t() >> 16) & 0x00000000FFFFFFFF);
	}

	[[ nodiscard ]] inline int32_t next_int32_t() noexcept
	{
		return std::bit_cast<int32_t>(next_uint32_t());
	}

	[[ nodiscard ]] inline double next_double() noexcept
	{
		return (static_cast<double>(next_uint64_t() >> 11)) * 0x1.0p-53;
	}

	[[ nodiscard ]] inline float next_float() noexcept
	{
		return static_cast<float>(next_uint32_t() >> 8) * 0x1.0p-24f;
	}

};	// struct xoshiro256p

// randomize a seed using std::random_device
[[ maybe_unused ]] inline static uint64_t randomize_seed(uint64_t seed)
{
	std::random_device rd;
	uint64_t rand_val = rd();
	rand_val <<= 32;
	rand_val |= rd();
	return rand_val ^ seed;
}

// return the number of floating point representable values (ulps) between two values.
template <std::floating_point T>
int ulps_distance(T first, T second)
{
	// sanity check on inputs -- shouldn't it be using || ?
	if (!dsga::detail::cxcm::isfinite(first) && !dsga::detail::cxcm::isfinite(second))
	{
		if constexpr (sizeof(T) == 4)
		{
			unsigned int bits1 = std::bit_cast<unsigned int>(first);
			unsigned int bits2 = std::bit_cast<unsigned int>(second);

			if (std::cmp_less(bits1, bits2))
				return static_cast<int>(bits2 - bits1);
			else if (std::cmp_less(bits2, bits1))
				return static_cast<int>(bits1 - bits2);
			else
				return 0;
		}
		else if constexpr (sizeof(T) == 8)
		{
			unsigned long long bits1 = std::bit_cast<unsigned long long>(first);
			unsigned long long bits2 = std::bit_cast<unsigned long long>(second);

			if (std::cmp_less(bits1, bits2))
				return static_cast<int>(bits2 - bits1);
			else if (std::cmp_less(bits2, bits1))
				return static_cast<int>(bits1 - bits2);
			else
				return 0;
		}
	}

	int count{};
	while (count < 20)
	{
		if (first == second)
			break;

		first = std::nextafter(first, second);
		++count;
	}

	return count;
}

// output the results of many ulps_distance() calls
//static void print_ulps_results(const std::map<int, int> &ulps_away, const char *header)
//{
//	std::printf("%s\n", header);
//
//	for (const auto& [ulps, count] : ulps_away)
//		std::printf("%d ulps : %d\n", ulps, count);
//}

// compare two floating point values as if they were unsigned integers of the same size,
// to determine which is larger, even if they are NaN or infinities.
// I don't really know if this is useful, but it was an interesting exercise.
template <dsga::detail::cxcm::concepts::basic_floating_point T>
int non_finite_compare(T val1, T val2)
{
	int compare_value = 0;			// -1 if val1 < val2, 0 if val1 == val2, 1 of val1 > val2

	if constexpr (sizeof(T) == 4)
	{
		unsigned int bits1 = std::bit_cast<unsigned int>(val1);
		unsigned int bits2 = std::bit_cast<unsigned int>(val2);

		if (bits1 < bits2)
			compare_value = -1;
		else if (bits2 < bits1)
			compare_value = 1;
	}
	else if constexpr (sizeof(T) == 8)
	{
		unsigned long long bits1 = std::bit_cast<unsigned long long>(val1);
		unsigned long long bits2 = std::bit_cast<unsigned long long>(val2);

		if (bits1 < bits2)
			compare_value = -1;
		else if (bits2 < bits1)
			compare_value = 1;
	}

	return compare_value;
}

//static void test_fast_rsqrt()
//{
//	const float scale{1.0e11f};
//
//	[[ maybe_unused ]] auto pi_bits = std::bit_cast<uint64_t>(std::numbers::pi_v<double>);
//	[[ maybe_unused ]] auto xoshiro = xoshiro256p(randomize_seed(pi_bits));
//
//	std::map<int, int> ulps_count;
//	int below{};
//	int above{};
//	for (int i = 0; i < 1'000'000'000; ++i)
//	{
//		float val{xoshiro.next_float() * scale};
//
//		auto d1 = dsga::cxcm::rsqrt(val);
//		auto d2 = dsga::cxcm::fast_rsqrt(val);
//
//		++ulps_count[ulps_distance<float>(d1, d2)];
//		if (d1 < d2)
//			++above;
//		else if (d2 < d1)
//			++below;
//	}
//
//	std::printf("scale : %g\n", scale);
//	print_ulps_results(ulps_count, "dsga::cxcm::rsqrt(float) vs dsga::cxcm::fast_rsqrt(float)");
//
//	std::printf("below std : %d\n", below);
//	std::printf("above std : %d\n", above);
//}

//static void test_lerp()
//{
////	const float scale{1.0e11f};
//	const float endpoint_scale{1.0e2f};
//	const float parameter_scale{1.0f};
//
//	[[ maybe_unused ]] auto pi_bits = std::bit_cast<uint64_t>(std::numbers::pi_v<double>);
//	[[ maybe_unused ]] auto xoshiro = xoshiro256p(randomize_seed(pi_bits));
//
//	std::map<int, int> ulps_count;
//	int below{};
//	int above{};
//	for (int i = 0; i < 1'000'000'000; ++i)
//	{
//		float val1{xoshiro.next_float() * endpoint_scale};
//		float val2{xoshiro.next_float() * endpoint_scale};
//		float val3{xoshiro.next_float() * parameter_scale};
//
//		auto d1 = std::lerp(val1, val2, val3);
//		auto d2 = dsga::functions::lambda_ops::mix1_op(val1, val2, val3);
//
//		++ulps_count[ulps_distance<float>(d1, d2)];
//		if (d1 < d2)
//			++above;
//		else if (d2 < d1)
//			++below;
//	}
//
////	std::printf("scale : %g\n", scale);
//	print_ulps_results(ulps_count, "std::lerp() vs dsga::alt1_mix1_op");
//
//	std::printf("below std : %d\n", below);
//	std::printf("above std : %d\n", above);
//}

//static void test_double_sqrt()
//{
//	const double scale{1.0e11};
////	const double scale{1.0e+3};
//
//	[[ maybe_unused ]] auto pi_bits = std::bit_cast<uint64_t>(std::numbers::pi_v<double>);
//	[[ maybe_unused ]] auto xoshiro = xoshiro256p(randomize_seed(pi_bits));
//
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{:#018x}\n", xoshiro.next_uint64_t());
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{:#010x}\n", xoshiro.next_uint32_t());
//// 
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{}\n", xoshiro.next_double());
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{}\n", xoshiro.next_float());
////
////	uint32_t high{};
////	for (int i = 0; i < 1'000'000'000; ++i)
////	{
////		if (xoshiro.next_float() >= 0.5f)
////			++high;
////	}
////	std::cout << std::format("high: {}\n", high);
//
//	std::map<int, int> ulps_count;
//	int below{};
//	int above{};
//	for (int i = 0; i < 1'000'000'000; ++i)
//	{
//		double val{xoshiro.next_double() * scale};
//
//		auto d1 = std::sqrt(val);
//		auto d2 = dsga::cxcm::sqrt(val);
//
//		++ulps_count[ulps_distance(d1, d2)];
//		if (d1 < d2)
//			++above;
//		else if (d2 < d1)
//			++below;
//	}
//
//	std::printf("scale : %g\n", scale);
//	print_ulps_results(ulps_count, "std::sqrt(double) vs dsga::cxcm::sqrt(double)");
//
//	std::printf("below std : %d\n", below);
//	std::printf("above std : %d\n", above);
//}

//static void test_double_rsqrt()
//{
//	const double scale{1.0e11};
////	const double scale{1.0e+3};
//
//	[[ maybe_unused ]] auto pi_bits = std::bit_cast<uint64_t>(std::numbers::pi_v<double>);
//	[[ maybe_unused ]] auto xoshiro = xoshiro256p(randomize_seed(pi_bits));
//
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{:#018x}\n", xoshiro.next_uint64_t());
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{:#010x}\n", xoshiro.next_uint32_t());
//// 
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{}\n", xoshiro.next_double());
////	for (int i = 0; i < 20; ++i)
////		std::cout << std::format("{}\n", xoshiro.next_float());
////
////	uint32_t high{};
////	for (int i = 0; i < 1'000'000'000; ++i)
////	{
////		if (xoshiro.next_float() >= 0.5f)
////			++high;
////	}
////	std::cout << std::format("high: {}\n", high);
//
//	std::map<int, int> ulps_count;
//	int below{};
//	int above{};
//	for (int i = 0; i < 1'000'000'000; ++i)
//	{
//		double val{xoshiro.next_double() * scale};
//
//		auto d1 = dsga::cxcm::rsqrt(val);
//		auto d2 = dsga::cxcm::fast_rsqrt(val);
//
//		++ulps_count[ulps_distance(d1, d2)];
//		if (d1 < d2)
//			++above;
//		else if (d2 < d1)
//			++below;
//	}
//
//	std::printf("scale : %g\n", scale);
//	print_ulps_results(ulps_count, "std::rsqrt(double) vs dsga::cxcm::fast_rsqrt(double)");
//
//	std::printf("below std : %d\n", below);
//	std::printf("above std : %d\n", above);
//}

//static void test_all_floats_sqrt()
//{
//	const double scale{1.0e11};
////	const double scale{1.0e+3};
//
//	std::map<int, int> ulps_count;
//	long long below{};
//	long long above{};
//	long long same{};
//	for (unsigned int i = 0x00000000; i < 0xFFFFFFFF; ++i)
//	{
//		float f = dsga::uintBitsToFloat(i);
//
//		auto f1 = std::sqrt(f);
//		auto f2 = dsga::cxcm::strict::detail::constexpr_sqrt(f);
//
//		auto res1 = dsga::floatBitsToUint(f1);
//		auto res2 = dsga::floatBitsToUint(f2);
//
//		if (res1 < res2)
//			++above;
//		else if (res1 > res2)
//			++below;
//		else
//			++same;
//	}
//
//	std::printf("scale : %g\n", scale);
//	print_ulps_results(ulps_count, "std::sqrt(float) vs dsga::cxcm::strict::detail::constexpr_sqrt(float)");
//
//	std::printf("same std : %lld\n", same);
//	std::printf("below std : %lld\n", below);
//	std::printf("above std : %lld\n", above);
//}

//static void test_all_floats_rsqrt()
//{
//	std::map<int, int> ulps_count;
//	long long below{};
//	long long above{};
//	long long same{};
//	for (unsigned int i = 0x00000000; i < 0xFFFFFFFF; ++i)
//	{
//		float f = dsga::uintBitsToFloat(i);
//
//		auto f1 = dsga::cxcm::rsqrt(f);
//		auto f2 = dsga::cxcm::fast_rsqrt(f);
//
//		auto res1 = dsga::floatBitsToUint(f1);
//		auto res2 = dsga::floatBitsToUint(f2);
//
//		if (res1 < res2)
//			++above;
//		else if (res1 > res2)
//			++below;
//		else
//			++same;
//	}
//
//	 constexpr float fmax = dsga::uintBitsToFloat(0xFFFFFFFF);
//
//	auto f1 = dsga::cxcm::rsqrt(fmax);
//	auto f2 = dsga::cxcm::fast_rsqrt(fmax);
//
//	auto res1 = dsga::floatBitsToUint(f1);
//	auto res2 = dsga::floatBitsToUint(f2);
//
//	if (res1 < res2)
//		++above;
//	else if (res1 > res2)
//		++below;
//	else
//		++same;
//
//	print_ulps_results(ulps_count, "dsga::cxcm::rsqrt(float) vs dsga::cxcm::fast_rsqrt(float)");
//
//	std::printf("same std : %lld\n", same);
//	std::printf("below std : %lld\n", below);
//	std::printf("above std : %lld\n", above);
//}

//static void bench_rsqrt()
//{
//	constexpr double val = std::numbers::pi_v<double>;
//
//	ankerl::nanobench::Bench().run("fast_rsqrt(double)", [&]
//	{
//		auto x = dsga::cxcm::relaxed::fast_rsqrt(val);
//		ankerl::nanobench::doNotOptimizeAway(x);
//	});
//
//	ankerl::nanobench::Bench().run("inverse_sqrt(double)", [&]
//	{
//		auto x = dsga::cxcm::relaxed::detail::inverse_sqrt(val);
//		ankerl::nanobench::doNotOptimizeAway(x);
//	});
//}

//static void bench_sqrt()
//{
//	constexpr double val = std::numbers::pi_v<double>;
//
//	ankerl::nanobench::Bench().run("sqrt_converge(double)", [&]
//	{
//		auto x = dsga::cxcm::relaxed::sqrt(val);
//		ankerl::nanobench::doNotOptimizeAway(x);
//	});
//}


// Generate at startup — not constexpr, intentionally runtime values
// to prevent the compiler from constant-folding across benchmark iterations
static const std::array<double, 256> inputs = []
{
	std::array<double, 256> arr;
	std::mt19937_64 rng(std::random_device{}());

	// Distribute uniformly across the positive normal double exponent range.
	// Generate a random exponent in [-1022, 1023] and a random mantissa,
	// then assemble via bit manipulation to ensure even exponent band coverage
	// rather than clustering around any particular magnitude.
	std::uniform_int_distribution<uint64_t> mantissa_dist(0, (1ULL << 52) - 1);
	std::uniform_int_distribution<int> exponent_dist(-1022, 1023);

	for (auto& x : arr)
	{
		uint64_t mantissa = mantissa_dist(rng);
		uint64_t biased_exponent = static_cast<uint64_t>(exponent_dist(rng) + 1023);
		uint64_t bits = (biased_exponent << 52) | mantissa;
		std::memcpy(&x, &bits, sizeof(x));
	}

	return arr;
}();


static void bench_cross()
{
	auto v1 = dsga::vec3(20, 10, -30);
	auto v2 = dsga::vec3(70, 50, 0);
	[[ maybe_unused ]] float dist_tol = 1e-10f;

	[[ maybe_unused ]] auto mv1 = dsga::cross_matrix(v1);
	[[ maybe_unused ]] auto mv2 = dsga::cross_matrix(v2);

	dsga::dmat3 A(1, 1, 3, 0, 1, 2, 2, 1, 5);
	dsga::dmat3 B(1, 0, 2, 0, 3, 0, 4, 0, 5);

	auto point = dsga::dvec3(5, 2, 0);
	auto point2 = dsga::dvec3(5, 2, 20);
	auto p1 = dsga::dvec3(0);
	auto p2 = dsga::dvec3(1, 2, 0);
	auto p3 = dsga::dvec3(5, 0.5, 1);
	auto p4 = dsga::dvec3(4, 0, 2);
	auto t_val = 0.725;

	auto op1 = dsga::vec3(2, 4, 6);
	auto op2 = dsga::vec2(5, 7);
	auto m3 = dsga::outerProduct(dsga::vec4(2, 4, 6, 8), dsga::vec3(5, 7, 9));

	dsga::mat2x3 mm1(1, 2, 3, 4, 5, 6);
	dsga::mat2x3 mm2(5, 10, 15, 20, 25, 30);
	[[ maybe_unused ]] auto mm3 = dsga::transpose(mm2);
	[[ maybe_unused ]] auto mcm = dsga::matrixCompMult(mm1, mm2);

	auto dm_vec = dsga::dvec4(1, 2, 3, 4);

	auto angle = dsga::sqrt(3.0) / 2.0;	// 0.86602540378443860
	auto d1 = dsga::dvec4(dsga::cos(angle), dsga::sin(angle), 0, 0);
	auto d2 = dsga::dvec4(-dsga::sin(angle), dsga::cos(angle), 0, 0);
	auto d3 = dsga::dvec4(0, 0, 1, 0);
	auto d4 = dsga::dvec4(3, 5, 7, 1);

	auto f1 = dsga::fvec3(dsga::cos(angle), dsga::sin(angle), 0);
	auto f2 = dsga::fvec3(-dsga::sin(angle), dsga::cos(angle), 0);
	auto f3 = dsga::fvec3(3, 5, 1);

	auto m1 = dsga::dmat4(d1, d2, d3, d4);
	auto m2 = dsga::mat3(f1, f2, f3);
	[[ maybe_unused ]] auto m4 = dsga::dmat3(m1) * dsga::transpose(dsga::dmat3(m1));
	auto tol = 1e-10;

	auto v3 = dsga::dvec3(std::numbers::pi_v<double>, std::numeric_limits<double>::quiet_NaN(), 123456789.987654321);

	auto someaxis = dsga::dvec3(1, 2, 3);
	auto rot_vec = dsga::normalize(dsga::dvec3(1, 2, 3));
	auto rot_theta = std::numbers::pi_v<double> / 6.0;
	auto origin = dsga::dvec3(0, 0, 0);

	double my_val = 0.99999998888;
	double min_val = -1.0;
	double max_val =  1.0;

	// Spread across several exponent bands to prevent constant folding
	// and to avoid measuring a single degenerate case
//	static constexpr std::array<double, 8> inputs = { 0.01, 0.1, 0.5, 1.0, 2.0, 10.0, 100.0, 10000.0 };

	ankerl::nanobench::Bench bench;
	bench.warmup(100).epochs(1000).epochIterations(8);

	double result = 0.0;

	bench.run("1.0 / std::sqrt", [&] {
		for (auto x : inputs)
			result += 1.0 / std::sqrt(x);
		ankerl::nanobench::doNotOptimizeAway(result);
	});

	bench.run("fast_rsqrt", [&] {
		for (auto x : inputs)
			result += dsga::detail::cxcm::relaxed::impl::fast_rsqrt(x);
		ankerl::nanobench::doNotOptimizeAway(result);
	});

	bench.run("converging_sqrt<double>", [&] {
		for (auto x : inputs)
			result += dsga::detail::cxcm::relaxed::impl::converging_sqrt(x);
		ankerl::nanobench::doNotOptimizeAway(result);
	});

	bench.run("converging_inverse_sqrt<double>", [&] {
		for (auto x : inputs)
			result += dsga::detail::cxcm::relaxed::impl::converging_inverse_sqrt(x);
		ankerl::nanobench::doNotOptimizeAway(result);
	});

	bench.run("inverse_sqrt<double>", [&] {
		for (auto x : inputs)
			result += dsga::detail::cxcm::relaxed::impl::inverse_sqrt(x);
		ankerl::nanobench::doNotOptimizeAway(result);
	});

	ankerl::nanobench::Bench().run("matrix mult lambda loop NRVO", [&A, &B]
	{
		auto x = A * B;
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("fold transpose NRVO", [&A]
	{
		auto x = dsga::transpose(A);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("row()x3", [&A]
	{
		auto x = A.row(0);
		auto y = A.row(1);
		auto z = A.row(2);
		ankerl::nanobench::doNotOptimizeAway(x);
		ankerl::nanobench::doNotOptimizeAway(y);
		ankerl::nanobench::doNotOptimizeAway(z);
	});
	ankerl::nanobench::Bench().run("project_to_plane1()", [&point2, &p1, &p2, &p3]
	{
		auto proj = project_to_plane1(point2, p1, p2, p3);
		ankerl::nanobench::doNotOptimizeAway(proj);
	});
	ankerl::nanobench::Bench().run("project_to_plane2()", [&point2, &p1, &p2, &p3]
	{
		auto proj = project_to_plane2(point2, p1, p2, p3);
		ankerl::nanobench::doNotOptimizeAway(proj);
	});
	ankerl::nanobench::Bench().run("project_to_line1()", [&point, &p1, &p2]
	{
		auto proj = project_to_line1(point, p1, p2);
		ankerl::nanobench::doNotOptimizeAway(proj);
	});
	ankerl::nanobench::Bench().run("project_to_line2()", [&point, &p1, &p2]
	{
		auto proj2 = project_to_line2(point, p1, p2);
		ankerl::nanobench::doNotOptimizeAway(proj2);
	});
	ankerl::nanobench::Bench().run("distance_to_line()", [&point, &p1, &p2]
	{
		auto dist = distance_to_line(point, p1, p2);
		ankerl::nanobench::doNotOptimizeAway(dist);
	});
	ankerl::nanobench::Bench().run("outerProduct()", [&op1, &op2]
	{
		auto op_mat = dsga::outerProduct(op1, op2);
		ankerl::nanobench::doNotOptimizeAway(op_mat);
	});
	ankerl::nanobench::Bench().run("transpose()", [&m3]
	{
		auto transposed = dsga::transpose(m3);
		ankerl::nanobench::doNotOptimizeAway(transposed);
	});
	ankerl::nanobench::Bench().run("matrixCompMult()", [&mm1, &mm2]
	{
		auto matmult = dsga::matrixCompMult(mm1, mm2);
		ankerl::nanobench::doNotOptimizeAway(matmult);
	});
	ankerl::nanobench::Bench().run("diagonal_matrix()", [&dm_vec]
	{
		auto dm_mat = dsga::diagonal_matrix(dm_vec);
		ankerl::nanobench::doNotOptimizeAway(dm_mat);
	});
	ankerl::nanobench::Bench().run("identity_matrix()", []
	{
		auto dm_mat = dsga::identity_matrix<double, 4>();
		ankerl::nanobench::doNotOptimizeAway(dm_mat);
	});
	ankerl::nanobench::Bench().run("innerProduct()", [&v1, &v2]
	{
		auto inner_prod = dsga::innerProduct(v1, v2);
		ankerl::nanobench::doNotOptimizeAway(inner_prod);
	});
	ankerl::nanobench::Bench().run("dot()", [&v1, &v2]
	{
		auto dot_prod = dsga::dot(v1, v2);
		ankerl::nanobench::doNotOptimizeAway(dot_prod);
	});
	ankerl::nanobench::Bench().run("dot -> (v1 * v2).sum()", [&v1, &v2]
	{
		auto dot_prod = (v1 * v2).sum();
		ankerl::nanobench::doNotOptimizeAway(dot_prod);
	});
	ankerl::nanobench::Bench().run("dot -> for loop", [&v1, &v2]
	{
		float value = 0.0f;
		for (std::size_t i = 0; i < 3; ++i)
			value += v1[i] * v2[i];

		ankerl::nanobench::doNotOptimizeAway(value);
	});
	ankerl::nanobench::Bench().run("cubic recursive bezier", [&t_val, &p1, &p2, &p3, &p4]
	{
		auto bez = cubic_recursive_bezier_eval(p1, p2, p3, p4, t_val);
		ankerl::nanobench::doNotOptimizeAway(bez);
	});
	ankerl::nanobench::Bench().run("cubic polynomial bezier", [&t_val, &p1, &p2, &p3, &p4]
	{
		auto bez = cubic_polynomial_bezier_eval(p1, p2, p3, p4, t_val);
		ankerl::nanobench::doNotOptimizeAway(bez);
	});
	ankerl::nanobench::Bench().run("inversesqrt()", [&v3]
	{
		auto x = dsga::inversesqrt(v3);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("fast_inversesqrt()", [&v3]
	{
		auto x = dsga::fast_inversesqrt(v3);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("apply(relaxed::detail::fast_rsqrt)", [&v3]
	{
		auto x = v3.apply(&dsga::detail::cxcm::relaxed::impl::fast_rsqrt);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("determinant<3>()", [&m2]
	{
		auto x = dsga::determinant(m2);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("inverse<3>()", [&m2]
	{
		auto x = dsga::inverse(m2);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("determinant<4>()", [&m1]
	{
		auto x = dsga::determinant(m1);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("inverse<4>()", [&m1]
	{
		auto x = dsga::inverse(m1);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("transform_inverse()", [&m1]
	{
		auto x = dsga::transform_inverse(m1);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("normalize()", [&someaxis]
	{
		auto x = dsga::normalize(someaxis);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("rot_any_axis()", [&rot_theta, &rot_vec, &origin]
	{
		auto x = dsga::rot_any_axis(rot_theta, rot_vec, origin);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("is_transformation_matrix()", [&m1, tol]
	{
		auto x = dsga::is_transformation_matrix(m1, tol);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("renormalize_transformation_matrix()", [&m1, tol]
	{
		auto x = dsga::renormalize_transformation_matrix(m1, tol);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("dmat4(1.0)", []
	{
		auto dm_mat = dsga::dmat4(1.0);
		ankerl::nanobench::doNotOptimizeAway(dm_mat);
	});
	ankerl::nanobench::Bench().run("dmat4(individual elements from vectors)", [&d1, &d2, &d3, &d4]
	{
		auto x = dsga::dmat4(d1[0], d1[1], d1[2], d1[3],
							 d2[0], d2[1], d2[2], d2[3],
							 d3[0], d3[1], d3[2], d3[3],
							 d4[0], d4[1], d4[2], d4[3]);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("dmat4(d1, &d2, &d3, &d4)", [&d1, &d2, &d3, &d4]
	{
		auto x = dsga::dmat4(d1, d2, d3, d4);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("cross()", [&v1, &v2]
	{
		auto x = dsga::cross(v1, v2);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("cross_matrix()", [&v1]
	{
		auto x = dsga::cross_matrix(v1);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("std::clamp()", [&my_val, &min_val, &max_val]
	{
		auto x = std::clamp(my_val, min_val, max_val);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
	ankerl::nanobench::Bench().run("dsga::clamp()", [&my_val, &min_val, &max_val]
	{
		auto x = dsga::clamp(my_val, min_val, max_val);
		ankerl::nanobench::doNotOptimizeAway(x);
	});
}

//static void test_xoshiro()
//{
//	[[ maybe_unused ]] auto pi_bits = std::bit_cast<uint64_t>(std::numbers::pi_v<double>);
//	[[ maybe_unused ]] auto xoshiro = xoshiro256p(randomize_seed(pi_bits));
//
//	std::map<double, int> xtest_map;
//	for (int i = 0; i < 1'000'000'000; ++i)
//	{
//		xtest_map[std::floor(xoshiro.next_double() * 10)]++;
//	}
//
//	std::printf("%s\n", "xoshiro test");
//
//	for (auto [bucket, count] : xtest_map)
//		std::printf("%g bucket : %d\n", bucket, count);
//}

enum class bigger_enum : int
{
	a = 99, b = 73, c = 4
};

enum class test_enum : bool
{
};

static void enum_func([[ maybe_unused ]] test_enum e)
{
	return;
}

template <dsga::non_bool_scalar T>
T some_apply_func(T arg)
{
	static T val = 0;
	val += arg;

	return val;
}

static void replace(std::string &orig_string, const std::string search_string, const std::string &replace_string)
{
	std::size_t pos{};
	while ((pos = orig_string.find(search_string, pos)) != std::string::npos)
	{
		orig_string.replace(pos, search_string.size(), replace_string);
	}
}

static std::string swap_remove_space(const std::string &sToSwap)
{
	std::string sRVal = sToSwap;
	std::size_t count = sRVal.size();

	// swap adjacent characters, a pair at a time
	for (std::size_t i = 0; (i + 1) < count; i += 2)
	{
		char temp = sRVal[i];
		sRVal[i] = sRVal[i + 1];
		sRVal[i + 1] = temp;
	}

	// remove all spaces
	std::size_t blank_position = sRVal.find(" ");
	while (blank_position != std::string::npos)
	{
		sRVal.erase(blank_position, 1);
		blank_position = sRVal.find(" ");
	}

	return sRVal;
}

//
static auto bytes_to_short(std::byte a, std::byte b)
{
	std::byte bytes[2] = {a, b};
	return std::bit_cast<short>(bytes);
}


static void test_visualizer()
{
	// swizzle_vec with Count = 4
	auto a = dsga::dvec4{9, 8, 7, 6}.ywxz;

	[[ maybe_unused ]] auto a_fwd_begin_iter = a.begin();
	[[ maybe_unused ]] auto a_fwd_cbegin_iter = a.cbegin();

	[[ maybe_unused ]] auto a_fwd_end_iter = a.end();
	[[ maybe_unused ]] auto a_fwd_cend_iter = a.cend();

	[[ maybe_unused ]] auto a_rev_begin_iter = a.rbegin();
	[[ maybe_unused ]] auto a_rev_cbegin_iter = a.crbegin();

	[[ maybe_unused ]] auto a_rev_end_iter = a.rend();
	[[ maybe_unused ]] auto a_rev_cend_iter = a.crend();


	// swizzle_vec with Count = 3
	auto b = dsga::dvec3{40, 50, 60}.yzx;

	[[ maybe_unused ]] auto b_fwd_begin_iter = b.begin();
	[[ maybe_unused ]] auto b_fwd_cbegin_iter = b.cbegin();

	[[ maybe_unused ]] auto b_fwd_end_iter = b.end();
	[[ maybe_unused ]] auto b_fwd_cend_iter = b.cend();

	[[ maybe_unused ]] auto b_rev_begin_iter = b.rbegin();
	[[ maybe_unused ]] auto b_rev_cbegin_iter = b.crbegin();

	[[ maybe_unused ]] auto b_rev_end_iter = b.rend();
	[[ maybe_unused ]] auto b_rev_cend_iter = b.crend();


	// swizzle_vec with Count = 2
	auto c = dsga::dvec2{9, 8}.yx;

	[[ maybe_unused ]] auto c_fwd_begin_iter = c.begin();
	[[ maybe_unused ]] auto c_fwd_cbegin_iter = c.cbegin();

	[[ maybe_unused ]] auto c_fwd_end_iter = c.end();
	[[ maybe_unused ]] auto c_fwd_cend_iter = c.cend();

	[[ maybe_unused ]] auto c_rev_begin_iter = c.rbegin();
	[[ maybe_unused ]] auto c_rev_cbegin_iter = c.crbegin();

	[[ maybe_unused ]] auto c_rev_end_iter = c.rend();
	[[ maybe_unused ]] auto c_rev_cend_iter = c.crend();


	// swizzle_vec with Count = 1
	auto d = dsga::dscal{3333}.x;

	[[ maybe_unused ]] auto d_fwd_begin_iter = d.begin();
	[[ maybe_unused ]] auto d_fwd_cbegin_iter = d.cbegin();

	[[ maybe_unused ]] auto d_fwd_end_iter = d.end();
	[[ maybe_unused ]] auto d_fwd_cend_iter = d.cend();

	[[ maybe_unused ]] auto d_rev_begin_iter = d.rbegin();
	[[ maybe_unused ]] auto d_rev_cbegin_iter = d.crbegin();

	[[ maybe_unused ]] auto d_rev_end_iter = d.rend();
	[[ maybe_unused ]] auto d_rev_cend_iter = d.crend();
}

[[ maybe_unused ]] constexpr double dsga_one = dsga::detail::cxcm::sqrt(std::numbers::pi_v<double>) * std::numbers::inv_sqrtpi_v<double>;
[[ maybe_unused ]] constexpr double dsga_one_two = dsga::detail::cxcm::sqrt(std::numbers::pi_v<double>) * dsga::detail::cxcm::rsqrt(std::numbers::pi_v<double>);
[[ maybe_unused ]] constexpr bool invsqrt_equal = dsga::detail::cxcm::rsqrt(std::numbers::pi_v<double>) == std::numbers::inv_sqrtpi_v<double>;

inline constexpr const char *pm6_header_string = "\xC5\x50\x4D\x36\x0D\x0A\x1A\x0A";


template <dsga::vec_like V>
requires dsga::non_bool_scalar<dsga::vec_scalar_t<V>>
dsga::vec_scalar_t<V> magnitude_squared(V v)
{
	return[&v]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		return ((v[Is] * v[Is]) + ...);
	}(std::make_index_sequence<V::size()>{});
//	}(std::make_index_sequence<dsga::vec_size_v<V>>{});
	}


// this function is a place to just test out whatever
void lots_of_tests()
{
	// put fun code here

	[[ maybe_unused ]]auto dm_bool = dsga::vec_like<dsga::vec3>;

	[[ maybe_unused ]] auto dm_val = dsga::vec3{1, 2, 4};
	[[ maybe_unused ]] auto dm_mag = magnitude_squared(dm_val);


	auto range_for_vec = dsga::vec4{9, 8, 7, 6};
	[[ maybe_unused ]] auto some_iter = range_for_vec.zywx.begin();

	bigger_enum be = bigger_enum::a;
	[[ maybe_unused ]] auto be_val = dsga::to_underlying<bigger_enum>(be);


	[[ maybe_unused ]] auto array_size = sizeof(std::array<std::byte, 4>);

//	test_lerp();
	test_visualizer();

	[[ maybe_unused ]] auto aa = std::array{1, 2, 3, 4, 5};
	[[ maybe_unused ]] auto a1 = dsga::vec(1, 2, 3, 4);
	[[ maybe_unused ]] auto a1a = dsga::vec2(a1.z);
	[[ maybe_unused ]] auto a1a1 = dsga::vec2(dsga::iscal(4));

	auto testvec = dsga::dvec4(0);
	auto testvec2 = dsga::dvec4(1, 2, 3, 4);

	testvec.zwx = testvec2.yzx;
	testvec.y = testvec2.w;

	auto testvec3 = dsga::dvec4(9, 8, 7, 6);
	testvec2.wyxz = testvec3;

	auto testvec4 = dsga::dvec3(10, 20, 30);

	testvec2.wxz = testvec.xzy = testvec4;

	testvec4 = testvec3.wxz;

	[[ maybe_unused ]] auto whatnext = +testvec4;

	[[ maybe_unused ]] auto shorty = bytes_to_short(static_cast<std::byte>('\b'), static_cast<std::byte>('\n'));

	[[ maybe_unused ]] auto maybemat = dsga::invoke_detail::is_dsga_matrix_v<double>;
	[[ maybe_unused ]] auto maybemat2 = dsga::invoke_detail::is_dsga_matrix_v<dsga::mat2>;
	[[ maybe_unused ]] auto maybemat3 = dsga::invoke_detail::is_dsga_matrix_v<dsga::mat<double, 4, 3>>;

	constexpr auto foo_op = [](int arg1, bool pred, double arg2) noexcept
	{
		if (pred)
		{
			return arg1 + arg2;
		}
		else
		{
			return arg1 - arg2;
		}
	};

	[[ maybe_unused ]] constexpr auto bar_op = [](int arg1, bool pred, double arg2, const dsga::mat2 &mat) noexcept
	{
		if (pred)
		{
			return arg1 + arg2 - mat[0][0];
		}
		else
		{
			return arg1 - arg2 + mat[0][0];
		}
	};

	[[ maybe_unused ]] auto baz_op = [quxx = -10]() mutable noexcept -> int { return ++quxx; };
	[[ maybe_unused ]] auto invoke_val1 = dsga::invoke<4>(baz_op);

	[[ maybe_unused ]] auto invoke_val2 = dsga::invoke<4>(foo_op,
														  dsga::vec(true, false, true, false),
														  dsga::vec(1, 2, 3, 4),
														  dsga::vec(1.1, 2.2, 3.3, 4.4));

	[[ maybe_unused ]] auto invoke_val4 = dsga::invoke<4>(foo_op,
														  dsga::ivec4(1, 2, 3, 4),
														  dsga::bvec4(true, false, true, false),
														  dsga::dvec4(1.1, 2.2, 3.3, 4.4));

	[[ maybe_unused ]] auto invoke_val5 = dsga::invoke<4>(bar_op,
														  1,
														  dsga::bvec4(true, false, true, false),
														  dsga::dvec4(1.1, 2.2, 3.3, 4.4).xyzw,
														  std::array<dsga::mat2, 4>{dsga::mat2(100), dsga::mat2(200), dsga::mat2(300), dsga::mat2(400)});

	[[ maybe_unused ]] auto invoke_val6 = dsga::invoke<4>(bar_op,
														  dsga::ivec4(1, 2, 3, 4),
														  dsga::bvec4(true, false, true, false),
														  dsga::dvec4(1.1, 2.2, 3.3, 4.4),
														  dsga::mat2(400));

	[[ maybe_unused ]] double trying_to_be_1 = std::sqrt(std::numbers::pi_v<double>) * std::numbers::inv_sqrtpi_v<double>;
	[[ maybe_unused ]] constexpr double also_trying_to_be_1 = (dsga::detail::cxcm::strict::impl::constexpr_sqrt(std::numbers::pi_v<double>) *
															   dsga::detail::cxcm::strict::impl::constexpr_rsqrt(std::numbers::pi_v<double>));
	[[ maybe_unused ]] constexpr double also_trying_to_be_1a = dsga::detail::cxcm::relaxed::sqrt(std::numbers::pi_v<double>) * std::numbers::inv_sqrtpi_v<double>;

	[[ maybe_unused ]] auto spansize1 = sizeof(std::span<double, 4>);		//  8 bytes -- double *
	[[ maybe_unused ]] auto spansize2 = sizeof(std::span<double>);			// 16 bytes -- double * + size_t

	[[ maybe_unused ]] bool signed_wchar_t = std::is_signed_v<wchar_t>;
	[[ maybe_unused ]] bool unsigned_wchar_t = std::is_unsigned_v<wchar_t>;
	[[ maybe_unused ]] bool signed_char = std::is_signed_v<char>;
	[[ maybe_unused ]] bool unsigned_char = std::is_unsigned_v<char>;

	[[ maybe_unused ]] auto work1 = swap_remove_space(" A B CD EF ");
	[[ maybe_unused ]] auto work2 = swap_remove_space("A B CD EF ");
	[[ maybe_unused ]] auto work3 = swap_remove_space("ABCDEF ");
	[[ maybe_unused ]] auto work4 = swap_remove_space("ABCDEF");

	[[ maybe_unused ]] auto num_cores = std::thread::hardware_concurrency();

	constexpr auto point = dsga::dvec3(5, 2, 0);
	constexpr auto point2 = dsga::dvec3(5, 2, 20);
	constexpr auto p1 = dsga::dvec3(0);
	constexpr auto p2 = dsga::dvec3(1, 2, 0);
	constexpr auto p3 = dsga::dvec3(5, 0.5, 1);
	[[ maybe_unused ]] constexpr auto proj  = project_to_line1(point, p1, p2);
	[[ maybe_unused ]] auto proj2_no_constexpr = project_to_line2(point, p1, p2);

#if defined(_MSC_VER) && _MSC_VER >= 1950
	// MSVC2022 seems to have a bug with this one -- it doesn't compile when it's constexpr.
	// works fine if I remove the constexpr, but then I can't test the compile-time value
	[[ maybe_unused ]] constexpr auto proj2 = project_to_line2(point, p1, p2);
#endif

	[[ maybe_unused ]] constexpr auto dist1 = distance_to_line(point, p1, p2);
	[[ maybe_unused ]] constexpr auto planeproj1 = project_to_plane1(point2, p1, p2, p3);
	[[ maybe_unused ]] constexpr auto planeproj2 = project_to_plane2(point2, p1, p2, p3);
	[[ maybe_unused ]] constexpr auto planeprojdiff = planeproj1 - planeproj2;

	[[ maybe_unused ]] auto ang1 = angle_between(point, p2);

	auto b1 = dsga::dvec3(0);
	auto b2 = dsga::dvec3(1, 2, 0);
	auto b3 = dsga::dvec3(5, 0.5, 1);
	auto b4 = dsga::dvec3(4, 0, 2);
	auto t_val = 0.375;
	auto bez1 = cubic_recursive_bezier_eval(b1, b2, b3, b4, t_val);
	auto bez2 = cubic_polynomial_bezier_eval(b1, b2, b3, b4, t_val);
	[[ maybe_unused ]] auto bezeq = dsga::equal(bez1, bez2);
	[[ maybe_unused ]] auto bezeq_all = dsga::all(bezeq);
	auto qbez1 = quadratic_recursive_bezier_eval(b2, b3, b4, t_val);
	auto qbez2 = quadratic_polynomial_bezier_eval(b2, b3, b4, t_val);
	[[ maybe_unused ]] auto qbezeq = dsga::equal(qbez1, qbez2);
	[[ maybe_unused ]] auto qbezeq_all = dsga::all(qbezeq);

	std::string foostr = "this is a long long string";
	replace(foostr, "long", "wacky");

//	test_xoshiro();

	auto e1 = test_enum{true};
	auto e2 = test_enum{false};

	[[ maybe_unused ]] auto e1val = dsga::to_underlying(e1);
	[[ maybe_unused ]] auto e2val = dsga::to_underlying(e2);

	enum_func(e1);
//	enum_func(e1val);

	[[ maybe_unused ]] dsga::dvec4 v(1, 2, 3, 4);
	[[ maybe_unused ]] dsga::dvec4 v2(9, 8, 7, 6);

	[[ maybe_unused ]] auto dot0 = dsga::innerProduct(v, v2);
	[[ maybe_unused ]] auto dot1 = dsga::dot(v, v2);
	[[ maybe_unused ]] auto dot2 = (v * v2).sum();

	constexpr dsga::dmat3 A(1, 1, 3, 0, 1, 2, 2, 1, 5);
	constexpr dsga::dmat3 B(1, 0, 2, 0, 3, 0, 4, 0, 5);
	[[ maybe_unused ]] constexpr dsga::dmat3 C(5, 3, 13, 0, 3, 6, 14, 9, 37);
	constexpr dsga::dmat2x4 E(1, 1, 3, 0, 1, 2, 2, 1);
	constexpr dsga::dmat3x2 F(5, 3, 3, 6, 14, 9);
	dsga::dmat3 AA(1, 1, 3, 0, 1, 2, 2, 1, 5);
	dsga::dmat3 BB(1, 0, 2, 0, 3, 0, 4, 0, 5);
	[[ maybe_unused ]] auto EF = E * F;

	//	std::print("{}\n", v);
	auto sw = dsga::vec_storage{1.0, 2.0, 3.0};
	dsga::vec_storage<double, 3> sw2 = {};
	sw2 = sw;
	[[ maybe_unused ]] auto sw3 = sw2;

	//auto lam = [&v]() { return (+v).ywxz; };
	//lam() = dsga::dvec4(0, 9, 2, 5).ywxz;
	//dsga::dvec3(1, 2, 4).yzx = dsga::dvec3(0, 9, 2);		// must be an lvalue for this -- default copy constructor
	dsga::dvec3(1, 2, 4).yzx = dsga::dvec3(0, 9, 2).yzx;	// yuck -- have to live with this -- default copy assignment
	dsga::dvec4(1, 2, 4, 7).yzxw = v.yzxw;					// yuck -- have to live with this -- default copy assignment
//	dsga::dvec3(1, 2, 4) = dsga::dvec3(0, 9, 2);
	[[ maybe_unused ]] auto some_rval = dsga::dvec3(1, 2, 4).yzx;
	auto bv124 = dsga::dvec3(1, 2, 4);
	bv124.yzx = dsga::dvec3(0, 9, 2);


	[[ maybe_unused ]] auto ctad1 = dsga::vec(bv124.yzx);
	[[ maybe_unused ]] auto ctad2 = dsga::vec(1, 2, 3);
	[[ maybe_unused ]] auto ctad3 = std::array{ 5.0, 6.0, 7.0 };

	swap(v, v2);
	swap(v.xy, v2.xy);
	swap(v, v2.xyzw);
	[[ maybe_unused ]] auto l = dsga::length(v);
	[[ maybe_unused ]] auto ll = dsga::normalize(dsga::dvec3(1e-15, 1e-15, 1e-15));
	[[ maybe_unused ]] auto lll = dsga::length(ll);
	double lerp_val = 0.25;
	auto lerp1 = dsga::mix(v, v2, lerp_val);
	auto lerp2 = [&v, &v2](double t)
	{
		return [&v, &v2, t]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return dsga::vec((std::lerp(v[Is], v2[Is], t))...);
		}(std::make_index_sequence<v.Size>{});
	}(lerp_val);
	[[ maybe_unused ]] auto lerp_diff = dsga::abs(lerp1 - lerp2);
	[[ maybe_unused ]] auto lerp_eq = dsga::equal(lerp1, lerp2);
	[[ maybe_unused ]] constexpr auto somelength = v.length();
	[[ maybe_unused ]] constexpr auto somesize = v.size();

	auto somearr = std::array<double, 3>{55., 44., 33.};
	std::array<double, 3>{30, 60, 90}.swap(somearr);

	auto somevec = dsga::dvec3{55., 44., 33.};
	dsga::dvec3{30, 60, 90}.swap(somevec);


	[[ maybe_unused ]] auto mult1 = (A * B) == C;
	[[ maybe_unused ]] auto mult3 = (E * F);

	constexpr dsga::dmat4 Amat(1, 1, 3, 4, 0, 1, 2, 3, 2, 1, 5, -1, 0, 0, 0, 1);
	[[ maybe_unused ]] auto Ainv1 = dsga::inverse(Amat);

	constexpr dsga::mat4 Bmat(1, 1, 3, 4, 0, 1, 2, 3, 2, 1, 5, -1, 0, 0, 0, 1);
	[[ maybe_unused ]] auto Binv1 = dsga::inverse(Bmat);

	auto Ainv = dsga::inverse(A);
	[[ maybe_unused ]] auto ident1 = (A * Ainv) == dsga::dmat3(1);
	[[ maybe_unused ]] auto ident2 = (Ainv * A) == dsga::dmat3(1);
	[[ maybe_unused ]] auto ident3 = (Ainv * A) == dsga::identity_matrix<double, 3>();
	[[ maybe_unused ]] auto matdivA = Amat / 3;
	[[ maybe_unused ]] auto matdivB = Bmat / 3.0;
//	[[ maybe_unused ]] auto badrow = Bmat.row(4);			// runtime error - out of bounds

	swap(AA, BB);

	[[ maybe_unused ]] auto my_test_vec = dsga::dvec4(77);
	[[ maybe_unused ]] auto op2 = dsga::outerProduct(v, v2);

	[[ maybe_unused ]] auto someval = v.apply([]([[ maybe_unused ]] auto x) { return x * 99.0; });
	[[ maybe_unused ]] auto someval2 = v.apply([val = 0.0](auto x) mutable { return val += x; });
	[[ maybe_unused ]] auto applied = v.apply(some_apply_func<double>);

	[[ maybe_unused ]] auto somebools = dsga::compNot(dsga::notEqual(dsga::ivec3(1, 2, 3), dsga::ivec3(3, 2, 1)));
	[[ maybe_unused ]] auto someands1 = dsga::compAnd(somebools, dsga::compNot(somebools));
	[[ maybe_unused ]] auto someands2 = dsga::compAnd(somebools, dsga::bvec3(true));
	[[ maybe_unused ]] auto someors1 = dsga::compOr(somebools, dsga::compNot(somebools));
	[[ maybe_unused ]] auto someors2 = dsga::compOr(somebools, dsga::bvec3(false));

	[[ maybe_unused ]] dsga::dvec4 asdf(v);
	[[ maybe_unused ]] auto qwer = dsga::dvec4(v);
	[[ maybe_unused ]] dsga::dvec4 fizz = v;
	[[ maybe_unused ]] dsga::dvec4 buzz = dsga::ivec4(79).zzyx;
	[[ maybe_unused ]] auto bang = v.cshift(0);

	std::valarray<double> va{1, 2, 3, 4};
	dsga::dvec4 vd(1, 2, 3, 4);
	[[ maybe_unused ]] auto va_shift1 = va.shift(-33);
	[[ maybe_unused ]] auto dsga_shift1 = vd.shift(-33);
	[[ maybe_unused ]] auto va_cshift1 = va.cshift(-33);
	[[ maybe_unused ]] auto dsga_cshift1 = vd.cshift(-33);
	[[ maybe_unused ]] auto va_shift2 = va.shift(0);
	[[ maybe_unused ]] auto dsga_shift2 = vd.shift(0);
	[[ maybe_unused ]] auto va_cshift2 = va.cshift(0);
	[[ maybe_unused ]] auto dsga_cshift2 = vd.cshift(0);

	dsga::mat2x4 d24(5, 5, 5, 5, 5, 5, 5, 5);
	dsga::mat4x2 d42(9, 9, 9, 9, 9, 9, 9, 9);

	[[ maybe_unused ]] dsga::mat4 via_d24(d24);
	[[ maybe_unused ]] dsga::mat4 via_d42(d42);

	[[ maybe_unused ]] dsga::dmat4 via_via_d42(via_d42);


	auto mat_lam = [](dsga::dmat4 d4)
	{
		auto temp = d4;
		d4 = temp;
	};
	dsga::mat3 m3(3, 3, 3, 3, 3, 3, 3, 3, 3);
	mat_lam(dsga::mat4(m3));

	[[ maybe_unused ]] dsga::mat3x3 ex1(dsga::mat4x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
	[[ maybe_unused ]] dsga::mat2x3 ex2(dsga::mat4x2(1, 2, 3, 4, 5, 6, 7, 8));
	[[ maybe_unused ]] dsga::mat4x4 ex3(dsga::mat3x3(1, 2, 3, 4, 5, 6, 7, 8, 9));
	[[ maybe_unused ]] auto ex4 = dsga::dvec4(-10, ex1);

	[[ maybe_unused ]] auto what_am_i = dsga::mat3(0, 1, 0, 0, 0, 1, 1, 0, 0) * dsga::vec3(1, 2, 3);
	[[ maybe_unused ]] auto you_are_this = dsga::vec3(1, 2, 3).zxy;

	auto reverse_iter = ex4.wzyx.crbegin();
	[[ maybe_unused ]] auto reverse_end = ex4.wzyx.crend();
	auto reverse_next = reverse_iter + 1;
	[[ maybe_unused ]] auto reverse_lessthan = reverse_iter < reverse_next;

	auto forward_iter = ex4.wzyx.cbegin();
	[[ maybe_unused ]] auto forward_end = ex4.wzyx.cend();
	auto forward_next = forward_iter + 1;
	[[ maybe_unused ]] auto forward_lessthan = forward_iter < forward_next;

	[[ maybe_unused ]] auto vec1 = dsga::ivec3{1, 2, 3};
	[[ maybe_unused ]] auto vec2 = dsga::ivec3{};
	[[ maybe_unused ]] auto mat1 = dsga::mat2{1,2,3,4};

	[[ maybe_unused ]] auto agginit2 = vec1.zx;

	[[ maybe_unused ]] auto bag_sum = dsga::dscal(3) + dsga::dscal(2);
	[[ maybe_unused ]] auto swiz_sum = dsga::dscal(3).x + dsga::dscal(2).x;
	[[ maybe_unused ]] auto some_rads = dsga::radians(dsga::dscal(45).x);
	[[ maybe_unused ]] auto unop1 = +dsga::dscal(45);
	[[ maybe_unused ]] auto unop2 = -dsga::dscal(45).x;
	[[ maybe_unused ]] auto swiz_func_val1 = dsga::swizzle(dsga::dvec4(1, 2, 3, 4), 2);
	[[ maybe_unused ]] auto swiz_func_val2 = dsga::swizzle(dsga::dvec4(1, 2, 3, 4).xyz, 2, 2, 0);

	[[ maybe_unused ]] auto neq1 = dsga::notEqual(dsga::dvec4(0.5).xy, dsga::vec(3.0).xx);
	[[ maybe_unused ]] auto d_to_ll = dsga::doubleBitsToLongLong(dsga::dvec4(0.5));

	dsga::iscal some_scalar = 34;
	[[ maybe_unused ]] dsga::ivec3 some_vector = some_scalar.xxx;
	[[ maybe_unused ]] auto somesum1 = dsga::iscal(34) + 8;				// somesum1 is of type int
	[[ maybe_unused ]] auto somesum2 = dsga::ivec2(34) + 8;				// somesum2 is of type dsga::vec<int,2>
	[[ maybe_unused ]] auto somesum3 = dsga::ivec2(34) + 50.0;			// somesum3 is of type dsga::vec<double,2>
	[[ maybe_unused ]] auto isome_vec = dsga::ivec4(12, 54, 88, 99);	// some_vec is of type dsga::vec<int,4>
	[[ maybe_unused ]] auto ivalue3 = dsga::iscal(34) + +isome_vec.y;	// value3 is of type int
	[[ maybe_unused ]] auto ivalue4 = isome_vec.z + isome_vec.x;		// value4 is of type int
	[[ maybe_unused ]] auto ivalue5 = 100 + isome_vec.w;				// value5 is of type int

	[[ maybe_unused ]] auto qsome_vec = dsga::uvec4(0, 1, 2, 3);		// 4 dimensional vec
	[[ maybe_unused ]] auto qsome_swiz = qsome_vec.xyz;					// 3 dimensional swizzle_vec
//	[[ maybe_unused ]] auto qswiz_twice1 = qsome_vec.xyz.yx;			// error, can't swizzle swizzle_vec
//	[[ maybe_unused ]] auto qswiz_twice2 = qsome_swiz.yx;				// error, can't swizzle swizzle_vec
	[[ maybe_unused ]] auto qswiz_again = dsga::uvec2(qsome_swiz).yx;	// ok, converted to vec first

	[[ maybe_unused ]] auto qrsome_vec = dsga::uvec3(0, 1, 2);					// returns a vec<unsigned int, 3>
	[[ maybe_unused ]] auto swiz_1d = dsga::swizzle(qrsome_vec, 1);				// returns an unsigned int
	[[ maybe_unused ]] auto swiz_2d = dsga::swizzle(qrsome_vec, 2, 0);			// returns a vec<unsigned int, 2>
//	[[ maybe_unused ]] auto swiz_3d = dsga::swizzle(qrsome_vec, 2, 0, 3);		// error, index == 3 is beyond dimension 3 indexing range of some_vec
	[[ maybe_unused ]] auto swiz_4d = dsga::swizzle(qrsome_vec, 2, 0, 0, 1);	// returns a vec<unsigned int, 4>
	[[ maybe_unused ]] auto rt_ct_swiz = dsga::swizzle(qrsome_vec, 2, 0, 0, 1).xyzw;	// returns swizzle_vec version of swizzle() result - not dangling because it makes a copy of the swizzle_vec
	[[ maybe_unused ]] auto qrsome_swiz = dsga::swizzle(qrsome_vec, 1, 1, 1, 1).zxwy;	// makes a copy of an swizzle_vec

	auto pos = dsga::vec4(10, 20, 30, 40);
	pos.xy = dsga::vec3(1.0, 2.0, 3.0).yz;
	pos.zw = pos.wz;

	//auto pos_iter = pos.xyz.begin();
	//auto pos_end = pos.xyz.end();
	//while (pos_iter < pos_end)
	//{
	//	std::printf("iter val = %f\n", *pos_iter);
	//	++pos_iter;
	//}

	[[ maybe_unused ]] auto d2l1 = distance_to_line(dsga::dvec3{}, dsga::dvec3{1, 0, 0}, dsga::dvec3{0, 1, 0});

	auto p2ppoint = dsga::dvec3{std::numbers::sqrt2 / 2, std::numbers::sqrt2 / 2, 5.5};

	[[ maybe_unused ]] auto p2p1 = project_to_plane1(p2ppoint, dsga::dvec3{1, 2, 3}, dsga::dvec3{6, 5, 4}, dsga::dvec3{3, 6, 1});
	[[ maybe_unused ]] auto p2p2 = project_to_plane2(p2ppoint, dsga::dvec3{1, 2, 3}, dsga::dvec3{6, 5, 4}, dsga::dvec3{3, 6, 1});
	[[ maybe_unused ]] auto p2pdiff = p2p1 - p2p2;
	[[ maybe_unused ]] auto p2pdist = dsga::length(p2pdiff);
//	std::cout << "p2pdiff = " << p2pdiff << "\n";
//	std::cout << "p2pdist = " << p2pdist << "\n";

	[[ maybe_unused ]] auto pows = dsga::pow(dsga::scal(2), dsga::scal(2).x);
	[[ maybe_unused ]] auto dist = dsga::distance(1.5, 6.0);

	[[ maybe_unused ]] auto pvec = dsga::vec3{4, 5, 6};										// [4, 5, 6]
	[[ maybe_unused ]] auto pmat = dsga::mat2{10, 20, 30, 40};								// [[10, 20], [30, 40]]
	pmat[0] = pvec.zy;																		// [[6, 5], [30, 40]]

	[[ maybe_unused ]] auto truth1 = pvec.query([](float val) {return val < 5.5f; });		// [true, true, false]
	[[ maybe_unused ]] auto truth2 = dsga::mix(dsga::vec3(0), dsga::vec3(99), truth1);		// [99, 99, 0]

	[[ maybe_unused ]] auto is_same1 = (truth1 == dsga::bvec3{true, true, false});			// true
	[[ maybe_unused ]] auto is_same2 = (truth2 == dsga::vec3{99, 99, 0});					// true

	[[ maybe_unused ]] auto scal1 = dsga::fscal{2.5};
	[[ maybe_unused ]] auto scal2 = float{2.5f};

	[[ maybe_unused ]] auto hash1 = std::hash<dsga::mat2>{}(dsga::mat2(1, 2, 3, 4));
	[[ maybe_unused ]] auto hash2 = std::hash<dsga::vec4>{}(dsga::vec4(1, 2, 3, 4));
	[[ maybe_unused ]] auto hash3 = std::hash<dsga::vec4>{}(dsga::vec4(4, 3, 2, 1));
	[[ maybe_unused ]] auto hash3a = std::hash<dsga::swizzle_vec<float, 4, 4, 0, 1, 2, 3>>{}(dsga::vec4(4, 3, 2, 1).xyzw);
	[[ maybe_unused ]] auto hash3b = std::hash<dsga::swizzle_vec<float, 4, 4, 3, 2, 1, 0>>{}(dsga::vec4(4, 3, 2, 1).wzyx);

	[[ maybe_unused ]] auto op_mat = dsga::outerProduct(dsga::vec3(2, 4, 6), dsga::vec2(5, 7));

	auto convert_vec = static_cast<dsga::dvec3>(dsga::vec3(0, 1, 2));
	convert_vec = dsga::dvec3(9, 8, 7);

	[[ maybe_unused ]] auto vec_mat = dsga::mat4(dsga::dvec4(0), dsga::dvec4(1), dsga::dvec4(2), dsga::dvec4(3));

//	std::cout << "test mat = " << vec_mat << "\n";
//	std::cout << "test vec = " << convert_vec << "\n";
//	std::cout << "test vec2 = " << convert_vec.zyx << "\n";

//	std::cout << std::format("test mat = {}\n", vec_mat);
//	std::cout << std::format("test vec = {}\n", convert_vec);
//	std::cout << std::format("test vec2 = {}\n", convert_vec.zyx);

	[[ maybe_unused ]] auto somemod = dsga::mod(1., 0.);
	[[ maybe_unused ]] auto bits0 = dsga::doubleBitsToUlongLong(somemod);		// 0xfff8000000000000 - negative NaN
	[[ maybe_unused ]] constexpr auto infstuff = std::numeric_limits<double>::infinity();
	[[ maybe_unused ]] auto infstuff2 = infstuff * 0.0;
	[[ maybe_unused ]] auto infstuff3 = -infstuff < 0;
	[[ maybe_unused ]] constexpr auto bits1 = dsga::doubleBitsToUlongLong(std::numeric_limits<double>::infinity());		// 0x7ff0000000000000 - positive infinity,	0xfff0000000000000 - negative infinity
	[[ maybe_unused ]] auto bits2 = dsga::doubleBitsToUlongLong(infstuff2);		// 0x7ff8000000000000 - positive NaN,		0xfff8000000000000 - negative NaN
	[[ maybe_unused ]] constexpr auto bits3 = dsga::doubleBitsToUlongLong(-std::numeric_limits<double>::infinity());		// 0x7ff0000000000000 - positive infinity,	0xfff0000000000000 - negative infinity

	[[ maybe_unused ]] auto spn1 = std::span(convert_vec);
	[[ maybe_unused ]] auto spn2 = std::span(std::as_const(convert_vec));
//	[[ maybe_unused ]] auto spn3 = std::span(convert_vec.zyx);

	[[ maybe_unused ]] auto not_op_plus = convert_vec.z;
	[[ maybe_unused ]] auto op_plus = +convert_vec.z;
	[[ maybe_unused ]] auto op_plus2 = +convert_vec.xz;

//	std::cout << '\a' << "hello world\n";

	std::vector<std::uint8_t> intvec1 = {0x00, 0x01, 0x02, 0x03};
	std::vector<std::uint8_t> intvec2 = {0x09, 0x08, 0x07, 0x06, 0x05, 0x04};

	std::vector<unsigned char> ucharvec1 = {0x00, 0x01, 0x02, 0x03};
	std::vector<std::uint8_t> ucharvec2 = {0x09, 0x08, 0x07, 0x06, 0x05, 0x04};

	[[ maybe_unused ]] bool signed_char8_t = std::is_signed_v<char8_t>;
	[[ maybe_unused ]] bool unsigned_char8_t = std::is_unsigned_v<char8_t>;

	using namespace std::string_literals;

	[[ maybe_unused ]] std::u8string test_u8str = u8"abc";
	[[ maybe_unused ]] std::string test_str = "abc";
	[[ maybe_unused ]] std::u8string test_u8str2 = u8"abc"s;
	[[ maybe_unused ]] std::string test_str2 = "abc"s;
	[[ maybe_unused ]] std::u8string test_u8str3 = u8"\u00E1\aabc"s;
	[[ maybe_unused ]] std::string test_str3 = reinterpret_cast<const char *>(test_u8str3.c_str());			// Ok
	[[ maybe_unused ]] std::string test_str3a = "\u00E1\aabc";
	[[ maybe_unused ]] std::u8string test_u8str3a = reinterpret_cast<const char8_t *>(test_str3.c_str());	// UB

//	[[ maybe_unused ]] auto aswiggle = unsigned char('\u00E1');
	[[ maybe_unused ]] unsigned char bellc = u'\u00E1';
//	[[ maybe_unused ]] char bell = '\u00E1';
	[[ maybe_unused ]] char8_t bell8[] = u8"\u00E1";
	[[ maybe_unused ]] int bell8len = sizeof(bell8);

//	[[ maybe_unused ]] auto u8works = reinterpret_cast<const char *>(u8"text");	// Ok.
//	[[ maybe_unused ]] auto u8ub = reinterpret_cast<const char8_t *>("text");	// Undefined behavior

	intvec1.reserve(intvec1.size() + intvec2.size());
	intvec1.insert(intvec1.end(), intvec2.begin(), intvec2.end());

	auto center = dsga::dvec2(-5687.78, 6010.346);
	auto start = dsga::dvec2(182.109, 147.308);
	auto end = dsga::dvec2(178.651, 143.849);
	auto arcvec1 = start - center;
	auto arcvec2 = end - center;
	auto radius1 = dsga::length(arcvec1);
	auto radius2 = dsga::length(arcvec2);
	[[ maybe_unused ]] auto radius_delta = dsga::abs(radius1 - radius2);
	auto radians1 = angle_between(arcvec1, arcvec2);
	[[ maybe_unused ]] auto degrees1 = dsga::degrees(radians1);
	auto radians2 = vect_angle(arcvec1, arcvec2);
	[[ maybe_unused ]] auto degrees2 = dsga::degrees(radians2);
	[[ maybe_unused ]] auto radians_delta = dsga::abs(radians1 - radians2);
	[[ maybe_unused ]] auto degrees_delta = dsga::abs(degrees1 - degrees2);

//	radians1 -> 0.0005895373946'9708622
//	radians2 -> 0.0005895373945'7691477
//	delta    ->               1.2017145091525183e-13
// 
//	degrees1 -> 0.033778004581'281239
//	degrees2 -> 0.033778004574'395920
//	delta    ->              6.8853187040751607e-12
// 
//	radius1 -> 8296.4336592155669
//	radius2 -> 8296.4329521047766
//	delta   ->    0.00070711079024476930

	[[ maybe_unused ]] auto half_pi = (std::numbers::pi_v<double> / 2.0);
	[[ maybe_unused ]] auto acos_large_radians = dsga::acos(4.e-9);
	[[ maybe_unused ]] auto cos_large_radians = dsga::cos(acos_large_radians);
	[[ maybe_unused ]] auto all_but_tol = 1.0 - 4.e-9;
	[[ maybe_unused ]] auto acos_small_radians = dsga::acos(all_but_tol);
	[[ maybe_unused ]] auto cos_small_radians = dsga::cos(acos_small_radians);
	[[ maybe_unused ]] auto acos_small_degrees = dsga::degrees(acos_small_radians);

	[[ maybe_unused ]] auto delta_pi = dsga::abs(half_pi - acos_large_radians);
	[[ maybe_unused ]] auto cos_delta = dsga::cos(delta_pi);
	[[ maybe_unused ]] auto sin_delta = dsga::sin(delta_pi);
	[[ maybe_unused ]] auto acos_large_degrees = dsga::degrees(acos_large_radians);
	[[ maybe_unused ]] auto delta_90 = dsga::abs(90.0 - acos_large_degrees);
	[[ maybe_unused ]] auto delta_90_2 = dsga::degrees(delta_pi);

//	for acos(close_to_1), which is in the range where acos() is less stable (values considered more unreliable)
//
//	rad_near_0 -> 8.9442719106266479e-05
//	deg_near_0 -> 0.0051246903129631999
// 
//	half_pi	   -> 1.5707963267948966
// 
//	for acos(close_to_0), which is in the range where acos() is stable (values considered more reliable)
//
//	radians    -> 1.570796322'7948967
//	delta_pi   ->           3.9999998868722741e-09
//	degrees    -> 89.9999997'70816885
//	delta_90   ->          2.2918311515240930e-07
//	delta_90_2 ->          2.2918311157058805e-07
//
// 	-----------------------------------------------------------------------------------------------
// haven't done any extensive analysis, but acos(close_to_1) doesn't seem as unreliable as reported
// 	-----------------------------------------------------------------------------------------------

	constexpr auto pi = std::numbers::pi_v<double>;
	[[ maybe_unused ]] constexpr auto sqpi = dsga::detail::cxcm::impl::constexpr_sqrt(pi);
	[[ maybe_unused ]] constexpr auto invsqpi = 1.0 / dsga::detail::cxcm::relaxed::impl::fast_rsqrt(pi);
	[[ maybe_unused ]] constexpr auto fastsqpi = 1.0 / dsga::detail::cxcm::fast_rsqrt(pi);
	[[ maybe_unused ]] auto pidelta1 = dsga::abs(sqpi - invsqpi);
	[[ maybe_unused ]] auto sqpi2 = std::sqrt(pi);
	[[ maybe_unused ]] auto pidelta2 = dsga::abs(sqpi - sqpi2);
	auto simple_vec = dsga::vec4(1.0, 2.0, 3.0, 4.0);
	[[ maybe_unused ]] auto dfasat = dsga::inversesqrt(simple_vec);
	[[ maybe_unused ]] auto stdfasat = simple_vec.apply([](float val) -> float { return 1.0f / static_cast<float>(std::sqrt(val)); });
	[[ maybe_unused ]] auto stdfasat_eq = dsga::equal(dfasat, stdfasat);
	[[ maybe_unused ]] constexpr auto dfasat1 = 1.0 / dsga::inversesqrt(pi);
	[[ maybe_unused ]] constexpr auto dfasat2 = 1.0 / dsga::fast_inversesqrt(pi);
	[[ maybe_unused ]] auto dfasat_delta = dsga::abs(dfasat - stdfasat);
	[[ maybe_unused ]] auto dfasat_delta1 = dsga::abs(dfasat1 - stdfasat.x);
	[[ maybe_unused ]] auto dfasat_delta2 = dsga::abs(dfasat2 - stdfasat.x);
	[[ maybe_unused ]] auto pi_delta = dsga::abs(pi - (sqpi * sqpi));
	[[ maybe_unused ]] constexpr auto pi_again = sqpi / dsga::fast_inversesqrt(pi);
	[[ maybe_unused ]] auto header = pm6_header_string;

	//const char *use_after_free = nullptr;
	//{
	//	std::string to_be_deleted = header;
	//	use_after_free = to_be_deleted.c_str();
	//}

	//[[ maybe_unused ]] auto somechar = use_after_free[1];

	[[ maybe_unused ]] auto unary_plus_value1 = +center;
	[[ maybe_unused ]] auto unary_plus_value2 = +center.yxy;

	constexpr auto myang1 = pi / 3;
	constexpr auto myang2 = pi / 6;
	constexpr auto mylen1 = 3;
	constexpr auto mylen2 = 2;
	auto ang_vec1 = dsga::dvec2(mylen1 * dsga::cos(myang1), mylen1 * dsga::sin(myang1));
//	auto ang_vec1 = dsga::dvec2(1.5000000000000004, 2.5980762113533160);	// len 3, angle = pi / 3
	[[ maybe_unused ]] auto ang_1 = dsga::degrees(dsga::atan(ang_vec1.y, ang_vec1.x));
	auto ang_vec2 = dsga::dvec2(mylen2 * dsga::cos(myang2), mylen2 * dsga::sin(myang2));
//	auto ang_vec2 = dsga::dvec2(1.7320508075688774, 0.99999999999999989);	// len 2, angle = pi / 6
	[[ maybe_unused ]] auto ang_2 = dsga::degrees(dsga::atan(ang_vec2.y, ang_vec2.x));

	[[ maybe_unused ]] auto len1 = dsga::length(ang_vec1);
	[[ maybe_unused ]] auto len2 = dsga::length(ang_vec2);

	[[ maybe_unused ]] auto ang_rad1 = dsga::degrees(angle_between(ang_vec1, ang_vec2));
	[[ maybe_unused ]] auto ang_rad2 = dsga::degrees(vect_angle(ang_vec1, ang_vec2));
	[[ maybe_unused ]] auto ang_rad_delta = dsga::abs(angle_between(ang_vec1, ang_vec2) - vect_angle(ang_vec1, ang_vec2));

//	[[ maybe_unused ]] auto stringsize = sizeof std::string;
//	std::cout << "sizeof std::string = " << stringsize << '\n';		// 32 release, 40 debug

	auto some_scal = dsga::dscal(-5.5);
	[[ maybe_unused ]] auto some_scal_length = dsga::length(some_scal);

	auto centerA = dsga::vec(106.80800000000001, 158.41999999999999);
	auto startA = dsga::vec(106.62300000000000, 158.21600000000001);
	auto endA = dsga::vec(106.68300000000001, 158.66499999999999);
	auto arcvec1A = startA - centerA;
	auto arcvec2A = endA - centerA;
	auto radius1A = dsga::length(arcvec1A);
	auto radius2A = dsga::length(arcvec2A);
	[[ maybe_unused ]] auto radius_deltaA = dsga::abs(radius1A - radius2A);
	auto radians1A = angle_between(arcvec1A, arcvec2A);
	auto degrees1A = dsga::degrees(radians1A);
	auto radians2A = vect_angle(arcvec1A, arcvec2A);
	auto degrees2A = dsga::degrees(radians2A);
	[[ maybe_unused ]] auto radians_deltaA = dsga::abs(radians1A - radians2A);
	[[ maybe_unused ]] auto degrees_deltaA = dsga::abs(degrees1A - degrees2A);

	auto c90 = dsga::mat3(0, -1, 0,
						  1,  0, 0,
						  0,  0, 1);
	auto c90inv = dsga::inverse(c90);
	[[ maybe_unused ]] auto samemat = c90 == c90inv;

	auto somerotmat = dsga::mat3(-1, 0, 0,
								  0, 0, 1,
								  0, 1, 0);
	auto somerotmatinv = dsga::inverse(somerotmat);
	[[ maybe_unused ]] auto samerotmat = somerotmat == somerotmatinv;
	[[ maybe_unused ]] auto implicit_cast_mat = dsga::dmat3(somerotmat);
	[[ maybe_unused ]] auto implicit_cast_mat2 = dsga::mat3(implicit_cast_mat);
}

static void more_tests()
{
	auto v1 = dsga::dvec3(67, 93, 41);
	auto v2 = dsga::dvec3(11, 22, 33);
	[[ maybe_unused ]] auto v2_4 = dsga::dvec4(v2, 1);
	auto ang1 = angle_between(v1, v2);
	auto ang2 = vect_angle(v1, v2);
	[[ maybe_unused ]] auto angle_delta = dsga::abs(ang1 - ang2);

	auto somemat = dsga::dmat4x3(1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0);
	[[ maybe_unused ]] auto sometranspose = dsga::transpose(somemat);

	auto aturn_mat = dsga::dmat3(0, -1, 0,
								 0, 0, -1,
								 -1, 0, 0);
	[[ maybe_unused ]] auto three_to_four = dsga::dmat4(aturn_mat);
	auto aturn_mat_inv = dsga::inverse(aturn_mat);
	[[ maybe_unused ]] auto aturn_mat_eq = aturn_mat == aturn_mat_inv;

	[[ maybe_unused ]] auto center1 = triangle_incenter(dsga::scal(1), dsga::scal(2), dsga::scal(3));
	[[ maybe_unused ]] auto center2 = triangle_incenter(dsga::vec2(1, 1), dsga::vec2(4, 1), dsga::vec2(4, 5));
	[[ maybe_unused ]] auto center3 = triangle_incenter(dsga::vec3(1, 1, 7), dsga::vec3(4, 1, 7), dsga::vec3(4, 5, 7));
	[[ maybe_unused ]] auto c3radius = three_point_circle_radius(dsga::vec3(1, 1, 7), dsga::vec3(4, 1, 7), dsga::vec3(4, 5, 7));

	auto angle = std::numbers::sqrt3_v<double> / 2.0;					// 0.8660254037844386
	auto d1 = dsga::dvec4(dsga::cos(angle), dsga::sin(angle), 0, 0);
	auto d2 = dsga::dvec4(-dsga::sin(angle), dsga::cos(angle), 0, 0);
	auto d3 = dsga::dvec4(0, 0, 1, 0);
	auto d4 = dsga::dvec4(3, 5, 7, 1);

	auto m1 = dsga::dmat4(d1, d2, d3, d4);

	auto m2inv1 = dsga::inverse(m1);
	auto m2inv2 = dsga::transform_inverse(m1);
	[[ maybe_unused ]] auto samemat = m2inv1 == m2inv2;
	[[ maybe_unused ]] auto samemat1 = dsga::within_box(m2inv1, m2inv2, 1e-10);
	[[ maybe_unused ]] auto roundtrip1 = m1 * m2inv1;
	[[ maybe_unused ]] auto roundtrip2 = m1 * m2inv2;

	auto doubleinverse = dsga::inverse(dsga::inverse(m1));
	auto doubleinverse2 = dsga::transform_inverse(dsga::transform_inverse(m1));
	[[ maybe_unused ]] auto samemat2a = doubleinverse == doubleinverse2;
	[[ maybe_unused ]] auto samemat3 = m1 == doubleinverse;
	[[ maybe_unused ]] auto samemat4 = m1 == doubleinverse2;

	[[ maybe_unused ]] bool samemat2 = transform_inverse_example();

	[[ maybe_unused ]] bool transform_success = dsga::is_transformation_matrix(m2inv2, 1e-10);

	[[ maybe_unused ]] auto rot_2d = dsga::rot_2d(std::numbers::pi_v<double> / 3.0);
	[[ maybe_unused ]] auto rot_x = dsga::rot_x_axis(std::numbers::pi_v<double> / 3.0);
	[[ maybe_unused ]] auto rot_y = dsga::rot_y_axis(std::numbers::pi_v<double> / 3.0);
	[[ maybe_unused ]] auto rot_z = dsga::rot_z_axis(std::numbers::pi_v<double> / 3.0);

	[[ maybe_unused ]] auto rot_fixer = renormalize_transformation_matrix(dsga::rot_any_axis(std::numbers::pi_v<double>, dsga::dvec3(1, 1, 0), dsga::dvec3(7)), 1e-10);
	auto rot_any = dsga::rot_any_axis(std::numbers::pi_v<double>, dsga::dvec3(1, 1, 0), dsga::dvec3(1, 2, 3));
	auto trans_mat = dsga::translation_matrix(dsga::vec(8.0, 8.0, 8.0));
	auto rot_any2 = trans_mat * rot_any;
	[[ maybe_unused ]] auto transformed = rot_any2 * dsga::dvec4(3, 2, 0, 1);
	auto rot_transpose = dsga::transpose(rot_any2);
	[[ maybe_unused ]] auto transformed2 = dsga::dvec4(3, 2, 0, 1) * rot_transpose;
	[[ maybe_unused ]] bool is_xform_mat = dsga::is_transformation_matrix(rot_any, 1e-10);
	[[ maybe_unused ]] bool is_xform_mat2 = dsga::is_transformation_matrix(rot_any2, 1e-10);

	[[ maybe_unused ]] auto ident4 = dsga::identity_matrix<double, 4>();
	auto tol = 1e-10;
	auto vec_for_fix = dsga::dvec4(1, 1e-13, 3, 1e-11);
	auto fix_for_zeros = [tol](double x) noexcept { return dsga::abs(x) <= tol ? 0.0 : x; };
	[[ maybe_unused ]] auto desired_fix = vec_for_fix.apply(fix_for_zeros);

	[[ maybe_unused ]] auto mat_for_fix = dsga::renormalize_transformation_matrix(rot_any2, tol);

	auto mat_times_transpose1 = mat_for_fix * dsga::transpose(mat_for_fix);
	auto mat_times_transpose2 = dsga::transpose(mat_for_fix) * mat_for_fix;
	[[ maybe_unused ]] auto mtt_eq1 = mat_times_transpose1 == mat_times_transpose2;
	[[ maybe_unused ]] auto mat_times_transpose_det1 = dsga::determinant(mat_times_transpose1);
	[[ maybe_unused ]] auto mat_times_transpose_det2 = dsga::determinant(mat_times_transpose2);
	[[ maybe_unused ]] auto mtt1_mtt2 = mat_times_transpose1 * mat_times_transpose2;
	[[ maybe_unused ]] auto mtt2_mtt1 = mat_times_transpose2 * mat_times_transpose1;
	[[ maybe_unused ]] auto mtt_eq2 = mtt1_mtt2 == mtt2_mtt1;
	[[ maybe_unused ]] auto mtt_eq3 = mtt1_mtt2 == dsga::transpose(mtt2_mtt1);
	[[ maybe_unused ]] auto mtt_eq4 = dsga::transpose(mtt1_mtt2) == mtt2_mtt1;

	auto c1 = dsga::dvec3(1, 2, 3);
	auto c2 = dsga::dvec3(-3, 2, -1);

	auto cv1 = dsga::cross(c1, c2);
	auto cv2 = dsga::cross(c1.xyz, c2.xyz);
	[[ maybe_unused ]] auto cveq = cv1 == cv2;

	[[ maybe_unused ]] auto qtest = c1.query([]([[ maybe_unused ]] double val) { return val < 2; });

	[[ maybe_unused ]] dsga::dvec4 built_from_pieces{ c1.xy, 1, 0 };
	[[ maybe_unused ]] auto built_from_pieces2 = dsga::dvec4{ c1.xy, 1, 0 };
	[[ maybe_unused ]] dsga::dvec4 built_from_pieces3( c1.xy, 1, 0 );
	[[ maybe_unused ]] dsga::dvec3 built_from_pieces4( built_from_pieces3.wzyx );
	built_from_pieces4 = built_from_pieces3.yxzy;
	[[ maybe_unused ]] dsga::dvec4 built_from_pieces5( 13, 3, c1.zyx );
}

[[ maybe_unused ]] static void modf_test()
{
	constexpr auto v1 = dsga::dvec4(2.3, -2.3, 2.7, -2.7);
	auto intpart = dsga::dvec4();
	auto fracpart = dsga::modf(v1, intpart);
	[[ maybe_unused ]] auto sum1 = intpart.sum();
	[[ maybe_unused ]] auto sum2 = fracpart.sum();
	[[ maybe_unused ]] auto modftest = v1.apply([](double val) { double ip; double frac = std::modf(val, &ip); std::printf("%g\n", ip); return frac; });
}

template <std::floating_point T>
constexpr inline T PI = std::numbers::pi_v<T>;

template <std::floating_point T>
constexpr inline T tau = T(2 * PI<T>);

template <std::floating_point T>
constexpr inline T TWOPI = tau<T>;

// seems to be worse than full_ang()
double ang_full(double ang)
{
	double turns = dsga::fract(dsga::abs(ang) / tau<double>);

	if (ang < 0)
	{
		turns = (1.0 - turns);
	}

	return tau<double> * turns;
}

// seems to be worse than ang_in_minuspi_pluspi()
double ang_norm(double ang)
{
	double turns = dsga::fract(dsga::abs(ang) / tau<double>);

	if (ang < 0.0)
	{
		turns = (1.0 - turns);
	}

	if (turns > 0.5)
	{
		turns = (turns - 1.0);
	}

	return tau<double> * turns;
}


// Convert an angle in any system to the system [0, TWOPI).
// Convert an angle to a value in the range (0 <= value < TWOPI).
double full_ang(double angle)
{
	while (angle < 0.0)
		angle += TWOPI<double>;

	while (angle >= TWOPI<double>)
		angle -= TWOPI<double>;

	return angle;
}

// Convert an angle in any system to the system (-PI, PI).
// RETURNS: a double value in the range (-PI <= return value <= PI).
// NOTE:	very similar to full_ang()
double ang_in_minuspi_pluspi(double ang)							// ang is any real number radian value
{
	while (ang < -PI<double>)
		ang += TWOPI<double>;

	while (ang > PI<double>)
		ang -= TWOPI<double>;

	return ang;
}

static void even_more_tests()
{
	auto pi = PI<double>;
	auto angs1 = dsga::dvec4(pi / 2, -pi / 2, pi, 0);
	auto angs2 = dsga::dvec4(12, -12, -1, 1);

	[[ maybe_unused ]] auto full1 = angs1.apply(full_ang);
	[[ maybe_unused ]] auto full2 = angs2.apply(full_ang);
	[[ maybe_unused ]] auto full3 = angs1.apply(ang_full);
	[[ maybe_unused ]] auto full4 = angs2.apply(ang_full);
	[[ maybe_unused ]] auto norm1 = angs1.apply(ang_in_minuspi_pluspi);
	[[ maybe_unused ]] auto norm2 = angs2.apply(ang_in_minuspi_pluspi);
	[[ maybe_unused ]] auto norm3 = angs1.apply(ang_norm);
	[[ maybe_unused ]] auto norm4 = angs2.apply(ang_norm);

	[[ maybe_unused ]] auto full1_comp = dsga::equal(full1, full3);
	[[ maybe_unused ]] auto full2_comp = dsga::equal(full2, full4);
	[[ maybe_unused ]] auto norm1_comp = dsga::equal(norm1, norm3);
	[[ maybe_unused ]] auto norm2_comp = dsga::equal(norm2, norm4);

	constexpr auto v1 = dsga::dvec4(1, 2, 3, 4);
	constexpr auto v2 = dsga::dvec4(5, 6, 7, 8);
	constexpr auto compless = dsga::lessThan(v1, v2);
	[[ maybe_unused ]] constexpr auto compall = dsga::all(compless);

//	modf_test();

	[[ maybe_unused ]] constexpr auto comp1 = dsga::lessThan(dsga::dvec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN()), dsga::dvec4(3, 2, 1, 0));
	[[ maybe_unused ]] constexpr auto comp2 = dsga::lessThanEqual(dsga::dvec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN()), dsga::dvec4(3, 2, 1, 0));
	[[ maybe_unused ]] constexpr auto comp3 = dsga::greaterThan(dsga::dvec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN()), dsga::dvec4(3, 2, 1, 0));
	[[ maybe_unused ]] constexpr auto comp4 = dsga::greaterThanEqual(dsga::dvec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN()), dsga::dvec4(3, 2, 1, 0));
	[[ maybe_unused ]] constexpr auto comp5 = dsga::equal(dsga::dvec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN()), dsga::dvec4(3, 2, 1, 0));
	[[ maybe_unused ]] constexpr auto comp6 = dsga::notEqual(dsga::dvec4(1, 2, 3, std::numeric_limits<double>::quiet_NaN()), dsga::dvec4(3, 2, 1, 0));

	[[ maybe_unused ]] constexpr auto b1 = dsga::any(comp6);
	[[ maybe_unused ]] constexpr auto b2 = dsga::all(comp6);

	[[ maybe_unused ]] constexpr auto modify_test = []() noexcept
	{
		auto v = dsga::dvec4(1, 2, 3, 4); return v += 1;
	}();

	[[ maybe_unused ]] constexpr auto sq1 = dsga::sqrt(v2);
	[[ maybe_unused ]] constexpr auto sq2 = dsga::fast_inversesqrt(v2);
	[[ maybe_unused ]] constexpr auto sq3 = dsga::inversesqrt(v2);
	[[ maybe_unused ]] constexpr auto sq4 = dsga::equal(sq2, sq3);
	[[ maybe_unused ]] constexpr auto sq5 = dsga::within_distance(sq2, sq3, 1.e-16);

	[[ maybe_unused ]] constexpr auto c1 = dsga::clamp(2.0, 3.0, 1.0);
//	std::printf("clamp val = %g\n", c1);
	[[ maybe_unused ]] constexpr auto c2 = dsga::clamp(0.0, 3.0, 1.0);
//	std::printf("clamp val = %g\n", c2);
	[[ maybe_unused ]] constexpr auto c3 = dsga::clamp(std::numeric_limits<double>::quiet_NaN(), 3.0, 1.0);
//	std::printf("clamp val = %g\n", c3);
	[[ maybe_unused ]] constexpr auto c4 = dsga::clamp(0.0, std::numeric_limits<double>::quiet_NaN(), 1.0);
//	std::printf("clamp val = %g\n", c4);
	[[ maybe_unused ]] constexpr auto cc = dsga::clamp(2.0, 3.0, 1.0);
//	std::printf("clamp val = %g\n", cc);

	[[ maybe_unused ]] constexpr auto cc1 = dsga::clamp(0.0, 1.0, 3.0);
//	std::printf("clamp val = %g\n", cc1);
	[[ maybe_unused ]] constexpr auto cc2 = dsga::clamp(2.0, 1.0, 3.0);
//	std::printf("clamp val = %g\n", cc2);
	[[ maybe_unused ]] constexpr auto cc3 = dsga::clamp(4.0, 1.0, 3.0);
//	std::printf("clamp val = %g\n", cc3);

	[[ maybe_unused ]] constexpr auto min1 = dsga::min(0.0, std::numeric_limits<double>::quiet_NaN());
	[[ maybe_unused ]] constexpr auto min2 = dsga::min(std::numeric_limits<double>::quiet_NaN(), 0.0);
	[[ maybe_unused ]] constexpr auto max1 = dsga::max(0.0, std::numeric_limits<double>::quiet_NaN());
	[[ maybe_unused ]] constexpr auto max2 = dsga::max(std::numeric_limits<double>::quiet_NaN(), 0.0);

	[[ maybe_unused ]] constexpr auto smin1 = std::min(0.0, std::numeric_limits<double>::quiet_NaN());
	[[ maybe_unused ]] constexpr auto smin2 = std::min(std::numeric_limits<double>::quiet_NaN(), 0.0);
	[[ maybe_unused ]] constexpr auto smax1 = std::max(0.0, std::numeric_limits<double>::quiet_NaN());
	[[ maybe_unused ]] constexpr auto smax2 = std::max(std::numeric_limits<double>::quiet_NaN(), 0.0);

	[[ maybe_unused ]] auto ret = 20092 % 256;
	[[ maybe_unused ]] auto ret2 = 23739 % 256;
	[[ maybe_unused ]] auto ang = tau<double>;


	// vectors for making cross products
	auto cv1 = dsga::dvec3(4, 7, -2);
	auto cv2 = dsga::dvec3(6, -8, 5);

	// cross matrix representations of the vectors
	[[ maybe_unused ]] auto cm1 = dsga::cross_matrix(cv1);
	[[ maybe_unused ]] auto cm2 = dsga::cross_matrix(cv2);

	// 3 different ways to create a cross product
	[[ maybe_unused ]] auto plain_cross = dsga::cross(cv1, cv2);	// normal cross of vector1 and vector2
	[[ maybe_unused ]] auto cm1_cross = cv1 * cm2;					// vector1 * cross_matrix2
	[[ maybe_unused ]] auto cm2_cross = cm1 * cv2;					// cross_matrix1 * vector2

	// do we get the same results
	[[ maybe_unused ]] auto cm1_compare = dsga::all(dsga::equal(plain_cross, cm1_cross));
	[[ maybe_unused ]] auto cm2_compare = dsga::all(dsga::equal(plain_cross, cm2_cross));

	cv1 += dsga::dscal(100);
}

static constexpr double xmas_lights()
{
	constexpr double my_room_flat = 235;
	constexpr double their_room_flat = 273;
	constexpr double total_flat = my_room_flat + their_room_flat;
	constexpr double total_tax = 53.34;
	constexpr double my_tax = total_tax * (my_room_flat / total_flat);

	constexpr double my_final_cost = my_room_flat + my_tax;
	constexpr double their_final_cost = their_room_flat + (total_tax - my_tax);

	static_assert(my_final_cost + their_final_cost == total_flat + total_tax);
	static_assert(my_final_cost + their_final_cost == 561.34);

	return my_final_cost;
}

static void sugar_7862()
{
	// 48 - ccw arc
	constexpr auto s1 = dsga::dvec2(3.9189549, 0.37356279896);
	constexpr auto e1 = dsga::dvec2(3.37027294, 0.626136797647);
	[[ maybe_unused ]] constexpr auto c1 = dsga::dvec2(3.372232758, -0.0918605276);
	[[ maybe_unused ]] constexpr auto l1 = dsga::distance(s1, e1);	// 0.60402443497112135

	// 49 - line
	constexpr auto s2 = dsga::dvec2(3.370272893, 0.626173889);
	constexpr auto e2 = dsga::dvec2(3.3507548675, 0.6261206);
	[[ maybe_unused ]] constexpr auto l2 = dsga::distance(s2, e2);	// 0.019518098245888935

	[[ maybe_unused ]] constexpr auto e1s2 = dsga::distance(e1, s2);	// 3.7091382777825008e-05	-- delta less than SMALL

	// 50 - cw arc
	constexpr auto s3 = dsga::dvec2(3.3507548675, 0.6261206);
	constexpr auto e3 = dsga::dvec2(3.2649414836, 0.630617729);
	[[ maybe_unused ]] constexpr auto c3 = dsga::dvec2(3.348596193, 1.40591762571);
	[[ maybe_unused ]] constexpr auto l3 = dsga::distance(s3, e3);	// 0.085931141186483997

	[[ maybe_unused ]] constexpr auto e2s3 = dsga::distance(e2, s3);	// 0.0						-- same point

	// 51 - line
	constexpr auto s4 = dsga::dvec2(3.2649414836, 0.630617729);
	constexpr auto e4 = dsga::dvec2(3.193183313584, 0.638360421886);
	[[ maybe_unused ]] constexpr auto l4 = dsga::distance(s4, e4);	// 0.072174678781218457

	[[ maybe_unused ]] constexpr auto e3s4 = dsga::distance(e3, s4);	// 0.0						-- same point
}

void fizz_buzz()
{
	for (int i = 1; i <= 100; ++i)
	{
		std::string fb_string;
		if (i % 3 == 0)
			fb_string += "Fizz";
		if (i % 5 == 0)
			fb_string += "Buzz";
		if (fb_string.empty())
			fb_string = std::to_string(i);
		std::printf("%s\n", fb_string.c_str());
	}
}

void span_test()
{
	auto v1 = dsga::vec4(21, 32, 43, 54);
	const auto v2 = dsga::vec4(65, 76, 87, 98);

	auto sp1 = std::span(v1);
	auto sp2 = std::span(v2);

	for (auto v : sp1)
	{
		std::cout << v << '\n';
	}

	for (auto v : sp2)
	{
		std::cout << v << '\n';
	}

}

static void sandbox_function()
{
	lots_of_tests();

	more_tests();

	even_more_tests();

	sugar_7862();

	[[ maybe_unused ]] constexpr double venmo = xmas_lights();

//	fizz_buzz();

//	span_test();

	bench_cross();
//	bench_rsqrt();
//	bench_sqrt();

//	test_double_sqrt();
//	test_all_floats_sqrt();
//	test_all_floats_rsqrt();
//	test_fast_rsqrt();
//	test_double_rsqrt();
}

#if defined(__clang__) && (__clang_major__ < 13)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main([[ maybe_unused ]] int argc, [[ maybe_unused ]] char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	// watch window:
	// {,,ucrtbased.dll}_crtBreakAlloc
	// {,,ucrtbased.dll}__acrt_current_request_number
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// do STL binary to ASCII conversion
	if (argc == 3)
	{
		return stl_main(argc, argv);
	}

	print_dsga_version();

	sandbox_function();


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
