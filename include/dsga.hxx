//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// https://github.com/davidbrowne/dsga

// opening include guard
#if !defined(DSGA_DSGA_HXX)
#define DSGA_DSGA_HXX

#include <type_traits>				// requirements
#include <concepts>					// requirements
#include <array>					// underlying storage
#include <tuple>					// tuple interface for structured bindings, variadic constructors
#include <algorithm>				// min()
#include <numbers>					// pi_v<>, inv_pi_v<>
#include <limits>					// for cxcm
#include <cmath>					// for cxcm
#include <numeric>
#include <bit>						// bit_cast
#include <cassert>
#include <stdexcept>

//
// disable all asserts
//

#if defined(DISABLE_ASSERTS)

#define DSGA_DISABLE_ASSERTS
#define CXCM_DISABLE_ASSERTS

#endif


//
// dsga_constexpr_assert() derived from https://gist.github.com/oliora/928424f7675d58fadf49c70fdba70d2f
//

#if defined(DSGA_DISABLE_ASSERTS)

#define dsga_assertm(exp, msg) ((void)0)
#define dsga_constexpr_assert(cond, msg) ((void)0)

#else

#define dsga_assertm(exp, msg) assert(((void)msg, exp))

// this needs to be NOT constexpr, so attempted use of this function stops constexpr evaluation
template<class Assert>
inline void dsga_constexpr_assert_failed(Assert &&a) noexcept
{
	std::forward<Assert>(a)();
}

// When evaluated at compile time emits a compilation error if condition is not true.
// Invokes the standard assert at run time.
#define dsga_constexpr_assert(cond, msg) \
	((void)(!!(cond) ? 0 : (dsga_constexpr_assert_failed([](){ assert(((void)msg, !static_cast<bool>(#cond))); }), 0)))

#endif

//
// for cxcm nested namespace
//

#if defined(CXCM_DISABLE_ASSERTS)

#define cxcm_assertm(exp, msg) ((void)0)
#define cxcm_constexpr_assert(cond, msg) ((void)0)

#else

#define cxcm_assertm(exp, msg) assert(((void)msg, exp))

// this needs to be NOT constexpr, so attempted use of this function stops constexpr evaluation
template<class Assert>
inline void cxcm_constexpr_assert_failed(Assert &&a) noexcept
{
	std::forward<Assert>(a)();
}

// When evaluated at compile time emits a compilation error if condition is not true.
// Invokes the standard assert at run time.
#define cxcm_constexpr_assert(cond, msg) \
	((void)(!!(cond) ? 0 : (cxcm_constexpr_assert_failed([](){ assert(((void)msg, !static_cast<bool>(#cond))); }), 0)))

#endif

//
// Data Structures for Geometric Algebra (dsga)
//

// version info

constexpr inline int DSGA_MAJOR_VERSION = 2;
constexpr inline int DSGA_MINOR_VERSION = 0;
constexpr inline int DSGA_PATCH_VERSION = 0;

namespace dsga
{
	namespace cxcm
	{
		// copyright for cxcm - https://github.com/davidbrowne/cxcm

		//          Copyright David Browne 2020-2024.
		// Distributed under the Boost Software License, Version 1.0.
		//    (See accompanying file LICENSE_1_0.txt or copy at
		//          https://www.boost.org/LICENSE_1_0.txt)

		constexpr inline int CXCM_MAJOR_VERSION = 1;
		constexpr inline int CXCM_MINOR_VERSION = 1;
		constexpr inline int CXCM_PATCH_VERSION = 2;

		namespace dd_real
		{
			// https://www.davidhbailey.com/dhbsoftware/ - QD

			/*
				Modified BSD 3-Clause License

				This work was supported by the Director, Office of Science, Division
				of Mathematical, Information, and Computational Sciences of the
				U.S. Department of Energy under contract number DE-AC03-76SF00098.
 
				Copyright (c) 2000-2007

				1. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

					(1) Redistributions of source code must retain the copyright notice, this list of conditions and the following disclaimer.

					(2) Redistributions in binary form must reproduce the copyright notice, this list of conditions and the following disclaimer in the documentation
					    and/or other materials provided with the distribution.

					(3) Neither the name of the University of California, Lawrence Berkeley National Laboratory, U.S. Dept. of Energy nor the names of its contributors
					    may be used to endorse or promote products derived from this software without specific prior written permission.

				2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
				   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
				   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
				   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
				   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
				   OF THE POSSIBILITY OF SUCH DAMAGE.

				3. You are under no obligation whatsoever to provide any bug fixes, patches, or upgrades to the features, functionality or performance of the
				   source code ("Enhancements") to anyone; however, if you choose to make your Enhancements available either publicly, or directly to Lawrence
				   Berkeley National Laboratory, without imposing a separate written license agreement for such Enhancements, then you hereby grant the following
				   license: a non-exclusive, royalty-free perpetual license to install, use, modify, prepare derivative works, incorporate into other computer
				   software, distribute, and sublicense such enhancements or derivative works thereof, in binary and source code form.
			*/

			//
			// heavily modified dd_real type and support
			//

			// The following code computes s = fl(a+b) and error(a + b), assuming |a| >= |b|.
			constexpr double quick_two_sum(double a, double b, double &error) noexcept
			{
				double s = a + b;
				error = b - (s - a);
				return s;
			}

			// The following code computes s = fl(a+b) and error(a + b).
			constexpr double two_sum(double a, double b, double &error) noexcept
			{
				double s = a + b;
				double v = s - a;
				error = (a - (s - v)) + (b - v);
				return s;
			}

			// The following code splits a 53-bit IEEE double precision floating number a into a high word and a low word, each with 26
			// bits of significand, such that a is the sum of the high word with the low word. The high word will contain the first 26 bits,
			// while the low word will contain the lower 26 bits.
			constexpr void split(double a, double &high, double &low) noexcept
			{
				double temp = 134217729.0 * a;			// 134217729.0 = 2^27 + 1
				high = temp - (temp - a);
				low = a - high;
			}

			// The following code computes fl(a x b) and error(a x b).
			constexpr double two_prod(double a, double b, double &error) noexcept
			{
				double a_high, a_low, b_high, b_low;
				double p = a * b;
				split(a, a_high, a_low);
				split(b, b_high, b_low);
				error = ((a_high * b_high - p) + a_high * b_low + a_low * b_high) + a_low * b_low;
				return p;
			}

			// higher precision double-double
			struct dd_real
			{
				double x[2];

				constexpr dd_real() noexcept : x{}
				{
				}

				constexpr dd_real(double hi, double lo) noexcept : x{ hi, lo }
				{
				}

				explicit constexpr dd_real(double h) noexcept : x{ h, 0. }
				{
				}

				constexpr dd_real(const dd_real &) noexcept = default;
				constexpr dd_real(dd_real &&) noexcept = default;
				constexpr dd_real & operator =(const dd_real &) noexcept = default;
				constexpr dd_real & operator =(dd_real &&) noexcept = default;

				constexpr double operator [](unsigned int index) const noexcept
				{
					return x[index];
				}

				constexpr double & operator [](unsigned int index) noexcept
				{
					return x[index];
				}

				explicit constexpr operator double() const noexcept
				{
					return x[0];
				}

				explicit constexpr operator float() const noexcept
				{
					return static_cast<float>(x[0]);
				}

			};

			// double-double + double-double
			constexpr dd_real ieee_add(const dd_real &a, const dd_real &b) noexcept
			{
				// This one satisfies IEEE style error bound, due to K. Briggs and W. Kahan.
				double s1, s2, t1, t2;

				s1 = two_sum(a.x[0], b.x[0], s2);
				t1 = two_sum(a.x[1], b.x[1], t2);
				s2 += t1;
				s1 = quick_two_sum(s1, s2, s2);
				s2 += t2;
				s1 = quick_two_sum(s1, s2, s2);
				return dd_real(s1, s2);
			}

			// double-double + double
			constexpr dd_real ieee_add(const dd_real &a, double b) noexcept
			{
				// This one satisfies IEEE style error bound, due to K. Briggs and W. Kahan.
				double s1, s2;

				s1 = two_sum(a.x[0], b, s2);
				s1 = quick_two_sum(s1, s2 + a.x[1], s2);
				return dd_real(s1, s2);
			}

			// double-double - double-double
			constexpr dd_real ieee_subtract(const dd_real &a, const dd_real &b) noexcept
			{
				// This one satisfies IEEE style error bound, due to K. Briggs and W. Kahan.
				double s1, s2, t1, t2;

				s1 = two_sum(a.x[0], -b.x[0], s2);
				t1 = two_sum(a.x[1], -b.x[1], t2);
				s2 += t1;
				s1 = quick_two_sum(s1, s2, s2);
				s2 += t2;
				s1 = quick_two_sum(s1, s2, s2);
				return dd_real(s1, s2);
			}

			// double - double-double
			constexpr dd_real ieee_subtract(double a, const dd_real &b) noexcept
			{
				// This one satisfies IEEE style error bound, due to K. Briggs and W. Kahan.
				double s1, s2;

				s1 = two_sum(a, -b.x[0], s2);
				s1 = quick_two_sum(s1, s2 - b.x[1], s2);
				return dd_real(s1, s2);
			}

			// double-double + double-double
			constexpr dd_real operator +(const dd_real &a, const dd_real &b) noexcept
			{
				return ieee_add(a, b);
			}

			// double-double + double
			constexpr dd_real operator +(const dd_real &a, double b) noexcept
			{
				return ieee_add(a, b);
			}

			constexpr dd_real operator -(const dd_real &a, const dd_real &b) noexcept
			{
				return ieee_subtract(a, b);
			}

			constexpr dd_real operator -(double a, const dd_real &b) noexcept
			{
				return ieee_subtract(a, b);
			}

			constexpr dd_real &operator -=(dd_real &a, const dd_real &b) noexcept
			{
				a = (a - b);
				return a;
			}

			// double-double * double-double
			constexpr dd_real operator *(const dd_real &a, const dd_real &b) noexcept
			{
				double p1, p2;

				p1 = two_prod(a.x[0], b.x[0], p2);
				p2 += (a.x[0] * b.x[1] + a.x[1] * b.x[0]);
				p1 = quick_two_sum(p1, p2, p2);
				return dd_real(p1, p2);
			}

			// double-double * double
			constexpr dd_real operator *(const dd_real &a, double b) noexcept
			{
				double p1, p2;

				p1 = two_prod(a.x[0], b, p2);
				p1 = quick_two_sum(p1, p2 + (a.x[1] * b), p2);
				return dd_real(p1, p2);
			}

			// double * double-double
			constexpr dd_real operator *(double a, const dd_real &b) noexcept
			{
				return (b * a);
			}

			constexpr dd_real & operator *=(dd_real &a, const dd_real &b) noexcept
			{
				double p1, p2;
				p1 = two_prod(a.x[0], b.x[0], p2);
				p2 += (a.x[0] * b.x[1] + a.x[1] * b.x[0]);
				a.x[0] = quick_two_sum(p1, p2, a.x[1]);
				return a;
			}

			constexpr dd_real accurate_div(const dd_real &a, const dd_real &b) noexcept
			{
				double q1, q2, q3;

				q1 = a.x[0] / b.x[0];						// approximate quotient

				dd_real r = a - q1 * b;

				q2 = r.x[0] / b.x[0];
				r -= (q2 * b);

				q3 = r.x[0] / b.x[0];

				q1 = quick_two_sum(q1, q2, q2);

				double s1, s2;
				s1 = two_sum(q1, q3, s2);
				s1 = quick_two_sum(s1, s2 + q2, s2);

				return dd_real(s1, s2);
			}

			constexpr dd_real accurate_div(double a, const dd_real &b) noexcept
			{
				double q1, q2, q3;

				q1 = a / b.x[0];							// approximate quotient

				dd_real r = a - q1 * b;

				q2 = r.x[0] / b.x[0];
				r -= (q2 * b);

				q3 = r.x[0] / b.x[0];

				q1 = quick_two_sum(q1, q2, q2);

				double s1, s2;
				s1 = two_sum(q1, q3, s2);
				s1 = quick_two_sum(s1, s2 + q2, s2);

				return dd_real(s1, s2);
			}

			// double / double-double
			constexpr dd_real operator /(double a, const dd_real &b) noexcept
			{
				return accurate_div(a, b);
			}

			// double-double / double-double
			constexpr dd_real operator /(const dd_real &a, const dd_real &b) noexcept
			{
				return accurate_div(a, b);
			}

		} // namespace dd_real

		namespace limits
		{
			namespace detail
			{
				// long doubles vary between compilers and platforms. Windows MSVC and clang on Windows both use
				// the same representation as double. For gcc and linux, etc., it is often represented by an extended
				// precision data structure with 80 bits (64 bits of significand). sizeof(long double) on gcc on Windows
				// (at least MSYS2) is 16, implying it is 128 bits, but std::numeric_limits<long double> returns values
				// consistent with an 80 bit representation.
				constexpr long double get_largest_fractional_long_double() noexcept
				{
					if constexpr (std::numeric_limits<long double>::digits == 64)
					{
						// if digits is 64, then long double is using extended precision, and we can
						// just barely get away with casting to a long long to remove the fractional
						// part and keep the rest of the bits, without overflow.
						return 0x1.fffffffffffffffep+62L;
					}
					else
					{
						// assuming that long double does the same thing as double (which is true for
						// MSVC and clang on windows).
						return 0x1.fffffffffffffp+51L;
					}
				}
			}

			//
			// largest_fractional_value
			//

			// the largest floating point value that has a fractional representation

			template <std::floating_point T>
			constexpr inline T largest_fractional_value = T();

			template <>
			constexpr inline long double largest_fractional_value<long double> = detail::get_largest_fractional_long_double();

			template <>
			constexpr inline double largest_fractional_value<double> = 0x1.fffffffffffffp+51;

			template <>
			constexpr inline float largest_fractional_value<float> = 0x1.fffffep+22f;
		}

		//
		// floating-point negative zero support
		//

		template <std::floating_point T>
		constexpr bool is_negative_zero(T val) noexcept;

		template<>
		constexpr bool is_negative_zero<float>(float val) noexcept
		{
			return (0x80000000 == std::bit_cast<unsigned int>(val));
		}

		template<>
		constexpr bool is_negative_zero<double>(double val) noexcept
		{
			return (0x8000000000000000 == std::bit_cast<unsigned long long>(val));
		}

		template <std::floating_point T>
		constexpr inline T negative_zero = T(-0);

		template <>
		constexpr inline float negative_zero<float> = std::bit_cast<float>(0x80000000);

		template <>
		constexpr inline double negative_zero<double> = std::bit_cast<double>(0x8000000000000000);

		// don't worry about esoteric input.
		// much faster than strict or standard when non constant evaluated,
		// though standard library is a little better in debugger.
		namespace relaxed
		{

			//
			// abs(), fabs()
			//

			// absolute value

			template <std::floating_point T>
			constexpr T abs(T value) noexcept
			{
				return (value < T(0)) ? -value : value;
			}

			// undefined behavior if value is std::numeric_limits<T>::min()
			template <std::signed_integral T>
			constexpr T abs(T value) noexcept
			{
				return (value < T(0)) ? -value : value;
			}

			template <std::unsigned_integral T>
			constexpr T abs(T value) noexcept
			{
				return value;
			}

			template <std::floating_point T>
			constexpr T fabs(T value) noexcept
			{
				return abs(value);
			}

			template <std::integral T>
			constexpr double fabs(T value) noexcept
			{
				return abs(value);
			}

			//
			// trunc()
			//

			// this is the workhorse function for floor(), ceil(), and round().

			// rounds towards zero

			template <std::floating_point T>
			constexpr T trunc(T value) noexcept
			{
				return static_cast<T>(static_cast<long long>(value));
			}

			// float specialization
			template <>
			constexpr float trunc(float value) noexcept
			{
				return static_cast<float>(static_cast<int>(value));
			}

			//
			// floor()
			//

			// rounds towards negative infinity

			template <std::floating_point T>
			constexpr T floor(T value) noexcept
			{
				const T truncated_value = trunc(value);

				// truncation rounds to zero which is right direction for positive values,
				// but we need to go the other way for negative values.

				// negative non-integral value
				if (truncated_value > value)
					return (truncated_value - T(1.0f));

				// positive or integral value
				return truncated_value;
			}

			//
			// ceil()
			//

			// rounds towards positive infinity

			template <std::floating_point T>
			constexpr T ceil(T value) noexcept
			{
				const T truncated_value = trunc(value);

				// truncation rounds to zero which is right direction for negative values,
				// but we need to go the other way for positive values.

				// positive non-integral value
				if (truncated_value < value)
					return (truncated_value + T(1.0f));

				// negative or integral value
				return truncated_value;
			}

			//
			// round()
			//

			// rounds to nearest integral position, halfway cases away from zero

			template <std::floating_point T>
			constexpr T round(T value) noexcept
			{
				// zero could be handled either place, but here it is with the negative values.

				// positive value, taking care of halfway case.
				if (value > T(0))
					return trunc(value + T(0.5f));

				// negative or zero value, taking care of halfway case.
				return trunc(value - T(0.5f));
			}

			//
			// fract() - not in standard library
			//

			// the fractional part of a floating point number - always non-negative.

			template <std::floating_point T>
			constexpr T fract(T value) noexcept
			{
				return value - floor(value);
			}

			//
			// fmod()
			//

			// the floating point remainder of division

			template <std::floating_point T>
			constexpr T fmod(T x, T y) noexcept
			{
				return x - trunc(x / y) * y;
			}

			//
			// round_even() - not in standard library
			//

			// rounds to nearest integral position, halfway cases towards even

			template <std::floating_point T>
			constexpr T round_even(T value) noexcept
			{
				T trunc_value = trunc(value);
				bool is_even = (fmod(trunc_value, T(2)) == T(0));
				bool is_halfway = (fract(value) == T(0.5));

				// the special case
				if (is_halfway && is_even)
					return trunc_value;

				// zero could be handled either place, but here it is with the negative values.

				// positive value, taking care of halfway case.
				if (value > T(0))
					return trunc(value + T(0.5f));

				// negative or zero value, taking care of halfway case.
				return trunc(value - T(0.5f));
			}

			//
			// sqrt()
			//

			namespace detail
			{

				// "Improving the Accuracy of the Fast Inverse Square Root by Modifying Newton-Raphson Corrections" 2021
				// https://www.mdpi.com/1099-4300/23/1/86
				//
				// in comparison to inverse_sqrt(double), this method gives pretty good results:
				//    0 ulps: ~68.58%
				//    1 ulps: ~31.00%
				//    2 ulps:  ~0.42%
				//
				// depending on compiler/platform, this may not be faster than rsqrt()
				constexpr double fast_rsqrt(double x) noexcept
				{
					double halfx = 0.5 * x;
					long long i = std::bit_cast<long long>(x);
					i = 0x5FE6ED2102DCBFDA - (i >> 1);
					double y = std::bit_cast<double>(i);
					y *= 1.50087895511633457 - halfx * y * y;
					y *= 1.50000057967625766 - halfx * y * y;
					y *= 1.5000000000002520 - halfx * y * y;
					y *= 1.5000000000000000 - halfx * y * y;
					return y;
				}

				// float uses double internally, double uses dd_real internally
				template <std::floating_point T>
				constexpr T converging_sqrt(T arg) noexcept
				{
					const double boosted_arg = arg;
					double init_value = boosted_arg * fast_rsqrt(boosted_arg);

					if constexpr (std::is_same_v<T, double>)
					{
						// boosted_arg doesn't need to be a dd_real for [T = double]

						auto current_value = dd_real::dd_real(init_value);
						auto previous_value = dd_real::dd_real(0.0);

						while ((current_value[0] != previous_value[0]) && (current_value[0] * current_value[0] != boosted_arg))
						{
							previous_value = current_value;
							current_value = 0.5 * (current_value + (boosted_arg / current_value));
						}

						return static_cast<double>(current_value);
					}
					else if constexpr (std::is_same_v<T, float>)
					{
						double current_value = init_value;
						double previous_value = 0.0;

						while ((current_value != previous_value) && (current_value * current_value != boosted_arg))
						{
							previous_value = current_value;
							current_value = 0.5 * (current_value + (boosted_arg / current_value));
						}

						return static_cast<float>(current_value);
					}
				}

				// float uses double internally, double uses dd_real internally
				template <std::floating_point T>
				constexpr T inverse_sqrt(T arg) noexcept
				{
					// don't need this to be a dd_real
					const double boosted_arg = arg;

					if constexpr (std::is_same_v<T, double>)
					{
						// arg is already a double
						auto current_value = dd_real::dd_real(fast_rsqrt(arg));

						current_value *= (1.5 - ((0.5 * boosted_arg) * (current_value * current_value)));

						return static_cast<double>(current_value);
					}
					else if constexpr (std::is_same_v<T, float>)
					{
						double current_value = fast_rsqrt(boosted_arg);

						current_value *= (1.5 - (0.5 * boosted_arg * current_value * current_value));

						// do a couple more refinements for floating point (this needs testing to see if necessary)
						current_value *= (1.5 - (0.5 * boosted_arg * current_value * current_value));
						current_value *= (1.5 - (0.5 * boosted_arg * current_value * current_value));

						return static_cast<float>(current_value);
					}
				}

			}

			// constexpr square root, uses higher precision behind the scenes
			template <std::floating_point T>
			constexpr T sqrt(T value) noexcept
			{
				return detail::converging_sqrt(value);
			}

			// reciprocal of square root, uses higher precision behind the scenes
			template <std::floating_point T>
			constexpr T rsqrt(T value) noexcept
			{
				return detail::inverse_sqrt(value);
			}

			// fast reciprocal of square root
			template <std::floating_point T>
			constexpr T fast_rsqrt(T value) noexcept
			{
				return static_cast<T>(detail::fast_rsqrt(static_cast<double>(value)));
			}

		} // namespace relaxed

		//
		// isnan()
		//

		// make sure this isn't optimized away if used with fast-math

	#if defined(_MSC_VER) || defined(__clang__)
	#pragma float_control(precise, on, push)
	#endif

		template <std::floating_point T>
		#if defined(__GNUC__) && !defined(__clang__)
			__attribute__((optimize("-fno-fast-math")))
		#endif
		constexpr bool isnan(T value) noexcept
		{
			return (value != value);
		}

	#if defined(_MSC_VER) || defined(__clang__)
	#pragma float_control(pop)
	#endif

		//
		// isinf()
		//

		template <std::floating_point T>
		constexpr bool isinf(T value) noexcept
		{
			return (value == -std::numeric_limits<T>::infinity()) || (value == std::numeric_limits<T>::infinity());
		}

		//
		// fpclassify()
		//

		template <std::floating_point T>
		constexpr int fpclassify(T value) noexcept
		{
			if (isnan(value))
				return FP_NAN;
			else if (isinf(value))
				return FP_INFINITE;
			else if (value == 0)				// intentional use of the implicit cast of 0 to T.
				return FP_ZERO;
			else if (relaxed::abs(value) < std::numeric_limits<T>::min())
				return FP_SUBNORMAL;

			return FP_NORMAL;
		}

		//
		// isnormal()
		//

		template <std::floating_point T>
		constexpr bool isnormal(T value) noexcept
		{
			return (fpclassify(value) == FP_NORMAL);
		}

		//
		// isfinite()
		//

		template <std::floating_point T>
		constexpr bool isfinite(T value) noexcept
		{
			return !isnan(value) && !isinf(value);
		}

		//
		// signbit()
		//

		// +0 returns false and -0 returns true
		template <std::floating_point T>
		constexpr bool signbit(T value) noexcept
		{
			if constexpr (sizeof(T) == 4)
			{
				unsigned int bits = std::bit_cast<unsigned int>(value);
				return (bits & 0x80000000) != 0;
			}
			else if constexpr (sizeof(T) == 8)
			{
				unsigned long long bits = std::bit_cast<unsigned long long>(value);
				return (bits & 0x8000000000000000) != 0;
			}
		}

		//
		// copysign()
		//

		// +0 or -0 for sign is considered as *not* negative
		template <std::floating_point T>
		constexpr T copysign(T value, T sgn) noexcept
		{
			// +0 or -0 for sign is considered as *not* negative
			bool is_neg = signbit(sgn);

			if constexpr (sizeof(T) == 4)
			{
				unsigned int bits = std::bit_cast<unsigned int>(value);
				if (is_neg)
					bits |= 0x80000000;
				else
					bits &= 0x7FFFFFFF;

				return std::bit_cast<T>(bits);
			}
			else if constexpr (sizeof(T) == 8)
			{
				unsigned long long bits = std::bit_cast<unsigned long long>(value);
				if (is_neg)
					bits |= 0x8000000000000000;
				else
					bits &= 0x7FFFFFFFFFFFFFFF;

				return std::bit_cast<T>(bits);
			}
		}

		// try and match standard library requirements.
		// this namespace is pulled into parent namespace via inline.
		inline namespace strict
		{

			namespace detail
			{

				//
				// isnormal_or_subnormal()
				//

				// standard library screening requirement for these functions

				template <std::floating_point T>
				constexpr bool isnormal_or_subnormal(T value) noexcept
				{
					// intentional use of the implicit cast of 0 to T.
					return isfinite(value) && (value != 0);
				}

				//
				// fails_fractional_input_constraints()
				//

				// the fractional functions,e.g., trunc(), floor(), ceil(), round(), need the input to satisfy
				// certain constraints before it further processes the input. if this function returns true,
				// the constraints weren't met, and the fractional functions will do no further work and return
				// the value as is.

				template <std::floating_point T>
				constexpr bool fails_fractional_input_constraints(T value) noexcept
				{
					// if any of the following constraints are not met, return true:
					// no NaNs
					// no +/- infinity
					// no +/- 0
					// no value that can't even have a fractional part
					return !isnormal_or_subnormal(value) || (relaxed::abs(value) > limits::largest_fractional_value<T>);
				}

				//
				// constexpr_trunc()
				//

				// rounds towards zero

				template <std::floating_point T>
				constexpr T constexpr_trunc(T value) noexcept
				{
					// screen out unnecessary input
					if (fails_fractional_input_constraints(value))
						return value;

					return relaxed::trunc(value);
				}

				//
				// constexpr_floor()
				//

				// rounds towards negative infinity

				template <std::floating_point T>
				constexpr T constexpr_floor(T value) noexcept
				{
					// screen out unnecessary input
					if (fails_fractional_input_constraints(value))
						return value;

					return relaxed::floor(value);
				}

				//
				// constexpr_ceil()
				//

				// rounds towards positive infinity

				template <std::floating_point T>
				constexpr T constexpr_ceil(T value) noexcept
				{
					// screen out unnecessary input
					if (fails_fractional_input_constraints(value))
						return value;

					return relaxed::ceil(value);
				}

				//
				// constexpr_round()
				//

				// rounds to nearest integral position, halfway cases away from zero

				template <std::floating_point T>
				constexpr T constexpr_round(T value) noexcept
				{
					// screen out unnecessary input
					if (fails_fractional_input_constraints(value))
						return value;

					// halfway rounding can bump into max long long value for truncation
					// (for extended precision), so be more gentle at the end points.
					// this works because the largest_fractional_value remainder is T(0.5f).
					if (value == limits::largest_fractional_value<T>)
						return value + T(0.5f);
					else if (value == -limits::largest_fractional_value<T>)			// we technically don't have to do this for negative case (one more number in negative range)
						return value - T(0.5f);

					return relaxed::round(value);
				}

				//
				// constexpr_fract()
				//

				template <std::floating_point T>
				constexpr T constexpr_fract(T value) noexcept
				{
					// screen out unnecessary input
					if (fails_fractional_input_constraints(value))
						return value;

					return relaxed::fract(value);
				}

				//
				// constexpr_fmod()
				//

				template <std::floating_point T>
				constexpr T constexpr_fmod(T x, T y) noexcept
				{
					// screen out unnecessary input

					if (isnan(x) || isnan(y) || !isfinite(x))
						return std::numeric_limits<T>::quiet_NaN();

					if (isinf(y))
						return x;

					if (x == T(0) && y != T(0))
						return 0;

					if (y == 0)
						return std::numeric_limits<T>::quiet_NaN();

					return relaxed::fmod(x, y);
				}

				//
				// constexpr_round_even()
				//

				// rounds to nearest integral position, halfway cases away from zero

				template <std::floating_point T>
				constexpr T constexpr_round_even(T value) noexcept
				{
					// screen out unnecessary input
					if (fails_fractional_input_constraints(value))
						return value;

					// halfway rounding can bump into max long long value for truncation
					// (for extended precision), so be more gentle at the end points.
					// this works because the largest_fractional_value remainder is T(0.5f).
					if (value == limits::largest_fractional_value<T>)
						return value + T(0.5f);
					else if (value == -limits::largest_fractional_value<T>)			// we technically don't have to do this for negative case (one more number in negative range)
						return value - T(0.5f);

					return relaxed::round_even(value);
				}

				//
				// constexpr_sqrt()
				//

				template <std::floating_point T>
				constexpr T constexpr_sqrt(T value) noexcept
				{
					// screen out unnecessary input

					if (isnan(value))
					{
						if constexpr (sizeof(T) == 4)
						{
							unsigned int bits = std::bit_cast<unsigned int>(value);

							// set the is_quiet bit
							bits |= 0x00400000;

							return std::bit_cast<T>(bits);
						}
						else if constexpr (sizeof(T) == 8)
						{
							unsigned long long bits = std::bit_cast<unsigned long long>(value);

							// set the is_quiet bit
							bits |= 0x0008000000000000;

							return std::bit_cast<T>(bits);
						}
					}
					else if (value == std::numeric_limits<T>::infinity())
					{
						return value;
					}
					else if (value == -std::numeric_limits<T>::infinity())
					{
						return -std::numeric_limits<T>::quiet_NaN();
					}
					else if (value == T(0))
					{
						return value;
					}
					else if (value < T(0))
					{
						return -std::numeric_limits<T>::quiet_NaN();
					}

					return relaxed::sqrt(value);
				}

				//
				// constexpr_inverse_sqrt()
				//

				template <std::floating_point T>
				constexpr T constexpr_rsqrt(T value) noexcept
				{
					// screen out unnecessary input

					if (isnan(value))
					{
						if constexpr (sizeof(T) == 4)
						{
							unsigned int bits = std::bit_cast<unsigned int>(value);

							// set the is_quiet bit
							bits |= 0x00400000;

							return std::bit_cast<T>(bits);
						}
						else if constexpr (sizeof(T) == 8)
						{
							unsigned long long bits = std::bit_cast<unsigned long long>(value);

							// set the is_quiet bit
							bits |= 0x0008000000000000;

							return std::bit_cast<T>(bits);
						}
					}
					else if (value == std::numeric_limits<T>::infinity())
					{
						return T(0);
					}
					else if (value == -std::numeric_limits<T>::infinity())
					{
						return -std::numeric_limits<T>::quiet_NaN();
					}
					else if (value == T(0))
					{
						return std::numeric_limits<T>::infinity();
					}
					else if (value < T(0))
					{
						return -std::numeric_limits<T>::quiet_NaN();
					}

					return relaxed::rsqrt(value);
				}

				template <std::floating_point T>
				constexpr T constexpr_fast_rsqrt(T value) noexcept
				{
					// screen out unnecessary input

					if (isnan(value))
					{
						if constexpr (sizeof(T) == 4)
						{
							unsigned int bits = std::bit_cast<unsigned int>(value);

							// set the is_quiet bit
							bits |= 0x00400000;

							return std::bit_cast<T>(bits);
						}
						else if constexpr (sizeof(T) == 8)
						{
							unsigned long long bits = std::bit_cast<unsigned long long>(value);

							// set the is_quiet bit
							bits |= 0x0008000000000000;

							return std::bit_cast<T>(bits);
						}
					}
					else if (value == std::numeric_limits<T>::infinity())
					{
						return T(0);
					}
					else if (value == -std::numeric_limits<T>::infinity())
					{
						return -std::numeric_limits<T>::quiet_NaN();
					}
					else if (value == T(0))
					{
						return std::numeric_limits<T>::infinity();
					}
					else if (value < T(0))
					{
						return -std::numeric_limits<T>::quiet_NaN();
					}

					return relaxed::fast_rsqrt(value);
				}

			} // namespace detail

			//
			// abs(), fabs()
			//


			// absolute value

			template <std::floating_point T>
			constexpr T abs(T value) noexcept
			{
				if (!detail::isnormal_or_subnormal(value))
					return value;

				return relaxed::abs(value);
			}

			// don't know what to do if someone tries to negate the most negative number.
			// standard says behavior is undefined if you can't represent the result by return type.
			template <std::integral T>
			constexpr T abs(T value) noexcept
			{
				cxcm_constexpr_assert(value != std::numeric_limits<T>::min(), "undefined behavior in abs()");

				return relaxed::abs(value);
			}

			template <std::floating_point T>
			constexpr T fabs(T value) noexcept
			{
				if (!detail::isnormal_or_subnormal(value))
					return value;

				return relaxed::fabs(value);
			}

			template <std::integral T>
			constexpr double fabs(T value) noexcept
			{
				cxcm_constexpr_assert(value != std::numeric_limits<T>::min(), "undefined behavior in fabs()");

				return relaxed::fabs(value);
			}

			//
			// trunc()
			//

			// rounds towards zero

			template <std::floating_point T>
			constexpr T trunc(T value) noexcept
			{
				if (std::is_constant_evaluated())
				{
					return detail::constexpr_trunc(value);
				}
				else
				{
					return std::trunc(value);
				}
			}

			template <std::integral T>
			constexpr double trunc(T value) noexcept
			{
				return value;
			}

			//
			// floor()
			//

			// rounds towards negative infinity

			template <std::floating_point T>
			constexpr T floor(T value) noexcept
			{
				if (std::is_constant_evaluated())
				{
					return detail::constexpr_floor(value);
				}
				else
				{
					return std::floor(value);
				}
			}

			template <std::integral T>
			constexpr double floor(T value) noexcept
			{
				return value;
			}

			//
			// ceil()
			//

			// rounds towards positive infinity

			template <std::floating_point T>
			constexpr T ceil(T value) noexcept
			{
				if (std::is_constant_evaluated())
				{
					return detail::constexpr_ceil(value);
				}
				else
				{
					return std::ceil(value);
				}
			}

			template <std::integral T>
			constexpr double ceil(T value) noexcept
			{
				return value;
			}

			//
			// round()
			//

			// rounds to nearest integral position, halfway cases away from zero

			template <std::floating_point T>
			constexpr T round(T value) noexcept
			{
				if (std::is_constant_evaluated())
				{
					return detail::constexpr_round(value);
				}
				else
				{
					return std::round(value);
				}
			}

			template <std::integral T>
			constexpr double round(T value) noexcept
			{
				return value;
			}

			//
			// fract()
			//

			// there is no standard c++ version of this, so always call constexpr version

			// the fractional part of a floating point number - always non-negative.

			template <std::floating_point T>
			constexpr T fract(T value) noexcept
			{
				return detail::constexpr_fract(value);
			}

			template <std::integral T>
			constexpr double fract(T /* value */) noexcept
			{
				return 0.0;
			}

			//
			// fmod()
			//

			// the floating point remainder of division

			template <std::floating_point T>
			constexpr T fmod(T x, T y) noexcept
			{
				if (std::is_constant_evaluated())
				{
					return detail::constexpr_fmod(x, y);
				}
				else
				{
					return std::fmod(x, y);
				}
			}

			//
			// round_even()
			//

			// there is no standard c++ version of this, so always call constexpr version

			// rounds to nearest integral position, halfway cases towards even

			template <std::floating_point T>
			constexpr T round_even(T value) noexcept
			{
				return detail::constexpr_round_even(value);
			}

			template <std::integral T>
			constexpr double round_even(T value) noexcept
			{
				return value;
			}

			//
			// sqrt()
			//

			template <std::floating_point T>
			constexpr T sqrt(T value) noexcept
			{
				if (std::is_constant_evaluated())
				{
					return detail::constexpr_sqrt(value);
				}
				else
				{
					return std::sqrt(value);
				}
			}

			//
			// rsqrt() - inverse square root
			//

			// there is no standard c++ version of this, so always call constexpr version

			template <std::floating_point T>
			constexpr T rsqrt(T value) noexcept
			{
					return detail::constexpr_rsqrt(value);
			}

			//
			// fast_rsqrt() - fast good approximation to inverse square root
			//

			// there is no standard c++ version of this, so always call constexpr version

			template <std::floating_point T>
			constexpr T fast_rsqrt(T value) noexcept
			{
				return detail::constexpr_fast_rsqrt(value);
			}

		} // namespace strict

	} // namespace cxcm

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//
	// fundamental type concepts
	//

	// plain undecorated boolean type
	template <typename T>
	concept bool_scalar = std::same_as<bool, T>;

	// plain undecorated signed types
	template <typename T>
	concept signed_scalar = (std::same_as<int, T> || std::same_as<long long, T>);

	// plain undecorated unsigned types
	template <typename T>
	concept unsigned_scalar = (std::same_as<unsigned int, T> || std::same_as<unsigned long long, T> || std::same_as<std::size_t, T>);

	// plain undecorated integral types
	template <typename T>
	concept numeric_integral_scalar = (signed_scalar<T> || unsigned_scalar<T>);

	// plain undecorated floating point types
	template <typename T>
	concept floating_point_scalar = (std::same_as<float, T> || std::same_as<double, T>);

	// plain undecorated integral and floating point types
	template <typename T>
	concept non_bool_scalar = (numeric_integral_scalar<T> || floating_point_scalar<T>);

	// plain undecorated arithmetic types
	template <typename T>
	concept dimensional_scalar = (non_bool_scalar<T> || bool_scalar<T>);

	// want the size to be between 1 and 4, inclusive
	template <std::size_t Size>
	concept dimensional_size = ((Size >= 1) && (Size <= 4));

	// dimensional storage needs the arithmetic type and size restrictions
	template <typename T, std::size_t Size>
	concept dimensional_storage = dimensional_scalar<T> && dimensional_size<Size>;

	// we want dimensional_storage_t to have length from 1 to 4 (1 gives just a sneaky kind of T that can swizzle),
	// and the storage has to have room for all the data. We also need dimensional_storage_t to support operator[] to access
	// the data. It needs to also support iterators so we can use it in ranged-for loops, algorithms, etc.

	// the underlying storage for vector and indexed_vector types. originally this was to be a template parameter and
	// fairly generic, but that is a detail that can happen in a future version of this library. it makes things
	// much simpler to not have to pass this stuff around in template parameters.

	// as alluded to above, dimensional_storage_t has two roles: 1) the storage in a vector for each dimension of
	// the vector, and 2) the backing storage used by a swizzle of a vector (storage is in that vector), that
	// is used to index into as required by the swizzle.

	// this implementation uses std::array as the backing storage type. It satisfies the requirements described above.

	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
	using dimensional_storage_t = std::array<T, Size>;

	// https://stackoverflow.com/questions/63326542/checking-for-constexpr-in-a-concept
	// test whether default-constructable callable C's operator() can be called in a constexpr context
	template <typename C, auto val = std::bool_constant<(C{}(), true)>{}>
	consteval auto is_constexpr(C) noexcept { return val(); }

	namespace detail
	{
		// the concepts and requirements will help indexed_vector determine if it can be assigned to, like an lvalue reference,
		// i.e., if all indexes are unique then it can be used as an lvalue reference, i.e., is writable to.

		// see if all the std::size_t index values are unique
		template <std::size_t First, std::size_t ...Rest>
		consteval bool unique_indexes(std::index_sequence<First, Rest...>) noexcept
		{
			if constexpr (sizeof...(Rest) == 0)
				return true;
			else
				return ((First != Rest) && ...) && unique_indexes(std::index_sequence<Rest...>{});
		}

		// is Count the same as the number of Is... and is Count an appropriate size for a vector
		template <std::size_t Count, std::size_t ...Is>
		consteval bool valid_index_count() noexcept
		{
			return (sizeof...(Is) == Count) && dimensional_size<Count>;
		}

		// are the values of Is... in the range 0 <= Is... < Size
		// not checking here about Size and number of Is
		template <std::size_t Size, std::size_t ...Is>
		consteval bool valid_range_indexes() noexcept
		{
			return ((Is < Size) && ...);
		}

		// are types the same size
		template <typename T, typename U>
		concept same_sizeof = (sizeof(T) == sizeof(U));

		//
		// https://stackoverflow.com/questions/40617854/implement-c-template-for-generating-an-index-sequence-with-a-given-range
		//

		// return std::index_sequence with positive offset
		template<std::size_t N, std::size_t... Seq>
		constexpr std::index_sequence<N + Seq ...> add(std::index_sequence<Seq...>) noexcept { return {}; }

		// return std::index_sequence with negative offset
		template<std::size_t N, std::size_t... Seq>
		constexpr std::index_sequence<N - Seq ...> subtract(std::index_sequence<Seq...>) noexcept { return {}; }

		// return std::index_sequence -> [Start, End)
		template<std::size_t Start, std::size_t End>
		constexpr auto index_range() noexcept
		{
			if constexpr (Start <= End)
			{
				return add<Start>(std::make_index_sequence<End - Start>{});
			}
			else
			{
				return subtract<Start>(std::make_index_sequence<Start - End>{});
			}
		}

		// return std::index_sequence -> [Start, End]
		template<std::size_t Start, std::size_t End>
		constexpr auto closed_index_range() noexcept
		{
			if constexpr (Start <= End)
			{
				return add<Start>(std::make_index_sequence<End - Start + 1>{});
			}
			else
			{
				return subtract<Start>(std::make_index_sequence<Start - End + 1>{});
			}
		}

		// is T an array-like thing (indexable) that has values that can be used in a std::index_sequence
		template <typename T>
		concept sequence_indexable = requires (T t, std::size_t i)
		{
			{ t[i] } -> std::convertible_to<std::size_t>;
			requires is_constexpr([] { [[ maybe_unused ]] auto val = T{}[0]; });

			{ t.size() } -> std::convertible_to<std::size_t>;
			requires is_constexpr([] { [[ maybe_unused ]] auto val = T{}.size(); });
		};

		// return std::index_sequence from constexpr std::array<std::size_t, N> elements
		template <sequence_indexable auto vals, std::size_t ...Is>
		requires ((vals[Is] >= 0) && ...)
		constexpr std::index_sequence<vals[Is]...> indexable_to_sequence(std::index_sequence<Is...>) noexcept { return {}; }
	}

	// do the argument indexes and count/size make for valid indirect indexing
	template <std::size_t Size, std::size_t Count, std::size_t ...Is>
	concept indexable = detail::valid_index_count<Count, Is...>() && detail::valid_range_indexes<Size, Is...>();

	// writable_swizzle can determine whether a particular indexed_vector can be used as an lvalue reference
	template <std::size_t Size, std::size_t Count, std::size_t ...Is>
	requires indexable<Size, Count, Is...>
	constexpr inline bool writable_swizzle = detail::unique_indexes(std::index_sequence<Is...>{});

	// half-open/half-closed interval in a std::index_sequence -> [Start, End)
	template<std::size_t Start, std::size_t End>
	using make_index_range = decltype(detail::index_range<Start, End>());

	// closed interval in a std::index_sequence -> [Start, End]
	template<std::size_t Start, std::size_t End>
	using make_closed_index_range = decltype(detail::closed_index_range<Start, End>());

	// constexpr std::array<std::size_t, N> elements in a std::index_sequence
	template <detail::sequence_indexable auto vals>
	using make_array_sequence = decltype(detail::indexable_to_sequence<vals>(std::make_index_sequence<vals.size()>{}));

	// build an array from the indexes of an index_sequence
	template <std::size_t... Is>
	constexpr std::array<std::size_t, sizeof...(Is)> make_sequence_array(std::index_sequence<Is...>) noexcept { return { Is... }; }

	// convert a std::index_sequence<Is...> to a std:index_sequence with the Is... in reverse order from input
	template<std::size_t ...Is>
	constexpr auto make_reverse_sequence(std::index_sequence<Is...> seq) noexcept
	{
		if constexpr (sizeof...(Is) > 1)
		{
			constexpr auto vals = make_sequence_array(seq);

			return [&]<std::size_t ...Js>(std::index_sequence<Js...>) noexcept
			{
				return std::index_sequence<vals[vals.size() - 1 - Js]...>{};
			}(std::make_index_sequence<vals.size()>{});
		}
		else
		{
			return std::index_sequence<Is...>{};
		}
	}

	// is the second type also the common type of the two types
	template <typename T, typename U>
	concept promotes_to =
	requires
	{
		typename std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
		requires std::same_as<std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>, std::remove_cvref_t<U>>;
	};

	// determine whether implicit conversions are allowed for non-boolean arithmetic purposes
	template <typename T, typename U>
	concept implicitly_convertible_to = non_bool_scalar<T> && non_bool_scalar<U> && promotes_to<T, U>;

	// there is no use for this enum, it is meant as FEO (For Exposition Only). we will separate domains by the names of the swizzle union
	// members we create, as opposed to using this enum class as a template parameter. we only intend to implement the xyzw swizzle accessors.
	// if we intend to implement the other swizzle mask sets, then values of this enum class would come in handy as a template parameter, as
	// the spec states that it isn't allowed to mix and match accessors from different mask sets.
	enum class swizzle_mask_sets
	{
		xyzw,						// spatial points and vectors
		rgba,						// colors
		stpq						// texture coordinates
	};

	//
	// common initial sequence wrapper with basic storage access -- forwards function calls to wrapped storage.
	// this struct is an aggregate
	//

	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
	struct storage_wrapper
	{
		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical storage - logical contiguous order is same as physical contiguous order.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		// underlying storage
		dimensional_storage_t<T, Size> store;

		// using directives related to storage
		using value_type = dimensional_storage_t<T, Size>::value_type;
		using iterator = dimensional_storage_t<T, Size>::iterator;
		using const_iterator = dimensional_storage_t<T, Size>::const_iterator;
		using reverse_iterator = dimensional_storage_t<T, Size>::reverse_iterator;
		using const_reverse_iterator = dimensional_storage_t<T, Size>::const_reverse_iterator;

		[[nodiscard]] constexpr int length() const noexcept					{ return Count; }
		static constexpr std::integral_constant<std::size_t, Count> size =	{};

		// logical and physically contiguous access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable
		{
			dsga_constexpr_assert(index >= 0 && static_cast<std::size_t>(index) < Count, "index out of bounds");
			return store[static_cast<std::size_t>(index)];
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept
		{
			dsga_constexpr_assert(index >= 0 && static_cast<std::size_t>(index) < Count, "index out of bounds");
			return store[static_cast<std::size_t>(index)];
		}

		// in general, data() should be used with sequence() as the "logically contiguous" offsets
		[[nodiscard]] constexpr T * data() noexcept requires Writable		{ return store.data(); }
		[[nodiscard]] constexpr const T * data() const noexcept				{ return store.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept				{ return sequence_pack{}; }

		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void set(Args ...args) noexcept
		{
			[&]<std::size_t ...Js, typename ...As>(std::index_sequence<Js ...>, As ...same_args) noexcept
			{
				((store[Js] = static_cast<T>(same_args)),...);
			}(std::make_index_sequence<Count>{}, args...);
		}

		constexpr void swap(storage_wrapper &sw) noexcept requires Writable	{ store.swap(sw.store); 	}

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable		{ return store.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept					{ return store.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept				{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable		{ return store.end(); }
		[[nodiscard]] constexpr auto end() const noexcept					{ return store.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable	{ return store.rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept				{ return store.crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable		{ return store.rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept					{ return store.crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept					{ return rend(); }
	};

	template <dimensional_scalar T, std::size_t Size>
	constexpr void swap(storage_wrapper<T, Size> &lhs, storage_wrapper<T, Size> &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	template <dimensional_scalar T1, std::size_t C, dimensional_scalar T2>
	requires implicitly_convertible_to<T2, T1>
	constexpr bool operator ==(const storage_wrapper<T1, C> &first,
							   const storage_wrapper<T2, C> &second) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return ((!std::isunordered(first[Is], second[Is]) && (first[Is] == static_cast<T1>(second[Is]))) && ...);
		}(std::make_index_sequence<C>{});
	}

	//
	// CTAD deduction guide
	//

	template <dimensional_scalar T, dimensional_scalar ...U>
	storage_wrapper(T, U...) -> storage_wrapper<T, 1 + sizeof...(U)>;

	// basic_vector will act as the primary vector class in this library.
	//
	// T is the type of the elements stored in the vector/storage
	// Size is number of elements referencable in vector/storage

	// the foundational vector type for dsga
	// 
	// template parameters:
	//
	//		T - the scalar type stored
	//		Size - the number of actual elements in storage
	//
	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
	struct basic_vector;

	//
	// This is a CRTP base struct for the vector structs, primarily for data access.
	// It will help with arithmetic operators, compound assignment operators, and functions.
	//
	// template parameters:
	//
	//		Writable - bool value about whether struct can be modified (e.g., "foo.set(3, 4, 5);", "foo[0] = 3;")
	//		T - the scalar type stored
	//		Count - the number of indexes available to access ScalarType data
	//		Derived - the CRTP struct/class that is derived from this struct
	//
	// It provides:
	//
	//		set() - relies on set() in Derived - access in logical order
	//		operator[] - relies on operator[] in Derived - access in logical order
	//		size() - relies on Count template parameter
	//		length() - relies on Count template parameter
	//		data() - relies on data() in Derived - access in physical order
	//		sequence() - relies on sequence() in Derived - the physical order to logical order mapping
	//		as_derived() - relies on Derived template parameter - useful for returning references to Derived when you just have a vector_base
	//

	template <bool Writable, dimensional_scalar T, std::size_t Count, typename Derived>
	requires dimensional_storage<T, Count>
	struct vector_base
	{
		// CRTP access to Derived class
		[[nodiscard]] constexpr Derived &as_derived() noexcept requires Writable	{ return static_cast<Derived &>(*this); }
		[[nodiscard]] constexpr const Derived &as_derived() const noexcept			{ return static_cast<const Derived &>(*this); }

		// logically contiguous write access to all data that allows for self-assignment that works properly
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void set(Args ...args) noexcept									{ this->as_derived().set(args...); }

		// logically contiguous access to piecewise data as index goes from 0 to (Count - 1)
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable	{ return this->as_derived()[index]; }

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept			{ return this->as_derived()[index]; }

		// physically contiguous access via pointer.
		// DON"T ASSSUME data() contiguous order is same as operator[] "logically contiguous" order
		// data() should be used with sequence() as the "logically contiguous" offsets
		[[nodiscard]] constexpr T * data() noexcept requires Writable				{ return this->as_derived().data(); }
		[[nodiscard]] constexpr const T * data() const noexcept						{ return this->as_derived().data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous.
		// this is only really helpful if you use data() in your API, because operator [] already adjusts for sequencing.
		[[nodiscard]] static constexpr auto sequence() noexcept						{ return Derived::sequence(); }

		// number of accessible T elements - required by spec
		[[nodiscard]] constexpr int length() const noexcept							{ return Count; }

		// not required by spec, but more c++ container-like
		static constexpr std::integral_constant<std::size_t, Count> size =			{};

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable				{ return this->as_derived().begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept							{ return this->as_derived().cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept						{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable				{ return this->as_derived().end(); }
		[[nodiscard]] constexpr auto end() const noexcept							{ return this->as_derived().cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept							{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable			{ return this->as_derived().rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept						{ return this->as_derived().crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept						{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable				{ return this->as_derived().rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept							{ return this->as_derived().crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept							{ return rend(); }

		//
		// functions similar to std::valarray
		//

		// UnOp is a lambda or callable that takes either a "T" or a "const T &", and it returns a T.
		//
		// Every initializer clause is sequenced before any initializer clause that follows it in the braced-init-list (i.e.,
		// left-to-right). This is in contrast with the arguments of a function call expression, which are indeterminately
		// sequenced (since C++17), e.g., MSVC and gcc appear to sequence function call arguments right-to-left, while clang
		// appears to sequence function call arguments left-to-right.
		template <typename UnOp>
		requires (std::same_as<T, std::invoke_result_t<UnOp, T>> || std::same_as<T, std::invoke_result_t<UnOp, const T &>>)
		[[nodiscard]] constexpr basic_vector<T, Count> apply(UnOp op) const noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return basic_vector<T, Count>{ op((*this)[Is])... };		// braced init list is evaluated in element order
			}(std::make_index_sequence<Count>{});
		}

		// positive count is left shift, negative count is right shift
		[[nodiscard]] constexpr basic_vector<T, Count> shift(int by) const noexcept
		{
			constexpr auto max_val = static_cast<int>(Count);
			auto copy = basic_vector<T, Count>(*this);
			if (by > 0)
			{
				int count = by > max_val ? max_val : by;
				auto shifted_end = std::shift_left(copy.begin(), copy.end(), count);
				std::ranges::fill_n(shifted_end, count, T(0));
			}
			else if (by < 0)
			{
				int count = -by > max_val ? max_val : -by;
				std::shift_right(copy.begin(), copy.end(), count);
				std::ranges::fill_n(copy.begin(), count, T(0));
			}

			return copy;
		}

		// positive count is left shift, negative count is right shift
		[[nodiscard]] constexpr basic_vector<T, Count> cshift(int by) const noexcept
		{
			constexpr auto max_val = static_cast<int>(Count);
			by %= max_val;

			basic_vector<T, Count> dest{};
			auto pivot = this->begin();
			if (by > 0)
			{
				pivot += by;
			}
			else if (by < 0)
			{
				pivot += (max_val + by);
			}
			std::ranges::rotate_copy(*this, pivot, dest.begin());

			return dest;
		}

		// min value in vector
		[[nodiscard]] constexpr T min() const noexcept requires non_bool_scalar<T>
		{
			T smallest = (*this)[0];
			for (std::size_t i = 1; i < Count; ++i)
			{
				if ((*this)[i] < smallest)
					smallest = (*this)[i];
			}

			return smallest;
		}

		// max value in vector
		[[nodiscard]] constexpr T max() const noexcept requires non_bool_scalar<T>
		{
			T largest = (*this)[0];
			for (std::size_t i = 1; i < Count; ++i)
			{
				if ((*this)[i] > largest)
					largest = (*this)[i];
			}

			return largest;
		}

		// sum of values in vector
		[[nodiscard]] constexpr T sum() const noexcept requires non_bool_scalar<T>
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return ((*this)[Is] + ...);
			}(std::make_index_sequence<Count>{});
		}
	};

	// indexed_vector will act as a swizzle of a basic_vector. basic_vector relies on the anonymous union of indexed_vector data members.
	//
	// T is the type of the elements stored in the underlying storage
	// Size relates to the number of elements in the underlying storage, which informs the values the Is can hold
	// Count is the number of elements accessible in swizzle -- often works alongside with basic_vector's Size
	// Is... are the number of swizzlable values available -- there are Count of them, and their values are in the range:  0 <= Indexes < Size

	// we want indexed_vector (vector swizzles) to have length from 1 to 4 (1 is just a sneaky type of T swizzle) in order
	// to work with the basic_vector which also has these lengths. The number of indexes is the same as the Count, between 1 and 4.
	// The indexes are valid for indexing into the values in the storage which is Size big.

	// the type of a basic_vector swizzle
	// 
	// template parameters:
	//
	//		T - the scalar type stored
	//		Size - the number of actual elements in storage
	//		Count - the number of indexes available to access ScalarType data
	//		Is - an ordered variable set of indexes into the storage -- there will be Count of them
	//
	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct indexed_vector;

	//
	// random-access iterators for indexed_vector so it can participate in range-for loop amongst other things.
	// make sure that it doesn't out-live it's indexed_vector or there will be
	// a dangling pointer.
	//

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable<Size, Count, Is...>
	struct indexed_vector_const_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = const T *;
		using reference = const T &;

		// range of valid values for mapper_index
		constexpr static int begin_index = 0;
		constexpr static int end_index = Count;

		// the data
		const indexed_vector<T, Size, Count, Is ...> *mapper_ptr;
		int mapper_index;

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr indexed_vector_const_iterator(const indexed_vector<T, Size, Count, Is ...> &mapper, int index) noexcept
			: mapper_ptr(std::addressof(mapper)), mapper_index(index)
		{
			dsga_constexpr_assert((mapper_index >= begin_index) && (mapper_index <= end_index), "index not in range");
		}

		constexpr indexed_vector_const_iterator() noexcept = default;
		constexpr indexed_vector_const_iterator(const indexed_vector_const_iterator &) noexcept = default;
		constexpr indexed_vector_const_iterator(indexed_vector_const_iterator &&) noexcept = default;
		constexpr indexed_vector_const_iterator &operator =(const indexed_vector_const_iterator &) & noexcept = default;
		constexpr indexed_vector_const_iterator &operator =(indexed_vector_const_iterator &&) & noexcept = default;
		constexpr ~indexed_vector_const_iterator() = default;

		[[nodiscard]] constexpr reference operator *() const noexcept
		{
			dsga_constexpr_assert(nullptr != mapper_ptr, "can't deref nullptr");
			dsga_constexpr_assert((mapper_index >= begin_index) && (mapper_index < end_index), "index not in range");

			return (*mapper_ptr)[mapper_index];
		}

		[[nodiscard]] constexpr pointer operator ->() const noexcept
		{
			dsga_constexpr_assert(nullptr != mapper_ptr, "can't deref nullptr");
			dsga_constexpr_assert((mapper_index >= begin_index) && (mapper_index < end_index), "index not in range");

			return std::addressof((*mapper_ptr)[mapper_index]);
		}

		constexpr indexed_vector_const_iterator &operator ++() noexcept
		{
			dsga_constexpr_assert(mapper_index < end_index, "don't increment past end_index");

			++mapper_index;
			return *this;
		}

		constexpr indexed_vector_const_iterator operator ++(int) noexcept
		{
			dsga_constexpr_assert(mapper_index < end_index, "don't increment past end_index");

			indexed_vector_const_iterator temp = *this;
			++mapper_index;
			return temp;
		}

		constexpr indexed_vector_const_iterator &operator --() noexcept
		{
			dsga_constexpr_assert(mapper_index > begin_index, "don't decrement past begin_index");

			--mapper_index;
			return *this;
		}

		constexpr indexed_vector_const_iterator operator --(int) noexcept
		{
			dsga_constexpr_assert(mapper_index > begin_index, "don't decrement past begin_index");

			indexed_vector_const_iterator temp = *this;
			--mapper_index;
			return temp;
		}

		constexpr indexed_vector_const_iterator &operator +=(const int offset) noexcept
		{
			dsga_constexpr_assert(((mapper_index + offset) >= begin_index) && ((mapper_index + offset) < end_index), "offset not in range");

			mapper_index += offset;
			return *this;
		}

		constexpr indexed_vector_const_iterator &operator -=(const int offset) noexcept
		{
			dsga_constexpr_assert(((mapper_index - offset) >= begin_index) && ((mapper_index - offset) < end_index), "offset not in range");

			mapper_index -= offset;
			return *this;
		}

		[[nodiscard]] constexpr int operator -(const indexed_vector_const_iterator &iter) const noexcept
		{
			dsga_constexpr_assert(mapper_ptr == iter.mapper_ptr, "different indexed_vector source");

			return static_cast<int>(mapper_index) - static_cast<int>(iter.mapper_index);
		}

		[[nodiscard]] constexpr bool operator ==(const indexed_vector_const_iterator &iter) const noexcept
		{
			dsga_constexpr_assert(mapper_ptr == iter.mapper_ptr, "different indexed_vector source");

			return ((mapper_ptr == iter.mapper_ptr) && (mapper_index == iter.mapper_index));
		}

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const indexed_vector_const_iterator &iter) const noexcept
		{
			dsga_constexpr_assert(mapper_ptr == iter.mapper_ptr, "different indexed_vector source");

			return mapper_index <=> iter.mapper_index;
		}

		[[nodiscard]] constexpr reference operator [](const int offset) const noexcept
		{
			dsga_constexpr_assert(nullptr != mapper_ptr, "can't deref nullptr");
			dsga_constexpr_assert(((mapper_index + offset) >= begin_index) && ((mapper_index + offset) < end_index), "offset not in range");

			return (*mapper_ptr)[mapper_index + offset];
		}

		[[nodiscard]] constexpr indexed_vector_const_iterator operator +(const int offset) const noexcept
		{
			indexed_vector_const_iterator temp = *this;
			temp += offset;
			return temp;
		}

		[[nodiscard]] constexpr indexed_vector_const_iterator operator -(const int offset) const noexcept
		{
			indexed_vector_const_iterator temp = *this;
			temp -= offset;
			return temp;
		}

		[[nodiscard]] friend constexpr indexed_vector_const_iterator operator +(const int offset, indexed_vector_const_iterator iter) noexcept
		{
			iter += offset;
			return iter;
		}
	};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable<Size, Count, Is...>
	struct indexed_vector_iterator : indexed_vector_const_iterator<T, Size, Count, Is...>
	{
		// let base class do all the work
		using base_iter = indexed_vector_const_iterator<T, Size, Count, Is...>;

		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = T *;
		using reference = T &;
		using const_reference = const T &;

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr indexed_vector_iterator(indexed_vector<T, Size, Count, Is ...> &mapper, int index) noexcept
			: base_iter(mapper, index)
		{
		}

		constexpr indexed_vector_iterator() noexcept = default;
		constexpr indexed_vector_iterator(const indexed_vector_iterator &) noexcept = default;
		constexpr indexed_vector_iterator(indexed_vector_iterator &&) noexcept = default;
		constexpr indexed_vector_iterator &operator =(const indexed_vector_iterator &) & noexcept = default;
		constexpr indexed_vector_iterator &operator =(indexed_vector_iterator &&) & noexcept = default;
		constexpr ~indexed_vector_iterator() = default;

		[[nodiscard]] constexpr reference operator *() const noexcept
		{
			return const_cast<reference>(base_iter::operator*());
		}

		[[nodiscard]] constexpr pointer operator ->() const noexcept
		{
			return const_cast<pointer>(base_iter::operator->());
		}

		constexpr indexed_vector_iterator &operator ++() noexcept
		{
			base_iter::operator++();
			return *this;
		}

		constexpr indexed_vector_iterator operator ++(int) noexcept
		{
			indexed_vector_iterator temp = *this;
			base_iter::operator++();
			return temp;
		}

		constexpr indexed_vector_iterator &operator --() noexcept
		{
			base_iter::operator--();
			return *this;
		}

		constexpr indexed_vector_iterator operator --(int) noexcept
		{
			indexed_vector_iterator temp = *this;
			base_iter::operator--();
			return temp;
		}

		constexpr indexed_vector_iterator &operator +=(const int offset) noexcept
		{
			base_iter::operator+=(offset);
			return *this;
		}

		constexpr indexed_vector_iterator &operator -=(const int offset) noexcept
		{
			base_iter::operator-=(offset);
			return *this;
		}

		[[nodiscard]] constexpr indexed_vector_iterator operator +(const int offset) const noexcept
		{
			indexed_vector_iterator temp = *this;
			temp += offset;
			return temp;
		}

		[[nodiscard]] friend constexpr indexed_vector_iterator operator +(const int offset, indexed_vector_iterator iter) noexcept
		{
			iter += offset;
			return iter;
		}

		using base_iter::operator-;

		[[nodiscard]] constexpr indexed_vector_iterator operator -(const int offset) const noexcept
		{
			indexed_vector_iterator temp = *this;
			temp -= offset;
			return temp;
		}

		[[nodiscard]] constexpr reference operator [](const int offset) const noexcept
		{
			return const_cast<reference>(base_iter::operator[](offset));
		}
	};

	//
	// indexed_vector - swizzle classes that are types of union members in basic_vector
	//

	// for swizzling 1D-4D parts of basic_vector
	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	requires indexable<Size, Count, Is...>
	struct indexed_vector<T, Size, Count, Is...>
		: vector_base<writable_swizzle<Size, Count, Is...>, T, Count, indexed_vector<T, Size, Count, Is...>>
	{
		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, Is...>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because operator[] does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<Is...>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> base;

		// using directives related to storage
		using value_type = dimensional_storage_t<T, Size>::value_type;
		using iterator = indexed_vector_iterator<T, Size, Count, Is...>;
		using const_iterator = indexed_vector_const_iterator<T, Size, Count, Is...>;
		using reverse_iterator = std::reverse_iterator<indexed_vector_iterator<T, Size, Count, Is...>>;
		using const_reverse_iterator = std::reverse_iterator<indexed_vector_const_iterator<T, Size, Count, Is...>>;

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) & noexcept
		{
			[&]<std::size_t ...Js>(std::index_sequence<Js ...>) noexcept
			{
				set(other[Js]...);
			}(std::make_index_sequence<Count>{});

			return *this;
		}

		// scalar assignment
		// assignment for some scalar type that converts to T and is only for indexed_vector of [Count == 1]
		template <dimensional_scalar U>
		requires Writable && implicitly_convertible_to<U, T> && (Count == 1)
		constexpr indexed_vector &operator =(U other) & noexcept
		{
			set(other);

			return *this;
		}

		//
		// scalar conversion operators
		//

		// this is extremely important and is only for indexed_vector of [Count == 1]
		explicit(false) constexpr operator T() const noexcept requires (Count == 1)
		{
			return base[offsets[0]];
		}

		template <typename U>
		requires std::convertible_to<T, U> && (Count == 1)
		explicit constexpr operator U() const noexcept
		{
			return static_cast<U>(base[offsets[0]]);
		}

		// logically contiguous - used by operator [] for read/write access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable
		{
			dsga_constexpr_assert(index >= 0 && static_cast<std::size_t>(index) < Count, "index out of bounds");
			return base[offsets[static_cast<std::size_t>(index)]];
		}

		// logically contiguous - used by operator [] for read access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept
		{
			dsga_constexpr_assert(index >= 0 && static_cast<std::size_t>(index) < Count, "index out of bounds");
			return base[offsets[static_cast<std::size_t>(index)]];
		}

		// physically contiguous
		[[nodiscard]] constexpr T *data() noexcept requires Writable		{ return base.data(); }

		// physically contiguous
		[[nodiscard]] constexpr const T *data() const noexcept				{ return base.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept				{ return sequence_pack{}; }

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable		{ return indexed_vector_iterator<T, Size, Count, Is...>(*this, 0); }
		[[nodiscard]] constexpr auto begin() const noexcept					{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, 0); }
		[[nodiscard]] constexpr auto cbegin() const noexcept				{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable		{ return indexed_vector_iterator<T, Size, Count, Is...>(*this, Count); }
		[[nodiscard]] constexpr auto end() const noexcept					{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, Count); }
		[[nodiscard]] constexpr auto cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable	{ return std::reverse_iterator<indexed_vector_iterator<T, Size, Count, Is...>>(end()); }
		[[nodiscard]] constexpr auto rbegin() const noexcept				{ return std::reverse_iterator<indexed_vector_const_iterator<T, Size, Count, Is...>>(end()); }
		[[nodiscard]] constexpr auto crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable		{ return std::reverse_iterator<indexed_vector_iterator<T, Size, Count, Is...>>(begin()); }
		[[nodiscard]] constexpr auto rend() const noexcept					{ return std::reverse_iterator<indexed_vector_const_iterator<T, Size, Count, Is...>>(begin()); }
		[[nodiscard]] constexpr auto crend() const noexcept					{ return rend(); }

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ... Args>
		requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
		constexpr void set(Args ...args) noexcept
		{
			// these Is are likely not sequential as they are in indexable order,
			// and we are accessing the internal storage directly. we are not using
			// the indirection built into indexed_vector::operator []() for this function.
			((base[Is] = static_cast<T>(args)), ...);
		}
	};

	//
	// convenience using types for indexed_vector as members of basic_vector
	//

	template <dimensional_scalar T, std::size_t Size, std::size_t I>
	requires indexable<Size, 1, I>
	using dexvec1 = indexed_vector<T, Size, 1, I>;

	template <dimensional_scalar T, std::size_t Size, std::size_t ...Is>
	requires indexable<Size, 2, Is...>
	using dexvec2 = indexed_vector<T, Size, 2, Is...>;

	template <dimensional_scalar T, std::size_t Size, std::size_t ...Is>
	requires indexable<Size, 3, Is...>
	using dexvec3 = indexed_vector<T, Size, 3, Is...>;

	template <dimensional_scalar T, std::size_t Size, std::size_t ...Is>
	requires indexable<Size, 4, Is...>
	using dexvec4 = indexed_vector<T, Size, 4, Is...>;

	//
	// basic_matrix will act as the primary matrix class in this library.
	//
	// T is the type of the elements stored in the matrix
	// C is the number of elements in a column
	// R is the number of elements in a row
	//

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4)))
	struct basic_matrix;

	//
	// this detail namespace provides support for variadic constructors for basic_vector and basic_matrix
	//

	namespace detail
	{

		// how many components can the item supply

		template <typename T>
		struct component_count;

		template <dimensional_scalar T>
		struct component_count<T>
		{
			static constexpr std::size_t value = 1;
		};

		template <dimensional_scalar T, std::size_t C>
		struct component_count<basic_vector<T, C>>
		{
			static constexpr std::size_t value = C;
		};

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		struct component_count<indexed_vector<T, S, C, Is...>>
		{
			static constexpr std::size_t value = C;
		};

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		struct component_count<vector_base<W, T, C, D>>
		{
			static constexpr std::size_t value = C;
		};

		template <floating_point_scalar T, std::size_t C, std::size_t R>
		struct component_count<basic_matrix<T, C, R>>
		{
			static constexpr std::size_t value = C * R;
		};

		// make sure Count and Args... are valid together w.r.t. component count.
		// Args is expected to be a combination of derived vector_base classes and
		// dimensional_scalars, possibly a matrix too.
		template <std::size_t Count, typename ...Args>
		struct component_match;

		// can't have 0 Args unless Count is 0
		template <std::size_t Count, typename ...Args>
		requires (sizeof...(Args) == 0)
		struct component_match<Count, Args...>
		{
			static constexpr bool valid = (Count == 0);
		};

		// check Count components needed with the info from variadic template Args and their component counts.
		// make sure the component count from the Args is sufficient for Count, and that we use all the Args.
		// if the last Arg isn't necessary to get to Count components, then the Args are invalid.
		//
		// Args is expected to be a combination of derived vector_base classes and dimensional_scalars.
		//
		// "...there must be enough components provided in the arguments to provide an initializer for
		// every component in the constructed value. It is a compile-time error to provide extra
		// arguments beyond this last used argument." section 5.4.2 of the spec for constructors (use case for this).
		template <std::size_t Count, typename ...Args>
		requires (sizeof...(Args) > 0) && (Count > 0)
		struct component_match<Count, Args...>
		{
			// total number components in Args...
			static constexpr std::size_t total_count = (component_count<Args>::value + ... + 0);
			using tuple_pack = std::tuple<Args...>;

			// get the last Arg type in the pack
			using last_type = std::tuple_element_t<std::tuple_size_v<tuple_pack> - 1, tuple_pack>;

			// see what the component count is if we didn't use the last Arg type in pack
			static constexpr std::size_t previous_count = total_count - component_count<last_type>::value;

			// check the conditions that we need exactly all those Args and that they give us enough components.
			static constexpr bool valid = (previous_count < Count) && (total_count >= Count);
		};

		template <std::size_t Count, typename ...Args>
		inline constexpr bool component_match_v = component_match<Count, Args...>::valid;

		// do Args... supply the correct number of components for Count without having leftover Args
		template <std::size_t Count, typename ...Args>
		concept met_component_count = component_match_v<Count, Args...>;

		// create a tuple from a scalar

		constexpr auto to_tuple(dimensional_scalar auto arg) noexcept
		{
			return std::tuple(arg);
		}

		// create a tuple from a vector

		template <dimensional_scalar T, std::size_t C>
		constexpr auto to_tuple(const basic_vector<T, C> &arg) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple(arg[Is]...);
			}(std::make_index_sequence<C>{});
		}

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		constexpr auto to_tuple(const indexed_vector<T, S, C, Is...> &arg) noexcept
		{
			return [&]<std::size_t ...Js>(std::index_sequence<Js...>) noexcept
			{
				return std::tuple(arg[Js]...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		constexpr auto to_tuple(const vector_base<W, T, C, D> &arg) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple(arg[Is]...);
			}(std::make_index_sequence<C>{});
		}

		template <floating_point_scalar T, std::size_t C, std::size_t R>
		constexpr auto to_tuple(const basic_matrix<T, C, R> &arg) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple_cat(to_tuple(arg[Is])...);
			}(std::make_index_sequence<C>{});
		}

		// flatten the Args out in a big tuple. Args is expected to be a combination of derived vector_base classes
		// and dimensional_scalars, possibly a matrix (for basic_vector).
		template <typename ...Args>
		constexpr auto flatten_args_to_tuple(const Args & ...args) noexcept
		{
			return std::tuple_cat(to_tuple(args)...);
		}

		template <typename U, typename T>
		struct valid_matrix_component : std::false_type
		{
		};

		template <dimensional_scalar U, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<U, T> : std::true_type
		{
		};

		template <dimensional_scalar U, std::size_t C, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<basic_vector<U, C>, T> : std::true_type
		{
		};

		template <dimensional_scalar U, std::size_t S, std::size_t C, std::size_t ...Is, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<indexed_vector<U, S, C, Is...>, T> : std::true_type
		{
		};

		template <bool W, dimensional_scalar U, std::size_t C, typename D, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<vector_base<W, U, C, D>, T> : std::true_type
		{
		};

		template <typename U, typename T>
		struct valid_vector_component : std::false_type
		{
		};

		template <dimensional_scalar U, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<U, T> : std::true_type
		{
		};

		template <dimensional_scalar U, std::size_t C, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<basic_vector<U, C>, T> : std::true_type
		{
		};

		template <dimensional_scalar U, std::size_t S, std::size_t C, std::size_t ...Is, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<indexed_vector<U, S, C, Is...>, T> : std::true_type
		{
		};

		template <bool W, dimensional_scalar U, std::size_t C, typename D, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<vector_base<W, U, C, D>, T> : std::true_type
		{
		};

		template <floating_point_scalar U, std::size_t C, std::size_t R, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<basic_matrix<U, C, R>, T> : std::true_type
		{
		};
	}

	//
	// basic_vector - the fundamental vector class
	//

	template <dimensional_scalar T>
	struct basic_vector<T, 1> : vector_base<true, T, 1, basic_vector<T, 1>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 1;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because operator[] does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			base;

			dexvec1<T, Size, 0>					x;				// Writable

			dexvec2<T, Size, 0, 0>				xx;

			dexvec3<T, Size, 0, 0, 0>			xxx;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
		};

		// using directives related to storage
		using value_type = storage_wrapper<T, Size>::value_type;
		using iterator = storage_wrapper<T, Size>::iterator;
		using const_iterator = storage_wrapper<T, Size>::const_iterator;
		using reverse_iterator = storage_wrapper<T, Size>::reverse_iterator;
		using const_reverse_iterator = storage_wrapper<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) & noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) & noexcept = default;

		//
		// constructors
		//

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T>
		explicit(false) constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0]) }
		{
		}

		template <typename U>
		requires std::convertible_to<U, T>
		explicit(!implicitly_convertible_to<U, T>) constexpr basic_vector(U value) noexcept
			: base{ static_cast<T>(value) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr basic_vector(const U &u, const Args & ...args) noexcept
			: base{}
		{
			auto arg_tuple = detail::flatten_args_to_tuple(u, args...);
			[this, &arg_tuple]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Count>{});
		}

		//
		// implicit assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) & noexcept
		{
			set(other[0]);
			return *this;
		}

		template <typename U>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(U value) & noexcept
		{
			set(value);
			return *this;
		}

		//
		// scalar conversion operators
		//

		// this is extremely important and is only for basic_vector of [Size == 1]
		explicit(false) constexpr operator T() const noexcept
		{
			return base[0];
		}

		template <typename U>
		requires std::convertible_to<T, U>
		explicit constexpr operator U() const noexcept
		{
			return static_cast<U>(base[0]);
		}

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable		{ return base[index]; }

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept				{ return base[index]; }

		// physically contiguous
		[[nodiscard]] constexpr T *data() noexcept requires Writable		{ return base.data(); }

		// physically contiguous
		[[nodiscard]] constexpr const T *data() const noexcept				{ return base.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept				{ return sequence_pack{}; }

		constexpr void swap(basic_vector &bv) noexcept requires Writable	{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable		{ return base.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept				{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable		{ return base.end(); }
		[[nodiscard]] constexpr auto end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable	{ return base.rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept				{ return base.crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable		{ return base.rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename U>
		requires Writable && std::convertible_to<U, T>
		constexpr void set(U value) noexcept
		{
			base[0] = static_cast<T>(value);
		}
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 2> : vector_base<true, T, 2, basic_vector<T, 2>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 2;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because operator[] does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			base;

			dexvec1<T, Size, 0>					x;				// Writable
			dexvec1<T, Size, 1>					y;				// Writable

			dexvec2<T, Size, 0, 0>				xx;
			dexvec2<T, Size, 0, 1>				xy;				// Writable
			dexvec2<T, Size, 1, 0>				yx;				// Writable
			dexvec2<T, Size, 1, 1>				yy;

			dexvec3<T, Size, 0, 0, 0>			xxx;
			dexvec3<T, Size, 0, 0, 1>			xxy;
			dexvec3<T, Size, 0, 1, 0>			xyx;
			dexvec3<T, Size, 0, 1, 1>			xyy;
			dexvec3<T, Size, 1, 0, 0>			yxx;
			dexvec3<T, Size, 1, 0, 1>			yxy;
			dexvec3<T, Size, 1, 1, 0>			yyx;
			dexvec3<T, Size, 1, 1, 1>			yyy;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
		};

		// using directives related to storage
		using value_type = storage_wrapper<T, Size>::value_type;
		using iterator = storage_wrapper<T, Size>::iterator;
		using const_iterator = storage_wrapper<T, Size>::const_iterator;
		using reverse_iterator = storage_wrapper<T, Size>::reverse_iterator;
		using const_reverse_iterator = storage_wrapper<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) & noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) & noexcept = default;

		//
		// constructors
		//

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr basic_vector(U value) noexcept
			: base{ static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to <U2, T>
		explicit constexpr basic_vector(U1 xvalue, U2 yvalue) noexcept
			: base{ static_cast<T>(xvalue), static_cast<T>(yvalue) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		explicit(false) constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0]), static_cast<T>(other[1]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr basic_vector(const U &u, const Args & ...args) noexcept
			: base{}
		{
			auto arg_tuple = detail::flatten_args_to_tuple(u, args...);
			[this, &arg_tuple]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Count>{});
		}

		//
		// assignment operator
		//

		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) & noexcept
		{
			set(other[0], other[1]);
			return *this;
		}

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable		{ return base[index]; }

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept				{ return base[index]; }

		// physically contiguous
		[[nodiscard]] constexpr T *data() noexcept requires Writable		{ return base.data(); }

		// physically contiguous
		[[nodiscard]] constexpr const T *data() const noexcept				{ return base.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept				{ return sequence_pack{}; }

		constexpr void swap(basic_vector &bv) noexcept requires Writable	{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable		{ return base.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept				{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable		{ return base.end(); }
		[[nodiscard]] constexpr auto end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable	{ return base.rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept				{ return base.crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable		{ return base.rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
		constexpr void set(Args ...args) noexcept
		{
			[&]<std::size_t ...Js>(std::index_sequence<Js ...>) noexcept
			{
				((base[Js] = static_cast<T>(args)), ...);
			}(std::make_index_sequence<Count>{});
		}
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 3> : vector_base<true, T, 3, basic_vector<T, 3>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 3;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because operator[] does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			base;

			dexvec1<T, Size, 0>					x;				// Writable
			dexvec1<T, Size, 1>					y;				// Writable
			dexvec1<T, Size, 2>					z;				// Writable

			dexvec2<T, Size, 0, 0>				xx;
			dexvec2<T, Size, 0, 1>				xy;				// Writable
			dexvec2<T, Size, 0, 2>				xz;				// Writable
			dexvec2<T, Size, 1, 0>				yx;				// Writable
			dexvec2<T, Size, 1, 1>				yy;
			dexvec2<T, Size, 1, 2>				yz;				// Writable
			dexvec2<T, Size, 2, 0>				zx;				// Writable
			dexvec2<T, Size, 2, 1>				zy;				// Writable
			dexvec2<T, Size, 2, 2>				zz;

			dexvec3<T, Size, 0, 0, 0>			xxx;
			dexvec3<T, Size, 0, 0, 1>			xxy;
			dexvec3<T, Size, 0, 0, 2>			xxz;
			dexvec3<T, Size, 0, 1, 0>			xyx;
			dexvec3<T, Size, 0, 1, 1>			xyy;
			dexvec3<T, Size, 0, 1, 2>			xyz;			// Writable
			dexvec3<T, Size, 0, 2, 0>			xzx;
			dexvec3<T, Size, 0, 2, 1>			xzy;			// Writable
			dexvec3<T, Size, 0, 2, 2>			xzz;
			dexvec3<T, Size, 1, 0, 0>			yxx;
			dexvec3<T, Size, 1, 0, 1>			yxy;
			dexvec3<T, Size, 1, 0, 2>			yxz;			// Writable
			dexvec3<T, Size, 1, 1, 0>			yyx;
			dexvec3<T, Size, 1, 1, 1>			yyy;
			dexvec3<T, Size, 1, 1, 2>			yyz;
			dexvec3<T, Size, 1, 2, 0>			yzx;			// Writable
			dexvec3<T, Size, 1, 2, 1>			yzy;
			dexvec3<T, Size, 1, 2, 2>			yzz;
			dexvec3<T, Size, 2, 0, 0>			zxx;
			dexvec3<T, Size, 2, 0, 1>			zxy;			// Writable
			dexvec3<T, Size, 2, 0, 2>			zxz;
			dexvec3<T, Size, 2, 1, 0>			zyx;			// Writable
			dexvec3<T, Size, 2, 1, 1>			zyy;
			dexvec3<T, Size, 2, 1, 2>			zyz;
			dexvec3<T, Size, 2, 2, 0>			zzx;
			dexvec3<T, Size, 2, 2, 1>			zzy;
			dexvec3<T, Size, 2, 2, 2>			zzz;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dexvec4<T, Size, 0, 0, 0, 2>		xxxz;
			dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dexvec4<T, Size, 0, 0, 1, 2>		xxyz;
			dexvec4<T, Size, 0, 0, 2, 0>		xxzx;
			dexvec4<T, Size, 0, 0, 2, 1>		xxzy;
			dexvec4<T, Size, 0, 0, 2, 2>		xxzz;
			dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dexvec4<T, Size, 0, 1, 0, 2>		xyxz;
			dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dexvec4<T, Size, 0, 1, 1, 2>		xyyz;
			dexvec4<T, Size, 0, 1, 2, 0>		xyzx;
			dexvec4<T, Size, 0, 1, 2, 1>		xyzy;
			dexvec4<T, Size, 0, 1, 2, 2>		xyzz;
			dexvec4<T, Size, 0, 2, 0, 0>		xzxx;
			dexvec4<T, Size, 0, 2, 0, 1>		xzxy;
			dexvec4<T, Size, 0, 2, 0, 2>		xzxz;
			dexvec4<T, Size, 0, 2, 1, 0>		xzyx;
			dexvec4<T, Size, 0, 2, 1, 1>		xzyy;
			dexvec4<T, Size, 0, 2, 1, 2>		xzyz;
			dexvec4<T, Size, 0, 2, 2, 0>		xzzx;
			dexvec4<T, Size, 0, 2, 2, 1>		xzzy;
			dexvec4<T, Size, 0, 2, 2, 2>		xzzz;
			dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dexvec4<T, Size, 1, 0, 0, 2>		yxxz;
			dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dexvec4<T, Size, 1, 0, 1, 2>		yxyz;
			dexvec4<T, Size, 1, 0, 2, 0>		yxzx;
			dexvec4<T, Size, 1, 0, 2, 1>		yxzy;
			dexvec4<T, Size, 1, 0, 2, 2>		yxzz;
			dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dexvec4<T, Size, 1, 1, 0, 2>		yyxz;
			dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
			dexvec4<T, Size, 1, 1, 1, 2>		yyyz;
			dexvec4<T, Size, 1, 1, 2, 0>		yyzx;
			dexvec4<T, Size, 1, 1, 2, 1>		yyzy;
			dexvec4<T, Size, 1, 1, 2, 2>		yyzz;
			dexvec4<T, Size, 1, 2, 0, 0>		yzxx;
			dexvec4<T, Size, 1, 2, 0, 1>		yzxy;
			dexvec4<T, Size, 1, 2, 0, 2>		yzxz;
			dexvec4<T, Size, 1, 2, 1, 0>		yzyx;
			dexvec4<T, Size, 1, 2, 1, 1>		yzyy;
			dexvec4<T, Size, 1, 2, 1, 2>		yzyz;
			dexvec4<T, Size, 1, 2, 2, 0>		yzzx;
			dexvec4<T, Size, 1, 2, 2, 1>		yzzy;
			dexvec4<T, Size, 1, 2, 2, 2>		yzzz;
			dexvec4<T, Size, 2, 0, 0, 0>		zxxx;
			dexvec4<T, Size, 2, 0, 0, 1>		zxxy;
			dexvec4<T, Size, 2, 0, 0, 2>		zxxz;
			dexvec4<T, Size, 2, 0, 1, 0>		zxyx;
			dexvec4<T, Size, 2, 0, 1, 1>		zxyy;
			dexvec4<T, Size, 2, 0, 1, 2>		zxyz;
			dexvec4<T, Size, 2, 0, 2, 0>		zxzx;
			dexvec4<T, Size, 2, 0, 2, 1>		zxzy;
			dexvec4<T, Size, 2, 0, 2, 2>		zxzz;
			dexvec4<T, Size, 2, 1, 0, 0>		zyxx;
			dexvec4<T, Size, 2, 1, 0, 1>		zyxy;
			dexvec4<T, Size, 2, 1, 0, 2>		zyxz;
			dexvec4<T, Size, 2, 1, 1, 0>		zyyx;
			dexvec4<T, Size, 2, 1, 1, 1>		zyyy;
			dexvec4<T, Size, 2, 1, 1, 2>		zyyz;
			dexvec4<T, Size, 2, 1, 2, 0>		zyzx;
			dexvec4<T, Size, 2, 1, 2, 1>		zyzy;
			dexvec4<T, Size, 2, 1, 2, 2>		zyzz;
			dexvec4<T, Size, 2, 2, 0, 0>		zzxx;
			dexvec4<T, Size, 2, 2, 0, 1>		zzxy;
			dexvec4<T, Size, 2, 2, 0, 2>		zzxz;
			dexvec4<T, Size, 2, 2, 1, 0>		zzyx;
			dexvec4<T, Size, 2, 2, 1, 1>		zzyy;
			dexvec4<T, Size, 2, 2, 1, 2>		zzyz;
			dexvec4<T, Size, 2, 2, 2, 0>		zzzx;
			dexvec4<T, Size, 2, 2, 2, 1>		zzzy;
			dexvec4<T, Size, 2, 2, 2, 2>		zzzz;
		};

		// using directives related to storage
		using value_type = storage_wrapper<T, Size>::value_type;
		using iterator = storage_wrapper<T, Size>::iterator;
		using const_iterator = storage_wrapper<T, Size>::const_iterator;
		using reverse_iterator = storage_wrapper<T, Size>::reverse_iterator;
		using const_reverse_iterator = storage_wrapper<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) & noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) & noexcept = default;

		//
		// constructors
		//

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr basic_vector(U value) noexcept
			: base{ static_cast<T>(value), static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue) noexcept
			: base{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		explicit(false) constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0]), static_cast<T>(other[1]), static_cast<T>(other[2]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr basic_vector(const U &u, const Args & ...args) noexcept
			: base{}
		{
			auto arg_tuple = detail::flatten_args_to_tuple(u, args...);
			[this, &arg_tuple]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Count>{});
		}

		//
		// assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) & noexcept
		{
			set(other[0], other[1], other[2]);
			return *this;
		}

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable		{ return base[index]; }

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept				{ return base[index]; }

		// physically contiguous
		[[nodiscard]] constexpr T *data() noexcept requires Writable		{ return base.data(); }

		// physically contiguous
		[[nodiscard]] constexpr const T *data() const noexcept				{ return base.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept				{ return sequence_pack{}; }

		constexpr void swap(basic_vector &bv) noexcept requires Writable	{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable		{ return base.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept				{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable		{ return base.end(); }
		[[nodiscard]] constexpr auto end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable	{ return base.rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept				{ return base.crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable		{ return base.rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
		constexpr void set(Args ...args) noexcept
		{
			[&]<std::size_t ...Js>(std::index_sequence<Js ...>) noexcept
			{
				((base[Js] = static_cast<T>(args)), ...);
			}(std::make_index_sequence<Count>{});
		}
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 4> : vector_base<true, T, 4, basic_vector<T, 4>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 4;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because operator[] does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			base;

			dexvec1<T, Size, 0>					x;				// Writable
			dexvec1<T, Size, 1>					y;				// Writable
			dexvec1<T, Size, 2>					z;				// Writable
			dexvec1<T, Size, 3>					w;				// Writable

			dexvec2<T, Size, 0, 0>				xx;
			dexvec2<T, Size, 0, 1>				xy;				// Writable
			dexvec2<T, Size, 0, 2>				xz;				// Writable
			dexvec2<T, Size, 0, 3>				xw;				// Writable
			dexvec2<T, Size, 1, 0>				yx;				// Writable
			dexvec2<T, Size, 1, 1>				yy;
			dexvec2<T, Size, 1, 2>				yz;				// Writable
			dexvec2<T, Size, 1, 3>				yw;				// Writable
			dexvec2<T, Size, 2, 0>				zx;				// Writable
			dexvec2<T, Size, 2, 1>				zy;				// Writable
			dexvec2<T, Size, 2, 2>				zz;
			dexvec2<T, Size, 2, 3>				zw;				// Writable
			dexvec2<T, Size, 3, 0>				wx;				// Writable
			dexvec2<T, Size, 3, 1>				wy;				// Writable
			dexvec2<T, Size, 3, 2>				wz;				// Writable
			dexvec2<T, Size, 3, 3>				ww;

			dexvec3<T, Size, 0, 0, 0>			xxx;
			dexvec3<T, Size, 0, 0, 1>			xxy;
			dexvec3<T, Size, 0, 0, 2>			xxz;
			dexvec3<T, Size, 0, 0, 3>			xxw;
			dexvec3<T, Size, 0, 1, 0>			xyx;
			dexvec3<T, Size, 0, 1, 1>			xyy;
			dexvec3<T, Size, 0, 1, 2>			xyz;			// Writable
			dexvec3<T, Size, 0, 1, 3>			xyw;			// Writable
			dexvec3<T, Size, 0, 2, 0>			xzx;
			dexvec3<T, Size, 0, 2, 1>			xzy;			// Writable
			dexvec3<T, Size, 0, 2, 2>			xzz;
			dexvec3<T, Size, 0, 2, 3>			xzw;			// Writable
			dexvec3<T, Size, 0, 3, 0>			xwx;
			dexvec3<T, Size, 0, 3, 1>			xwy;			// Writable
			dexvec3<T, Size, 0, 3, 2>			xwz;			// Writable
			dexvec3<T, Size, 0, 3, 3>			xww;
			dexvec3<T, Size, 1, 0, 0>			yxx;
			dexvec3<T, Size, 1, 0, 1>			yxy;
			dexvec3<T, Size, 1, 0, 2>			yxz;			// Writable
			dexvec3<T, Size, 1, 0, 3>			yxw;			// Writable
			dexvec3<T, Size, 1, 1, 0>			yyx;
			dexvec3<T, Size, 1, 1, 1>			yyy;
			dexvec3<T, Size, 1, 1, 2>			yyz;
			dexvec3<T, Size, 1, 1, 3>			yyw;
			dexvec3<T, Size, 1, 2, 0>			yzx;			// Writable
			dexvec3<T, Size, 1, 2, 1>			yzy;
			dexvec3<T, Size, 1, 2, 2>			yzz;
			dexvec3<T, Size, 1, 2, 3>			yzw;			// Writable
			dexvec3<T, Size, 1, 3, 0>			ywx;			// Writable
			dexvec3<T, Size, 1, 3, 1>			ywy;
			dexvec3<T, Size, 1, 3, 2>			ywz;			// Writable
			dexvec3<T, Size, 1, 3, 3>			yww;
			dexvec3<T, Size, 2, 0, 0>			zxx;
			dexvec3<T, Size, 2, 0, 1>			zxy;			// Writable
			dexvec3<T, Size, 2, 0, 2>			zxz;
			dexvec3<T, Size, 2, 0, 3>			zxw;			// Writable
			dexvec3<T, Size, 2, 1, 0>			zyx;			// Writable
			dexvec3<T, Size, 2, 1, 1>			zyy;
			dexvec3<T, Size, 2, 1, 2>			zyz;
			dexvec3<T, Size, 2, 1, 3>			zyw;			// Writable
			dexvec3<T, Size, 2, 2, 0>			zzx;
			dexvec3<T, Size, 2, 2, 1>			zzy;
			dexvec3<T, Size, 2, 2, 2>			zzz;
			dexvec3<T, Size, 2, 2, 3>			zzw;
			dexvec3<T, Size, 2, 3, 0>			zwx;			// Writable
			dexvec3<T, Size, 2, 3, 1>			zwy;			// Writable
			dexvec3<T, Size, 2, 3, 2>			zwz;
			dexvec3<T, Size, 2, 3, 3>			zww;
			dexvec3<T, Size, 3, 0, 0>			wxx;
			dexvec3<T, Size, 3, 0, 1>			wxy;			// Writable
			dexvec3<T, Size, 3, 0, 2>			wxz;			// Writable
			dexvec3<T, Size, 3, 0, 3>			wxw;
			dexvec3<T, Size, 3, 1, 0>			wyx;			// Writable
			dexvec3<T, Size, 3, 1, 1>			wyy;
			dexvec3<T, Size, 3, 1, 2>			wyz;			// Writable
			dexvec3<T, Size, 3, 1, 3>			wyw;
			dexvec3<T, Size, 3, 2, 0>			wzx;			// Writable
			dexvec3<T, Size, 3, 2, 1>			wzy;
			dexvec3<T, Size, 3, 2, 2>			wzz;			// Writable
			dexvec3<T, Size, 3, 2, 3>			wzw;
			dexvec3<T, Size, 3, 3, 0>			wwx;
			dexvec3<T, Size, 3, 3, 1>			wwy;
			dexvec3<T, Size, 3, 3, 2>			wwz;
			dexvec3<T, Size, 3, 3, 3>			www;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dexvec4<T, Size, 0, 0, 0, 2>		xxxz;
			dexvec4<T, Size, 0, 0, 0, 3>		xxxw;
			dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dexvec4<T, Size, 0, 0, 1, 2>		xxyz;
			dexvec4<T, Size, 0, 0, 1, 3>		xxyw;
			dexvec4<T, Size, 0, 0, 2, 0>		xxzx;
			dexvec4<T, Size, 0, 0, 2, 1>		xxzy;
			dexvec4<T, Size, 0, 0, 2, 2>		xxzz;
			dexvec4<T, Size, 0, 0, 2, 3>		xxzw;
			dexvec4<T, Size, 0, 0, 3, 0>		xxwx;
			dexvec4<T, Size, 0, 0, 3, 1>		xxwy;
			dexvec4<T, Size, 0, 0, 3, 2>		xxwz;
			dexvec4<T, Size, 0, 0, 3, 3>		xxww;
			dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dexvec4<T, Size, 0, 1, 0, 2>		xyxz;
			dexvec4<T, Size, 0, 1, 0, 3>		xyxw;
			dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dexvec4<T, Size, 0, 1, 1, 2>		xyyz;
			dexvec4<T, Size, 0, 1, 1, 3>		xyyw;
			dexvec4<T, Size, 0, 1, 2, 0>		xyzx;
			dexvec4<T, Size, 0, 1, 2, 1>		xyzy;
			dexvec4<T, Size, 0, 1, 2, 2>		xyzz;
			dexvec4<T, Size, 0, 1, 2, 3>		xyzw;			// Writable
			dexvec4<T, Size, 0, 1, 3, 0>		xywx;
			dexvec4<T, Size, 0, 1, 3, 1>		xywy;
			dexvec4<T, Size, 0, 1, 3, 2>		xywz;			// Writable
			dexvec4<T, Size, 0, 1, 3, 3>		xyww;
			dexvec4<T, Size, 0, 2, 0, 0>		xzxx;
			dexvec4<T, Size, 0, 2, 0, 1>		xzxy;
			dexvec4<T, Size, 0, 2, 0, 2>		xzxz;
			dexvec4<T, Size, 0, 2, 0, 3>		xzxw;
			dexvec4<T, Size, 0, 2, 1, 0>		xzyx;
			dexvec4<T, Size, 0, 2, 1, 1>		xzyy;
			dexvec4<T, Size, 0, 2, 1, 2>		xzyz;
			dexvec4<T, Size, 0, 2, 1, 3>		xzyw;			// Writable
			dexvec4<T, Size, 0, 2, 2, 0>		xzzx;
			dexvec4<T, Size, 0, 2, 2, 1>		xzzy;
			dexvec4<T, Size, 0, 2, 2, 2>		xzzz;
			dexvec4<T, Size, 0, 2, 2, 3>		xzzw;
			dexvec4<T, Size, 0, 2, 3, 0>		xzwx;
			dexvec4<T, Size, 0, 2, 3, 1>		xzwy;			// Writable
			dexvec4<T, Size, 0, 2, 3, 2>		xzwz;
			dexvec4<T, Size, 0, 2, 3, 3>		xzww;
			dexvec4<T, Size, 0, 3, 0, 0>		xwxx;
			dexvec4<T, Size, 0, 3, 0, 1>		xwxy;
			dexvec4<T, Size, 0, 3, 0, 2>		xwxz;
			dexvec4<T, Size, 0, 3, 0, 3>		xwxw;
			dexvec4<T, Size, 0, 3, 1, 0>		xwyx;
			dexvec4<T, Size, 0, 3, 1, 1>		xwyy;
			dexvec4<T, Size, 0, 3, 1, 2>		xwyz;			// Writable
			dexvec4<T, Size, 0, 3, 1, 3>		xwyw;
			dexvec4<T, Size, 0, 3, 2, 0>		xwzx;
			dexvec4<T, Size, 0, 3, 2, 1>		xwzy;			// Writable
			dexvec4<T, Size, 0, 3, 2, 2>		xwzz;
			dexvec4<T, Size, 0, 3, 2, 3>		xwzw;
			dexvec4<T, Size, 0, 3, 3, 0>		xwwx;
			dexvec4<T, Size, 0, 3, 3, 1>		xwwy;
			dexvec4<T, Size, 0, 3, 3, 2>		xwwz;
			dexvec4<T, Size, 0, 3, 3, 3>		xwww;
			dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dexvec4<T, Size, 1, 0, 0, 2>		yxxz;
			dexvec4<T, Size, 1, 0, 0, 3>		yxxw;
			dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dexvec4<T, Size, 1, 0, 1, 2>		yxyz;
			dexvec4<T, Size, 1, 0, 1, 3>		yxyw;
			dexvec4<T, Size, 1, 0, 2, 0>		yxzx;
			dexvec4<T, Size, 1, 0, 2, 1>		yxzy;
			dexvec4<T, Size, 1, 0, 2, 2>		yxzz;
			dexvec4<T, Size, 1, 0, 2, 3>		yxzw;			// Writable
			dexvec4<T, Size, 1, 0, 3, 0>		yxwx;
			dexvec4<T, Size, 1, 0, 3, 1>		yxwy;
			dexvec4<T, Size, 1, 0, 3, 2>		yxwz;			// Writable
			dexvec4<T, Size, 1, 0, 3, 3>		yxww;
			dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dexvec4<T, Size, 1, 1, 0, 2>		yyxz;
			dexvec4<T, Size, 1, 1, 0, 3>		yyxw;
			dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
			dexvec4<T, Size, 1, 1, 1, 2>		yyyz;
			dexvec4<T, Size, 1, 1, 1, 3>		yyyw;
			dexvec4<T, Size, 1, 1, 2, 0>		yyzx;
			dexvec4<T, Size, 1, 1, 2, 1>		yyzy;
			dexvec4<T, Size, 1, 1, 2, 2>		yyzz;
			dexvec4<T, Size, 1, 1, 2, 3>		yyzw;
			dexvec4<T, Size, 1, 1, 3, 0>		yywx;
			dexvec4<T, Size, 1, 1, 3, 1>		yywy;
			dexvec4<T, Size, 1, 1, 3, 2>		yywz;
			dexvec4<T, Size, 1, 1, 3, 3>		yyww;
			dexvec4<T, Size, 1, 2, 0, 0>		yzxx;
			dexvec4<T, Size, 1, 2, 0, 1>		yzxy;
			dexvec4<T, Size, 1, 2, 0, 2>		yzxz;
			dexvec4<T, Size, 1, 2, 0, 3>		yzxw;			// Writable
			dexvec4<T, Size, 1, 2, 1, 0>		yzyx;
			dexvec4<T, Size, 1, 2, 1, 1>		yzyy;
			dexvec4<T, Size, 1, 2, 1, 2>		yzyz;
			dexvec4<T, Size, 1, 2, 1, 3>		yzyw;
			dexvec4<T, Size, 1, 2, 2, 0>		yzzx;
			dexvec4<T, Size, 1, 2, 2, 1>		yzzy;
			dexvec4<T, Size, 1, 2, 2, 2>		yzzz;
			dexvec4<T, Size, 1, 2, 2, 3>		yzzw;
			dexvec4<T, Size, 1, 2, 3, 0>		yzwx;			// Writable
			dexvec4<T, Size, 1, 2, 3, 1>		yzwy;
			dexvec4<T, Size, 1, 2, 3, 2>		yzwz;
			dexvec4<T, Size, 1, 2, 3, 3>		yzww;
			dexvec4<T, Size, 1, 3, 0, 0>		ywxx;
			dexvec4<T, Size, 1, 3, 0, 1>		ywxy;
			dexvec4<T, Size, 1, 3, 0, 2>		ywxz;			// Writable
			dexvec4<T, Size, 1, 3, 0, 3>		ywxw;
			dexvec4<T, Size, 1, 3, 1, 0>		ywyx;
			dexvec4<T, Size, 1, 3, 1, 1>		ywyy;
			dexvec4<T, Size, 1, 3, 1, 2>		ywyz;
			dexvec4<T, Size, 1, 3, 1, 3>		ywyw;
			dexvec4<T, Size, 1, 3, 2, 0>		ywzx;			// Writable
			dexvec4<T, Size, 1, 3, 2, 1>		ywzy;
			dexvec4<T, Size, 1, 3, 2, 2>		ywzz;
			dexvec4<T, Size, 1, 3, 2, 3>		ywzw;
			dexvec4<T, Size, 1, 3, 3, 0>		ywwx;
			dexvec4<T, Size, 1, 3, 3, 1>		ywwy;
			dexvec4<T, Size, 1, 3, 3, 2>		ywwz;
			dexvec4<T, Size, 1, 3, 3, 3>		ywww;
			dexvec4<T, Size, 2, 0, 0, 0>		zxxx;
			dexvec4<T, Size, 2, 0, 0, 1>		zxxy;
			dexvec4<T, Size, 2, 0, 0, 2>		zxxz;
			dexvec4<T, Size, 2, 0, 0, 3>		zxxw;
			dexvec4<T, Size, 2, 0, 1, 0>		zxyx;
			dexvec4<T, Size, 2, 0, 1, 1>		zxyy;
			dexvec4<T, Size, 2, 0, 1, 2>		zxyz;
			dexvec4<T, Size, 2, 0, 1, 3>		zxyw;			// Writable
			dexvec4<T, Size, 2, 0, 2, 0>		zxzx;
			dexvec4<T, Size, 2, 0, 2, 1>		zxzy;
			dexvec4<T, Size, 2, 0, 2, 2>		zxzz;
			dexvec4<T, Size, 2, 0, 2, 3>		zxzw;
			dexvec4<T, Size, 2, 0, 3, 0>		zxwx;
			dexvec4<T, Size, 2, 0, 3, 1>		zxwy;			// Writable
			dexvec4<T, Size, 2, 0, 3, 2>		zxwz;
			dexvec4<T, Size, 2, 0, 3, 3>		zxww;
			dexvec4<T, Size, 2, 1, 0, 0>		zyxx;
			dexvec4<T, Size, 2, 1, 0, 1>		zyxy;
			dexvec4<T, Size, 2, 1, 0, 2>		zyxz;
			dexvec4<T, Size, 2, 1, 0, 3>		zyxw;			// Writable
			dexvec4<T, Size, 2, 1, 1, 0>		zyyx;
			dexvec4<T, Size, 2, 1, 1, 1>		zyyy;
			dexvec4<T, Size, 2, 1, 1, 2>		zyyz;
			dexvec4<T, Size, 2, 1, 1, 3>		zyyw;
			dexvec4<T, Size, 2, 1, 2, 0>		zyzx;
			dexvec4<T, Size, 2, 1, 2, 1>		zyzy;
			dexvec4<T, Size, 2, 1, 2, 2>		zyzz;
			dexvec4<T, Size, 2, 1, 2, 3>		zyzw;
			dexvec4<T, Size, 2, 1, 3, 0>		zywx;			// Writable
			dexvec4<T, Size, 2, 1, 3, 1>		zywy;
			dexvec4<T, Size, 2, 1, 3, 2>		zywz;
			dexvec4<T, Size, 2, 1, 3, 3>		zyww;
			dexvec4<T, Size, 2, 2, 0, 0>		zzxx;
			dexvec4<T, Size, 2, 2, 0, 1>		zzxy;
			dexvec4<T, Size, 2, 2, 0, 2>		zzxz;
			dexvec4<T, Size, 2, 2, 0, 3>		zzxw;
			dexvec4<T, Size, 2, 2, 1, 0>		zzyx;
			dexvec4<T, Size, 2, 2, 1, 1>		zzyy;
			dexvec4<T, Size, 2, 2, 1, 2>		zzyz;
			dexvec4<T, Size, 2, 2, 1, 3>		zzyw;
			dexvec4<T, Size, 2, 2, 2, 0>		zzzx;
			dexvec4<T, Size, 2, 2, 2, 1>		zzzy;
			dexvec4<T, Size, 2, 2, 2, 2>		zzzz;
			dexvec4<T, Size, 2, 2, 2, 3>		zzzw;
			dexvec4<T, Size, 2, 2, 3, 0>		zzwx;
			dexvec4<T, Size, 2, 2, 3, 1>		zzwy;
			dexvec4<T, Size, 2, 2, 3, 2>		zzwz;
			dexvec4<T, Size, 2, 2, 3, 3>		zzww;
			dexvec4<T, Size, 2, 3, 0, 0>		zwxx;
			dexvec4<T, Size, 2, 3, 0, 1>		zwxy;			// Writable
			dexvec4<T, Size, 2, 3, 0, 2>		zwxz;
			dexvec4<T, Size, 2, 3, 0, 3>		zwxw;
			dexvec4<T, Size, 2, 3, 1, 0>		zwyx;			// Writable
			dexvec4<T, Size, 2, 3, 1, 1>		zwyy;
			dexvec4<T, Size, 2, 3, 1, 2>		zwyz;
			dexvec4<T, Size, 2, 3, 1, 3>		zwyw;
			dexvec4<T, Size, 2, 3, 2, 0>		zwzx;
			dexvec4<T, Size, 2, 3, 2, 1>		zwzy;
			dexvec4<T, Size, 2, 3, 2, 2>		zwzz;
			dexvec4<T, Size, 2, 3, 2, 3>		zwzw;
			dexvec4<T, Size, 2, 3, 3, 0>		zwwx;
			dexvec4<T, Size, 2, 3, 3, 1>		zwwy;
			dexvec4<T, Size, 2, 3, 3, 2>		zwwz;
			dexvec4<T, Size, 2, 3, 3, 3>		zwww;
			dexvec4<T, Size, 3, 0, 0, 0>		wxxx;
			dexvec4<T, Size, 3, 0, 0, 1>		wxxy;
			dexvec4<T, Size, 3, 0, 0, 2>		wxxz;
			dexvec4<T, Size, 3, 0, 0, 3>		wxxw;
			dexvec4<T, Size, 3, 0, 1, 0>		wxyx;
			dexvec4<T, Size, 3, 0, 1, 1>		wxyy;
			dexvec4<T, Size, 3, 0, 1, 2>		wxyz;			// Writable
			dexvec4<T, Size, 3, 0, 1, 3>		wxyw;
			dexvec4<T, Size, 3, 0, 2, 0>		wxzx;
			dexvec4<T, Size, 3, 0, 2, 1>		wxzy;			// Writable
			dexvec4<T, Size, 3, 0, 2, 2>		wxzz;
			dexvec4<T, Size, 3, 0, 2, 3>		wxzw;
			dexvec4<T, Size, 3, 0, 3, 0>		wxwx;
			dexvec4<T, Size, 3, 0, 3, 1>		wxwy;
			dexvec4<T, Size, 3, 0, 3, 2>		wxwz;
			dexvec4<T, Size, 3, 0, 3, 3>		wxww;
			dexvec4<T, Size, 3, 1, 0, 0>		wyxx;
			dexvec4<T, Size, 3, 1, 0, 1>		wyxy;
			dexvec4<T, Size, 3, 1, 0, 2>		wyxz;			// Writable
			dexvec4<T, Size, 3, 1, 0, 3>		wyxw;
			dexvec4<T, Size, 3, 1, 1, 0>		wyyx;
			dexvec4<T, Size, 3, 1, 1, 1>		wyyy;
			dexvec4<T, Size, 3, 1, 1, 2>		wyyz;
			dexvec4<T, Size, 3, 1, 1, 3>		wyyw;
			dexvec4<T, Size, 3, 1, 2, 0>		wyzx;			// Writable
			dexvec4<T, Size, 3, 1, 2, 1>		wyzy;
			dexvec4<T, Size, 3, 1, 2, 2>		wyzz;
			dexvec4<T, Size, 3, 1, 2, 3>		wyzw;
			dexvec4<T, Size, 3, 1, 3, 0>		wywx;
			dexvec4<T, Size, 3, 1, 3, 1>		wywy;
			dexvec4<T, Size, 3, 1, 3, 2>		wywz;
			dexvec4<T, Size, 3, 1, 3, 3>		wyww;
			dexvec4<T, Size, 3, 2, 0, 0>		wzxx;
			dexvec4<T, Size, 3, 2, 0, 1>		wzxy;			// Writable
			dexvec4<T, Size, 3, 2, 0, 2>		wzxz;
			dexvec4<T, Size, 3, 2, 0, 3>		wzxw;
			dexvec4<T, Size, 3, 2, 1, 0>		wzyx;			// Writable
			dexvec4<T, Size, 3, 2, 1, 1>		wzyy;
			dexvec4<T, Size, 3, 2, 1, 2>		wzyz;
			dexvec4<T, Size, 3, 2, 1, 3>		wzyw;
			dexvec4<T, Size, 3, 2, 2, 0>		wzzx;
			dexvec4<T, Size, 3, 2, 2, 1>		wzzy;
			dexvec4<T, Size, 3, 2, 2, 2>		wzzz;
			dexvec4<T, Size, 3, 2, 2, 3>		wzzw;
			dexvec4<T, Size, 3, 2, 3, 0>		wzwx;
			dexvec4<T, Size, 3, 2, 3, 1>		wzwy;
			dexvec4<T, Size, 3, 2, 3, 2>		wzwz;
			dexvec4<T, Size, 3, 2, 3, 3>		wzww;
			dexvec4<T, Size, 3, 3, 0, 0>		wwxx;
			dexvec4<T, Size, 3, 3, 0, 1>		wwxy;
			dexvec4<T, Size, 3, 3, 0, 2>		wwxz;
			dexvec4<T, Size, 3, 3, 0, 3>		wwxw;
			dexvec4<T, Size, 3, 3, 1, 0>		wwyx;
			dexvec4<T, Size, 3, 3, 1, 1>		wwyy;
			dexvec4<T, Size, 3, 3, 1, 2>		wwyz;
			dexvec4<T, Size, 3, 3, 1, 3>		wwyw;
			dexvec4<T, Size, 3, 3, 2, 0>		wwzx;
			dexvec4<T, Size, 3, 3, 2, 1>		wwzy;
			dexvec4<T, Size, 3, 3, 2, 2>		wwzz;
			dexvec4<T, Size, 3, 3, 2, 3>		wwzw;
			dexvec4<T, Size, 3, 3, 3, 0>		wwwx;
			dexvec4<T, Size, 3, 3, 3, 1>		wwwy;
			dexvec4<T, Size, 3, 3, 3, 2>		wwwz;
			dexvec4<T, Size, 3, 3, 3, 3>		wwww;
		};

		// using directives related to storage
		using value_type = storage_wrapper<T, Size>::value_type;
		using iterator = storage_wrapper<T, Size>::iterator;
		using const_iterator = storage_wrapper<T, Size>::const_iterator;
		using reverse_iterator = storage_wrapper<T, Size>::reverse_iterator;
		using const_reverse_iterator = storage_wrapper<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) & noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) & noexcept = default;

		//
		// constructors
		//

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr basic_vector(U value) noexcept
			: base{ static_cast<T>(value), static_cast<T>(value), static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2, typename U3, typename U4>
		requires
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue,
										U4 wvalue) noexcept
			: base{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue), static_cast<T>(wvalue) }
		{
		}

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		explicit(false) constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: base{ static_cast<T>(other[0]), static_cast<T>(other[1]), static_cast<T>(other[2]), static_cast<T>(other[3]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr basic_vector(const U &u, const Args & ...args) noexcept
			: base{}
		{
			auto arg_tuple = detail::flatten_args_to_tuple(u, args...);
			[this, &arg_tuple]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Count>{});
		}

		//
		// assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) & noexcept
		{
			set(other[0], other[1], other[2], other[3]);
			return *this;
		}

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable		{ return base[index]; }

		// logically and physically contiguous - used by operator [] for access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept				{ return base[index]; }

		// physically contiguous
		[[nodiscard]] constexpr T *data() noexcept requires Writable		{ return base.data(); }

		// physically contiguous
		[[nodiscard]] constexpr const T *data() const noexcept				{ return base.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept				{ return sequence_pack{}; }

		constexpr void swap(basic_vector &bv) noexcept requires Writable	{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr auto begin() noexcept requires Writable		{ return base.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept				{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept requires Writable		{ return base.end(); }
		[[nodiscard]] constexpr auto end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept requires Writable	{ return base.rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept				{ return base.crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept requires Writable		{ return base.rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
		constexpr void set(Args ...args) noexcept
		{
			[&]<std::size_t ...Js>(std::index_sequence<Js ...>) noexcept
			{
				((base[Js] = static_cast<T>(args)), ...);
			}(std::make_index_sequence<Count>{});
		}
	};

	template <dimensional_scalar T, std::size_t Size>
	constexpr void swap(basic_vector<T, Size> &lhs, basic_vector<T, Size> &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	//
	// CTAD deduction guides for basic_vector
	//

	template <dimensional_scalar T, dimensional_scalar ...U>
	basic_vector(T, U...) -> basic_vector<T, 1 + sizeof...(U)>;

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	basic_vector(const vector_base<W, T, C, D> &) -> basic_vector<T, C>;

	//
	// machinery for vector operators and functions
	//

	namespace machinery
	{
		// return types from executing callables (lambdas) on arguments of various types

		template <typename UnOp, dimensional_scalar T>
		using unop_return_t = std::invoke_result_t<UnOp, T>;

		template <typename BinOp, dimensional_scalar T, dimensional_scalar U>
		using binop_return_t = std::invoke_result_t<BinOp, T, U>;

		template <typename TernOp, dimensional_scalar T, dimensional_scalar U, dimensional_scalar V>
		using ternop_return_t = std::invoke_result_t<TernOp, T, U, V>;

		//
		// this machinery relies on vector_base::operator[] to be logically contiguous operation on a derived vector type,
		// regardless of whether it is physically contiguous. apply the operation on components of vector_base arguments,
		// either returning a new vector (or scalar) or modifying an existing vector.
		//
		// apply_make() - one argument, one type -- return a new vector or scalar
		// apply_unitype_make() - all arguments are cast to their common type -- return a new vector or scalar
		// apply_multitype_make() - all arguments keep their types -- return a new vector or scalar
		// apply_unitype_modify() - all arguments are cast to their common type -- modify the first argument with new values
		// apply_multitype_modify() - all arguments keep their types -- modify the first argument with new values

		// unary

		template <bool W, dimensional_scalar T, std::size_t C, typename D, typename UnOp>
		constexpr auto apply_make(const vector_base<W, T, C, D> &arg,
								  UnOp &op) noexcept
		{
			if  constexpr (C == 1)
			{
				return op(arg[0]);
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<unop_return_t<UnOp, T>, C>{ op(arg[Is])... };
				}(std::make_index_sequence<C>{});
			}
		}

		// binary

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2, typename BinOp>
		constexpr auto apply_unitype_make(const vector_base<W1, T1, C, D1> &lhs,
										  const vector_base<W2, T2, C, D2> &rhs,
										  BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<T1, T2>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(lhs[0]), static_cast<ArgT>(rhs[0]));
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<binop_return_t<BinOp, ArgT, ArgT>, C>{ op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs[Is]))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, typename BinOp>
		constexpr auto apply_unitype_make(const vector_base<W, T, C, D> &lhs,
										  U rhs,
										  BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<T, U>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(lhs[0]), static_cast<ArgT>(rhs));
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<binop_return_t<BinOp, ArgT, ArgT>, C>{ op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, typename BinOp>
		constexpr auto apply_unitype_make(U lhs,
										  const vector_base<W, T, C, D> &rhs,
										  BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<T, U>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(lhs), static_cast<ArgT>(rhs[0]));
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<binop_return_t<BinOp, ArgT, ArgT>, C>{ op(static_cast<ArgT>(lhs), static_cast<ArgT>(rhs[Is]))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2, typename BinOp>
		constexpr auto apply_multitype_make(const vector_base<W1, T1, C, D1> &lhs,
											const vector_base<W2, T2, C, D2> &rhs,
											BinOp &op) noexcept
		{
			if constexpr (C == 1)
			{
				return op(lhs[0], rhs[0]);
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<binop_return_t<BinOp, T1, T2>, C>{ op(lhs[Is], rhs[Is])... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, typename BinOp>
		constexpr auto apply_multitype_make(const vector_base<W, T, C, D> &lhs,
											U rhs,
											BinOp &op) noexcept
		{
			if constexpr (C == 1)
			{
				return op(lhs[0], rhs);
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<binop_return_t<BinOp, T, U>, C>{ op(lhs[Is], rhs)... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, typename BinOp>
		constexpr auto apply_multitype_make(U lhs,
											const vector_base<W, T, C, D> &rhs,
											BinOp &op) noexcept
		{
			if constexpr (C == 1)
			{
				return op(lhs, rhs[0]);
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<binop_return_t<BinOp, U, T>, C>{ op(lhs, rhs[Is])... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2, typename BinOp>
		requires W1
		constexpr void apply_unitype_modify(vector_base<W1, T1, C, D1> &lhs,
											const vector_base<W2, T2, C, D2> &rhs,
											BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<T1, T2>;
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs[Is]))... );
			}(std::make_index_sequence<C>{});
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, typename BinOp>
		requires W
		constexpr void apply_unitype_modify(vector_base<W, T, C, D> &lhs,
											U rhs,
											BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<T, U>;
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs))... );
			}(std::make_index_sequence<C>{});
		}

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2, typename BinOp>
		requires W1
		constexpr void apply_multitype_modify(vector_base<W1, T1, C, D1> &lhs,
											  const vector_base<W2, T2, C, D2> &rhs,
											  BinOp &op) noexcept
		{
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(lhs[Is], rhs[Is])... );
			}(std::make_index_sequence<C>{});
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, typename BinOp>
		requires W
		constexpr void apply_multitype_modify(vector_base<W, T, C, D> &lhs,
											  U rhs,
											  BinOp &op) noexcept
		{
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(lhs[Is], rhs)... );
			}(std::make_index_sequence<C>{});
		}

		// ternary

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dimensional_scalar T2, typename D2, bool W3, dimensional_scalar T3, typename D3, typename TernOp>
		constexpr auto apply_unitype_make(const vector_base<W1, T1, C, D1> &x,
										  const vector_base<W2, T2, C, D2> &y,
										  const vector_base<W3, T3, C, D3> &z,
										  TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<T1, T2, T3>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(x[0]), static_cast<ArgT>(y[0]), static_cast<ArgT>(z[0]));
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<ternop_return_t<TernOp, ArgT, ArgT, ArgT>, C>{ op(static_cast<ArgT>(x[Is]), static_cast<ArgT>(y[Is]), static_cast<ArgT>(z[Is]))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2, dimensional_scalar U, typename TernOp>
		constexpr auto apply_unitype_make(const vector_base<W1, T1, C, D1> &x,
										  const vector_base<W2, T2, C, D2> &y,
										  U z,
										  TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<T1, T2, U>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(x[0]), static_cast<ArgT>(y[0]), static_cast<ArgT>(z));
			}
			else
			{
				return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<ternop_return_t<TernOp, ArgT, ArgT, ArgT>, C>{ op(static_cast<ArgT>(x[Is]), static_cast<ArgT>(y[Is]), static_cast<ArgT>(z))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, dimensional_scalar V, typename TernOp>
		constexpr auto apply_unitype_make(const vector_base<W, T, C, D> &x,
										  U y,
										  V z,
										  TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<T, U, V>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(x[0]), static_cast<ArgT>(y), static_cast<ArgT>(z));
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<ternop_return_t<TernOp, ArgT, ArgT, ArgT>, C>{ op(static_cast<ArgT>(x[Is]), static_cast<ArgT>(y), static_cast<ArgT>(z))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U, dimensional_scalar V, typename TernOp>
		constexpr auto apply_unitype_make(U x,
										  V y,
										  const vector_base<W, T, C, D> &z,
										  TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<T, U, V>;

			if constexpr (C == 1)
			{
				return op(static_cast<ArgT>(x), static_cast<ArgT>(y), static_cast<ArgT>(z[0]));
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<ternop_return_t<TernOp, ArgT, ArgT, ArgT>, C>{ op(static_cast<ArgT>(x), static_cast<ArgT>(y), static_cast<ArgT>(z[Is]))... };
				}(std::make_index_sequence<C>{});
			}
		}

		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dimensional_scalar T2, typename D2,
			bool W3, dimensional_scalar T3, typename D3,
			typename TernOp>
		constexpr auto apply_multitype_make(const vector_base<W1, T1, C, D1> &x,
											const vector_base<W2, T2, C, D2> &y,
											const vector_base<W3, T3, C, D3> &z,
											TernOp &op) noexcept
		{
			if constexpr (C == 1)
			{
				return op(x[0], y[0], z[0]);
			}
			else
			{
				return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					return basic_vector<ternop_return_t<TernOp, T1, T2, T3>, C>{ op(x[Is], y[Is], z[Is])... };
				}(std::make_index_sequence<C>{});
			}
		}
	}

	//
	// operators
	//

	// binary operators +=, +

	constexpr inline auto plus_op = [](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept { return lhs + rhs; };

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator +=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator +=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], plus_op);
		return lhs.as_derived();
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator +=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator +(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, plus_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, plus_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], plus_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator +(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, plus_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator +(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, plus_op);
	}

	// binary operators -=, -

	constexpr inline auto minus_op = [](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept { return lhs - rhs; };

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator -=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator -=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], minus_op);
		return lhs.as_derived();
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator -=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator -(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, minus_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, minus_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], minus_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator -(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, minus_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator -(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, minus_op);
	}

	// binary operators *=, *

	constexpr inline auto times_op = [](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept { return lhs * rhs; };

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator *=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator *=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], times_op);
		return lhs.as_derived();
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator *=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator *(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, times_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, times_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], times_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator *(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, times_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator *(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, times_op);
	}

	// binary operators /=, /

	constexpr inline auto div_op = [](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept { return lhs / rhs; };

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator /=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator /=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], div_op);
		return lhs.as_derived();
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator /=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator /(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, div_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, div_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], div_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator /(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, div_op);
	}

	template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator /(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, div_op);
	}

	// binary operators %=, % -- uses c++ modulus operator rules

	constexpr inline auto modulus_op = [](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept
	{ dsga_constexpr_assert(rhs != 0, "(lhs % 0) is undefined"); return lhs % rhs; };

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, modulus_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], modulus_op);
		return lhs.as_derived();
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator %=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, modulus_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator %(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, modulus_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, modulus_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], modulus_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator %(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, modulus_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator %(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, modulus_op);
	}

	// unary operator ~

	constexpr inline auto bit_not_op = [](numeric_integral_scalar auto arg) noexcept { return ~arg; };

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D>
	[[nodiscard]] constexpr auto operator ~(const vector_base<W, T, C, D> &arg) noexcept
	{
		return machinery::apply_make(arg, bit_not_op);
	}

	// binary operators <<=, <<

	constexpr inline auto lshift_op =
		[]<numeric_integral_scalar T1, numeric_integral_scalar T2>(T1 lhs, T2 rhs) noexcept -> T1 { return static_cast<T1>(lhs << rhs); };

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator <<=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator <<=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs[0], lshift_op);
		return lhs.as_derived();
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator <<=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator <<(const vector_base<W1, T1, C1, D1> &lhs,
											 const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_multitype_make(lhs, rhs, lshift_op);
		else if constexpr (C1 == 1)
			return machinery::apply_multitype_make(lhs[0], rhs, lshift_op);
		else if constexpr (C2 == 1)
			return machinery::apply_multitype_make(lhs, rhs[0], lshift_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator <<(const vector_base<W, T, C, D> &lhs,
											 U rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, lshift_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator <<(U lhs,
											 const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, lshift_op);
	}

	// binary operators >>=, >>

	constexpr inline auto rshift_op =
		[]<numeric_integral_scalar T1, numeric_integral_scalar T2>(T1 lhs, T2 rhs) noexcept -> T1 { return static_cast<T1>(lhs >> rhs); };

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs[0], rshift_op);
		return lhs.as_derived();
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator >>=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
	[[nodiscard]] constexpr auto operator >>(const vector_base<W1, T1, C1, D1> &lhs,
											 const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_multitype_make(lhs, rhs, rshift_op);
		else if constexpr (C1 == 1)
			return machinery::apply_multitype_make(lhs[0], rhs, rshift_op);
		else if constexpr (C2 == 1)
			return machinery::apply_multitype_make(lhs, rhs[0], rshift_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator >>(const vector_base<W, T, C, D> &lhs,
											 U rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, rshift_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr auto operator >>(U lhs,
											 const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, rshift_op);
	}

	// binary operators &=, &

	constexpr inline auto and_op = [](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept { return lhs & rhs; };

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && detail::same_sizeof<T1, T2>
	constexpr auto &operator &=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1) && detail::same_sizeof<T1, T2>
	constexpr auto &operator &=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], and_op);
		return lhs.as_derived();
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires W && implicitly_convertible_to<U, T> && detail::same_sizeof<T, U>
	constexpr auto &operator &=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1) && detail::same_sizeof<T1, T2>
	[[nodiscard]] constexpr auto operator &(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, and_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, and_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], and_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
	[[nodiscard]] constexpr auto operator &(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, and_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
	[[nodiscard]] constexpr auto operator &(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, and_op);
	}

	// binary operators |=, |

	constexpr inline auto or_op = [](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept { return lhs | rhs; };

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && detail::same_sizeof<T1, T2>
	constexpr auto &operator |=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1) && detail::same_sizeof<T1, T2>
	constexpr auto &operator |=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], or_op);
		return lhs.as_derived();
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires W && implicitly_convertible_to<U, T> && detail::same_sizeof<T, U>
	constexpr auto &operator |=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1) && detail::same_sizeof<T1, T2>
	[[nodiscard]] constexpr auto operator |(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, or_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, or_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], or_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
	[[nodiscard]] constexpr auto operator |(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, or_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
	[[nodiscard]] constexpr auto operator |(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, or_op);
	}

	// binary operators ^=, ^

	constexpr inline auto xor_op = [](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept { return lhs ^ rhs; };

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && detail::same_sizeof<T1, T2>
	constexpr auto &operator ^=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1) && detail::same_sizeof<T1, T2>
	constexpr auto &operator ^=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1, D2> &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], xor_op);
		return lhs.as_derived();
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires W && implicitly_convertible_to<U, T> && detail::same_sizeof<T, U>
	constexpr auto &operator ^=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1) && detail::same_sizeof<T1, T2>
	[[nodiscard]] constexpr auto operator ^(const vector_base<W1, T1, C1, D1> &lhs,
											const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return machinery::apply_unitype_make(lhs, rhs, xor_op);
		else if constexpr (C1 == 1)
			return machinery::apply_unitype_make(lhs[0], rhs, xor_op);
		else if constexpr (C2 == 1)
			return machinery::apply_unitype_make(lhs, rhs[0], xor_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
	[[nodiscard]] constexpr auto operator ^(const vector_base<W, T, C, D> &lhs,
											U rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, xor_op);
	}

	template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
	requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
	[[nodiscard]] constexpr auto operator ^(U lhs,
											const vector_base<W, T, C, D> &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, xor_op);
	}

	// unary operator +

	template <bool W, non_bool_scalar T, std::size_t C, typename D>
	[[nodiscard]] constexpr auto operator +(const vector_base<W, T, C, D> &arg) noexcept
	{
		if constexpr (C == 1)
		{
			return arg[0];
		}
		else
		{
			return basic_vector<T, C>(arg);					// no-op copy
		}
	}

	// unary operator -

	constexpr inline auto neg_op = [](non_bool_scalar auto arg) noexcept { return -arg; };

	template <bool W, non_bool_scalar T, std::size_t C, typename D>
	[[nodiscard]] constexpr auto operator -(const vector_base<W, T, C, D> &arg) noexcept
	{
		return machinery::apply_make(arg, neg_op);
	}

	// unary operators ++

	// pre-increment
	template <bool W, non_bool_scalar T, std::size_t C, typename D>
	requires W
	constexpr auto &operator ++(vector_base<W, T, C, D> &arg) noexcept
	{
		arg += T(1);
		return arg.as_derived();
	}

	// post-increment
	template <bool W, non_bool_scalar T, std::size_t C, typename D>
	requires W
	constexpr auto operator ++(vector_base<W, T, C, D> &arg, int) noexcept
	{
		basic_vector<T, C> value(arg);
		arg += T(1);
		return value;
	}

	// unary operators --

	// pre-decrement
	template <bool W, non_bool_scalar T, std::size_t C, typename D>
	requires W
	constexpr auto &operator --(vector_base<W, T, C, D> &arg) noexcept
	{
		arg -= T(1);
		return arg.as_derived();
	}

	// post-decrement
	template <bool W, non_bool_scalar T, std::size_t C, typename D>
	requires W
	constexpr auto operator --(vector_base<W, T, C, D> &arg, int) noexcept
	{
		basic_vector<T, C> value(arg);
		arg -= T(1);
		return value;
	}

	//
	// get<> part of tuple protocol -- needed for structured bindings
	//

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr auto & get(storage_wrapper<T, S> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr const auto & get(const storage_wrapper<T, S> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr auto && get(storage_wrapper<T, S> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr const auto && get(const storage_wrapper<T, S> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	//

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && (N >= 0) && (N < C)
	[[nodiscard]] constexpr auto & get(vector_base<W, T, C, D> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr const auto & get(const vector_base<W, T, C, D> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr auto && get(vector_base<W, T, C, D> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr const auto && get(const vector_base<W, T, C, D> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	//
	//
	// vector functions
	//
	//

	namespace functions
	{
		//
		// 8.7 - vector relational
		//
		// these are defined first as they don't depend on the other functions, and the other functions can depend on them.
		//

		constexpr inline auto less_op = []<non_bool_scalar T>(T x, T y) noexcept -> bool
		{
			return std::isless(x, y);
		};

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto lessThan(const vector_base<W1, T, C, D1> &x,
											  const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, less_op);
		}

		constexpr inline auto less_equal_op = []<non_bool_scalar T>(T x, T y) noexcept -> bool
		{
			return std::islessequal(x, y);
		};

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto lessThanEqual(const vector_base<W1, T, C, D1> &x,
												   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, less_equal_op);
		}

		constexpr inline auto greater_op = []<non_bool_scalar T>(T x, T y) noexcept -> bool
		{
			return std::isgreater(x, y);
		};

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto greaterThan(const vector_base<W1, T, C, D1> &x,
												 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, greater_op);
		}

		constexpr inline auto greater_equal_op = []<non_bool_scalar T>(T x, T y) noexcept -> bool
		{
			return std::isgreaterequal(x, y);
		};

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto greaterThanEqual(const vector_base<W1, T, C, D1> &x,
													  const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, greater_equal_op);
		}

		constexpr inline auto equal_op = []<non_bool_scalar T>(T x, T y) noexcept -> bool
		{
			return std::isunordered(x, y) ? false : x == y;
		};

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto equal(const vector_base<W1, T, C, D1> &x,
										   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, equal_op);
		}

		constexpr inline auto bool_equal_op = [](bool x, bool y) noexcept { return x == y; };

		template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto equal(const vector_base<W1, bool, C, D1> &x,
										   const vector_base<W2, bool, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, bool_equal_op);
		}

		constexpr inline auto not_equal_op = []<non_bool_scalar T>(T x, T y) noexcept -> bool
		{
			return std::isunordered(x, y) ? true : x != y;
		};

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto notEqual(const vector_base<W1, T, C, D1> &x,
											  const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, not_equal_op);
		}

		constexpr inline auto bool_not_equal_op = [](bool x, bool y) noexcept { return x != y; };

		template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto notEqual(const vector_base<W1, bool, C, D1> &x,
											  const vector_base<W2, bool, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, bool_not_equal_op);
		}

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr bool any(const vector_base<W, bool, C, D> &x) noexcept
		{
			return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (x[Is] || ...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr bool all(const vector_base<W, bool, C, D> &x) noexcept
		{
			return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (x[Is] && ...);
			}(std::make_index_sequence<C>{});
		}

		constexpr inline auto logical_not_op = [](bool x) noexcept { return !x; };

		// c++ does not allow a function named not()
		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto logicalNot(const vector_base<W, bool, C, D> &x) noexcept
		{
			return machinery::apply_make(x, logical_not_op);
		}

		// not in GLSL
		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr bool none(const vector_base<W, bool, C, D> &x) noexcept
		{
			return !any(x);
		}

		//
		// 8.1 - angle and trigonometry
		//

		template <floating_point_scalar T>
		inline constexpr T degrees_per_radian_v = std::numbers::inv_pi_v<T> * T(180);

		template <floating_point_scalar T>
		inline constexpr T radians_per_degree_v = std::numbers::pi_v<T> / T(180);

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto radians(const vector_base<W, T, C, D> &deg) noexcept
		{
			return deg * radians_per_degree_v<T>;
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto radians(T deg) noexcept
		{
			return deg * radians_per_degree_v<T>;
		}

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto degrees(const vector_base<W, T, C, D> &rad) noexcept
		{
			return rad * degrees_per_radian_v<T>;
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto degrees(T rad) noexcept
		{
			return rad * degrees_per_radian_v<T>;
		}

		constexpr inline auto sin_op = [](floating_point_scalar auto arg) noexcept { return std::sin(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto sin(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, sin_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto sin(T arg) noexcept
		{
			return sin_op(arg);
		}

		constexpr inline auto cos_op = [](floating_point_scalar auto arg) noexcept { return std::cos(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto cos(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, cos_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto cos(T arg) noexcept
		{
			return cos_op(arg);
		}

		constexpr inline auto tan_op = [](floating_point_scalar auto arg) noexcept { return std::tan(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto tan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, tan_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto tan(T arg) noexcept
		{
			return tan_op(arg);
		}

		constexpr inline auto asin_op = [](floating_point_scalar auto arg) noexcept { return std::asin(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto asin(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, asin_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto asin(T arg) noexcept
		{
			return asin_op(arg);
		}

		constexpr inline auto acos_op = [](floating_point_scalar auto arg) noexcept { return std::acos(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto acos(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, acos_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto acos(T arg) noexcept
		{
			return acos_op(arg);
		}

		constexpr inline auto atan_op = [](floating_point_scalar auto arg) noexcept { return std::atan(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto atan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, atan_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto atan(T arg) noexcept
		{
			return atan_op(arg);
		}

		constexpr inline auto atan2_op = []<floating_point_scalar U>(U arg_y, U arg_x) noexcept { return std::atan2(arg_y, arg_x); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1,
		bool W2, typename D2>
		[[nodiscard]] inline auto atan(const vector_base<W1, T, C, D1> &y,
									   const vector_base<W2, T, C, D2> &x) noexcept
		{
			return machinery::apply_unitype_make(y, x, atan2_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto atan(T y,
									   T x) noexcept
		{
			return atan2_op(y, x);
		}

		constexpr inline auto sinh_op = [](floating_point_scalar auto arg) noexcept { return std::sinh(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto sinh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, sinh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto sinh(T arg) noexcept
		{
			return sinh_op(arg);
		}

		constexpr inline auto cosh_op = [](floating_point_scalar auto arg) noexcept { return std::cosh(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto cosh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, cosh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto cosh(T arg) noexcept
		{
			return cosh_op(arg);
		}

		constexpr inline auto tanh_op = [](floating_point_scalar auto arg) noexcept { return std::tanh(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto tanh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, tanh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto tanh(T arg) noexcept
		{
			return tanh_op(arg);
		}

		constexpr inline auto asinh_op = [](floating_point_scalar auto arg) noexcept { return std::asinh(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto asinh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, asinh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto asinh(T arg) noexcept
		{
			return asinh_op(arg);
		}

		constexpr inline auto acosh_op = [](floating_point_scalar auto arg) noexcept { return std::acosh(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto acosh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, acosh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto acosh(T arg) noexcept
		{
			return acosh_op(arg);
		}

		constexpr inline auto atanh_op = [](floating_point_scalar auto arg) noexcept { return std::atanh(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto atanh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, atanh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto atanh(T arg) noexcept
		{
			return atanh_op(arg);
		}

		//
		// 8.2 - exponential
		//

		constexpr inline auto pow_op = []<floating_point_scalar U>(U base, U exp) noexcept { return std::pow(base, exp); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] inline auto pow(const vector_base<W1, T, C, D1> &base,
									  const vector_base<W2, T, C, D2> &exp) noexcept
		{
			return machinery::apply_unitype_make(base, exp, pow_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto pow(T base,
									  T exp) noexcept
		{
			return pow_op(base, exp);
		}

		constexpr inline auto exp_op = [](floating_point_scalar auto arg) noexcept { return std::exp(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto exp(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, exp_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto exp(T arg) noexcept
		{
			return exp_op(arg);
		}

		constexpr inline auto log_op = [](floating_point_scalar auto arg) noexcept { return std::log(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto log(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, log_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto log(T arg) noexcept
		{
			return log_op(arg);
		}

		constexpr inline auto exp2_op = [](floating_point_scalar auto arg) noexcept { return std::exp2(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto exp2(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, exp2_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto exp2(T arg) noexcept
		{
			return exp2_op(arg);
		}

		constexpr inline auto log2_op = [](floating_point_scalar auto arg) noexcept { return std::log2(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] inline auto log2(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, log2_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto log2(T arg) noexcept
		{
			return log2_op(arg);
		}

		constexpr inline auto sqrt_op = [](floating_point_scalar auto arg) noexcept { return cxcm::sqrt(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto sqrt(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, sqrt_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto sqrt(T arg) noexcept
		{
			return sqrt_op(arg);
		}

		constexpr inline auto fast_rsqrt_op = [](floating_point_scalar auto arg) noexcept { return cxcm::fast_rsqrt(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto fast_inversesqrt(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, fast_rsqrt_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto fast_inversesqrt(T arg) noexcept
		{
			return fast_rsqrt_op(arg);
		}

		constexpr inline auto rsqrt_op = [](floating_point_scalar auto arg) noexcept { return cxcm::rsqrt(arg); };

		// double specializations

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto inversesqrt(const vector_base<W, double, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, rsqrt_op);
		}

		[[nodiscard]] constexpr auto inversesqrt(double arg) noexcept
		{
			return rsqrt_op(arg);
		}

		// float specializations - cxcm::rsqrt(float) is 100% match with cxcm::fast_rsqrt(float)

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto inversesqrt(const vector_base<W, float, C, D> &arg) noexcept
		{
			return fast_inversesqrt(arg);
		}

		[[nodiscard]] constexpr auto inversesqrt(float arg) noexcept
		{
			return fast_inversesqrt(arg);
		}

		//
		// 8.3 - common
		//

		constexpr inline auto abs_op = []<dimensional_scalar T>(T arg) noexcept { return cxcm::abs(arg); };

		template <bool W, non_bool_scalar T, std::size_t C, typename D>
		requires (!unsigned_scalar<T>)
		[[nodiscard]] constexpr auto abs(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, abs_op);
		}

		template <non_bool_scalar T>
		requires (!unsigned_scalar<T>)
		[[nodiscard]] constexpr auto abs(T arg) noexcept
		{
			return abs_op(arg);
		}

		constexpr inline auto sign_op = []<dimensional_scalar T>(T arg) noexcept -> T { return T(T(T(0) < arg) - T(arg < T(0))); };

		template <bool W, non_bool_scalar T, std::size_t C, typename D>
		requires (!unsigned_scalar<T>)
		[[nodiscard]] constexpr auto sign(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, sign_op);
		}

		template <non_bool_scalar T>
		requires (!unsigned_scalar<T>)
		[[nodiscard]] constexpr auto sign(T arg) noexcept
		{
			return sign_op(arg);
		}

		constexpr inline auto floor_op = [](floating_point_scalar auto arg) noexcept { return cxcm::floor(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto floor(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, floor_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto floor(T arg) noexcept
		{
			return floor_op(arg);
		}

		constexpr inline auto trunc_op = [](floating_point_scalar auto arg) noexcept { return cxcm::trunc(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto trunc(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, trunc_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto trunc(T arg) noexcept
		{
			return trunc_op(arg);
		}

		constexpr inline auto round_op = [](floating_point_scalar auto arg) noexcept { return cxcm::round(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto round(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, round_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto round(T arg) noexcept
		{
			return round_op(arg);
		}

		constexpr inline auto round_even_op = [](floating_point_scalar auto arg) noexcept { return cxcm::round_even(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto roundEven(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, round_even_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto roundEven(T arg) noexcept
		{
			return round_even_op(arg);
		}

		constexpr inline auto ceil_op = [](floating_point_scalar auto arg) noexcept { return cxcm::ceil(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto ceil(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, ceil_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto ceil(T arg) noexcept
		{
			return ceil_op(arg);
		}

		constexpr inline auto fract_op = [](floating_point_scalar auto arg) noexcept { return cxcm::fract(arg); };

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto fract(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, fract_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto fract(T arg) noexcept
		{
			return fract_op(arg);
		}

		constexpr inline auto mod_op = []<floating_point_scalar T>(T x, T y) noexcept { return x - y * cxcm::floor(x / y); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto mod(const vector_base<W1, T, C, D1> &x,
										 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, mod_op);
		}

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto mod(const vector_base<W, T, C, D> &x,
										 T y) noexcept
		{
			return machinery::apply_unitype_make(x, y, mod_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto mod(T x,
										 T y) noexcept
		{
			return mod_op(x, y);
		}

		constexpr inline auto modf_op = []<floating_point_scalar T>(T x, T y) noexcept { return x - y; };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires W2
		[[nodiscard]] constexpr auto modf(const vector_base<W1, T, C, D1> &arg,
										  vector_base<W2, T, C, D2> &i) noexcept
		{
			if constexpr (C == 1)
				i.as_derived() = basic_vector<T, 1>(trunc(arg));
			else
				i.as_derived() = trunc(arg);

			return machinery::apply_unitype_make(arg, i, modf_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto modf(T arg,
										  T &i) noexcept
		{
			i = trunc(arg);
			return modf_op(arg, i);
		}

		constexpr inline auto min_op = []<non_bool_scalar T>(T x, T y) noexcept { return y < x ? y : x; };

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto min(const vector_base<W1, T, C, D1> &x,
										 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, min_op);
		}

		template <bool W, non_bool_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto min(const vector_base<W, T, C, D> &x,
										 T y) noexcept
		{
			return machinery::apply_unitype_make(x, y, min_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr auto min(T x,
										 T y) noexcept
		{
			return min_op(x, y);
		}

		constexpr inline auto max_op = []<non_bool_scalar T>(T x, T y) noexcept { return y < x ? x : y; };

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto max(const vector_base<W1, T, C, D1> &x,
										 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, max_op);
		}

		template <bool W, non_bool_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto max(const vector_base<W, T, C, D> &x,
										 T y) noexcept
		{
			return machinery::apply_unitype_make(x, y, max_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr auto max(T x,
										 T y) noexcept
		{
			return max_op(x, y);
		}

		constexpr inline auto clamp_op = []<non_bool_scalar T>(T x, T min_val, T max_val) noexcept -> T { return std::clamp(x, min_val, max_val); };

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		[[nodiscard]] constexpr auto clamp(const vector_base<W1, T, C, D1> &x,
										   const vector_base<W2, T, C, D2> &min_val,
										   const vector_base<W3, T, C, D3> &max_val) noexcept
		{
			dsga_constexpr_assert(all(lessThanEqual(min_val, max_val)), "(max_val < min_val) is UB");
			return machinery::apply_unitype_make(x, min_val, max_val, clamp_op);
		}

		template <bool W, non_bool_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto clamp(const vector_base<W, T, C, D> &x,
										   T min_val,
										   T max_val) noexcept
		{
			dsga_constexpr_assert(min_val <= max_val, "(max_val < min_val) is UB");
			return machinery::apply_unitype_make(x, min_val, max_val, clamp_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr auto clamp(T x,
										   T min_val,
										   T max_val) noexcept
		{
			dsga_constexpr_assert(min_val <= max_val, "(max_val < min_val) is UB");
			return clamp_op(x, min_val, max_val);
		}

		constexpr inline auto mix1_op = []<floating_point_scalar T>(T x, T y, T a) noexcept { return std::lerp(x, y, a); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		[[nodiscard]] constexpr auto mix(const vector_base<W1, T, C, D1> &x,
										 const vector_base<W2, T, C, D2> &y,
										 const vector_base<W3, T, C, D3> &a) noexcept
		{
			return machinery::apply_unitype_make(x, y, a, mix1_op);
		}

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto mix(const vector_base<W1, T, C, D1> &x,
										 const vector_base<W2, T, C, D2> &y,
										 T a) noexcept
		{
			return machinery::apply_unitype_make(x, y, a, mix1_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto mix(T x,
										 T y,
										 T a) noexcept
		{
			return mix1_op(x, y, a);
		}

		constexpr inline auto mix2_op = []<dimensional_scalar T, bool_scalar B>(T x, T y, B a) noexcept -> T { return a ? y : x; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, bool_scalar B, typename D3>
		[[nodiscard]] constexpr auto mix(const vector_base<W1, T, C, D1> &x,
										 const vector_base<W2, T, C, D2> &y,
										 const vector_base<W3, B, C, D3> &a) noexcept
		{
			return machinery::apply_multitype_make(x, y, a, mix2_op);
		}

		template <dimensional_scalar T, bool_scalar B>
		[[nodiscard]] constexpr auto mix(T x,
										 T y,
										 B a) noexcept
		{
			return mix2_op(x, y, a);
		}

		constexpr inline auto step_op = []<floating_point_scalar T>(T edge, T x) noexcept { return ((x < edge) ? T(0) : T(1)); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] constexpr auto step(const vector_base<W1, T, C, D1> &edge,
										  const vector_base<W2, T, C, D2> &x) noexcept
		{
			return machinery::apply_unitype_make(edge, x, step_op);
		}

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto step(T edge,
										  const vector_base<W, T, C, D> &x) noexcept
		{
			return machinery::apply_unitype_make(edge, x, step_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto step(T edge,
										  T x) noexcept
		{
			return step_op(edge, x);
		}

		constexpr inline auto smoothstep_op = []<floating_point_scalar T>(T edge0, T edge1, T x) noexcept
		{
			T t = clamp_op((x - edge0) / (edge1 - edge0), T(0), T(1));
			return t * t * (T(3) - T(2) * t);
		};

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		[[nodiscard]] constexpr auto smoothstep(const vector_base<W1, T, C, D1> &edge0,
												const vector_base<W2, T, C, D2> &edge1,
												const vector_base<W3, T, C, D3> &x) noexcept
		{
			return machinery::apply_unitype_make(edge0, edge1, x, smoothstep_op);
		}

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto smoothstep(T edge0,
												T edge1,
												const vector_base<W, T, C, D> &x) noexcept
		{
			return machinery::apply_unitype_make(edge0, edge1, x, smoothstep_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto smoothstep(T edge0,
												T edge1,
												T x) noexcept
		{
			return smoothstep_op(edge0, edge1, x);
		}

		constexpr inline auto isnan_op = [](floating_point_scalar auto arg) noexcept { return cxcm::isnan(arg); };

		// MSVC has a problem when I try to implement this with vector_base -- don't know about gcc or clang

		template <floating_point_scalar T, std::size_t C>
		[[nodiscard]] constexpr auto isnan(const basic_vector<T, C> &arg) noexcept
		{
			return machinery::apply_make(arg, isnan_op);
		}

		template <floating_point_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		[[nodiscard]] constexpr auto isnan(const indexed_vector<T, S, C, Is...> &arg) noexcept
		{
			return machinery::apply_make(arg, isnan_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto isnan(T arg) noexcept
		{
			return isnan_op(arg);
		}

		constexpr inline auto isinf_op = [](floating_point_scalar auto arg) noexcept { return cxcm::isinf(arg); };

		// MSVC has a problem when I try to implement this with vector_base -- don't know about gcc or clang

		template <floating_point_scalar T, std::size_t C>
		[[nodiscard]] constexpr auto isinf(const basic_vector<T, C> &arg) noexcept
		{
			return machinery::apply_make(arg, isinf_op);
		}

		template <floating_point_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		[[nodiscard]] constexpr auto isinf(const indexed_vector<T, S, C, Is...> &arg) noexcept
		{
			return machinery::apply_make(arg, isinf_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto isinf(T arg) noexcept
		{
			return isinf_op(arg);
		}

		//
		// guarantee the byte sizes of these primitives
		//

		static_assert(sizeof(float) == sizeof(int), "float and int must be same byte size");
		static_assert(sizeof(float) == sizeof(unsigned int), "float and unsigned int must be same byte size");
		static_assert(sizeof(double) == sizeof(long long), "double and long long must be same byte size");
		static_assert(sizeof(double) == sizeof(unsigned long long), "double and unsigned long long must be same byte size");

		constexpr inline auto float_bits_to_int_op = [](float arg) noexcept { return std::bit_cast<int>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto floatBitsToInt(const vector_base<W, float, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, float_bits_to_int_op);
		}

		[[nodiscard]] constexpr int floatBitsToInt(float arg) noexcept
		{
			return float_bits_to_int_op(arg);
		}

		constexpr inline auto float_bits_to_uint_op = [](float arg) noexcept { return std::bit_cast<unsigned int>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto floatBitsToUint(const vector_base<W, float, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, float_bits_to_uint_op);
		}

		[[nodiscard]] constexpr unsigned int floatBitsToUint(float arg) noexcept
		{
			return float_bits_to_uint_op(arg);
		}

		constexpr inline auto double_bits_to_long_long_op = [](double arg) noexcept { return std::bit_cast<long long>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto doubleBitsToLongLong(const vector_base<W, double, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, double_bits_to_long_long_op);
		}

		[[nodiscard]] constexpr long long doubleBitsToLongLong(double arg) noexcept
		{
			return double_bits_to_long_long_op(arg);
		}

		constexpr inline auto double_bits_to_ulong_long_op = [](double arg) noexcept { return std::bit_cast<unsigned long long>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto doubleBitsToUlongLong(const vector_base<W, double, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, double_bits_to_ulong_long_op);
		}

		[[nodiscard]] constexpr unsigned long long doubleBitsToUlongLong(double arg) noexcept
		{
			return double_bits_to_ulong_long_op(arg);
		}

		constexpr inline auto int_bits_to_float_op = [](int arg) noexcept { return std::bit_cast<float>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto intBitsToFloat(const vector_base<W, int, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, int_bits_to_float_op);
		}

		[[nodiscard]] constexpr float intBitsToFloat(int arg) noexcept
		{
			return int_bits_to_float_op(arg);
		}

		constexpr inline auto uint_bits_to_float_op = [](unsigned int arg) noexcept { return std::bit_cast<float>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto uintBitsToFloat(const vector_base<W, unsigned int, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, uint_bits_to_float_op);
		}

		[[nodiscard]] constexpr float uintBitsToFloat(unsigned int arg) noexcept
		{
			return uint_bits_to_float_op(arg);
		}

		constexpr inline auto long_long_bits_to_double_op = [](long long arg) noexcept { return std::bit_cast<double>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto longLongBitsToDouble(const vector_base<W, long long, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, long_long_bits_to_double_op);
		}

		[[nodiscard]] constexpr double longLongBitsToDouble(long long arg) noexcept
		{
			return long_long_bits_to_double_op(arg);
		}

		constexpr inline auto ulong_long_bits_to_double_op = [](unsigned long long arg) noexcept { return std::bit_cast<double>(arg); };

		template <bool W, std::size_t C, typename D>
		[[nodiscard]] constexpr auto ulongLongBitsToDouble(const vector_base<W, unsigned long long, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, ulong_long_bits_to_double_op);
		}

		[[nodiscard]] constexpr double ulongLongBitsToDouble(unsigned long long arg) noexcept
		{
			return ulong_long_bits_to_double_op(arg);
		}

		inline auto fma_op = []<floating_point_scalar T>(T a, T b, T c) noexcept { return std::fma(a, b, c); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		[[nodiscard]] inline auto fma(const vector_base<W1, T, C, D1> &a,
									  const vector_base<W2, T, C, D2> &b,
									  const vector_base<W3, T, C, D3> &c) noexcept
		{
			return machinery::apply_unitype_make(a, b, c, fma_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto fma(T a,
									  T b,
									  T c) noexcept
		{
			return fma_op(a, b, c);
		}

		inline auto frexp_op = []<floating_point_scalar T>(T x, int &exp) noexcept { return std::frexp(x, &exp); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires W2
		[[nodiscard]] inline auto frexp(const vector_base<W1, T, C, D1> &x,
										vector_base<W2, int, C, D2> &exp) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return basic_vector<T, C>(frexp_op(x[Is], exp[Is])...);
			}(std::make_index_sequence<C>{});
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto frexp(T x,
										int &exp) noexcept
		{
			return frexp_op(x, exp);
		}

		inline auto ldexp_op = []<floating_point_scalar T>(T x, int exp) noexcept { return std::ldexp(x, exp); };

		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		[[nodiscard]] inline auto ldexp(const vector_base<W1, T, C, D1> &x,
										const vector_base<W2, int, C, D2> &exp) noexcept
		{
			return machinery::apply_multitype_make(x, exp, ldexp_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline auto ldexp(T x,
										int exp) noexcept
		{
			return ldexp_op(x, exp);
		}

		//
		// byteswap() for non-boolean typed vectors
		//

		// since dsga is designed for c++20, we can't use std::byteswap() from c++23
		constexpr auto byteswap_op = []<numeric_integral_scalar T>(T x) noexcept -> T
		{
			auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(x);
			std::ranges::reverse(value_representation);
			return std::bit_cast<T>(value_representation);
		};

		template <bool W, numeric_integral_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto byteswap(const vector_base<W, T, C, D> &arg) noexcept
		{
			return machinery::apply_make(arg, byteswap_op);
		}

		template <numeric_integral_scalar T>
		[[nodiscard]] constexpr auto byteswap(T arg) noexcept
		{
			return byteswap_op(arg);
		}

		//
		// to_underlying() for underlying enum value - not really a good fit for dsga, but a helpful function anyway
		//

		// since dsga is designed for c++20, we can't use std::to_underlying() from c++23
		template <typename E>
		requires std::is_enum_v<E>
		[[nodiscard]] constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
		{
			return static_cast<std::underlying_type_t<E>>(e);
		}

		//
		// 8.4 is omitted
		//

		// not in GLSL -- dot() is just for floating point, innerProduct() will work with everything but bool
		template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
		[[nodiscard]] constexpr auto innerProduct(const vector_base<W1, T1, C, D1> &x,
												  const vector_base<W2, T2, C, D2> &y) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return ((x[Is] * y[Is]) + ...);
			}(std::make_index_sequence<C>{});
		}

		//
		// 8.5 - geometric
		//

		template <bool W1, floating_point_scalar T1, std::size_t C, typename D1, bool W2, floating_point_scalar T2, typename D2>
		[[nodiscard]] constexpr auto dot(const vector_base<W1, T1, C, D1> &x,
										 const vector_base<W2, T2, C, D2> &y) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return ((x[Is] * y[Is]) + ...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W1, floating_point_scalar T1, typename D1, bool W2, floating_point_scalar T2, typename D2>
		[[nodiscard]] constexpr auto cross(const vector_base<W1, T1, 3, D1> &a,
										   const vector_base<W2, T2, 3, D2> &b) noexcept
		{
			return basic_vector((a[1] * b[2]) - (b[1] * a[2]),
								(a[2] * b[0]) - (b[2] * a[0]),
								(a[0] * b[1]) - (b[0] * a[1]));
		}

		template <floating_point_scalar T1, floating_point_scalar T2>
		[[nodiscard]] constexpr auto cross(const basic_vector<T1, 3> &a,
										   const basic_vector<T2, 3> &b) noexcept
		{
			return (a.yzx * b.zxy) - (a.zxy * b.yzx);
		}

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		[[nodiscard]] constexpr auto length(const vector_base<W, T, C, D> &x) noexcept
		{
			return cxcm::sqrt(dot(x, x));
		}

		template <bool W, floating_point_scalar T, typename D>
		[[nodiscard]] constexpr auto length(const vector_base<W, T, 1, D> &x) noexcept
		{
			return cxcm::abs(x[0]);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto length(const T &x) noexcept
		{
			return cxcm::abs(x);
		}

		template <bool W1, floating_point_scalar T1, std::size_t C, typename D1, bool W2, floating_point_scalar T2, typename D2>
		[[nodiscard]] constexpr auto distance(const vector_base<W1, T1, C, D1> &p0,
											  const vector_base<W2, T2, C, D2> &p1) noexcept
		{
			return length(p0 - p1);
		}

		template <floating_point_scalar T1, floating_point_scalar T2>
		[[nodiscard]] constexpr auto distance(T1 p0,
											  T2 p1) noexcept
		{
			return length(p0 - p1);
		}

		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		requires (C > 1)
		[[nodiscard]] constexpr auto normalize(const vector_base<W, T, C, D> &x) noexcept
		{
			auto len = length(x);
			if (T(0.0) == len)
				return basic_vector<T, C>(std::numeric_limits<T>::quiet_NaN());

			[[likely]] return x / len;
		}

		//
		// vec4 ftransform() omitted
		//
		
		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		requires (C > 1)
		[[nodiscard]] constexpr auto faceforward(const vector_base<W1, T, C, D1> &n,
												 const vector_base<W2, T, C, D2> &i,
												 const vector_base<W3, T, C, D3> &nref) noexcept
		{
			return (dot(nref, i) < T(0)) ? +n : -n;
		}

		// n must be normalized in order to achieve desired results
		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires (C > 1)
		[[nodiscard]] constexpr auto reflect(const vector_base<W1, T, C, D1> &i,
											 const vector_base<W2, T, C, D2> &n) noexcept
		{
			return i - T(2) * dot(n, i) * n;
		}

		// i and n must be normalized in order to achieve desired results
		template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires (C > 1)
		[[nodiscard]] constexpr auto refract(const vector_base<W1, T, C, D1> &i,
											 const vector_base<W2, T, C, D2> &n,
											 T eta) noexcept
		{
			T k = T(1) - eta * eta * (T(1) - dot(n, i) * dot(n, i));

			if (k < T(0))
				return basic_vector<T, C>(T(0));

			return eta * i - (eta * dot(n, i) + cxcm::sqrt(k)) * n;
		}

		//
		// 8.8 - 8.19 are omitted
		//

		//
		// runtime swizzle function -- if 1 < number of indexes <= 4, returns a stand-alone basic_vector as opposed to
		// an indexed_vector union data member. If number of indexes == 1, returns a scalar value for that indexed value.
		// return value is *not* bound to the lifetime of the input argument, unlike how v.xyz is a member of v.
		// It will throw if the index arguments are out of bounds (arg must be < C) or if number of index arguments are
		// not in range 1 <= num args <= 4.
		//
		// Not in GLSL -- inspired by the Odin Programming Language.
		//

		template <bool W, dimensional_scalar T, std::size_t C, typename D, typename Arg>
		requires std::convertible_to<Arg, std::size_t>
		inline auto swizzle(const vector_base<W, T, C, D> &v, const Arg &index)
		{
			bool index_valid = (static_cast<std::size_t>(index) < C);
			dsga_constexpr_assert(index_valid, "index out of range");

			if (!index_valid)
				throw std::out_of_range("swizzle() index out of range");

			return v[index];
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D, typename ...Args>
		requires (std::convertible_to<Args, std::size_t> && ...) && (sizeof...(Args) > 0) && (sizeof...(Args) <= 4)
		inline basic_vector<T, sizeof...(Args)> swizzle(const vector_base<W, T, C, D> &v, const Args &...Is)
		{
			bool indexes_valid = ((static_cast<std::size_t>(Is) < C) && ...);
			dsga_constexpr_assert(indexes_valid, "indexes out of range");

			if (!indexes_valid)
				throw std::out_of_range("swizzle() indexes out of range");

			return basic_vector<T, sizeof...(Args)>{ v[Is]... };
		}

		//
		// basic tolerance comparisons -- will assert if tolerance has any negative values.
		// these functions do not give component-wise results -- they return a single bool value about the situation.
		//

		template <non_bool_scalar T, non_bool_scalar U>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_tolerance(T x,
													  U tolerance) noexcept
		{
			dsga_constexpr_assert(tolerance >= U(0), "tolerance should not be negative");
			return cxcm::abs(x) <= static_cast<T>(tolerance);
		}

		template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_tolerance(const vector_base<W, T, C, D> &x,
													  U tolerance) noexcept
		{
			if constexpr (C == 1)
			{
				return within_tolerance(x[0], tolerance);
			}
			else
			{
				dsga_constexpr_assert(tolerance >= U(0), "tolerance should not be negative");
				return all(lessThanEqual(abs(x), basic_vector<T, C>(static_cast<T>(tolerance))));
			}
		}

		template <bool W1, non_bool_scalar T, std::size_t C1, typename D1, bool W2, non_bool_scalar U, std::size_t C2, typename D2>
		requires ((C1 == C2) || (C2 == 1)) && implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_tolerance(const vector_base<W1, T, C1, D1> &x,
													  const vector_base<W2, U, C2, D2> &tolerance) noexcept
		{
			if constexpr (C1 == C2)
			{
				if constexpr (C1 == 1 && C2 == 1)
				{
					return within_tolerance(x[0], tolerance[0]);
				}
				else
				{
					dsga_constexpr_assert(all(greaterThanEqual(tolerance, basic_vector<U, C2>(0))), "tolerance should not be negative");
					if constexpr (std::same_as<T, U>)
					{
						return all(lessThanEqual(abs(x), tolerance));
					}
					else
					{
						return all(lessThanEqual(abs(x), static_cast<basic_vector<T, C2>>(tolerance)));
					}
				}
			}
			else		// (C2 == 1)
			{
				return within_tolerance(x, tolerance[0]);
			}
		}

		// Euclidean distance check - imagine x in the center of a region (offset number line, circle, sphere, hypersphere)
		// whose diameter is 2 * tolerance, return whether y is also in the region.
		// uses less than or equal comparison: if y is on region boundary, that counts as within.

		template <non_bool_scalar T, non_bool_scalar U>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_distance(T x,
													 T y,
													 U tolerance) noexcept
		{
			return within_tolerance(x - y, tolerance);
		}

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, non_bool_scalar U>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
													 const vector_base<W2, T, C, D2> &y,
													 U tolerance) noexcept
		{
			return within_tolerance(distance(x, y), tolerance);
		}

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, non_bool_scalar U, typename D3>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
													 const vector_base<W2, T, C, D2> &y,
													 const vector_base<W3, U, 1, D3> &tolerance) noexcept
		{
			return within_distance(x, y, tolerance[0]);
		}

		// tolerance-box component check - imagine x in the center of an orthogonal region (offset number line, rectangle, box, hyperbox)
		// whose side lengths are 2 * tolerance, return whether y is also in the orthogonal region.
		// uses less than or equal comparison: if y is on the region boundary, that counts as within

		template <non_bool_scalar T, non_bool_scalar U>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_box(T x,
												T y,
												U tolerance) noexcept
		{
			return within_tolerance(x - y, tolerance);
		}

		template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, non_bool_scalar U>
		requires implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_box(const vector_base<W1, T, C, D1> &x,
												const vector_base<W2, T, C, D2> &y,
												U tolerance) noexcept
		{
			return within_tolerance(x - y, tolerance);
		}

		template <bool W1, non_bool_scalar T, std::size_t C1, typename D1, bool W2, typename D2, bool W3, non_bool_scalar U, std::size_t C2, typename D3>
		requires ((C1 == C2) || (C2 == 1)) && implicitly_convertible_to<U, T>
		[[nodiscard]] constexpr bool within_box(const vector_base<W1, T, C1, D1> &x,
												const vector_base<W2, T, C1, D2> &y,
												const vector_base<W3, U, C2, D3> &tolerance) noexcept
		{
			if constexpr (C1 == C2)
			{
				return within_tolerance(x - y, tolerance);
			}
			else		// (C2 == 1)
			{
				return within_box(x, y, tolerance[0]);
			}
		}
	}

	//
	// component-wise equality operator for vectors, scalar boolean result: ==, != (thanks to c++20).
	// most vector equality/inequality testing should use free functions equal()/notEqual(), but
	// these have a scalar result and are useful for unit testing.
	//

	// implicitly_convertible_to does not work with bool, so need another function for that case if we want to support that (which we don't)
	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1>
	constexpr bool operator ==(const vector_base<W1, T1, C, D1> &first,
							   const vector_base<W2, T2, C, D2> &second) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return ((!std::isunordered(first[Is], second[Is]) && (first[Is] == static_cast<T1>(second[Is]))) && ...);
		}(std::make_index_sequence<C>{});
	}

	template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
	constexpr bool operator ==(const vector_base<W1, T, C, D1> &first,
							   const vector_base<W2, T, C, D2> &second) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return ((!std::isunordered(first[Is], second[Is]) && (first[Is] == second[Is])) && ...);
		}(std::make_index_sequence<C>{});
	}

	// when Count == 1, treat it like a scalar value for equality comparison
	template <bool W, dimensional_scalar T, typename D, dimensional_scalar U>
	requires std::convertible_to<U, T> || std::convertible_to<T, U>
	constexpr bool operator ==(const vector_base<W, T, 1, D> &first,
							   U second) noexcept
	{
		using commontype = std::common_type_t<T, U>;
		return (static_cast<commontype>(first[0]) == static_cast<commontype>(second));
	}

	//
	// basic_matrix
	//

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4)))
	struct basic_matrix
	{
		static constexpr std::size_t ComponentCount = C * R;

		// number of columns
		[[nodiscard]] constexpr int length() const noexcept						{ return C; }

		// number of rows
		[[nodiscard]] constexpr int column_length() const noexcept				{ return R; }

		// returns number of columns (row size), not number of elements
		// not required by spec, but more c++ container-like
		static constexpr std::integral_constant<std::size_t, C> size =			{};

		// returns number of rows
		// not required by spec, but more c++ container-like
		static constexpr std::integral_constant<std::size_t, R> column_size =	{};

		// data storage for matrix
		std::array<basic_vector<T, R>, C> columns;

		//
		// operator [] gets the column vector
		//

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr basic_vector<T, R> &operator [](const U &index) noexcept
		{
			dsga_constexpr_assert(index >= 0 && static_cast<std::size_t>(index) < C, "index out of bounds");
			return columns[static_cast<std::size_t>(index)];
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const basic_vector<T, R> &operator [](const U &index) const noexcept
		{
			dsga_constexpr_assert(index >= 0 && static_cast<std::size_t>(index) < C, "index out of bounds");
			return columns[static_cast<std::size_t>(index)];
		}

		// get a row of the matrix as a vector
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr basic_vector<T, C> row(const U &row_index) const noexcept
		{
			dsga_constexpr_assert(row_index >= 0 && static_cast<std::size_t>(row_index) < R, "row_index out of bounds");

			// for each column of the matrix, get a row component, and bundle
			// these components up into a vector that represents the row
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return basic_vector<T, C>{ columns[Is][row_index]... };
			}(std::make_index_sequence<C>{});
		}

		//
		// defaulted functions
		//

		constexpr basic_matrix() noexcept = default;
		constexpr ~basic_matrix() noexcept = default;

		constexpr basic_matrix(const basic_matrix &) noexcept = default;
		constexpr basic_matrix(basic_matrix &&) noexcept = default;
		constexpr basic_matrix &operator =(const basic_matrix &) & noexcept = default;
		constexpr basic_matrix &operator =(basic_matrix &&) & noexcept = default;

		//
		// constructors
		//

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_matrix_component<U, T>::value) && (detail::valid_matrix_component<Args, T>::value && ...) && detail::met_component_count<ComponentCount, U, Args...>
		explicit constexpr basic_matrix(const U &u, const Args & ...args) noexcept
			: columns{}
		{
			auto arg_tuple = detail::flatten_args_to_tuple(u, args...);
			[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				(([&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
				{
					constexpr std::size_t Col = Is;
					columns[Col].set( std::get<Col * R + Js>(arg_tuple)... );
				}(std::make_index_sequence<R>{})), ...);
			}(std::make_index_sequence<C>{});
		}

		// diagonal constructor for square matrices
		template <typename U>
		requires std::convertible_to<U, T> && (C == R)
		explicit constexpr basic_matrix(U arg) noexcept
			: columns{}
		{
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((columns[Is][Is] = static_cast<T>(arg)), ...);
			}(std::make_index_sequence<C>{});
		}

		// implicit constructor from a matrix - uses implicitly convertible vector assignment
		template <floating_point_scalar U>
		requires implicitly_convertible_to<U, T>
		explicit(false) constexpr basic_matrix(const basic_matrix<U, C, R> &arg) noexcept
			: columns{}
		{
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((columns[Is] = arg[Is]), ...);
			}(std::make_index_sequence<C>{});
		}

		// implicit constructor from a matrix
		template <floating_point_scalar U, std::size_t Cols, std::size_t Rows>
		requires implicitly_convertible_to<U, T> && (Cols != C || Rows != R)
		explicit(false) constexpr basic_matrix(const basic_matrix<U, Cols, Rows> &arg) noexcept
			: columns{}
		{
			[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				(([&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
				{
					constexpr std::size_t Col = Is;
					((columns[Col][Js] = static_cast<T>(arg[Col][Js])), ...);
				}(std::make_index_sequence<std::min(R, Rows)>{})), ...);
			}(std::make_index_sequence<std::min(C, Cols)>{});

			// for square matrix, extend identity diagonal as needed
			if constexpr (C == R)
			{
				[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					((columns[Is][Is] = T(1.0)), ...);
				}(make_index_range<std::min(std::min(Cols, C), std::min(Rows, R)), C>{});
			}
		}

		// explicit constructor from a matrix - doesn't matter if (C == Cols) or (R == Rows)
		template <floating_point_scalar U, std::size_t Cols, std::size_t Rows>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_matrix(const basic_matrix<U, Cols, Rows> &arg) noexcept
			: columns{}
		{
			[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				(([&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
				{
					constexpr std::size_t Col = Is;
					((columns[Col][Js] = static_cast<T>(arg[Col][Js])), ...);
				}(std::make_index_sequence<std::min(R, Rows)>{})), ...);
			}(std::make_index_sequence<std::min(C, Cols)>{});

			// for square matrix, extend identity diagonal as needed
			if constexpr (C == R)
			{
				[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					((columns[Is][Is] = T(1.0)), ...);
				}(make_index_range<std::min(std::min(Cols, C), std::min(Rows, R)), C>{});
			}
		}

		//
		// assignment operators
		//

		template <floating_point_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_matrix &operator =(const basic_matrix<U, C, R> &other) & noexcept
		{
			[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((columns[Is] = other[Is]), ...);			// let basic_vector do any type conversion if needed
			}(std::make_index_sequence<C>{});

			return *this;
		}

		// pointer interface
		[[nodiscard]] constexpr basic_vector<T, R> * data() noexcept				{ return columns.data(); }
		[[nodiscard]] constexpr const basic_vector<T, R> * data() const noexcept	{ return columns.data(); }

		constexpr void swap(basic_matrix &bm) noexcept			{ columns.swap(bm.columns); }

		// support for range-based for loop -- gives column vectors
		[[nodiscard]] constexpr auto begin() noexcept			{ return columns.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return columns.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return begin(); }
		[[nodiscard]] constexpr auto end() noexcept				{ return columns.end(); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return columns.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return end(); }

		[[nodiscard]] constexpr auto rbegin() noexcept			{ return columns.rbegin(); }
		[[nodiscard]] constexpr auto rbegin() const noexcept	{ return columns.crbegin(); }
		[[nodiscard]] constexpr auto crbegin() const noexcept	{ return rbegin(); }
		[[nodiscard]] constexpr auto rend() noexcept			{ return columns.rend(); }
		[[nodiscard]] constexpr auto rend() const noexcept		{ return columns.crend(); }
		[[nodiscard]] constexpr auto crend() const noexcept		{ return rend(); }
	};

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr void swap(basic_matrix<T, C, R> &lhs, basic_matrix<T, C, R> &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	//
	// get<> part of tuple protocol -- needed for structured bindings
	//

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr auto & get(basic_matrix<T, C, R> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr const auto & get(const basic_matrix<T, C, R> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr auto && get(basic_matrix<T, C, R> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr const auto && get(const basic_matrix<T, C, R> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	//
	// matrix functions
	//

	namespace functions
	{
		//
		// 8.6 is matrix functions
		//

		// component-wise matrix multiplication, since operator * is linear-algebraic for a matrix with a vector or other matrix
		template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
		[[nodiscard]] constexpr auto matrixCompMult(const basic_matrix<T, C, R> &lhs,
													const basic_matrix<U, C, R> &rhs) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] * rhs[Is])... };
			}(std::make_index_sequence<C>{});
		}

		// outerProduct() - matrix from a column vector times a row vector
		template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
		requires (floating_point_scalar<T1> || floating_point_scalar<T2>) && ((C1 >= 2) && (C1 <= 4)) && ((C2 >= 2) && (C2 <= 4))
		[[nodiscard]] constexpr auto outerProduct(const vector_base<W1, T1, C1, D1> &lhs,
												  const vector_base<W2, T2, C2, D2> &rhs) noexcept
		{
			return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
			{
				return basic_matrix<std::common_type_t<T1, T2>, C1, C2>{
					[&]<std::size_t ...Is>(std::index_sequence <Is...>, auto row) noexcept
					{
						return basic_vector<std::common_type_t<T1, T2>, C1>{ (lhs[Is] * row)... };
					}(std::make_index_sequence<C1>{}, rhs[Js]) ... };
			}(std::make_index_sequence<C2>{});
		}

		// transpose a matrix
		template <floating_point_scalar T, std::size_t C, std::size_t R>
		[[nodiscard]] constexpr basic_matrix<T, R, C> transpose(const basic_matrix<T, C, R> &arg) noexcept
		{
			auto val = basic_matrix<T, R, C>{};

			[&] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				(([&] <std::size_t ...Js>(std::index_sequence <Js...>, std::size_t row) noexcept
				{
					((val[row][Js] = arg[Js][row]), ...);
				}(std::make_index_sequence<C>{}, Is)), ...);
			}(std::make_index_sequence<R>{});

			return val;
		}

		// determinant() - only on square matrices

		// going for efficiency
		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto determinant(const basic_matrix<T, 2, 2> &arg) noexcept
		{
			return 
				+ arg[0][0] * arg[1][1]
				- arg[0][1] * arg[1][0]
				;
		}

		// going for efficiency
		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto determinant(const basic_matrix<T, 3, 3> &arg) noexcept
		{
			// same results as dot(arg[0], cross(arg[1], arg[2]))
			return
				+ arg[0][0] * arg[1][1] * arg[2][2]
				+ arg[1][0] * arg[2][1] * arg[0][2]
				+ arg[2][0] * arg[0][1] * arg[1][2]
				- arg[2][0] * arg[1][1] * arg[0][2]
				- arg[1][0] * arg[0][1] * arg[2][2]
				- arg[0][0] * arg[2][1] * arg[1][2]
				;
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto determinant(const basic_matrix<T, 4, 4> &arg) noexcept
		{
			return
				+ arg[0][0] * arg[1][1] * arg[2][2] * arg[3][3] + arg[0][0] * arg[2][1] * arg[3][2] * arg[1][3] + arg[0][0] * arg[3][1] * arg[1][2] * arg[2][3]
				- arg[0][0] * arg[3][1] * arg[2][2] * arg[1][3] - arg[0][0] * arg[2][1] * arg[1][2] * arg[3][3] - arg[0][0] * arg[1][1] * arg[3][2] * arg[2][3]
				- arg[1][0] * arg[0][1] * arg[2][2] * arg[3][3] - arg[2][0] * arg[0][1] * arg[3][2] * arg[1][3] - arg[3][0] * arg[0][1] * arg[1][2] * arg[2][3]
				+ arg[3][0] * arg[0][1] * arg[2][2] * arg[1][3] + arg[2][0] * arg[0][1] * arg[1][2] * arg[3][3] + arg[1][0] * arg[0][1] * arg[3][2] * arg[2][3]

				+ arg[1][0] * arg[2][1] * arg[0][2] * arg[3][3] + arg[2][0] * arg[3][1] * arg[0][2] * arg[1][3] + arg[3][0] * arg[1][1] * arg[0][2] * arg[2][3]
				- arg[3][0] * arg[2][1] * arg[0][2] * arg[1][3] - arg[2][0] * arg[1][1] * arg[0][2] * arg[3][3] - arg[1][0] * arg[3][1] * arg[0][2] * arg[2][3]
				- arg[1][0] * arg[2][1] * arg[3][2] * arg[0][3] - arg[2][0] * arg[3][1] * arg[1][2] * arg[0][3] - arg[3][0] * arg[1][1] * arg[2][2] * arg[0][3]
				+ arg[3][0] * arg[2][1] * arg[1][2] * arg[0][3] + arg[2][0] * arg[1][1] * arg[3][2] * arg[0][3] + arg[1][0] * arg[3][1] * arg[2][2] * arg[0][3]
				;
		}

		// inverse() - only on square matrices

		// going for efficiency
		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto inverse(const basic_matrix<T, 2, 2> &arg) noexcept
		{
			return basic_matrix<T, 2, 2>{ arg[1][1], -arg[0][1],
										 -arg[1][0],  arg[0][0] } / determinant(arg);
		}

		// going for efficiency
		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto inverse(const basic_matrix<T, 3, 3> &arg) noexcept
		{
			return basic_matrix<T, 3, 3>{
				+(arg[1][1] * arg[2][2] - arg[2][1] * arg[1][2]),
				-(arg[0][1] * arg[2][2] - arg[2][1] * arg[0][2]),
				+(arg[0][1] * arg[1][2] - arg[1][1] * arg[0][2]),
				-(arg[1][0] * arg[2][2] - arg[2][0] * arg[1][2]),
				+(arg[0][0] * arg[2][2] - arg[2][0] * arg[0][2]),
				-(arg[0][0] * arg[1][2] - arg[1][0] * arg[0][2]),
				+(arg[1][0] * arg[2][1] - arg[2][0] * arg[1][1]),
				-(arg[0][0] * arg[2][1] - arg[2][0] * arg[0][1]),
				+(arg[0][0] * arg[1][1] - arg[1][0] * arg[0][1])
				} / determinant(arg);
		}

		// going for efficiency
		template <floating_point_scalar T>
		[[nodiscard]] constexpr auto inverse(const basic_matrix<T, 4, 4> &arg) noexcept
		{
			return basic_matrix<T, 4, 4>{
				+ arg[1][1] * arg[2][2] * arg[3][3] + arg[2][1] * arg[3][2] * arg[1][3] + arg[3][1] * arg[1][2] * arg[2][3]
				- arg[3][1] * arg[2][2] * arg[1][3] - arg[2][1] * arg[1][2] * arg[3][3] - arg[1][1] * arg[3][2] * arg[2][3],

				- arg[0][1] * arg[2][2] * arg[3][3] - arg[2][1] * arg[3][2] * arg[0][3] - arg[3][1] * arg[0][2] * arg[2][3]
				+ arg[3][1] * arg[2][2] * arg[0][3] + arg[2][1] * arg[0][2] * arg[3][3] + arg[0][1] * arg[3][2] * arg[2][3],

				+ arg[0][1] * arg[1][2] * arg[3][3] + arg[1][1] * arg[3][2] * arg[0][3] + arg[3][1] * arg[0][2] * arg[1][3]
				- arg[3][1] * arg[1][2] * arg[0][3] - arg[1][1] * arg[0][2] * arg[3][3] - arg[0][1] * arg[3][2] * arg[1][3],

				- arg[0][1] * arg[1][2] * arg[2][3] - arg[1][1] * arg[2][2] * arg[0][3] - arg[2][1] * arg[0][2] * arg[1][3]
				+ arg[2][1] * arg[1][2] * arg[0][3] + arg[1][1] * arg[0][2] * arg[2][3] + arg[0][1] * arg[2][2] * arg[1][3],


				- arg[1][0] * arg[2][2] * arg[3][3] - arg[2][0] * arg[3][2] * arg[1][3] - arg[3][0] * arg[1][2] * arg[2][3]
				+ arg[3][0] * arg[2][2] * arg[1][3] + arg[2][0] * arg[1][2] * arg[3][3] + arg[1][0] * arg[3][2] * arg[2][3],

				+ arg[0][0] * arg[2][2] * arg[3][3] + arg[2][0] * arg[3][2] * arg[0][3] + arg[3][0] * arg[0][2] * arg[2][3]
				- arg[3][0] * arg[2][2] * arg[0][3] - arg[2][0] * arg[0][2] * arg[3][3] - arg[0][0] * arg[3][2] * arg[2][3],

				- arg[0][0] * arg[1][2] * arg[3][3] - arg[1][0] * arg[3][2] * arg[0][3] - arg[3][0] * arg[0][2] * arg[1][3]
				+ arg[3][0] * arg[1][2] * arg[0][3] + arg[1][0] * arg[0][2] * arg[3][3] + arg[0][0] * arg[3][2] * arg[1][3],

				+ arg[0][0] * arg[1][2] * arg[2][3] + arg[1][0] * arg[2][2] * arg[0][3] + arg[2][0] * arg[0][2] * arg[1][3]
				- arg[2][0] * arg[1][2] * arg[0][3] - arg[1][0] * arg[0][2] * arg[2][3] - arg[0][0] * arg[2][2] * arg[1][3],


				+ arg[1][0] * arg[2][1] * arg[3][3] + arg[2][0] * arg[3][1] * arg[1][3] + arg[3][0] * arg[1][1] * arg[2][3]
				- arg[3][0] * arg[2][1] * arg[1][3] - arg[2][0] * arg[1][1] * arg[3][3] - arg[1][0] * arg[3][1] * arg[2][3],

				- arg[0][0] * arg[2][1] * arg[3][3] - arg[2][0] * arg[3][1] * arg[0][3] - arg[3][0] * arg[0][1] * arg[2][3]
				+ arg[3][0] * arg[2][1] * arg[0][3] + arg[2][0] * arg[0][1] * arg[3][3] + arg[0][0] * arg[3][1] * arg[2][3],

				+ arg[0][0] * arg[1][1] * arg[3][3] + arg[1][0] * arg[3][1] * arg[0][3] + arg[3][0] * arg[0][1] * arg[1][3]
				- arg[3][0] * arg[1][1] * arg[0][3] - arg[1][0] * arg[0][1] * arg[3][3] - arg[0][0] * arg[3][1] * arg[1][3],

				- arg[0][0] * arg[1][1] * arg[2][3] - arg[1][0] * arg[2][1] * arg[0][3] - arg[2][0] * arg[0][1] * arg[1][3]
				+ arg[2][0] * arg[1][1] * arg[0][3] + arg[1][0] * arg[0][1] * arg[2][3] + arg[0][0] * arg[2][1] * arg[1][3],


				- arg[1][0] * arg[2][1] * arg[3][2] - arg[2][0] * arg[3][1] * arg[1][2] - arg[3][0] * arg[1][1] * arg[2][2]
				+ arg[3][0] * arg[2][1] * arg[1][2] + arg[2][0] * arg[1][1] * arg[3][2] + arg[1][0] * arg[3][1] * arg[2][2],

				+ arg[0][0] * arg[2][1] * arg[3][2] + arg[2][0] * arg[3][1] * arg[0][2] + arg[3][0] * arg[0][1] * arg[2][2]
				- arg[3][0] * arg[2][1] * arg[0][2] - arg[2][0] * arg[0][1] * arg[3][2] - arg[0][0] * arg[3][1] * arg[2][2],

				- arg[0][0] * arg[1][1] * arg[3][2] - arg[1][0] * arg[3][1] * arg[0][2] - arg[3][0] * arg[0][1] * arg[1][2]
				+ arg[3][0] * arg[1][1] * arg[0][2] + arg[1][0] * arg[0][1] * arg[3][2] + arg[0][0] * arg[3][1] * arg[1][2],

				+ arg[0][0] * arg[1][1] * arg[2][2] + arg[1][0] * arg[2][1] * arg[0][2] + arg[2][0] * arg[0][1] * arg[1][2]
				- arg[2][0] * arg[1][1] * arg[0][2] - arg[1][0] * arg[0][1] * arg[2][2] - arg[0][0] * arg[2][1] * arg[1][2]
				} / determinant(arg);
		}

		// not in glsl
		//
		// returns a skew symmetric matrix that can be used for computing the cross product. vector and matrix are 3D.
		//
		// cross(u, v) == cross_matrix(u) * v == u * cross_matrix(v)
		template <bool W, floating_point_scalar T, typename D>
		[[nodiscard]] constexpr basic_matrix<T, 3, 3> cross_matrix(const vector_base<W, T, 3, D> &vec) noexcept
		{
			return basic_matrix<T, 3, 3>{   T(0),  vec[2], -vec[1],
										 -vec[2],    T(0),  vec[0],
										  vec[1], -vec[0],    T(0) };
		}

		// not in glsl
		//
		// returns a symmetric diagonal matrix (square) using the vector parameter as the diagonal values,
		// with all other elements being 0.
		//
		template <bool W, floating_point_scalar T, std::size_t C, typename D>
		requires (C > 1)
		[[nodiscard]] constexpr basic_matrix<T, C, C> diagonal_matrix(const vector_base<W, T, C, D> &vec) noexcept
		{
			auto square_mat = basic_matrix<T, C, C>{};

			[&] <std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((square_mat[Is][Is] = vec[Is]), ...);
			}(std::make_index_sequence<C>{});

			return square_mat;
		}
	}

	//
	// matrix operators
	//

	// component-wise equality operator for matrices, scalar boolean result: ==, != (thanks to c++20)
	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	constexpr bool operator ==(const basic_matrix<T, C, R> &lhs,
							   const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return ((lhs[Is] == rhs[Is]) && ...);
		}(std::make_index_sequence<C>{});
	}

	// unary operators

	// unary +
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr auto operator +(const basic_matrix<T, C, R> &arg) noexcept
	{
		return basic_matrix<T, C, R>(arg);
	}

	// unary -
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr auto operator -(const basic_matrix<T, C, R> &arg) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<T, C, R>{ (-arg[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// pre-increment
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr auto &operator ++(basic_matrix<T, C, R> &arg) noexcept
	{
		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((++arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return arg;
	}

	// post-increment
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr auto operator ++(basic_matrix<T, C, R> &arg, int) noexcept
	{
		basic_matrix<T, C, R> value(arg);
		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((++arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return value;
	}

	// pre-decrement
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr auto &operator --(basic_matrix<T, C, R> &arg) noexcept
	{
		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((--arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return arg;
	}

	// post-decrement
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr auto operator --(basic_matrix<T, C, R> &arg, int) noexcept
	{
		basic_matrix<T, C, R> value(arg);
		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((--arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return value;
	}

	// operator + with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator +(const basic_matrix<T, C, R> &lhs,
											U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] + rhs)... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator +(U lhs,
											const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs + rhs[Is])...};
		}(std::make_index_sequence<C>{});
	}

	// operator - with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator -(const basic_matrix<T, C, R> &lhs,
											U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] - rhs)... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator -(U lhs,
											const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs - rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator * with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator *(const basic_matrix<T, C, R> &lhs,
											U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] * rhs)... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator *(U lhs,
											const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs * rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator / with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator /(const basic_matrix<T, C, R> &lhs,
											U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] / rhs)... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	[[nodiscard]] constexpr auto operator /(U lhs,
											const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs / rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator + with same size matrices

	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	[[nodiscard]] constexpr auto operator +(const basic_matrix<T, C, R> &lhs,
											const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] + rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator - with same size matrices

	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	[[nodiscard]] constexpr auto operator -(const basic_matrix<T, C, R> &lhs,
											const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] - rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator / with same size matrices

	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	[[nodiscard]] constexpr auto operator /(const basic_matrix<T, C, R> &lhs,
											const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>{ (lhs[Is] / rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	//
	// linear-algebriac binary ops
	//

	// matrix * (column) vector => (column) vector

	template <floating_point_scalar T, std::size_t C, std::size_t R, bool W, non_bool_scalar U, typename D>
	[[nodiscard]] constexpr auto operator *(const basic_matrix<T, C, R> &lhs,
											const vector_base<W, U, C, D> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return ((lhs[Is] * rhs[Is]) + ...);
		}(std::make_index_sequence<C>{});
	}

	// (row) vector * matrix => (row) vector

	template <floating_point_scalar T, std::size_t C, std::size_t R, bool W, non_bool_scalar U, typename D>
	[[nodiscard]] constexpr auto operator *(const vector_base<W, U, R, D> &lhs,
											const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_vector{ functions::dot(lhs, rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// matrix * matrix => matrix

	template <floating_point_scalar T, std::size_t C1, std::size_t R1, floating_point_scalar U, std::size_t C2, std::size_t R2>
	requires (C1 == R2)
	[[nodiscard]] constexpr auto operator *(const basic_matrix<T, C1, R1> &lhs,
											const basic_matrix<U, C2, R2> &rhs) noexcept
	{
		// functions::dot() is the core of a different implementation of this function,
		// and is representative of the underlying multiplication and addition that occurs.
		using element_type_t = decltype(functions::dot(std::declval<basic_vector<T, C1>>(), std::declval<basic_vector<U, R2>>()));
		auto val = basic_matrix<element_type_t, C2, R1>{};

		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			 ((val[Is] = lhs * rhs[Is]), ...);
		}(std::make_index_sequence<C2>{});

		return val;
	}

	//
	// specialized using types
	//

	// boolean vectors
	using bscal = basic_vector<bool, 1>;
	using bvec2 = basic_vector<bool, 2>;
	using bvec3 = basic_vector<bool, 3>;
	using bvec4 = basic_vector<bool, 4>;

	// int vectors
	using iscal = basic_vector<int, 1>;
	using ivec2 = basic_vector<int, 2>;
	using ivec3 = basic_vector<int, 3>;
	using ivec4 = basic_vector<int, 4>;

	// unsigned int vectors
	using uscal = basic_vector<unsigned, 1>;
	using uvec2 = basic_vector<unsigned, 2>;
	using uvec3 = basic_vector<unsigned, 3>;
	using uvec4 = basic_vector<unsigned, 4>;

	// long long vectors (not in glsl)
	using llscal = basic_vector<long long, 1>;
	using llvec2 = basic_vector<long long, 2>;
	using llvec3 = basic_vector<long long, 3>;
	using llvec4 = basic_vector<long long, 4>;

	// unsigned long long vectors (not in glsl)
	using ullscal = basic_vector<unsigned long long, 1>;
	using ullvec2 = basic_vector<unsigned long long, 2>;
	using ullvec3 = basic_vector<unsigned long long, 3>;
	using ullvec4 = basic_vector<unsigned long long, 4>;

	// float vectors with out an 'f' prefix -- this is from glsl
	using scal = basic_vector<float, 1>;
	using vec2 = basic_vector<float, 2>;
	using vec3 = basic_vector<float, 3>;
	using vec4 = basic_vector<float, 4>;

	// also float vectors, but using the same naming convention as the other vectors do (not in glsl)
	using fscal = basic_vector<float, 1>;
	using fvec2 = basic_vector<float, 2>;
	using fvec3 = basic_vector<float, 3>;
	using fvec4 = basic_vector<float, 4>;

	// double vectors
	using dscal = basic_vector<double, 1>;
	using dvec2 = basic_vector<double, 2>;
	using dvec3 = basic_vector<double, 3>;
	using dvec4 = basic_vector<double, 4>;

	// float matrices
	using mat2x2 = basic_matrix<float, 2, 2>;
	using mat2x3 = basic_matrix<float, 2, 3>;
	using mat2x4 = basic_matrix<float, 2, 4>;
	using mat3x2 = basic_matrix<float, 3, 2>;
	using mat3x3 = basic_matrix<float, 3, 3>;
	using mat3x4 = basic_matrix<float, 3, 4>;
	using mat4x2 = basic_matrix<float, 4, 2>;
	using mat4x3 = basic_matrix<float, 4, 3>;
	using mat4x4 = basic_matrix<float, 4, 4>;

	using mat2 = basic_matrix<float, 2, 2>;
	using mat3 = basic_matrix<float, 3, 3>;
	using mat4 = basic_matrix<float, 4, 4>;

	// double matrices
	using dmat2x2 = basic_matrix<double, 2, 2>;
	using dmat2x3 = basic_matrix<double, 2, 3>;
	using dmat2x4 = basic_matrix<double, 2, 4>;
	using dmat3x2 = basic_matrix<double, 3, 2>;
	using dmat3x3 = basic_matrix<double, 3, 3>;
	using dmat3x4 = basic_matrix<double, 3, 4>;
	using dmat4x2 = basic_matrix<double, 4, 2>;
	using dmat4x3 = basic_matrix<double, 4, 3>;
	using dmat4x4 = basic_matrix<double, 4, 4>;

	using dmat2 = basic_matrix<double, 2, 2>;
	using dmat3 = basic_matrix<double, 3, 3>;
	using dmat4 = basic_matrix<double, 4, 4>;

	//
	// bring the vector and matrix free functions into the dsga namespace
	//
	using namespace functions;

	//
	// converting from external vector type or data to internal vector type
	//

	template <dimensional_scalar T, std::size_t S>
	requires dimensional_storage<T, S>
	[[nodiscard]] constexpr basic_vector<T, S> to_vector(const std::array<T, S> &arg) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return basic_vector<T, S>{ arg[Is]... };
		}(std::make_index_sequence<S>{});
	}

	template <dimensional_scalar T, std::size_t S>
	requires dimensional_storage<T, S>
	[[nodiscard]] constexpr basic_vector<T, S> to_vector(const T(&arg)[S]) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return basic_vector<T, S>{ arg[Is]... };
		}(std::make_index_sequence<S>{});
	}

	// converting from internal vector type to std::array

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	[[nodiscard]] constexpr std::array<T, C> to_array(const vector_base<W, T, C, D> &arg) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept -> std::array<T, C>
		{
			return { arg[Is]... };
		}(std::make_index_sequence<C>{});
	}

	// converting from array to a basic_matrix

	template <std::size_t C, std::size_t R, floating_point_scalar T, std::size_t S>
	requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4))) && (C * R <= S)
	[[nodiscard]] constexpr basic_matrix<T, C, R> to_matrix(const std::array<T, S> &arg) noexcept
	{
		return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return basic_matrix<T, C, R>(
				[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
				{
					constexpr auto cols = Js;
					return basic_vector<T, R>{ arg[cols * R + Is]... };
				}(std::make_index_sequence<R>{}) ...);
		}(std::make_index_sequence<C>{});
	}

	template <std::size_t C, std::size_t R, floating_point_scalar T, std::size_t S>
	requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4))) && (C * R <= S)
	[[nodiscard]] constexpr basic_matrix<T, C, R> to_matrix(const T(&arg)[S]) noexcept
	{
		return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return basic_matrix<T, C, R>(
				[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
				{
					constexpr auto cols = Js;
					return basic_vector<T, R>{ arg[cols * R + Is]... };
				}(std::make_index_sequence<R>{}) ...);
		}(std::make_index_sequence<C>{});
	}

	// converting from internal matrix type to std::array

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4)))
	[[nodiscard]] constexpr std::array<T, C * R> to_array(const basic_matrix<T, C, R> &arg) noexcept
	{
		auto matrix_tuple = [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return detail::flatten_args_to_tuple(arg[Is]...);
		}(std::make_index_sequence<C>{});

		return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return std::array<T, C * R>{ std::get<Js>(matrix_tuple)... };
		}(std::make_index_sequence<C * R>{});
	}

}	// namespace dsga

//
// tuple protocol for basic_vector and indexed_vector and vec_base -- supports structured bindings
//

template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::storage_wrapper<T, S>> : std::integral_constant<std::size_t, S>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::storage_wrapper<T, S>>
{
	using type = T;
};

template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::basic_vector<T, S>> : std::integral_constant<std::size_t, S>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::basic_vector<T, S>>
{
	using type = T;
};

template <dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_size<dsga::indexed_vector<T, S, C, Is...>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_element<I, dsga::indexed_vector<T, S, C, Is...>>
{
	using type = T;
};

template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_size<dsga::vector_base<W, T, C, D>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_element<I, dsga::vector_base<W, T, C, D>>
{
	using type = T;
};

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::tuple_size<dsga::basic_matrix<T, C, R>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::tuple_element<I, dsga::basic_matrix<T, C, R>>
{
	using type = dsga::basic_vector<T, R>;
};

// closing include guard
#endif
