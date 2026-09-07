//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// https://github.com/davidbrowne/dsga

// opening include guard
#if !defined(DSGA_DSGA_HXX)
#define DSGA_DSGA_HXX


#include <stdexcept>

// assertion macro, can be disabled by defining DSGA_DISABLE_ASSERT before including this header,
// or by defining DSGA_ASSERT to something else before including this header.
#ifndef DSGA_DISABLE_ASSERT

#ifndef DSGA_ASSERT
#define DSGA_ASSERT(cond, msg) \
		do { if (!(cond)) [[unlikely]] throw std::out_of_range(msg); } while (0)
#endif

#endif


#include <utility>
#include <limits>
#include <type_traits>				// requirements
#include <concepts>					// requirements
#include <cmath>
#include <bit>						// bit_cast

#include <array>					// underlying storage
#include <tuple>					// tuple interface for structured bindings, variadic constructors
#include <algorithm>				// min()
#include <numbers>					// pi_v<>, inv_pi_v<>

//
// Data Structures for Geometric Algebra (dsga)
//

namespace dsga
{
	//          Copyright David Browne 2020-2026.
	// Distributed under the Boost Software License, Version 1.0.
	//    (See accompanying file LICENSE_1_0.txt or copy at
	//          https://www.boost.org/LICENSE_1_0.txt)

	// version info

	constexpr inline int DSGA_MAJOR_VERSION = 3;
	constexpr inline int DSGA_MINOR_VERSION = 3;
	constexpr inline int DSGA_PATCH_VERSION = 2;

	namespace detail
	{
		namespace cxcm
		{
			//          Copyright David Browne 2020-2026.
			// Distributed under the Boost Software License, Version 1.0.
			//    (See accompanying file LICENSE_1_0.txt or copy at
			//          https://www.boost.org/LICENSE_1_0.txt)

			// https://github.com/davidbrowne/cxcm - cxcm

			// version info

			constexpr int CXCM_MAJOR_VERSION = 1;
			constexpr int CXCM_MINOR_VERSION = 3;
			constexpr int CXCM_PATCH_VERSION = 2;

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
					double temp = 134217729.0 * a;				// 134217729.0 = 2^27 + 1
					high = temp - (temp - a);
					low = a - high;
				}

				// The following code computes fl(a x b) and error(a x b).
				constexpr double two_prod(double a, double b, double &error) noexcept
				{
					double a_high = 0.0;
					double a_low = 0.0;
					double b_high = 0.0;
					double b_low = 0.0;

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

					constexpr dd_real(double hi, double lo) noexcept : x{hi, lo}
					{
					}

					explicit constexpr dd_real(double h) noexcept : x{h, 0.}
					{
					}

					explicit constexpr dd_real(float h) noexcept : x{static_cast<double>(h), 0.}
					{
					}

					constexpr dd_real(const dd_real &) noexcept = default;
					constexpr dd_real(dd_real &&) noexcept = default;
					constexpr dd_real &operator =(const dd_real &) noexcept = default;
					constexpr dd_real &operator =(dd_real &&) noexcept = default;

					constexpr double operator [](unsigned int index) const noexcept
					{
						return x[index];
					}

					constexpr double &operator [](unsigned int index) noexcept
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

				constexpr bool operator ==(const dd_real &a, const dd_real &b) noexcept
				{
					return (a.x[0] == b.x[0]) && (a.x[1] == b.x[1]);
				}

				// double-double + double-double
				constexpr dd_real ieee_add(const dd_real &a, const dd_real &b) noexcept
				{
					// This one satisfies IEEE style error bound, due to K. Briggs and W. Kahan.
					double s1 = 0.0;
					double s2 = 0.0;
					double t1 = 0.0;
					double t2 = 0.0;

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
					double s1 = 0.0;
					double s2 = 0.0;

					s1 = two_sum(a.x[0], b, s2);
					s1 = quick_two_sum(s1, s2 + a.x[1], s2);
					return dd_real(s1, s2);
				}

				// double-double - double-double
				constexpr dd_real ieee_subtract(const dd_real &a, const dd_real &b) noexcept
				{
					// This one satisfies IEEE style error bound, due to K. Briggs and W. Kahan.
					double s1 = 0.0;
					double s2 = 0.0;
					double t1 = 0.0;
					double t2 = 0.0;

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
					double s1 = 0.0;
					double s2 = 0.0;

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
					double p1 = 0.0;
					double p2 = 0.0;

					p1 = two_prod(a.x[0], b.x[0], p2);
					p2 += (a.x[0] * b.x[1] + a.x[1] * b.x[0]);
					p1 = quick_two_sum(p1, p2, p2);
					return dd_real(p1, p2);
				}

				// double-double * double
				constexpr dd_real operator *(const dd_real &a, double b) noexcept
				{
					double p1 = 0.0;
					double p2 = 0.0;

					p1 = two_prod(a.x[0], b, p2);
					p1 = quick_two_sum(p1, p2 + (a.x[1] * b), p2);
					return dd_real(p1, p2);
				}

				// double * double-double
				constexpr dd_real operator *(double a, const dd_real &b) noexcept
				{
					return (b * a);
				}

				constexpr dd_real &operator *=(dd_real &a, const dd_real &b) noexcept
				{
					double p1 = 0.0;
					double p2 = 0.0;

					p1 = two_prod(a.x[0], b.x[0], p2);
					p2 += (a.x[0] * b.x[1] + a.x[1] * b.x[0]);
					a.x[0] = quick_two_sum(p1, p2, a.x[1]);
					return a;
				}

				constexpr dd_real accurate_div(const dd_real &a, const dd_real &b) noexcept
				{
					double q1 = 0.0;
					double q2 = 0.0;
					double q3 = 0.0;

					q1 = a.x[0] / b.x[0];						// approximate quotient

					dd_real r = a - q1 * b;

					q2 = r.x[0] / b.x[0];
					r -= (q2 * b);

					q3 = r.x[0] / b.x[0];

					q1 = quick_two_sum(q1, q2, q2);

					double s1 = 0.0;
					double s2 = 0.0;
					s1 = two_sum(q1, q3, s2);
					s1 = quick_two_sum(s1, s2 + q2, s2);

					return dd_real(s1, s2);
				}

				constexpr dd_real accurate_div(double a, const dd_real &b) noexcept
				{
					double q1 = 0.0;
					double q2 = 0.0;
					double q3 = 0.0;

					q1 = a / b.x[0];							// approximate quotient

					dd_real r = a - q1 * b;

					q2 = r.x[0] / b.x[0];
					r -= (q2 * b);

					q3 = r.x[0] / b.x[0];

					q1 = quick_two_sum(q1, q2, q2);

					double s1 = 0.0;
					double s2 = 0.0;
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

			}	// namespace dd_real

			namespace concepts
			{
				template <typename T>
				concept basic_floating_point = (std::is_same_v<float, std::remove_cvref_t<T>> || std::is_same_v<double, std::remove_cvref_t<T>>);

			}	// namespace concepts

			namespace limits
			{
				namespace impl
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

				}	// namespace impl

				//
				// largest_fractional_value
				//

				// the largest floating point value that has a fractional representation

				template <cxcm::concepts::basic_floating_point T>
				constexpr inline T largest_fractional_value = T();

				template <>
				constexpr inline double largest_fractional_value<double> = 0x1.fffffffffffffp+51;

				template <>
				constexpr inline float largest_fractional_value<float> = 0x1.fffffep+22f;

			}	// namespace limits

			//
			// floating-point negative zero support
			//

			template <cxcm::concepts::basic_floating_point T>
			constexpr bool is_negative_zero(T) noexcept
			{
				return false;
			}

			template<>
			constexpr bool is_negative_zero(float val) noexcept
			{
				return (0x80000000u == std::bit_cast<unsigned int>(val));
			}

			template<>
			constexpr bool is_negative_zero(double val) noexcept
			{
				return (0x8000000000000000ull == std::bit_cast<unsigned long long>(val));
			}

			template <cxcm::concepts::basic_floating_point T>
			constexpr inline T negative_zero = T(-0);

			template <>
			constexpr inline float negative_zero<float> = std::bit_cast<float>(0x80000000u);

			template <>
			constexpr inline double negative_zero<double> = std::bit_cast<double>(0x8000000000000000ull);

			// don't worry about esoteric input.
			// much faster than strict or standard when non constant evaluated,
			// though standard library is a little better in debugger.
			namespace relaxed
			{
				//
				// abs(), fabs()
				//

				// absolute value

				template <cxcm::concepts::basic_floating_point T>
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

				template <cxcm::concepts::basic_floating_point T>
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

				template <cxcm::concepts::basic_floating_point T>
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

				template <cxcm::concepts::basic_floating_point T>
				constexpr T floor(T value) noexcept
				{
					const T truncated_value = trunc(value);

					// truncation rounds to zero which is right direction for positive values,
					// but we need to go the other way for negative values.

					// negative non-integral value
					if (truncated_value > value)
						return (truncated_value - T(1));

					// positive or integral value
					return truncated_value;
				}

				//
				// ceil()
				//

				// rounds towards positive infinity

				template <cxcm::concepts::basic_floating_point T>
				constexpr T ceil(T value) noexcept
				{
					const T truncated_value = trunc(value);

					// truncation rounds to zero which is right direction for negative values,
					// but we need to go the other way for positive values.

					// positive non-integral value
					if (truncated_value < value)
						return (truncated_value + T(1));

					// negative or integral value
					return truncated_value;
				}

				//
				// round()
				//

				// rounds to nearest integral position, halfway cases away from zero

				template <cxcm::concepts::basic_floating_point T>
				constexpr T round(T value) noexcept
				{
					// zero could be handled either place, but here it is with the negative values.

					// positive value, taking care of halfway case.
					if (value > T(0))
						return trunc(value + T(0.5));

					// negative or zero value, taking care of halfway case.
					return trunc(value - T(0.5));
				}

				//
				// fract() - not in standard library
				//

				// the fractional part of a floating point number - always non-negative.

				template <cxcm::concepts::basic_floating_point T>
				constexpr T fract(T value) noexcept
				{
					return value - floor(value);
				}

				//
				// fmod()
				//

				// the floating point remainder of division

				template <cxcm::concepts::basic_floating_point T>
				constexpr T fmod(T x, T y) noexcept
				{
					return x - trunc(x / y) * y;
				}

				//
				// round_even() - not in standard library
				//

				// rounds to nearest integral position, halfway cases towards even

				template <cxcm::concepts::basic_floating_point T>
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
						return trunc(value + T(0.5));

					// negative or zero value, taking care of halfway case.
					return trunc(value - T(0.5));
				}

				//
				// sqrt()
				//

				namespace impl
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
					template <cxcm::concepts::basic_floating_point T>
					constexpr T converging_sqrt(T arg) noexcept
					{
						// make sure this is a double so we can use it for higher precision when T is float
						const double boosted_arg = arg;

						// highly accurate initial guess for the square root, so there shouldn't be many convergence steps needed
						double init_value = boosted_arg * fast_rsqrt(boosted_arg);

						if constexpr (std::is_same_v<T, double>)
						{
							// 2 steps of saved previous values for detecting 2-cycle oscillations
							auto current_value = dd_real::dd_real(init_value);
							auto previous_value = dd_real::dd_real(0.0);
							auto preprevious_value = dd_real::dd_real(0.0);

							// in case there are more than 2 steps of oscillation, find a cutoff point to stop looping
							int iterations = 0;
							constexpr int max_iterations = 10;

							constexpr auto half = dd_real::dd_real(0.5);

							while ((current_value[0] != previous_value[0]) &&
								   (current_value[0] * current_value[0] != boosted_arg))
							{
								// update saved values and generate the next one
								preprevious_value = previous_value;
								previous_value = current_value;
								current_value = half * (current_value + (boosted_arg / current_value));

								// 2-cycle oscillation detected
								if (current_value[0] == preprevious_value[0])
									break;

								// longer cycle safety net
								if (++iterations >= max_iterations)
									break;
							}
							return static_cast<double>(current_value);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							// 2 steps of saved previous values for detecting 2-cycle oscillations
							double current_value = init_value;
							double previous_value = 0.0;
							double preprevious_value = 0.0;

							// in case there are more than 2 steps of oscillation, find a cutoff point to stop looping
							int iterations = 0;
							constexpr int max_iterations = 10;

							while ((current_value != previous_value) &&
								   (current_value * current_value != boosted_arg))
							{
								// update saved values and generate the next one
								preprevious_value = previous_value;
								previous_value = current_value;
								current_value = 0.5 * (current_value + (boosted_arg / current_value));

								// 2-cycle oscillation detected
								if (current_value == preprevious_value)
									break;

								// longer cycle safety net
								if (++iterations >= max_iterations)
									break;
							}
							return static_cast<float>(current_value);
						}
					}

					// float uses double internally, double uses dd_real internally
					template <cxcm::concepts::basic_floating_point T>
					constexpr T converging_inverse_sqrt(T arg) noexcept
					{
						// make sure this is a double so we can use it for higher precision when T is float
						const double boosted_arg = arg;

						// highly accurate initial guess for the inverse square root, so there shouldn't be many convergence steps needed
						double init_value = fast_rsqrt(boosted_arg);

						if constexpr (std::is_same_v<T, double>)
						{
							// 2 steps of saved previous values for detecting 2-cycle oscillations
							auto current_value = dd_real::dd_real(init_value);
							auto previous_value = dd_real::dd_real(0.0);
							auto preprevious_value = dd_real::dd_real(0.0);

							// in case there are more than 2 steps of oscillation, find a cutoff point to stop looping
							int iterations = 0;
							constexpr int max_iterations = 10;

							const auto half_arg = dd_real::dd_real(0.5 * boosted_arg);
							const auto three_halves = dd_real::dd_real(1.5);

							while ((current_value[0] != previous_value[0]) &&
								   (current_value[0] * current_value[0] * boosted_arg != 1.0))
							{
								// update saved values and generate the next one
								preprevious_value = previous_value;
								previous_value = current_value;
								current_value *= (three_halves - (half_arg * current_value * current_value));

								// 2-cycle oscillation detected
								if (current_value[0] == preprevious_value[0])
									break;

								// longer cycle safety net
								if (++iterations >= max_iterations)
									break;
							}
							return static_cast<double>(current_value);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							// 2 steps of saved previous values for detecting 2-cycle oscillations
							double current_value = init_value;
							double previous_value = 0.0;
							double preprevious_value = 0.0;

							// in case there are more than 2 steps of oscillation, find a cutoff point to stop looping
							int iterations = 0;
							constexpr int max_iterations = 10;

							while ((current_value != previous_value) &&
								   (current_value * current_value * boosted_arg != 1.0))
							{
								// update saved values and generate the next one
								preprevious_value = previous_value;
								previous_value = current_value;
								current_value *= (1.5 - (0.5 * boosted_arg * current_value * current_value));

								// 2-cycle oscillation detected
								if (current_value == preprevious_value)
									break;

								// longer cycle safety net
								if (++iterations >= max_iterations)
									break;
							}
							return static_cast<float>(current_value);
						}
					}

					template <cxcm::concepts::basic_floating_point T>
					constexpr T inverse_sqrt(T arg) noexcept
					{
						// make sure this is a double so we can use it for higher precision when T is float
						const double boosted_arg = arg;

						// highly accurate initial guess for the square root, so there shouldn't be many convergence steps needed
						double init_value = boosted_arg * fast_rsqrt(boosted_arg);

						if constexpr (std::is_same_v<T, double>)
						{
							// 2 steps of saved previous values for detecting 2-cycle oscillations
							auto current_value = dd_real::dd_real(init_value);
							auto previous_value = dd_real::dd_real(0.0);
							auto preprevious_value = dd_real::dd_real(0.0);

							// in case there are more than 2 steps of oscillation, find a cutoff point to stop looping
							int iterations = 0;
							constexpr int max_iterations = 10;

							constexpr auto one = dd_real::dd_real(1.0);
							constexpr auto half = dd_real::dd_real(0.5);

							while ((current_value[0] != previous_value[0]) &&
								   (current_value[0] * current_value[0] != boosted_arg))
							{
								// update saved values and generate the next one
								preprevious_value = previous_value;
								previous_value = current_value;
								current_value = half * (current_value + (boosted_arg / current_value));

								// 2-cycle oscillation detected
								if (current_value[0] == preprevious_value[0])
									break;

								// longer cycle safety net
								if (++iterations >= max_iterations)
									break;
							}
							return static_cast<double>(one / current_value);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							// 2 steps of saved previous values for detecting 2-cycle oscillations
							double current_value = init_value;
							double previous_value = 0.0;
							double preprevious_value = 0.0;

							// in case there are more than 2 steps of oscillation, find a cutoff point to stop looping
							int iterations = 0;
							constexpr int max_iterations = 10;

							while ((current_value != previous_value) &&
								   (current_value * current_value != boosted_arg))
							{
								// update saved values and generate the next one
								preprevious_value = previous_value;
								previous_value = current_value;
								current_value = 0.5 * (current_value + (boosted_arg / current_value));

								// 2-cycle oscillation detected
								if (current_value == preprevious_value)
									break;

								// longer cycle safety net
								if (++iterations >= max_iterations)
									break;
							}
							return static_cast<float>(1.0 / current_value);
						}
					}

				}	// namespace impl

				// constexpr square root, uses higher precision behind the scenes
				template <cxcm::concepts::basic_floating_point T>
				constexpr T sqrt(T value) noexcept
				{
					return impl::converging_sqrt(value);
				}

				// reciprocal of square root, uses higher precision behind the scenes
				template <cxcm::concepts::basic_floating_point T>
				constexpr T rsqrt(T value) noexcept
				{
					return impl::inverse_sqrt(value);
				}

				// fast reciprocal of square root
				template <cxcm::concepts::basic_floating_point T>
				constexpr T fast_rsqrt(T value) noexcept
				{
					return static_cast<T>(impl::fast_rsqrt(static_cast<double>(value)));
				}

			}	// namespace relaxed

			//
			// isnan()
			//

			// make sure this isn't optimized away if used with fast-math

	#if defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
	#pragma float_control(precise, on, push)
	#endif

			template <cxcm::concepts::basic_floating_point T>
	#if defined(__GNUC__) && !defined(__clang__)
			__attribute__((optimize("-fno-fast-math")))
	#endif
			constexpr bool isnan(T value) noexcept
			{
				return (value != value);
			}

	#if defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
	#pragma float_control(pop)
	#endif

			template <std::integral T>
			constexpr bool isnan(T value) noexcept
			{
				return isnan(static_cast<double>(value));
			}

			//
			// isinf()
			//

			// make sure this isn't optimized away if used with fast-math

	#if defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
	#pragma float_control(precise, on, push)
	#endif

			template <cxcm::concepts::basic_floating_point T>
	#if defined(__GNUC__) && !defined(__clang__)
			__attribute__((optimize("-fno-fast-math")))
	#endif
			constexpr bool isinf(T value) noexcept
			{
				return (value == -std::numeric_limits<T>::infinity()) || (value == std::numeric_limits<T>::infinity());
			}

	#if defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
	#pragma float_control(pop)
	#endif

			template <std::integral T>
			constexpr bool isinf(T value) noexcept
			{
				return isinf(static_cast<double>(value));
			}

			//
			// fpclassify()
			//

			template <cxcm::concepts::basic_floating_point T>
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

			template <std::integral T>
			constexpr int fpclassify(T value) noexcept
			{
				return fpclassify(static_cast<double>(value));
			}

			//
			// isnormal()
			//

			template <cxcm::concepts::basic_floating_point T>
			constexpr bool isnormal(T value) noexcept
			{
				return (fpclassify(value) == FP_NORMAL);
			}

			template <std::integral T>
			constexpr bool isnormal(T value) noexcept
			{
				return isnormal(static_cast<double>(value));
			}

			//
			// isfinite()
			//

			template <cxcm::concepts::basic_floating_point T>
			constexpr bool isfinite(T value) noexcept
			{
				return !isnan(value) && !isinf(value);
			}

			template <std::integral T>
			constexpr bool isfinite(T value) noexcept
			{
				return isfinite(static_cast<double>(value));
			}

			//
			// signbit()
			//

			// +0 returns false and -0 returns true
			template <cxcm::concepts::basic_floating_point T>
			constexpr bool signbit(T value) noexcept
			{
				static_assert(std::numeric_limits<T>::is_iec559, "IEC 559 required");

				if constexpr (sizeof(T) == 4)
				{
					unsigned int bits = std::bit_cast<unsigned int>(value);
					return (bits & 0x80000000u) != 0;
				}
				else if constexpr (sizeof(T) == 8)
				{
					unsigned long long bits = std::bit_cast<unsigned long long>(value);
					return (bits & 0x8000000000000000ull) != 0;
				}
			}

			template <std::integral T>
			constexpr bool signbit(T value) noexcept
			{
				return signbit(static_cast<double>(value));
			}

			//
			// copysign()
			//

			// +0 or -0 for sign makes a difference
			template <cxcm::concepts::basic_floating_point T>
			constexpr T copysign(T value, T sgn) noexcept
			{
				static_assert(std::numeric_limits<T>::is_iec559, "IEC 559 required");

				// +0 or -0 for sign makes a difference
				bool is_neg = signbit(sgn);

				if constexpr (sizeof(T) == 4)
				{
					unsigned int bits = std::bit_cast<unsigned int>(value);
					if (is_neg)
						bits |= 0x80000000u;
					else
						bits &= 0x7FFFFFFFu;

					return std::bit_cast<T>(bits);
				}
				else if constexpr (sizeof(T) == 8)
				{
					unsigned long long bits = std::bit_cast<unsigned long long>(value);
					if (is_neg)
						bits |= 0x8000000000000000ull;
					else
						bits &= 0x7FFFFFFFFFFFFFFFull;

					return std::bit_cast<T>(bits);
				}
			}

			template <std::integral T>
			constexpr double copysign(T value, T sgn) noexcept
			{
				return copysign(static_cast<double>(value), static_cast<double>(sgn));
			}

			// try and match standard library requirements.
			// this namespace is pulled into parent namespace via inline.
			inline namespace strict
			{
				namespace impl
				{
					//
					// make_nan_quiet()
					//

					// make a NaN into a quiet NaN - if input is not a NaN, it is returned unchanged
					template <cxcm::concepts::basic_floating_point T>
					constexpr T convert_to_quiet_nan(T value) noexcept
					{
						if (cxcm::isnan(value))
						{
							if constexpr (sizeof(T) == 4)
							{
								unsigned int bits = std::bit_cast<unsigned int>(value);

								// set the is_quiet bit
								bits |= 0x00400000u;

								return std::bit_cast<T>(bits);
							}
							else if constexpr (sizeof(T) == 8)
							{
								unsigned long long bits = std::bit_cast<unsigned long long>(value);

								// set the is_quiet bit
								bits |= 0x0008000000000000ull;

								return std::bit_cast<T>(bits);
							}
						}

						return value;
					}

					//
					// isnormal_or_subnormal()
					//

					// standard library screening requirement for these functions

					template <cxcm::concepts::basic_floating_point T>
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

					template <cxcm::concepts::basic_floating_point T>
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

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_trunc(T value) noexcept
					{
#if !defined(__GNUC__) || defined(__clang__)
						if (isnan(value))
							return convert_to_quiet_nan(value);
#endif

					// screen out unnecessary input
						if (fails_fractional_input_constraints(value))
							return value;

						return relaxed::trunc(value);
					}

					//
					// constexpr_floor()
					//

					// rounds towards negative infinity

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_floor(T value) noexcept
					{
#if !defined(__GNUC__) || defined(__clang__)
						if (isnan(value))
							return convert_to_quiet_nan(value);
#endif

					// screen out unnecessary input
						if (fails_fractional_input_constraints(value))
							return value;

						return relaxed::floor(value);
					}

					//
					// constexpr_ceil()
					//

					// rounds towards positive infinity

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_ceil(T value) noexcept
					{
#if !defined(__GNUC__) || defined(__clang__)
						if (isnan(value))
							return convert_to_quiet_nan(value);
#endif

					// screen out unnecessary input
						if (fails_fractional_input_constraints(value))
							return value;

						return relaxed::ceil(value);
					}

					//
					// constexpr_round()
					//

					// rounds to nearest integral position, halfway cases away from zero

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_round(T value) noexcept
					{
#if !defined(__GNUC__) || defined(__clang__)
						if (isnan(value))
							return convert_to_quiet_nan(value);
#endif

					// screen out unnecessary input
						if (fails_fractional_input_constraints(value))
							return value;

						// halfway rounding can bump into max long long value for truncation
						// (for extended precision), so be more gentle at the end points.
						// this works because the largest_fractional_value remainder is T(0.5).
						if (value == limits::largest_fractional_value<T>)
							return value + T(0.5);
						else if (value == -limits::largest_fractional_value<T>)			// we technically don't have to do this for negative case (one more number in negative range)
							return value - T(0.5);

						return relaxed::round(value);
					}

					//
					// constexpr_fract()
					//

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_fract(T value) noexcept
					{
#if !defined(__GNUC__) || defined(__clang__)
						if (isnan(value))
							return convert_to_quiet_nan(value);
#endif

					// screen out unnecessary input
						if (fails_fractional_input_constraints(value))
							return value;

						return relaxed::fract(value);
					}

					//
					// constexpr_fmod()
					//

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_fmod(T x, T y) noexcept
					{
						// screen out unnecessary input

						if (isnan(x) || isnan(y) || !isfinite(x))
							return std::numeric_limits<T>::quiet_NaN();

						if (isinf(y))
							return x;

						if (x == T(0) && y != T(0))
							return x;

						if (y == 0)
							return std::numeric_limits<T>::quiet_NaN();

						return relaxed::fmod(x, y);
					}

					//
					// constexpr_round_even()
					//

					// rounds to nearest integral position, halfway cases away from zero

					template <cxcm::concepts::basic_floating_point T>
					constexpr T constexpr_round_even(T value) noexcept
					{
#if !defined(__GNUC__) || defined(__clang__)
						if (isnan(value))
							return convert_to_quiet_nan(value);
#endif

					// screen out unnecessary input
						if (fails_fractional_input_constraints(value))
							return value;

						// halfway rounding can bump into max long long value for truncation
						// (for extended precision), so be more gentle at the end points.
						// this works because the largest_fractional_value remainder is T(0.5).
						if (value == limits::largest_fractional_value<T>)
							return value + T(0.5);
						else if (value == -limits::largest_fractional_value<T>)			// we technically don't have to do this for negative case (one more number in negative range)
							return value - T(0.5);

						return relaxed::round_even(value);
					}

					//
					// constexpr_sqrt()
					//

					// make sure this isn't optimized away if used with fast-math

#if defined(_MSC_VER) || defined(__clang__)
#pragma float_control(precise, on, push)
#endif

					template <cxcm::concepts::basic_floating_point T>
#if defined(__GNUC__) && !defined(__clang__)
					__attribute__((optimize("-fno-fast-math")))
#endif
					constexpr T constexpr_sqrt(T value) noexcept
					{
						// screen out unnecessary input

						if (isnan(value))
						{
							return impl::convert_to_quiet_nan(value);
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

#if defined(_MSC_VER) || defined(__clang__)
#pragma float_control(pop)
#endif

					//
					// constexpr_inverse_sqrt()
					//

					// make sure this isn't optimized away if used with fast-math

#if defined(_MSC_VER) || defined(__clang__)
#pragma float_control(precise, on, push)
#endif

					template <cxcm::concepts::basic_floating_point T>
#if defined(__GNUC__) && !defined(__clang__)
					__attribute__((optimize("-fno-fast-math")))
#endif
					constexpr T constexpr_rsqrt(T value) noexcept
					{
						// screen out unnecessary input

						if (isnan(value))
						{
							[[ unlikely ]] return impl::convert_to_quiet_nan(value);
						}
						else if (value == std::numeric_limits<T>::infinity())
						{
							[[ unlikely ]] return T(0);
						}
						else if (value == -std::numeric_limits<T>::infinity())
						{
							[[ unlikely ]] return -std::numeric_limits<T>::quiet_NaN();
						}
						else if (value == T(0))
						{
							[[ unlikely ]] return std::numeric_limits<T>::infinity();
						}
						else if (value < T(0))
						{
							[[ unlikely ]] return -std::numeric_limits<T>::quiet_NaN();
						}

						[[ likely ]] return relaxed::rsqrt(value);
					}

#if defined(_MSC_VER) || defined(__clang__)
#pragma float_control(pop)
#endif

					// make sure this isn't optimized away if used with fast-math

#if defined(_MSC_VER) || defined(__clang__)
#pragma float_control(precise, on, push)
#endif

					template <cxcm::concepts::basic_floating_point T>
#if defined(__GNUC__) && !defined(__clang__)
					__attribute__((optimize("-fno-fast-math")))
#endif
					constexpr T constexpr_fast_rsqrt(T value) noexcept
					{
						// screen out unnecessary input

						if (isnan(value))
						{
							return impl::convert_to_quiet_nan(value);
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

#if defined(_MSC_VER) || defined(__clang__)
#pragma float_control(pop)
#endif

				} // namespace impl

				//
				// abs(), fabs()
				//


				// absolute value

				template <cxcm::concepts::basic_floating_point T>
				constexpr T abs(T value) noexcept
				{
					auto new_value = cxcm::copysign(value, T(+1));

#if !defined(NDEBUG) && defined(_MSC_VER)
					if (isnan(new_value))
					{
						return impl::convert_to_quiet_nan(new_value);
					}
					else
					{
						return new_value;
					}
#else
					return new_value;
#endif
				}

				// don't know what to do if someone tries to negate the most negative number.
				// standard says behavior is undefined if you can't represent the result by return type.
				template <std::integral T>
				constexpr T abs(T value)
				{
					if (value == std::numeric_limits<T>::min())
					{
						[[ unlikely ]] throw std::domain_error("negation of min value is not a valid integral value");
					}

					[[ likely ]] return relaxed::abs(value);
				}

				template <cxcm::concepts::basic_floating_point T>
				constexpr T fabs(T value) noexcept
				{
					return cxcm::abs(value);
				}

				template <std::integral T>
				constexpr double fabs(T value)
				{
					if (value == std::numeric_limits<T>::min())
					{
						[[ unlikely ]] throw std::domain_error("negation of min value is not a valid integral value");
					}

					[[ likely ]] return relaxed::fabs(value);
				}

				//
				// trunc()
				//

				// rounds towards zero

				template <cxcm::concepts::basic_floating_point T>
				constexpr T trunc(T value) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_trunc(value);
					}
					else
					{
						return std::trunc(value);
					}
				}

				template <std::integral T>
				constexpr double trunc(T value) noexcept
				{
					return trunc(static_cast<double>(value));
				}

				//
				// floor()
				//

				// rounds towards negative infinity

				template <cxcm::concepts::basic_floating_point T>
				constexpr T floor(T value) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_floor(value);
					}
					else
					{
						return std::floor(value);
					}
				}

				template <std::integral T>
				constexpr double floor(T value) noexcept
				{
					return floor(static_cast<double>(value));
				}

				//
				// ceil()
				//

				// rounds towards positive infinity

				template <cxcm::concepts::basic_floating_point T>
				constexpr T ceil(T value) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_ceil(value);
					}
					else
					{
						return std::ceil(value);
					}
				}

				template <std::integral T>
				constexpr double ceil(T value) noexcept
				{
					return ceil(static_cast<double>(value));
				}

				//
				// round()
				//

				// rounds to nearest integral position, halfway cases away from zero

				template <cxcm::concepts::basic_floating_point T>
				constexpr T round(T value) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_round(value);
					}
					else
					{
						return std::round(value);
					}
				}

				template <std::integral T>
				constexpr double round(T value) noexcept
				{
					return round(static_cast<double>(value));
				}

				//
				// fract()
				//

				// there is no standard c++ version of this, so always call constexpr version

				// the fractional part of a floating point number - always non-negative.

				template <cxcm::concepts::basic_floating_point T>
				constexpr T fract(T value) noexcept
				{
					return impl::constexpr_fract(value);
				}

				template <std::integral T>
				constexpr double fract(T value) noexcept
				{
					return fract(static_cast<double>(value));
				}

				//
				// fmod()
				//

				// the floating point remainder of division

				template <cxcm::concepts::basic_floating_point T>
				constexpr T fmod(T x, T y) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_fmod(x, y);
					}
					else
					{
						return std::fmod(x, y);
					}
				}

				template <std::integral T>
				constexpr double fmod(T x, T y) noexcept
				{
					return fmod(static_cast<double>(x), static_cast<double>(y));
				}

				//
				// round_even()
				//

				// there is no standard c++ version of this, so always call constexpr version

				// rounds to nearest integral position, halfway cases towards even

				template <cxcm::concepts::basic_floating_point T>
				constexpr T round_even(T value) noexcept
				{
					return impl::constexpr_round_even(value);
				}

				template <std::integral T>
				constexpr double round_even(T value) noexcept
				{
					return round_even(static_cast<double>(value));
				}

				//
				// sqrt()
				//

				template <cxcm::concepts::basic_floating_point T>
				constexpr T sqrt(T value) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_sqrt(value);
					}
					else
					{
						return std::sqrt(value);
					}
				}

				template <std::integral T>
				constexpr double sqrt(T value) noexcept
				{
					return sqrt(static_cast<double>(value));
				}

				//
				// rsqrt() - inverse square root
				//

				// there is no standard c++ version of this

				template <cxcm::concepts::basic_floating_point T>
				constexpr T rsqrt(T value) noexcept
				{
					if (std::is_constant_evaluated())
					{
						return impl::constexpr_rsqrt(value);
					}
					else
					{
						return T(1) / std::sqrt(value);
					}

			}

				template <std::integral T>
				constexpr double rsqrt(T value) noexcept
				{
					return rsqrt(static_cast<double>(value));
				}

				//
				// fast_rsqrt() - fast good approximation to inverse square root
				//

				// there is no standard c++ version of this, so always call constexpr version

				template <cxcm::concepts::basic_floating_point T>
				constexpr T fast_rsqrt(T value) noexcept
				{
					return impl::constexpr_fast_rsqrt(value);
				}

				template <std::integral T>
				constexpr double fast_rsqrt(T value) noexcept
				{
					return fast_rsqrt(static_cast<double>(value));
				}

			}	// namespace strict

		}	// namespace cxcm

	}	// namespace detail

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

	// for vectors,want the size to be between 1 and 4, inclusive
	template <std::size_t N>
	concept vec_dimension = ((N >= 1) && (N <= 4));

	// for matrices, want the size of both columns and rows to be between 2 and 4, inclusive
	template <std::size_t N>
	concept mat_dimension = ((N >= 2) && (N <= 4));

	// dimensional storage needs the arithmetic type and size restrictions
	template <typename T, std::size_t Size>
	concept dimensional_storage = dimensional_scalar<T> && vec_dimension<Size>;

	// we want dimensional_storage_t to have length from 1 to 4 (1 gives just a sneaky kind of T that can swizzle),
	// and the storage has to have room for all the data. We also need dimensional_storage_t to support operator[] to access
	// the data. It needs to also support iterators so we can use it in ranged-for loops, algorithms, etc.

	// the underlying storage for vector and swizzle_vec types. originally this was to be a template parameter and
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
	consteval bool is_constexpr(C) noexcept { return val(); }

	namespace detail
	{
		// the concepts and requirements will help swizzle_vec determine if it can be assigned to, like an lvalue reference,
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
			return (sizeof...(Is) == Count) && vec_dimension<Count>;
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

	}	// namespace detail

	// do the argument indexes and count/size make for valid indirect indexing
	template <std::size_t Size, std::size_t Count, std::size_t ...Is>
	concept indexable = detail::valid_index_count<Count, Is...>() && detail::valid_range_indexes<Size, Is...>();

	// writable_swizzle can determine whether a particular swizzle_vec can be used as an lvalue reference
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

			return [&vals]<std::size_t ...Js>(std::index_sequence<Js...>) noexcept
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
	// this struct is an aggregate type
	//

	template <dimensional_scalar T, std::size_t S>
	requires dimensional_storage<T, S>
	struct vec_storage
	{
		// number of indexable elements
		static constexpr std::size_t Size = S;
		static constexpr std::size_t Count = S;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		// underlying storage
		dimensional_storage_t<T, Size> store;

		// using directives related to storage
		using value_type = T;
		using iterator = dimensional_storage_t<T, Size>::iterator;
		using const_iterator = dimensional_storage_t<T, Size>::const_iterator;
		using reverse_iterator = dimensional_storage_t<T, Size>::reverse_iterator;
		using const_reverse_iterator = dimensional_storage_t<T, Size>::const_reverse_iterator;

		[[nodiscard]] static constexpr int length() noexcept				{ return Count; }
		static constexpr std::integral_constant<std::size_t, Count> size =	{};

		// logical and physically contiguous access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index)
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return store[i];
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return store[i];
		}

		// logical and physically contiguous access to data
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index)
		{
			// store.at() call will throw std::out_of_range if index is out of bounds
			return store.at(static_cast<std::size_t>(index));
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const
		{
			// store.at() call will throw std::out_of_range if index is out of bounds
			return store.at(static_cast<std::size_t>(index));
		}

		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void set(Args ...args) noexcept
		{
			[this, &args...]<std::size_t ...Js>(std::index_sequence<Js ...>) noexcept
			{
				((store[Js] = static_cast<T>(args)),...);
			}(std::make_index_sequence<Count>{});
		}

		constexpr void swap(vec_storage &sw) noexcept	{ store.swap(sw.store); }

		// support for range-for loop
		[[nodiscard]] constexpr		  iterator			begin() noexcept						{ return store.begin(); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept					{ return store.cbegin(); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept					{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept							{ return store.end(); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept					{ return store.cend(); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept						{ return store.rbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept					{ return store.crbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept							{ return store.rend(); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept					{ return store.crend(); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept					{ return rend(); }

	};	// struct vec_storage

	// swap specialization
	template <dimensional_scalar T, std::size_t S>
	constexpr void swap(vec_storage<T, S> &lhs, vec_storage<T, S> &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	template <dimensional_scalar T1, std::size_t S, dimensional_scalar T2>
	requires implicitly_convertible_to<T2, T1>
	constexpr bool operator ==(const vec_storage<T1, S> &first,
							   const vec_storage<T2, S> &second) noexcept
	{
		return [&first, &second]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return ((first[Is] == static_cast<T1>(second[Is])) && ...);
		}(std::make_index_sequence<S>{});
	}

	//
	// CTAD deduction guide
	//

	template <dimensional_scalar T, dimensional_scalar ...U>
	vec_storage(T, U...) -> vec_storage<T, 1 + sizeof...(U)>;

	//
	// T is the type of the elements stored in the vector/storage
	// Size is number of elements referencable in vector/storage

	// the foundational vector type for dsga
	// 
	// template parameters:
	//
	//		T - the scalar type stored
	//		S - the number of actual elements in storage
	//
	template <dimensional_scalar T, std::size_t S>
	requires dimensional_storage<T, S>
	struct vec;

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
	//		sequence() - relies on sequence() in Derived - the physical order to logical order mapping
	//		as_derived() - relies on Derived template parameter - useful for returning references to Derived when you just have a vec_interface
	//		begin(), end(), cbegin(), cend(), rbegin(), rend(), crbegin(), crend() - iterator functions that rely on Derived
	//
	//		apply() - similar to std::valarray apply() which returns Ts - relies on operator[] in Derived
	//		query() - similar to apply() except it returns bools - relies on operator[] in Derived
	//		shift() - shifts elements left or right, filling with zero
	//		cshift() - circular shift of elements left or right
	//		min() - minimum element value - relies on operator[] in Derived
	//		max() - maximum element value - relies on operator[] in Derived
	//		sum() - sum of all element values - relies on operator[] in Derived

	template <bool Writable, dimensional_scalar T, std::size_t Count, typename Derived>
	requires dimensional_storage<T, Count>
	struct vec_interface
	{
		// CRTP access to Derived class
		[[nodiscard]] constexpr Derived &as_derived() noexcept requires Writable	{ return static_cast<Derived &>(*this); }
		[[nodiscard]] constexpr const Derived &as_derived() const noexcept			{ return static_cast<const Derived &>(*this); }

		// logically contiguous write access to all data that allows for self-assignment that works properly
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void set(Args ...args) noexcept											{ this->as_derived().set(args...); }

		// logically contiguous access to piecewise data as index goes from 0 to (Count - 1)
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable	{ return this->as_derived()[index]; }

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept			{ return this->as_derived()[index]; }

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index) noexcept requires Writable	{ return this->as_derived().at(index); }

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const noexcept			{ return this->as_derived().at(index); }

		// number of accessible T elements - required by spec
		[[nodiscard]] static constexpr int length() noexcept						{ return Count; }

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
		// sequenced (since C++17), e.g., MSVC and gcc appear to sequence the evaluation of expression arguments going from
		// right-to-left, while clang appears to go from left-to-right. https://godbolt.org/z/G6sbYd5rs
		template <typename UnOp>
		requires std::same_as<T, std::invoke_result_t<UnOp &, const T &>>
		[[nodiscard]] constexpr vec<T, Count> apply(UnOp op) const noexcept(std::is_nothrow_invocable_v<UnOp &, const T &>)
		{
			return [this, &op]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec<T, Count>{ op((*this)[Is])... };			// braced init list is evaluated in element order
			}(std::make_index_sequence<Count>{});
		}

		// not in std::valarray, but potentially useful
		// apply a predicate function to each element, and return a vec<bool, Count>
		template <typename UnOp>
		requires std::same_as<bool, std::invoke_result_t<UnOp &, const T &>>
		[[nodiscard]] constexpr vec<bool, Count> query(UnOp op) const noexcept(std::is_nothrow_invocable_v<UnOp &, const T &>)
		{
			return [this, &op]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec<bool, Count>{ op((*this)[Is])... };		// braced init list is evaluated in element order
			}(std::make_index_sequence<Count>{});
		}

		// positive count is left shift, negative count is right shift
		[[nodiscard]] constexpr vec<T, Count> shift(int by) const noexcept
		{
			constexpr auto quick_clamp = [](int val, int low, int high) noexcept
			{
				return (val < low) ? low : ((val > high) ? high : val);
			};
			constexpr int max_val = static_cast<int>(Count);
			by = quick_clamp(by, -max_val, max_val);				// avoids UB if trying to negate "by" when it is INT_MIN 
			auto copy = vec<T, Count>(*this);

			if (by > 0)
			{
				auto shifted_end = std::shift_left(copy.begin(), copy.end(), by);
				std::ranges::fill_n(shifted_end, by, T(0));
			}
			else if (by < 0)
			{
				std::shift_right(copy.begin(), copy.end(), -by);
				std::ranges::fill_n(copy.begin(), -by, T(0));
			}

			return copy;
		}

		// positive count is left shift, negative count is right shift
		[[nodiscard]] constexpr vec<T, Count> cshift(int by) const noexcept
		{
			constexpr auto max_val = static_cast<int>(Count);
			by %= max_val;

			vec<T, Count> dest{};
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
			return [this]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (... + (*this)[Is]);
			}(std::make_index_sequence<Count>{});
		}

protected:

		~vec_interface() = default;

	};	// struct vec_interface

	// swap generalization for vec_interface types that don't have their own swap function
	template <dimensional_scalar T, std::size_t S, typename D1, typename D2>
	constexpr void swap(vec_interface<true, T, S, D1> &lhs, vec_interface<true, T, S, D2> &rhs) noexcept
	{
		[&lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			((std::swap(lhs[Is], rhs[Is])), ...);
		}(std::make_index_sequence<S>{});
	}

	// swizzle_vec will act as a swizzle of a vec, the result of "component group notation". vec relies
	// on the anonymous union of swizzle_vec data members. both swizzle_vec and vec have their own storage
	// (linked via anonymous union and common initial sequence).
	//
	// T is the type of the elements stored in the underlying storage
	// Size relates to the number of elements in the underlying storage, which informs the values the Is can hold
	// Count is the number of elements accessible in swizzle -- often works alongside with vec's Size
	// Is... are the number of swizzlable values available -- there are Count of them, and their values are in the range:  0 <= Is < Size

	// we want swizzle_vec (vector swizzles) to have length from 1 to 4 in order to work with the vec
	// which also has these lengths. The number of indices is the same as the Count, between 1 and 4.
	// The indices are valid for indexing into the values in the storage (which has Size elements) if
	// the index values are in the range 0 <= Is < Size.

	// the type of a vec swizzle
	// 
	// template parameters:
	//
	//		T - the scalar type stored
	//		Size - the number of actual elements in storage
	//		Count - the number of indices available to access ScalarType data
	//		Is - an ordered variable set of indices into the storage -- there will be Count of them
	//
	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct swizzle_vec;

	//
	// type traits used to get information about a vec_like or writable_vec_like type.
	// these include checks for the scalar type, the number of elements (size), the
	// derived type, whether it is writeable, whether it has a set() function or not,
	// and whether it has named swizzle data members.
	//

	// get the scalar type of a vec or swizzle_vec or vec_interface

	template <typename V>
	struct vec_scalar;

	template <dimensional_scalar T, std::size_t S>
	struct vec_scalar<vec<T, S>>
	{
		using type = T;
	};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct vec_scalar<swizzle_vec<T, Size, Count, Is...>>
	{
		using type = T;
	};

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	struct vec_scalar<vec_interface<W, T, C, D>>
	{
		using type = T;
	};

	template <typename V>
	using vec_scalar_t = vec_scalar<std::remove_cvref_t<V>>::type;

	// get the derived type of a vec or swizzle_vec or vec_interface

	template <typename V>
	struct vec_derived;

	template <dimensional_scalar T, std::size_t S>
	struct vec_derived<vec<T, S>>
	{
		using type = vec<T, S>;
	};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct vec_derived<swizzle_vec<T, Size, Count, Is...>>
	{
		using type = swizzle_vec<T, Size, Count, Is...>;
	};

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	struct vec_derived<vec_interface<W, T, C, D>>
	{
		using type = D;
	};

	template <typename V>
	using vec_derived_t = vec_derived<std::remove_cvref_t<V>>::type;

	// get the number of accessible elements in a vec or swizzle_vec or vec_interface

	template <typename V>
	struct vec_size;

	template <dimensional_scalar T, std::size_t S>
	struct vec_size<vec<T, S>>
	{
		static constexpr std::size_t value = S;
	};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct vec_size<swizzle_vec<T, Size, Count, Is...>>
	{
		static constexpr std::size_t value = Count;
	};

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	struct vec_size<vec_interface<W, T, C, D>>
	{
		static constexpr std::size_t value = C;
	};

	template <typename V>
	inline constexpr std::size_t vec_size_v = vec_size<std::remove_cvref_t<V>>::value;

	// get whether V is allowed to be an l-value in a vec or swizzle_vec or vec_interface

	template <typename V>
	struct vec_writable : std::false_type {};

	template <dimensional_scalar T, std::size_t S>
	struct vec_writable<vec<T, S>> : std::true_type {};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct vec_writable<swizzle_vec<T, Size, Count, Is...>>
	{
		static constexpr bool value = writable_swizzle<Size, Count, Is...>;
	};

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	struct vec_writable<vec_interface<W, T, C, D>>
	{
		static constexpr bool value = W;
	};

	template <typename V>
	inline constexpr bool vec_writable_v = vec_writable<std::remove_cvref_t<V>>::value;

	// get whether V has swizzle data members

	template <typename V>
	struct vec_named_swizzle : std::false_type {};

	template <dimensional_scalar T, std::size_t S>
	struct vec_named_swizzle<vec<T, S>> : std::true_type {};

	template <typename V>
	inline constexpr bool vec_named_swizzle_v = vec_named_swizzle<std::remove_cvref_t<V>>::value;

	//
	// has_set: checks that V has a set() member callable with exactly vec_size_v<V> arguments of type vec_scalar_t<V>
	//

	template <typename V, typename Seq>
	struct has_set : std::false_type {};

	template <typename V, std::size_t... Is>
	struct has_set<V, std::index_sequence<Is...>>
	{
		static constexpr bool value = requires(V v, decltype((void(Is), vec_scalar_t<V>{}))... args)
		{
			v.set(args...);
		};
	};

	template <typename V>
	concept has_set_v = has_set<V, std::make_index_sequence<vec_size_v<V>>>::value;

	//
	// vec_like: read-only, vec-shaped types (vec, swizzle_vec, etc.)
	//

	template <typename V>
	concept vec_like = requires (const std::remove_cvref_t<V> cv, int i_int, std::size_t i_size)
	{
		typename vec_scalar_t<V>;
		typename vec_derived_t<V>;
		requires vec_dimension<vec_size_v<V>>;
		requires dimensional_scalar<vec_scalar_t<V>>;
		{ vec_writable_v<V> }				-> std::convertible_to<bool>;
		{ std::remove_cvref_t<V>::size() }	-> std::convertible_to<std::size_t>;

		// operator[] and at() for both index types
		{ cv[i_int] }		-> std::convertible_to<vec_scalar_t<V>>;
		{ cv[i_size] }		-> std::convertible_to<vec_scalar_t<V>>;
		{ cv.at(i_int) }	-> std::convertible_to<vec_scalar_t<V>>;
		{ cv.at(i_size) }	-> std::convertible_to<vec_scalar_t<V>>;

		// const iteration
		{ *cv.begin() }		-> std::convertible_to<vec_scalar_t<V>>;
		{ *cv.rbegin() }	-> std::convertible_to<vec_scalar_t<V>>;
		requires std::random_access_iterator<decltype(cv.begin())>;
		requires std::random_access_iterator<decltype(cv.rbegin())>;
	};

	//
	// writable_vec_like: vec_like types that also support mutation
	// via set() and non-const as_derived()
	//

	template <typename V>
	concept writable_vec_like = (!std::is_const_v<std::remove_reference_t<V>>) && vec_like<V> && has_set_v<V> && vec_writable_v<V> &&
	requires (std::remove_cvref_t<V> v, int i_int, std::size_t i_size)
	{
		{ v.as_derived() }	-> std::same_as<vec_derived_t<V>&>;

		// verifies non-const operator[] signature and at() signature for both index types
		{ v[i_int] }		-> std::same_as<vec_scalar_t<V>&>;
		{ v[i_size] }		-> std::same_as<vec_scalar_t<V>&>;
		{ v.at(i_int) }		-> std::same_as<vec_scalar_t<V>&>;
		{ v.at(i_size) }	-> std::same_as<vec_scalar_t<V>&>;

		{ *v.begin() }		-> std::same_as<vec_scalar_t<V>&>;
		{ *v.rbegin() }		-> std::same_as<vec_scalar_t<V>&>;
		requires std::random_access_iterator<decltype(v.begin())>;
		requires std::random_access_iterator<decltype(v.rbegin())>;
	};

	// checks if all the vec_like types have the same scalar type and size. This could be used in conjunction with
	// a type concept that makes sure all the types match a type constraint, like floating_point_scalar<scalar_t<V1>>.
	// we only need to check the first type for this, since all the types are the same if they pass this concept.
	template <typename V1, typename... Vs>
	concept same_vec_shape = vec_like<V1> && (vec_like<Vs> && ...) &&
							 ((std::same_as<vec_scalar_t<V1>, vec_scalar_t<Vs>> && (vec_size_v<V1> == vec_size_v<Vs>)) && ...);

	//
	// random-access iterators for swizzle_vec so they can participate in range-for loop amongst other things.
	// make sure that it doesn't out-live it's swizzle_vec or there will be a dangling pointer.
	//

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable<Size, Count, Is...>
	struct swizzle_vec_const_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = const T *;
		using reference = const T &;

		// range of valid values for mapper_index
		constexpr static std::ptrdiff_t begin_index = 0;
		constexpr static std::ptrdiff_t end_index = Count;

	private:

		// the data
		const swizzle_vec<T, Size, Count, Is ...> *mapper_ptr;
		std::ptrdiff_t mapper_index;

	public:

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr swizzle_vec_const_iterator(const swizzle_vec<T, Size, Count, Is ...> &mapper, int index)
			: mapper_ptr(std::addressof(mapper)), mapper_index(index)
		{
			// I don't want to throw from a constructor, but my hand is forced
			if ((mapper_index < begin_index) || (mapper_index > end_index))
			{
				[[ unlikely ]] throw std::out_of_range("index not in range");
			}
		}

		// default constructor for singular value, which is required by std::random_access_iterator.
		// it also removes the triviality of the struct/class.
		constexpr swizzle_vec_const_iterator() noexcept
			: mapper_ptr(nullptr), mapper_index(0)
		{
		}

		constexpr swizzle_vec_const_iterator(const swizzle_vec_const_iterator &) noexcept = default;
		constexpr swizzle_vec_const_iterator(swizzle_vec_const_iterator &&) noexcept = default;
		constexpr swizzle_vec_const_iterator &operator =(const swizzle_vec_const_iterator &) & noexcept = default;
		constexpr swizzle_vec_const_iterator &operator =(swizzle_vec_const_iterator &&) & noexcept = default;
		constexpr ~swizzle_vec_const_iterator() = default;

		[[nodiscard]] constexpr reference operator *() const
		{
			if (mapper_ptr == nullptr)
			{
				[[ unlikely ]] throw std::runtime_error("can't deref nullptr");
			}
			else if ((mapper_index < begin_index) || (mapper_index >= end_index))
			{
				[[ unlikely ]] throw std::out_of_range("index not in range");
			}

			[[ likely ]] return (*mapper_ptr)[mapper_index];
		}

		[[nodiscard]] constexpr pointer operator ->() const
		{
			if (mapper_ptr == nullptr)
			{
				[[ unlikely ]] throw std::runtime_error("can't deref nullptr");
			}
			else if ((mapper_index < begin_index) || (mapper_index >= end_index))
			{
				[[ unlikely ]] throw std::out_of_range("index not in range");
			}

			[[ likely ]] return std::addressof((*mapper_ptr)[mapper_index]);
		}

		constexpr swizzle_vec_const_iterator &operator ++()
		{
			if (mapper_index >= end_index)
			{
				[[ unlikely ]] throw std::runtime_error("don't increment past end_index");
			}

			[[ likely ]] ++mapper_index;
			return *this;
		}

		constexpr swizzle_vec_const_iterator operator ++(int)
		{
			if (mapper_index >= end_index)
			{
				[[ unlikely ]] throw std::runtime_error("don't increment past end_index");
			}

			swizzle_vec_const_iterator temp = *this;
			[[ likely ]] ++mapper_index;
			return temp;
		}

		constexpr swizzle_vec_const_iterator &operator --()
		{
			if (mapper_index <= begin_index)
			{
				[[ unlikely ]] throw std::runtime_error("don't decrement past begin_index");
			}

			[[ likely ]] --mapper_index;
			return *this;
		}

		constexpr swizzle_vec_const_iterator operator --(int)
		{
			if (mapper_index <= begin_index)
			{
				[[ unlikely ]] throw std::runtime_error("don't decrement past begin_index");
			}

			swizzle_vec_const_iterator temp = *this;
			[[ likely ]] --mapper_index;
			return temp;
		}

		constexpr swizzle_vec_const_iterator &operator +=(const int offset)
		{
			if (((mapper_index + offset) < begin_index) || ((mapper_index + offset) >= end_index))
			{
				[[ unlikely ]] throw std::out_of_range("offset not in range");
			}

			[[ likely ]] mapper_index += offset;
			return *this;
		}

		constexpr swizzle_vec_const_iterator &operator -=(const int offset)
		{
			if (((mapper_index - offset) < begin_index) || ((mapper_index - offset) >= end_index))
			{
				[[ unlikely ]] throw std::out_of_range("offset not in range");
			}

			[[ likely ]] mapper_index -= offset;
			return *this;
		}

		[[nodiscard]] constexpr int operator -(const swizzle_vec_const_iterator &iter) const
		{
			if (mapper_ptr != iter.mapper_ptr)
			{
				[[ unlikely ]] throw std::invalid_argument("different swizzle_vec source");
			}

			[[ likely ]] return static_cast<int>(mapper_index) - static_cast<int>(iter.mapper_index);
		}

		[[nodiscard]] constexpr bool operator ==(const swizzle_vec_const_iterator &iter) const
		{
			if (mapper_ptr != iter.mapper_ptr)
			{
				[[ unlikely ]] throw std::invalid_argument("different swizzle_vec source");
			}

			[[ likely ]] return ((mapper_ptr == iter.mapper_ptr) && (mapper_index == iter.mapper_index));
		}

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const swizzle_vec_const_iterator &iter) const
		{
			if (mapper_ptr != iter.mapper_ptr)
			{
				[[ unlikely ]] throw std::invalid_argument("different swizzle_vec source");
			}

			[[ likely ]] return mapper_index <=> iter.mapper_index;
		}

		[[nodiscard]] constexpr reference operator [](const int offset) const
		{
			if (mapper_ptr == nullptr)
			{
				[[ unlikely ]] throw std::runtime_error("can't deref nullptr");
			}
			else if (((mapper_index + offset) < begin_index) || ((mapper_index + offset) >= end_index))
			{
				[[ unlikely ]] throw std::out_of_range("index not in range");
			}

			[[ likely ]] return (*mapper_ptr)[mapper_index + offset];
		}

		[[nodiscard]] constexpr swizzle_vec_const_iterator operator +(const int offset) const
		{
			swizzle_vec_const_iterator temp = *this;
			temp += offset;
			return temp;
		}

		[[nodiscard]] constexpr swizzle_vec_const_iterator operator -(const int offset) const
		{
			swizzle_vec_const_iterator temp = *this;
			temp -= offset;
			return temp;
		}

		[[nodiscard]] friend constexpr swizzle_vec_const_iterator operator +(const int offset, swizzle_vec_const_iterator iter)
		{
			iter += offset;
			return iter;
		}

	};	// struct swizzle_vec_const_iterator

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable<Size, Count, Is...>
	struct swizzle_vec_iterator : swizzle_vec_const_iterator<T, Size, Count, Is...>
	{
		// let base class do all the work
		using base_iter = swizzle_vec_const_iterator<T, Size, Count, Is...>;

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
		constexpr swizzle_vec_iterator(swizzle_vec<T, Size, Count, Is ...> &mapper, int index)
			: base_iter(mapper, index)
		{
		}

		// default constructor for singular value, which is required by std::random_access_iterator
		constexpr swizzle_vec_iterator() noexcept
			: swizzle_vec_const_iterator<T, Size, Count, Is...>()
		{
		}

		constexpr swizzle_vec_iterator(const swizzle_vec_iterator &) noexcept = default;
		constexpr swizzle_vec_iterator(swizzle_vec_iterator &&) noexcept = default;
		constexpr swizzle_vec_iterator &operator =(const swizzle_vec_iterator &) & noexcept = default;
		constexpr swizzle_vec_iterator &operator =(swizzle_vec_iterator &&) & noexcept = default;
		constexpr ~swizzle_vec_iterator() = default;

		[[nodiscard]] constexpr reference operator *() const
		{
			return const_cast<reference>(base_iter::operator*());
		}

		[[nodiscard]] constexpr pointer operator ->() const
		{
			return const_cast<pointer>(base_iter::operator->());
		}

		constexpr swizzle_vec_iterator &operator ++()
		{
			base_iter::operator++();
			return *this;
		}

		constexpr swizzle_vec_iterator operator ++(int)
		{
			swizzle_vec_iterator temp = *this;
			base_iter::operator++();
			return temp;
		}

		constexpr swizzle_vec_iterator &operator --()
		{
			base_iter::operator--();
			return *this;
		}

		constexpr swizzle_vec_iterator operator --(int)
		{
			swizzle_vec_iterator temp = *this;
			base_iter::operator--();
			return temp;
		}

		constexpr swizzle_vec_iterator &operator +=(const int offset)
		{
			base_iter::operator+=(offset);
			return *this;
		}

		constexpr swizzle_vec_iterator &operator -=(const int offset)
		{
			base_iter::operator-=(offset);
			return *this;
		}

		[[nodiscard]] constexpr swizzle_vec_iterator operator +(const int offset) const
		{
			swizzle_vec_iterator temp = *this;
			temp += offset;
			return temp;
		}

		[[nodiscard]] friend constexpr swizzle_vec_iterator operator +(const int offset, swizzle_vec_iterator iter)
		{
			iter += offset;
			return iter;
		}

		using base_iter::operator-;

		[[nodiscard]] constexpr swizzle_vec_iterator operator -(const int offset) const
		{
			swizzle_vec_iterator temp = *this;
			temp -= offset;
			return temp;
		}

		[[nodiscard]] constexpr reference operator [](const int offset) const
		{
			return const_cast<reference>(base_iter::operator[](offset));
		}

	};	// struct swizzle_vec_iterator

	//
	// swizzle_vec - swizzle classes that are types of union members in vec
	//

	// for swizzling 1D-4D parts of vec
	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	requires indexable<Size, Count, Is...>
	struct swizzle_vec<T, Size, Count, Is...>
		: vec_interface<writable_swizzle<Size, Count, Is...>, T, Count, swizzle_vec<T, Size, Count, Is...>>
	{
		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue.
		static constexpr bool Writable = writable_swizzle<Size, Count, Is...>;

		//
		// the underlying ordered storage sequence for this logical vector - essential for indirection.
		//

		// as a parameter pack, e.g., <2, 2, 3, 1> is a swizzle for a vec<T, 4> v, i.e., v.zzwy. this is what the offsets array
		// is created from. the offsets array and sequence_pack are two differnt ways to map the logical order of the swizzle to
		// the physical order of the underlying storage. sequence_pack is just the template parameters pack Is... as a named type.
		using sequence_pack = std::index_sequence<Is...>;

		// this is the same data that is in sequence_pack, but as an array. both sequence_pack and offsets are different methods
		// for using indirection into the shared data storage used by all the swizzles in a vec, which is also shared with the
		// vec's storage through the union and the common initial sequence rule. it is used by operator[] to map its logical
		// order to the physical. this relies on sequence_pack for initialization.
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		// common initial sequence data. the storage is Size in length, not Count, which is number of indices. for example, a
		// vec<T, 4> named v has Size == 4, but a swizzle_vec like v.zwy has Count == 3. all the storage must be the same for
		// a vec<T, N> and its swizzle_vecs, even if it isn't used for a particular swizzle_vec, since the storage data is
		// shared with the vec and all the other swizzle_vecs in the union.
		dimensional_storage_t<T, Size> base;

		// using directives related to storage
		using value_type = T;
		using iterator = swizzle_vec_iterator<T, Size, Count, Is...>;
		using const_iterator = swizzle_vec_const_iterator<T, Size, Count, Is...>;
		using reverse_iterator = std::reverse_iterator<swizzle_vec_iterator<T, Size, Count, Is...>>;
		using const_reverse_iterator = std::reverse_iterator<swizzle_vec_const_iterator<T, Size, Count, Is...>>;

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr swizzle_vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept
		{
			set(other);
			return *this;
		}

		// scalar assignment
		// assignment for some scalar type that converts to T and is only for swizzle_vec of [Count == 1]
		template <dimensional_scalar U>
		requires Writable && implicitly_convertible_to<U, T> && (Count == 1)
		constexpr swizzle_vec &operator =(U other) & noexcept
		{
			set(other);
			return *this;
		}

		//
		// scalar conversion operators
		//

		// this is extremely important and is only for swizzle_vec of [Count == 1]
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

		// logically contiguous. data accessed through offsets indirection.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index) requires Writable
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[offsets[i]];
		}

		// logically contiguous. data accessed through offsets indirection.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[offsets[i]];
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index) noexcept requires Writable
		{
			return base.at(offsets.at(static_cast<std::size_t>(index)));
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const noexcept
		{
			return base.at(offsets.at(static_cast<std::size_t>(index)));
		}

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] static constexpr auto sequence() noexcept					{ return sequence_pack{}; }

		// support for range-for loop
		[[nodiscard]] constexpr		  iterator			begin() noexcept requires Writable		{ return iterator(*this, iterator::begin_index); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept					{ return const_iterator(*this, const_iterator::begin_index); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept					{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept requires Writable		{ return iterator(*this, iterator::end_index); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept					{ return const_iterator(*this, const_iterator::end_index); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept requires Writable		{ return reverse_iterator(end()); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept					{ return const_reverse_iterator(end()); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept requires Writable		{ return reverse_iterator(begin()); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept					{ return const_reverse_iterator(begin()); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept					{ return rend(); }

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ... Args>
		requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
		constexpr void set(Args ...args) noexcept
		{
			// these Is.. are likely not sequential since they are in logical order,
			// and we are accessing the internal storage directly. we are not using
			// the offsets array indirection that is built into operator []() for
			// this function, nor are we using sequence_pack{}. we are using the
			// Is... directly as indices into the storage since this is a member
			// function of swizzle_vec, and it has easy access to the Is... pack.
			((base.at(Is) = static_cast<T>(args)), ...);
		}

		// this set() uses the other set() to do the work. it is for setting our values
		// from another vec_interface, which could be a swizzle_vec or a vec. it allows
		// for self-assignment without aliasing issues.
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && std::convertible_to<U, T>
		constexpr void set(const vec_interface<W, U, Count, D> &other) & noexcept
		{
			// the Js... parameter pack is just <0, 1, 2, ..., Count - 1> as indices into
			// the other's logical storage. it is unrelated to the Is... parameter pack.
			[this, &other] <std::size_t ...Js>(std::index_sequence<Js ...>) noexcept
			{
				this->set(other[Js]...);
			}(std::make_index_sequence<Count>{});
		}

	};	// struct swizzle_vec

	namespace dex
	{
		//
		// convenience using types for swizzle_vec as members of vec
		//

		template <dimensional_scalar T, std::size_t Size, std::size_t I>
		requires indexable<Size, 1, I>
		using dexvec1 = swizzle_vec<T, Size, 1, I>;

		template <dimensional_scalar T, std::size_t Size, std::size_t ...Is>
		requires indexable<Size, 2, Is...>
		using dexvec2 = swizzle_vec<T, Size, 2, Is...>;

		template <dimensional_scalar T, std::size_t Size, std::size_t ...Is>
		requires indexable<Size, 3, Is...>
		using dexvec3 = swizzle_vec<T, Size, 3, Is...>;

		template <dimensional_scalar T, std::size_t Size, std::size_t ...Is>
		requires indexable<Size, 4, Is...>
		using dexvec4 = swizzle_vec<T, Size, 4, Is...>;

	}	// namespace dex

	//
	// mat will act as the primary matrix class in this library.
	//
	// T is the type of the elements stored in the matrix
	// C is the number of elements in a column
	// R is the number of elements in a row
	//

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	requires mat_dimension<C> && mat_dimension<R>
	struct mat;

	//
	// this detail namespace provides support for variadic constructors for vec and mat
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
		struct component_count<vec<T, C>>
		{
			static constexpr std::size_t value = C;
		};

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		struct component_count<swizzle_vec<T, S, C, Is...>>
		{
			static constexpr std::size_t value = C;
		};

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		struct component_count<vec_interface<W, T, C, D>>
		{
			static constexpr std::size_t value = C;
		};

		template <floating_point_scalar T, std::size_t C, std::size_t R>
		struct component_count<mat<T, C, R>>
		{
			static constexpr std::size_t value = C * R;
		};

		// make sure Count and Args... are valid together w.r.t. component count.
		// Args is expected to be a combination of derived vec_interface classes and
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
		// Args is expected to be a combination of derived vec_interface classes and dimensional_scalars.
		//
		// "...there must be enough components provided in the arguments to provide an initializer for
		// every component in the constructed value. It is a compile-time error to provide extra
		// arguments beyond this last used argument." section 5.4.2 of the spec for constructors (use case for this).
		template <std::size_t Count, typename ...Args>
		requires (sizeof...(Args) > 0) && (Count > 0)
		struct component_match<Count, Args...>
		{
			// total number components in Args...
			static constexpr std::size_t total_count = (0 + ... + component_count<Args>::value);
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

		template <dimensional_scalar T, std::size_t S>
		constexpr auto to_tuple(const vec<T, S> &arg) noexcept
		{
			return [&arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple(arg[Is]...);
			}(std::make_index_sequence<S>{});
		}

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		constexpr auto to_tuple(const swizzle_vec<T, S, C, Is...> &arg) noexcept
		{
			return [&arg]<std::size_t ...Js>(std::index_sequence<Js...>) noexcept
			{
				return std::tuple(arg[Js]...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		constexpr auto to_tuple(const vec_interface<W, T, C, D> &arg) noexcept
		{
			return [&arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple(arg[Is]...);
			}(std::make_index_sequence<C>{});
		}

		// create a tuple from a matrix

		template <floating_point_scalar T, std::size_t C, std::size_t R>
		constexpr auto to_tuple(const mat<T, C, R> &arg) noexcept
		{
			return [&arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple_cat(to_tuple(arg[Is])...);
			}(std::make_index_sequence<C>{});
		}

		// flatten the Args out in a big tuple. Args is expected to be a combination of derived vec_interface classes
		// and dimensional_scalars, possibly a matrix (for vec).
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

		template <dimensional_scalar U, std::size_t S, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<vec<U, S>, T> : std::true_type
		{
		};

		template <dimensional_scalar U, std::size_t S, std::size_t C, std::size_t ...Is, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<swizzle_vec<U, S, C, Is...>, T> : std::true_type
		{
		};

		template <bool W, dimensional_scalar U, std::size_t C, typename D, floating_point_scalar T>
		requires std::convertible_to<U, T>
		struct valid_matrix_component<vec_interface<W, U, C, D>, T> : std::true_type
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
		struct valid_vector_component<vec<U, C>, T> : std::true_type
		{
		};

		template <dimensional_scalar U, std::size_t S, std::size_t C, std::size_t ...Is, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<swizzle_vec<U, S, C, Is...>, T> : std::true_type
		{
		};

		template <bool W, dimensional_scalar U, std::size_t C, typename D, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<vec_interface<W, U, C, D>, T> : std::true_type
		{
		};

		template <floating_point_scalar U, std::size_t C, std::size_t R, dimensional_scalar T>
		requires std::convertible_to<U, T>
		struct valid_vector_component<mat<U, C, R>, T> : std::true_type
		{
		};

	}	// namespace detail

	//
	// vec - the fundamental vector class
	//

	template <dimensional_scalar T>
	struct vec<T, 1> : vec_interface<true, T, 1, vec<T, 1>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 1;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		union
		{
			vec_storage<T, Size>					base;

			dex::dexvec1<T, Size, 0>				x;				// Writable

			dex::dexvec2<T, Size, 0, 0>				xx;

			dex::dexvec3<T, Size, 0, 0, 0>			xxx;

			dex::dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
		};

		// using directives related to storage
		using value_type = T;
		using iterator = vec_storage<T, Size>::iterator;
		using const_iterator = vec_storage<T, Size>::const_iterator;
		using reverse_iterator = vec_storage<T, Size>::reverse_iterator;
		using const_reverse_iterator = vec_storage<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr vec() noexcept = default;
		constexpr ~vec() noexcept = default;

		constexpr vec(const vec &) noexcept = default;
		constexpr vec(vec &&) noexcept = default;
		constexpr vec &operator =(const vec &) & noexcept = default;
		constexpr vec &operator =(vec &&) & noexcept = default;

		//
		// constructors
		//

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T>
		explicit(false) constexpr vec(const vec_interface<W, U, C, D> &other)
			: base{ static_cast<T>(other[0]) }
		{
		}

		template <typename U>
		requires std::convertible_to<U, T>
		explicit(!implicitly_convertible_to<U, T>) constexpr vec(U value) noexcept
			: base{ static_cast<T>(value) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr vec(const U &u, const Args & ...args) noexcept
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
		requires implicitly_convertible_to<U, T>
		constexpr vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept
		{
			set(other[0]);
			return *this;
		}

		template <typename U>
		requires implicitly_convertible_to<U, T>
		constexpr vec &operator =(U value) & noexcept
		{
			set(value);
			return *this;
		}

		//
		// scalar conversion operators
		//

		// this is extremely important and is only for vec of [Size == 1]
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

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index)
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index)											{ return base.at(index); }

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const								{ return base.at(index); }

		constexpr void swap(vec &bv) noexcept													{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr		  iterator			begin() noexcept						{ return base.begin(); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept					{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept							{ return base.end(); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept						{ return base.rbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept					{ return base.crbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept							{ return base.rend(); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename U>
		requires std::convertible_to<U, T>
		constexpr void set(U value) noexcept
		{
			base.set(value);
		}

	};	// struct vec<T, 1>

	template <dimensional_scalar T>
	struct vec<T, 2> : vec_interface<true, T, 2, vec<T, 2>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 2;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		union
		{
			vec_storage<T, Size>					base;

			dex::dexvec1<T, Size, 0>				x;				// Writable
			dex::dexvec1<T, Size, 1>				y;				// Writable

			dex::dexvec2<T, Size, 0, 0>				xx;
			dex::dexvec2<T, Size, 0, 1>				xy;				// Writable
			dex::dexvec2<T, Size, 1, 0>				yx;				// Writable
			dex::dexvec2<T, Size, 1, 1>				yy;

			dex::dexvec3<T, Size, 0, 0, 0>			xxx;
			dex::dexvec3<T, Size, 0, 0, 1>			xxy;
			dex::dexvec3<T, Size, 0, 1, 0>			xyx;
			dex::dexvec3<T, Size, 0, 1, 1>			xyy;
			dex::dexvec3<T, Size, 1, 0, 0>			yxx;
			dex::dexvec3<T, Size, 1, 0, 1>			yxy;
			dex::dexvec3<T, Size, 1, 1, 0>			yyx;
			dex::dexvec3<T, Size, 1, 1, 1>			yyy;

			dex::dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dex::dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dex::dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dex::dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dex::dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dex::dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dex::dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dex::dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dex::dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dex::dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dex::dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dex::dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dex::dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dex::dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dex::dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dex::dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
		};

		// using directives related to storage
		using value_type = T;
		using iterator = vec_storage<T, Size>::iterator;
		using const_iterator = vec_storage<T, Size>::const_iterator;
		using reverse_iterator = vec_storage<T, Size>::reverse_iterator;
		using const_reverse_iterator = vec_storage<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr vec() noexcept = default;
		constexpr ~vec() noexcept = default;

		constexpr vec(const vec &) noexcept = default;
		constexpr vec(vec &&) noexcept = default;
		constexpr vec &operator =(const vec &) & noexcept = default;
		constexpr vec &operator =(vec &&) & noexcept = default;

		//
		// constructors
		//

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr vec(U value) noexcept
			: base{ static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to <U2, T>
		explicit constexpr vec(U1 xvalue, U2 yvalue) noexcept
			: base{ static_cast<T>(xvalue), static_cast<T>(yvalue) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		explicit(false) constexpr vec(const vec_interface<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0]), static_cast<T>(other[1]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr vec(const U &u, const Args & ...args) noexcept
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
		requires implicitly_convertible_to<U, T>
		constexpr vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept
		{
			set(other[0], other[1]);
			return *this;
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index)
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index)											{ return base.at(index); }

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const								{ return base.at(index); }

		constexpr void swap(vec &bv) noexcept													{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr		  iterator			begin() noexcept						{ return base.begin(); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept					{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept							{ return base.end(); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept						{ return base.rbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept					{ return base.crbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept							{ return base.rend(); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
		constexpr void set(Args ...args) noexcept
		{
			base.set(args...);
		}

	};	// struct vec<T, 2>

	template <dimensional_scalar T>
	struct vec<T, 3> : vec_interface<true, T, 3, vec<T, 3>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 3;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		union
		{
			vec_storage<T, Size>					base;

			dex::dexvec1<T, Size, 0>				x;				// Writable
			dex::dexvec1<T, Size, 1>				y;				// Writable
			dex::dexvec1<T, Size, 2>				z;				// Writable

			dex::dexvec2<T, Size, 0, 0>				xx;
			dex::dexvec2<T, Size, 0, 1>				xy;				// Writable
			dex::dexvec2<T, Size, 0, 2>				xz;				// Writable
			dex::dexvec2<T, Size, 1, 0>				yx;				// Writable
			dex::dexvec2<T, Size, 1, 1>				yy;
			dex::dexvec2<T, Size, 1, 2>				yz;				// Writable
			dex::dexvec2<T, Size, 2, 0>				zx;				// Writable
			dex::dexvec2<T, Size, 2, 1>				zy;				// Writable
			dex::dexvec2<T, Size, 2, 2>				zz;

			dex::dexvec3<T, Size, 0, 0, 0>			xxx;
			dex::dexvec3<T, Size, 0, 0, 1>			xxy;
			dex::dexvec3<T, Size, 0, 0, 2>			xxz;
			dex::dexvec3<T, Size, 0, 1, 0>			xyx;
			dex::dexvec3<T, Size, 0, 1, 1>			xyy;
			dex::dexvec3<T, Size, 0, 1, 2>			xyz;			// Writable
			dex::dexvec3<T, Size, 0, 2, 0>			xzx;
			dex::dexvec3<T, Size, 0, 2, 1>			xzy;			// Writable
			dex::dexvec3<T, Size, 0, 2, 2>			xzz;
			dex::dexvec3<T, Size, 1, 0, 0>			yxx;
			dex::dexvec3<T, Size, 1, 0, 1>			yxy;
			dex::dexvec3<T, Size, 1, 0, 2>			yxz;			// Writable
			dex::dexvec3<T, Size, 1, 1, 0>			yyx;
			dex::dexvec3<T, Size, 1, 1, 1>			yyy;
			dex::dexvec3<T, Size, 1, 1, 2>			yyz;
			dex::dexvec3<T, Size, 1, 2, 0>			yzx;			// Writable
			dex::dexvec3<T, Size, 1, 2, 1>			yzy;
			dex::dexvec3<T, Size, 1, 2, 2>			yzz;
			dex::dexvec3<T, Size, 2, 0, 0>			zxx;
			dex::dexvec3<T, Size, 2, 0, 1>			zxy;			// Writable
			dex::dexvec3<T, Size, 2, 0, 2>			zxz;
			dex::dexvec3<T, Size, 2, 1, 0>			zyx;			// Writable
			dex::dexvec3<T, Size, 2, 1, 1>			zyy;
			dex::dexvec3<T, Size, 2, 1, 2>			zyz;
			dex::dexvec3<T, Size, 2, 2, 0>			zzx;
			dex::dexvec3<T, Size, 2, 2, 1>			zzy;
			dex::dexvec3<T, Size, 2, 2, 2>			zzz;

			dex::dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dex::dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dex::dexvec4<T, Size, 0, 0, 0, 2>		xxxz;
			dex::dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dex::dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dex::dexvec4<T, Size, 0, 0, 1, 2>		xxyz;
			dex::dexvec4<T, Size, 0, 0, 2, 0>		xxzx;
			dex::dexvec4<T, Size, 0, 0, 2, 1>		xxzy;
			dex::dexvec4<T, Size, 0, 0, 2, 2>		xxzz;
			dex::dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dex::dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dex::dexvec4<T, Size, 0, 1, 0, 2>		xyxz;
			dex::dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dex::dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dex::dexvec4<T, Size, 0, 1, 1, 2>		xyyz;
			dex::dexvec4<T, Size, 0, 1, 2, 0>		xyzx;
			dex::dexvec4<T, Size, 0, 1, 2, 1>		xyzy;
			dex::dexvec4<T, Size, 0, 1, 2, 2>		xyzz;
			dex::dexvec4<T, Size, 0, 2, 0, 0>		xzxx;
			dex::dexvec4<T, Size, 0, 2, 0, 1>		xzxy;
			dex::dexvec4<T, Size, 0, 2, 0, 2>		xzxz;
			dex::dexvec4<T, Size, 0, 2, 1, 0>		xzyx;
			dex::dexvec4<T, Size, 0, 2, 1, 1>		xzyy;
			dex::dexvec4<T, Size, 0, 2, 1, 2>		xzyz;
			dex::dexvec4<T, Size, 0, 2, 2, 0>		xzzx;
			dex::dexvec4<T, Size, 0, 2, 2, 1>		xzzy;
			dex::dexvec4<T, Size, 0, 2, 2, 2>		xzzz;
			dex::dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dex::dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dex::dexvec4<T, Size, 1, 0, 0, 2>		yxxz;
			dex::dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dex::dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dex::dexvec4<T, Size, 1, 0, 1, 2>		yxyz;
			dex::dexvec4<T, Size, 1, 0, 2, 0>		yxzx;
			dex::dexvec4<T, Size, 1, 0, 2, 1>		yxzy;
			dex::dexvec4<T, Size, 1, 0, 2, 2>		yxzz;
			dex::dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dex::dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dex::dexvec4<T, Size, 1, 1, 0, 2>		yyxz;
			dex::dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dex::dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
			dex::dexvec4<T, Size, 1, 1, 1, 2>		yyyz;
			dex::dexvec4<T, Size, 1, 1, 2, 0>		yyzx;
			dex::dexvec4<T, Size, 1, 1, 2, 1>		yyzy;
			dex::dexvec4<T, Size, 1, 1, 2, 2>		yyzz;
			dex::dexvec4<T, Size, 1, 2, 0, 0>		yzxx;
			dex::dexvec4<T, Size, 1, 2, 0, 1>		yzxy;
			dex::dexvec4<T, Size, 1, 2, 0, 2>		yzxz;
			dex::dexvec4<T, Size, 1, 2, 1, 0>		yzyx;
			dex::dexvec4<T, Size, 1, 2, 1, 1>		yzyy;
			dex::dexvec4<T, Size, 1, 2, 1, 2>		yzyz;
			dex::dexvec4<T, Size, 1, 2, 2, 0>		yzzx;
			dex::dexvec4<T, Size, 1, 2, 2, 1>		yzzy;
			dex::dexvec4<T, Size, 1, 2, 2, 2>		yzzz;
			dex::dexvec4<T, Size, 2, 0, 0, 0>		zxxx;
			dex::dexvec4<T, Size, 2, 0, 0, 1>		zxxy;
			dex::dexvec4<T, Size, 2, 0, 0, 2>		zxxz;
			dex::dexvec4<T, Size, 2, 0, 1, 0>		zxyx;
			dex::dexvec4<T, Size, 2, 0, 1, 1>		zxyy;
			dex::dexvec4<T, Size, 2, 0, 1, 2>		zxyz;
			dex::dexvec4<T, Size, 2, 0, 2, 0>		zxzx;
			dex::dexvec4<T, Size, 2, 0, 2, 1>		zxzy;
			dex::dexvec4<T, Size, 2, 0, 2, 2>		zxzz;
			dex::dexvec4<T, Size, 2, 1, 0, 0>		zyxx;
			dex::dexvec4<T, Size, 2, 1, 0, 1>		zyxy;
			dex::dexvec4<T, Size, 2, 1, 0, 2>		zyxz;
			dex::dexvec4<T, Size, 2, 1, 1, 0>		zyyx;
			dex::dexvec4<T, Size, 2, 1, 1, 1>		zyyy;
			dex::dexvec4<T, Size, 2, 1, 1, 2>		zyyz;
			dex::dexvec4<T, Size, 2, 1, 2, 0>		zyzx;
			dex::dexvec4<T, Size, 2, 1, 2, 1>		zyzy;
			dex::dexvec4<T, Size, 2, 1, 2, 2>		zyzz;
			dex::dexvec4<T, Size, 2, 2, 0, 0>		zzxx;
			dex::dexvec4<T, Size, 2, 2, 0, 1>		zzxy;
			dex::dexvec4<T, Size, 2, 2, 0, 2>		zzxz;
			dex::dexvec4<T, Size, 2, 2, 1, 0>		zzyx;
			dex::dexvec4<T, Size, 2, 2, 1, 1>		zzyy;
			dex::dexvec4<T, Size, 2, 2, 1, 2>		zzyz;
			dex::dexvec4<T, Size, 2, 2, 2, 0>		zzzx;
			dex::dexvec4<T, Size, 2, 2, 2, 1>		zzzy;
			dex::dexvec4<T, Size, 2, 2, 2, 2>		zzzz;
		};

		// using directives related to storage
		using value_type = T;
		using iterator = vec_storage<T, Size>::iterator;
		using const_iterator = vec_storage<T, Size>::const_iterator;
		using reverse_iterator = vec_storage<T, Size>::reverse_iterator;
		using const_reverse_iterator = vec_storage<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr vec() noexcept = default;
		constexpr ~vec() noexcept = default;

		constexpr vec(const vec &) noexcept = default;
		constexpr vec(vec &&) noexcept = default;
		constexpr vec &operator =(const vec &) & noexcept = default;
		constexpr vec &operator =(vec &&) & noexcept = default;

		//
		// constructors
		//

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr vec(U value) noexcept
			: base{ static_cast<T>(value), static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr vec(U1 xvalue,
							   U2 yvalue,
							   U3 zvalue) noexcept
			: base{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		explicit(false) constexpr vec(const vec_interface<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0]), static_cast<T>(other[1]), static_cast<T>(other[2]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr vec(const U &u, const Args & ...args) noexcept
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
		requires implicitly_convertible_to<U, T>
		constexpr vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept
		{
			set(other[0], other[1], other[2]);
			return *this;
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index)
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index)											{ return base.at(index); }

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const								{ return base.at(index); }

		constexpr void swap(vec &bv) noexcept													{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr		  iterator			begin() noexcept						{ return base.begin(); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept					{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept							{ return base.end(); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept						{ return base.rbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept					{ return base.crbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept							{ return base.rend(); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
		constexpr void set(Args ...args) noexcept
		{
			base.set(args...);
		}

	};	// struct vec<T, 3>

	template <dimensional_scalar T>
	struct vec<T, 4> : vec_interface<true, T, 4, vec<T, 4>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 4;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		union
		{
			vec_storage<T, Size>					base;

			dex::dexvec1<T, Size, 0>				x;				// Writable
			dex::dexvec1<T, Size, 1>				y;				// Writable
			dex::dexvec1<T, Size, 2>				z;				// Writable
			dex::dexvec1<T, Size, 3>				w;				// Writable

			dex::dexvec2<T, Size, 0, 0>				xx;
			dex::dexvec2<T, Size, 0, 1>				xy;				// Writable
			dex::dexvec2<T, Size, 0, 2>				xz;				// Writable
			dex::dexvec2<T, Size, 0, 3>				xw;				// Writable
			dex::dexvec2<T, Size, 1, 0>				yx;				// Writable
			dex::dexvec2<T, Size, 1, 1>				yy;
			dex::dexvec2<T, Size, 1, 2>				yz;				// Writable
			dex::dexvec2<T, Size, 1, 3>				yw;				// Writable
			dex::dexvec2<T, Size, 2, 0>				zx;				// Writable
			dex::dexvec2<T, Size, 2, 1>				zy;				// Writable
			dex::dexvec2<T, Size, 2, 2>				zz;
			dex::dexvec2<T, Size, 2, 3>				zw;				// Writable
			dex::dexvec2<T, Size, 3, 0>				wx;				// Writable
			dex::dexvec2<T, Size, 3, 1>				wy;				// Writable
			dex::dexvec2<T, Size, 3, 2>				wz;				// Writable
			dex::dexvec2<T, Size, 3, 3>				ww;

			dex::dexvec3<T, Size, 0, 0, 0>			xxx;
			dex::dexvec3<T, Size, 0, 0, 1>			xxy;
			dex::dexvec3<T, Size, 0, 0, 2>			xxz;
			dex::dexvec3<T, Size, 0, 0, 3>			xxw;
			dex::dexvec3<T, Size, 0, 1, 0>			xyx;
			dex::dexvec3<T, Size, 0, 1, 1>			xyy;
			dex::dexvec3<T, Size, 0, 1, 2>			xyz;			// Writable
			dex::dexvec3<T, Size, 0, 1, 3>			xyw;			// Writable
			dex::dexvec3<T, Size, 0, 2, 0>			xzx;
			dex::dexvec3<T, Size, 0, 2, 1>			xzy;			// Writable
			dex::dexvec3<T, Size, 0, 2, 2>			xzz;
			dex::dexvec3<T, Size, 0, 2, 3>			xzw;			// Writable
			dex::dexvec3<T, Size, 0, 3, 0>			xwx;
			dex::dexvec3<T, Size, 0, 3, 1>			xwy;			// Writable
			dex::dexvec3<T, Size, 0, 3, 2>			xwz;			// Writable
			dex::dexvec3<T, Size, 0, 3, 3>			xww;
			dex::dexvec3<T, Size, 1, 0, 0>			yxx;
			dex::dexvec3<T, Size, 1, 0, 1>			yxy;
			dex::dexvec3<T, Size, 1, 0, 2>			yxz;			// Writable
			dex::dexvec3<T, Size, 1, 0, 3>			yxw;			// Writable
			dex::dexvec3<T, Size, 1, 1, 0>			yyx;
			dex::dexvec3<T, Size, 1, 1, 1>			yyy;
			dex::dexvec3<T, Size, 1, 1, 2>			yyz;
			dex::dexvec3<T, Size, 1, 1, 3>			yyw;
			dex::dexvec3<T, Size, 1, 2, 0>			yzx;			// Writable
			dex::dexvec3<T, Size, 1, 2, 1>			yzy;
			dex::dexvec3<T, Size, 1, 2, 2>			yzz;
			dex::dexvec3<T, Size, 1, 2, 3>			yzw;			// Writable
			dex::dexvec3<T, Size, 1, 3, 0>			ywx;			// Writable
			dex::dexvec3<T, Size, 1, 3, 1>			ywy;
			dex::dexvec3<T, Size, 1, 3, 2>			ywz;			// Writable
			dex::dexvec3<T, Size, 1, 3, 3>			yww;
			dex::dexvec3<T, Size, 2, 0, 0>			zxx;
			dex::dexvec3<T, Size, 2, 0, 1>			zxy;			// Writable
			dex::dexvec3<T, Size, 2, 0, 2>			zxz;
			dex::dexvec3<T, Size, 2, 0, 3>			zxw;			// Writable
			dex::dexvec3<T, Size, 2, 1, 0>			zyx;			// Writable
			dex::dexvec3<T, Size, 2, 1, 1>			zyy;
			dex::dexvec3<T, Size, 2, 1, 2>			zyz;
			dex::dexvec3<T, Size, 2, 1, 3>			zyw;			// Writable
			dex::dexvec3<T, Size, 2, 2, 0>			zzx;
			dex::dexvec3<T, Size, 2, 2, 1>			zzy;
			dex::dexvec3<T, Size, 2, 2, 2>			zzz;
			dex::dexvec3<T, Size, 2, 2, 3>			zzw;
			dex::dexvec3<T, Size, 2, 3, 0>			zwx;			// Writable
			dex::dexvec3<T, Size, 2, 3, 1>			zwy;			// Writable
			dex::dexvec3<T, Size, 2, 3, 2>			zwz;
			dex::dexvec3<T, Size, 2, 3, 3>			zww;
			dex::dexvec3<T, Size, 3, 0, 0>			wxx;
			dex::dexvec3<T, Size, 3, 0, 1>			wxy;			// Writable
			dex::dexvec3<T, Size, 3, 0, 2>			wxz;			// Writable
			dex::dexvec3<T, Size, 3, 0, 3>			wxw;
			dex::dexvec3<T, Size, 3, 1, 0>			wyx;			// Writable
			dex::dexvec3<T, Size, 3, 1, 1>			wyy;
			dex::dexvec3<T, Size, 3, 1, 2>			wyz;			// Writable
			dex::dexvec3<T, Size, 3, 1, 3>			wyw;
			dex::dexvec3<T, Size, 3, 2, 0>			wzx;			// Writable
			dex::dexvec3<T, Size, 3, 2, 1>			wzy;
			dex::dexvec3<T, Size, 3, 2, 2>			wzz;			// Writable
			dex::dexvec3<T, Size, 3, 2, 3>			wzw;
			dex::dexvec3<T, Size, 3, 3, 0>			wwx;
			dex::dexvec3<T, Size, 3, 3, 1>			wwy;
			dex::dexvec3<T, Size, 3, 3, 2>			wwz;
			dex::dexvec3<T, Size, 3, 3, 3>			www;

			dex::dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dex::dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dex::dexvec4<T, Size, 0, 0, 0, 2>		xxxz;
			dex::dexvec4<T, Size, 0, 0, 0, 3>		xxxw;
			dex::dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dex::dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dex::dexvec4<T, Size, 0, 0, 1, 2>		xxyz;
			dex::dexvec4<T, Size, 0, 0, 1, 3>		xxyw;
			dex::dexvec4<T, Size, 0, 0, 2, 0>		xxzx;
			dex::dexvec4<T, Size, 0, 0, 2, 1>		xxzy;
			dex::dexvec4<T, Size, 0, 0, 2, 2>		xxzz;
			dex::dexvec4<T, Size, 0, 0, 2, 3>		xxzw;
			dex::dexvec4<T, Size, 0, 0, 3, 0>		xxwx;
			dex::dexvec4<T, Size, 0, 0, 3, 1>		xxwy;
			dex::dexvec4<T, Size, 0, 0, 3, 2>		xxwz;
			dex::dexvec4<T, Size, 0, 0, 3, 3>		xxww;
			dex::dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dex::dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dex::dexvec4<T, Size, 0, 1, 0, 2>		xyxz;
			dex::dexvec4<T, Size, 0, 1, 0, 3>		xyxw;
			dex::dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dex::dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dex::dexvec4<T, Size, 0, 1, 1, 2>		xyyz;
			dex::dexvec4<T, Size, 0, 1, 1, 3>		xyyw;
			dex::dexvec4<T, Size, 0, 1, 2, 0>		xyzx;
			dex::dexvec4<T, Size, 0, 1, 2, 1>		xyzy;
			dex::dexvec4<T, Size, 0, 1, 2, 2>		xyzz;
			dex::dexvec4<T, Size, 0, 1, 2, 3>		xyzw;			// Writable
			dex::dexvec4<T, Size, 0, 1, 3, 0>		xywx;
			dex::dexvec4<T, Size, 0, 1, 3, 1>		xywy;
			dex::dexvec4<T, Size, 0, 1, 3, 2>		xywz;			// Writable
			dex::dexvec4<T, Size, 0, 1, 3, 3>		xyww;
			dex::dexvec4<T, Size, 0, 2, 0, 0>		xzxx;
			dex::dexvec4<T, Size, 0, 2, 0, 1>		xzxy;
			dex::dexvec4<T, Size, 0, 2, 0, 2>		xzxz;
			dex::dexvec4<T, Size, 0, 2, 0, 3>		xzxw;
			dex::dexvec4<T, Size, 0, 2, 1, 0>		xzyx;
			dex::dexvec4<T, Size, 0, 2, 1, 1>		xzyy;
			dex::dexvec4<T, Size, 0, 2, 1, 2>		xzyz;
			dex::dexvec4<T, Size, 0, 2, 1, 3>		xzyw;			// Writable
			dex::dexvec4<T, Size, 0, 2, 2, 0>		xzzx;
			dex::dexvec4<T, Size, 0, 2, 2, 1>		xzzy;
			dex::dexvec4<T, Size, 0, 2, 2, 2>		xzzz;
			dex::dexvec4<T, Size, 0, 2, 2, 3>		xzzw;
			dex::dexvec4<T, Size, 0, 2, 3, 0>		xzwx;
			dex::dexvec4<T, Size, 0, 2, 3, 1>		xzwy;			// Writable
			dex::dexvec4<T, Size, 0, 2, 3, 2>		xzwz;
			dex::dexvec4<T, Size, 0, 2, 3, 3>		xzww;
			dex::dexvec4<T, Size, 0, 3, 0, 0>		xwxx;
			dex::dexvec4<T, Size, 0, 3, 0, 1>		xwxy;
			dex::dexvec4<T, Size, 0, 3, 0, 2>		xwxz;
			dex::dexvec4<T, Size, 0, 3, 0, 3>		xwxw;
			dex::dexvec4<T, Size, 0, 3, 1, 0>		xwyx;
			dex::dexvec4<T, Size, 0, 3, 1, 1>		xwyy;
			dex::dexvec4<T, Size, 0, 3, 1, 2>		xwyz;			// Writable
			dex::dexvec4<T, Size, 0, 3, 1, 3>		xwyw;
			dex::dexvec4<T, Size, 0, 3, 2, 0>		xwzx;
			dex::dexvec4<T, Size, 0, 3, 2, 1>		xwzy;			// Writable
			dex::dexvec4<T, Size, 0, 3, 2, 2>		xwzz;
			dex::dexvec4<T, Size, 0, 3, 2, 3>		xwzw;
			dex::dexvec4<T, Size, 0, 3, 3, 0>		xwwx;
			dex::dexvec4<T, Size, 0, 3, 3, 1>		xwwy;
			dex::dexvec4<T, Size, 0, 3, 3, 2>		xwwz;
			dex::dexvec4<T, Size, 0, 3, 3, 3>		xwww;
			dex::dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dex::dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dex::dexvec4<T, Size, 1, 0, 0, 2>		yxxz;
			dex::dexvec4<T, Size, 1, 0, 0, 3>		yxxw;
			dex::dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dex::dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dex::dexvec4<T, Size, 1, 0, 1, 2>		yxyz;
			dex::dexvec4<T, Size, 1, 0, 1, 3>		yxyw;
			dex::dexvec4<T, Size, 1, 0, 2, 0>		yxzx;
			dex::dexvec4<T, Size, 1, 0, 2, 1>		yxzy;
			dex::dexvec4<T, Size, 1, 0, 2, 2>		yxzz;
			dex::dexvec4<T, Size, 1, 0, 2, 3>		yxzw;			// Writable
			dex::dexvec4<T, Size, 1, 0, 3, 0>		yxwx;
			dex::dexvec4<T, Size, 1, 0, 3, 1>		yxwy;
			dex::dexvec4<T, Size, 1, 0, 3, 2>		yxwz;			// Writable
			dex::dexvec4<T, Size, 1, 0, 3, 3>		yxww;
			dex::dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dex::dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dex::dexvec4<T, Size, 1, 1, 0, 2>		yyxz;
			dex::dexvec4<T, Size, 1, 1, 0, 3>		yyxw;
			dex::dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dex::dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
			dex::dexvec4<T, Size, 1, 1, 1, 2>		yyyz;
			dex::dexvec4<T, Size, 1, 1, 1, 3>		yyyw;
			dex::dexvec4<T, Size, 1, 1, 2, 0>		yyzx;
			dex::dexvec4<T, Size, 1, 1, 2, 1>		yyzy;
			dex::dexvec4<T, Size, 1, 1, 2, 2>		yyzz;
			dex::dexvec4<T, Size, 1, 1, 2, 3>		yyzw;
			dex::dexvec4<T, Size, 1, 1, 3, 0>		yywx;
			dex::dexvec4<T, Size, 1, 1, 3, 1>		yywy;
			dex::dexvec4<T, Size, 1, 1, 3, 2>		yywz;
			dex::dexvec4<T, Size, 1, 1, 3, 3>		yyww;
			dex::dexvec4<T, Size, 1, 2, 0, 0>		yzxx;
			dex::dexvec4<T, Size, 1, 2, 0, 1>		yzxy;
			dex::dexvec4<T, Size, 1, 2, 0, 2>		yzxz;
			dex::dexvec4<T, Size, 1, 2, 0, 3>		yzxw;			// Writable
			dex::dexvec4<T, Size, 1, 2, 1, 0>		yzyx;
			dex::dexvec4<T, Size, 1, 2, 1, 1>		yzyy;
			dex::dexvec4<T, Size, 1, 2, 1, 2>		yzyz;
			dex::dexvec4<T, Size, 1, 2, 1, 3>		yzyw;
			dex::dexvec4<T, Size, 1, 2, 2, 0>		yzzx;
			dex::dexvec4<T, Size, 1, 2, 2, 1>		yzzy;
			dex::dexvec4<T, Size, 1, 2, 2, 2>		yzzz;
			dex::dexvec4<T, Size, 1, 2, 2, 3>		yzzw;
			dex::dexvec4<T, Size, 1, 2, 3, 0>		yzwx;			// Writable
			dex::dexvec4<T, Size, 1, 2, 3, 1>		yzwy;
			dex::dexvec4<T, Size, 1, 2, 3, 2>		yzwz;
			dex::dexvec4<T, Size, 1, 2, 3, 3>		yzww;
			dex::dexvec4<T, Size, 1, 3, 0, 0>		ywxx;
			dex::dexvec4<T, Size, 1, 3, 0, 1>		ywxy;
			dex::dexvec4<T, Size, 1, 3, 0, 2>		ywxz;			// Writable
			dex::dexvec4<T, Size, 1, 3, 0, 3>		ywxw;
			dex::dexvec4<T, Size, 1, 3, 1, 0>		ywyx;
			dex::dexvec4<T, Size, 1, 3, 1, 1>		ywyy;
			dex::dexvec4<T, Size, 1, 3, 1, 2>		ywyz;
			dex::dexvec4<T, Size, 1, 3, 1, 3>		ywyw;
			dex::dexvec4<T, Size, 1, 3, 2, 0>		ywzx;			// Writable
			dex::dexvec4<T, Size, 1, 3, 2, 1>		ywzy;
			dex::dexvec4<T, Size, 1, 3, 2, 2>		ywzz;
			dex::dexvec4<T, Size, 1, 3, 2, 3>		ywzw;
			dex::dexvec4<T, Size, 1, 3, 3, 0>		ywwx;
			dex::dexvec4<T, Size, 1, 3, 3, 1>		ywwy;
			dex::dexvec4<T, Size, 1, 3, 3, 2>		ywwz;
			dex::dexvec4<T, Size, 1, 3, 3, 3>		ywww;
			dex::dexvec4<T, Size, 2, 0, 0, 0>		zxxx;
			dex::dexvec4<T, Size, 2, 0, 0, 1>		zxxy;
			dex::dexvec4<T, Size, 2, 0, 0, 2>		zxxz;
			dex::dexvec4<T, Size, 2, 0, 0, 3>		zxxw;
			dex::dexvec4<T, Size, 2, 0, 1, 0>		zxyx;
			dex::dexvec4<T, Size, 2, 0, 1, 1>		zxyy;
			dex::dexvec4<T, Size, 2, 0, 1, 2>		zxyz;
			dex::dexvec4<T, Size, 2, 0, 1, 3>		zxyw;			// Writable
			dex::dexvec4<T, Size, 2, 0, 2, 0>		zxzx;
			dex::dexvec4<T, Size, 2, 0, 2, 1>		zxzy;
			dex::dexvec4<T, Size, 2, 0, 2, 2>		zxzz;
			dex::dexvec4<T, Size, 2, 0, 2, 3>		zxzw;
			dex::dexvec4<T, Size, 2, 0, 3, 0>		zxwx;
			dex::dexvec4<T, Size, 2, 0, 3, 1>		zxwy;			// Writable
			dex::dexvec4<T, Size, 2, 0, 3, 2>		zxwz;
			dex::dexvec4<T, Size, 2, 0, 3, 3>		zxww;
			dex::dexvec4<T, Size, 2, 1, 0, 0>		zyxx;
			dex::dexvec4<T, Size, 2, 1, 0, 1>		zyxy;
			dex::dexvec4<T, Size, 2, 1, 0, 2>		zyxz;
			dex::dexvec4<T, Size, 2, 1, 0, 3>		zyxw;			// Writable
			dex::dexvec4<T, Size, 2, 1, 1, 0>		zyyx;
			dex::dexvec4<T, Size, 2, 1, 1, 1>		zyyy;
			dex::dexvec4<T, Size, 2, 1, 1, 2>		zyyz;
			dex::dexvec4<T, Size, 2, 1, 1, 3>		zyyw;
			dex::dexvec4<T, Size, 2, 1, 2, 0>		zyzx;
			dex::dexvec4<T, Size, 2, 1, 2, 1>		zyzy;
			dex::dexvec4<T, Size, 2, 1, 2, 2>		zyzz;
			dex::dexvec4<T, Size, 2, 1, 2, 3>		zyzw;
			dex::dexvec4<T, Size, 2, 1, 3, 0>		zywx;			// Writable
			dex::dexvec4<T, Size, 2, 1, 3, 1>		zywy;
			dex::dexvec4<T, Size, 2, 1, 3, 2>		zywz;
			dex::dexvec4<T, Size, 2, 1, 3, 3>		zyww;
			dex::dexvec4<T, Size, 2, 2, 0, 0>		zzxx;
			dex::dexvec4<T, Size, 2, 2, 0, 1>		zzxy;
			dex::dexvec4<T, Size, 2, 2, 0, 2>		zzxz;
			dex::dexvec4<T, Size, 2, 2, 0, 3>		zzxw;
			dex::dexvec4<T, Size, 2, 2, 1, 0>		zzyx;
			dex::dexvec4<T, Size, 2, 2, 1, 1>		zzyy;
			dex::dexvec4<T, Size, 2, 2, 1, 2>		zzyz;
			dex::dexvec4<T, Size, 2, 2, 1, 3>		zzyw;
			dex::dexvec4<T, Size, 2, 2, 2, 0>		zzzx;
			dex::dexvec4<T, Size, 2, 2, 2, 1>		zzzy;
			dex::dexvec4<T, Size, 2, 2, 2, 2>		zzzz;
			dex::dexvec4<T, Size, 2, 2, 2, 3>		zzzw;
			dex::dexvec4<T, Size, 2, 2, 3, 0>		zzwx;
			dex::dexvec4<T, Size, 2, 2, 3, 1>		zzwy;
			dex::dexvec4<T, Size, 2, 2, 3, 2>		zzwz;
			dex::dexvec4<T, Size, 2, 2, 3, 3>		zzww;
			dex::dexvec4<T, Size, 2, 3, 0, 0>		zwxx;
			dex::dexvec4<T, Size, 2, 3, 0, 1>		zwxy;			// Writable
			dex::dexvec4<T, Size, 2, 3, 0, 2>		zwxz;
			dex::dexvec4<T, Size, 2, 3, 0, 3>		zwxw;
			dex::dexvec4<T, Size, 2, 3, 1, 0>		zwyx;			// Writable
			dex::dexvec4<T, Size, 2, 3, 1, 1>		zwyy;
			dex::dexvec4<T, Size, 2, 3, 1, 2>		zwyz;
			dex::dexvec4<T, Size, 2, 3, 1, 3>		zwyw;
			dex::dexvec4<T, Size, 2, 3, 2, 0>		zwzx;
			dex::dexvec4<T, Size, 2, 3, 2, 1>		zwzy;
			dex::dexvec4<T, Size, 2, 3, 2, 2>		zwzz;
			dex::dexvec4<T, Size, 2, 3, 2, 3>		zwzw;
			dex::dexvec4<T, Size, 2, 3, 3, 0>		zwwx;
			dex::dexvec4<T, Size, 2, 3, 3, 1>		zwwy;
			dex::dexvec4<T, Size, 2, 3, 3, 2>		zwwz;
			dex::dexvec4<T, Size, 2, 3, 3, 3>		zwww;
			dex::dexvec4<T, Size, 3, 0, 0, 0>		wxxx;
			dex::dexvec4<T, Size, 3, 0, 0, 1>		wxxy;
			dex::dexvec4<T, Size, 3, 0, 0, 2>		wxxz;
			dex::dexvec4<T, Size, 3, 0, 0, 3>		wxxw;
			dex::dexvec4<T, Size, 3, 0, 1, 0>		wxyx;
			dex::dexvec4<T, Size, 3, 0, 1, 1>		wxyy;
			dex::dexvec4<T, Size, 3, 0, 1, 2>		wxyz;			// Writable
			dex::dexvec4<T, Size, 3, 0, 1, 3>		wxyw;
			dex::dexvec4<T, Size, 3, 0, 2, 0>		wxzx;
			dex::dexvec4<T, Size, 3, 0, 2, 1>		wxzy;			// Writable
			dex::dexvec4<T, Size, 3, 0, 2, 2>		wxzz;
			dex::dexvec4<T, Size, 3, 0, 2, 3>		wxzw;
			dex::dexvec4<T, Size, 3, 0, 3, 0>		wxwx;
			dex::dexvec4<T, Size, 3, 0, 3, 1>		wxwy;
			dex::dexvec4<T, Size, 3, 0, 3, 2>		wxwz;
			dex::dexvec4<T, Size, 3, 0, 3, 3>		wxww;
			dex::dexvec4<T, Size, 3, 1, 0, 0>		wyxx;
			dex::dexvec4<T, Size, 3, 1, 0, 1>		wyxy;
			dex::dexvec4<T, Size, 3, 1, 0, 2>		wyxz;			// Writable
			dex::dexvec4<T, Size, 3, 1, 0, 3>		wyxw;
			dex::dexvec4<T, Size, 3, 1, 1, 0>		wyyx;
			dex::dexvec4<T, Size, 3, 1, 1, 1>		wyyy;
			dex::dexvec4<T, Size, 3, 1, 1, 2>		wyyz;
			dex::dexvec4<T, Size, 3, 1, 1, 3>		wyyw;
			dex::dexvec4<T, Size, 3, 1, 2, 0>		wyzx;			// Writable
			dex::dexvec4<T, Size, 3, 1, 2, 1>		wyzy;
			dex::dexvec4<T, Size, 3, 1, 2, 2>		wyzz;
			dex::dexvec4<T, Size, 3, 1, 2, 3>		wyzw;
			dex::dexvec4<T, Size, 3, 1, 3, 0>		wywx;
			dex::dexvec4<T, Size, 3, 1, 3, 1>		wywy;
			dex::dexvec4<T, Size, 3, 1, 3, 2>		wywz;
			dex::dexvec4<T, Size, 3, 1, 3, 3>		wyww;
			dex::dexvec4<T, Size, 3, 2, 0, 0>		wzxx;
			dex::dexvec4<T, Size, 3, 2, 0, 1>		wzxy;			// Writable
			dex::dexvec4<T, Size, 3, 2, 0, 2>		wzxz;
			dex::dexvec4<T, Size, 3, 2, 0, 3>		wzxw;
			dex::dexvec4<T, Size, 3, 2, 1, 0>		wzyx;			// Writable
			dex::dexvec4<T, Size, 3, 2, 1, 1>		wzyy;
			dex::dexvec4<T, Size, 3, 2, 1, 2>		wzyz;
			dex::dexvec4<T, Size, 3, 2, 1, 3>		wzyw;
			dex::dexvec4<T, Size, 3, 2, 2, 0>		wzzx;
			dex::dexvec4<T, Size, 3, 2, 2, 1>		wzzy;
			dex::dexvec4<T, Size, 3, 2, 2, 2>		wzzz;
			dex::dexvec4<T, Size, 3, 2, 2, 3>		wzzw;
			dex::dexvec4<T, Size, 3, 2, 3, 0>		wzwx;
			dex::dexvec4<T, Size, 3, 2, 3, 1>		wzwy;
			dex::dexvec4<T, Size, 3, 2, 3, 2>		wzwz;
			dex::dexvec4<T, Size, 3, 2, 3, 3>		wzww;
			dex::dexvec4<T, Size, 3, 3, 0, 0>		wwxx;
			dex::dexvec4<T, Size, 3, 3, 0, 1>		wwxy;
			dex::dexvec4<T, Size, 3, 3, 0, 2>		wwxz;
			dex::dexvec4<T, Size, 3, 3, 0, 3>		wwxw;
			dex::dexvec4<T, Size, 3, 3, 1, 0>		wwyx;
			dex::dexvec4<T, Size, 3, 3, 1, 1>		wwyy;
			dex::dexvec4<T, Size, 3, 3, 1, 2>		wwyz;
			dex::dexvec4<T, Size, 3, 3, 1, 3>		wwyw;
			dex::dexvec4<T, Size, 3, 3, 2, 0>		wwzx;
			dex::dexvec4<T, Size, 3, 3, 2, 1>		wwzy;
			dex::dexvec4<T, Size, 3, 3, 2, 2>		wwzz;
			dex::dexvec4<T, Size, 3, 3, 2, 3>		wwzw;
			dex::dexvec4<T, Size, 3, 3, 3, 0>		wwwx;
			dex::dexvec4<T, Size, 3, 3, 3, 1>		wwwy;
			dex::dexvec4<T, Size, 3, 3, 3, 2>		wwwz;
			dex::dexvec4<T, Size, 3, 3, 3, 3>		wwww;
		};

		// using directives related to storage
		using value_type = T;
		using iterator = vec_storage<T, Size>::iterator;
		using const_iterator = vec_storage<T, Size>::const_iterator;
		using reverse_iterator = vec_storage<T, Size>::reverse_iterator;
		using const_reverse_iterator = vec_storage<T, Size>::const_reverse_iterator;

		//
		// defaulted functions
		//

		constexpr vec() noexcept = default;
		constexpr ~vec() noexcept = default;

		constexpr vec(const vec &) noexcept = default;
		constexpr vec(vec &&) noexcept = default;
		constexpr vec &operator =(const vec &) & noexcept = default;
		constexpr vec &operator =(vec &&) & noexcept = default;

		//
		// constructors
		//

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr vec(U value) noexcept
			: base{ static_cast<T>(value), static_cast<T>(value), static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2, typename U3, typename U4>
		requires
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		explicit constexpr vec(U1 xvalue,
							   U2 yvalue,
							   U3 zvalue,
							   U4 wvalue) noexcept
			: base{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue), static_cast<T>(wvalue) }
		{
		}

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		explicit(false) constexpr vec(const vec_interface<W, U, Count, D> &other) noexcept
			: base{ static_cast<T>(other[0]), static_cast<T>(other[1]), static_cast<T>(other[2]), static_cast<T>(other[3]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
		explicit constexpr vec(const U &u, const Args & ...args) noexcept
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
		requires implicitly_convertible_to<U, T>
		constexpr vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept
		{
			set(other[0], other[1], other[2], other[3]);
			return *this;
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &operator [](const U &index)
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &operator [](const U &index) const
		{
			std::size_t i = static_cast<std::size_t>(index);
			DSGA_ASSERT((i < Count), "index is out of bounds");
			return base[i];
		}

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr T &at(const U &index)											{ return base.at(index); }

		// logically and physically contiguous - rely on vec_storage<T, Size>::at() to check for out of bounds access.
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const T &at(const U &index) const								{ return base.at(index); }

		constexpr void swap(vec &bv) noexcept													{ base.swap(bv.base); }

		// support for range-for loop
		[[nodiscard]] constexpr		  iterator			begin() noexcept						{ return base.begin(); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept					{ return base.cbegin(); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept					{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept							{ return base.end(); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept					{ return base.cend(); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept					{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept						{ return base.rbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept					{ return base.crbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept				{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept							{ return base.rend(); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept					{ return base.crend(); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept					{ return rend(); }

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment without aliasing issues
		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
		constexpr void set(Args ...args) noexcept
		{
			base.set(args...);
		}

	};	// struct vec<T, 4>

	// swap specialization
	template <dimensional_scalar T, std::size_t Size>
	constexpr void swap(vec<T, Size> &lhs, vec<T, Size> &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	//
	// CTAD deduction guides for vec
	//

	template <dimensional_scalar T, dimensional_scalar ...U>
	vec(T, U...) -> vec<T, 1 + sizeof...(U)>;

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	vec(const vec_interface<W, T, C, D> &) -> vec<T, C>;

	//
	// machinery for vector operators and functions
	//

	namespace machinery
	{
		// we want to treat a vector of length 1 as a scalar, but processing machinery works on vectors, so this
		// is a type trait so we can have the same templated type to work on without special-casing for a vector
		// of length 1. This is useful for functions that want to return a vector or scalar depending on the size
		// of the vector.

		template <typename T, std::size_t C>
		struct vec_or_scalar
		{
			using type = vec<T, C>;
		};

		template <typename T>
		struct vec_or_scalar<T, 1>
		{
			using type = T;
		};

		template <typename T, std::size_t C>
		using vec_or_scalar_t = typename vec_or_scalar<T, C>::type;

		// return types from executing callables (lambdas) on arguments of various types

		template <typename UnOp, dimensional_scalar T>
		using unop_return_t = std::invoke_result_t<UnOp, T>;

		template <typename BinOp, dimensional_scalar T, dimensional_scalar U>
		using binop_return_t = std::invoke_result_t<BinOp, T, U>;

		template <typename TernOp, dimensional_scalar T, dimensional_scalar U, dimensional_scalar V>
		using ternop_return_t = std::invoke_result_t<TernOp, T, U, V>;

		// this machinery relies on vec_like or writable_vec_like operator[] to be a logically contiguous operation
		// on a vector type, regardless of whether it is physically contiguous. apply the operation on components of
		// vec_like or writable_vec_like type arguments, either returning a new vector (or scalar) or modifying an
		// existing vector.
		//
		// apply_make() - one argument, one type -- return a new vector or scalar
		// apply_unitype_make() - all arguments are cast to their common type -- return a new vector or scalar
		// apply_multitype_make() - all arguments keep their types -- return a new vector or scalar
		// apply_unitype_modify() - all arguments are cast to their common type -- modify the first argument with new values
		// apply_multitype_modify() - all arguments keep their types -- modify the first argument with new values

		// unary

		template <vec_like V, typename UnOp>
		constexpr auto apply_make(const V &arg,
								  const UnOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V>;
			using out_scalar_t = unop_return_t<UnOp, vec_scalar_t<V>>;

			return [&op, &arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(arg[Is])... };
			}(std::make_index_sequence<C>{});
		}

		// binary

		template <vec_like V1, vec_like V2, typename BinOp>
		requires (vec_size_v<V1> == vec_size_v<V2>)
		constexpr auto apply_unitype_make(const V1 &lhs,
										  const V2 &rhs,
										  const BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V1>, vec_scalar_t<V2>>;
			using out_scalar_t = binop_return_t<BinOp, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V1>;

			return [&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs[Is]))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V, dimensional_scalar U, typename BinOp>
		constexpr auto apply_unitype_make(const V &lhs,
										  U rhs,
										  const BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V>, U>;
			using out_scalar_t = binop_return_t<BinOp, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V>;

			return [&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V, dimensional_scalar U, typename BinOp>
		constexpr auto apply_unitype_make(U lhs,
										  const V &rhs,
										  const BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V>, U>;
			using out_scalar_t = binop_return_t<BinOp, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V>;

			return [&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(static_cast<ArgT>(lhs), static_cast<ArgT>(rhs[Is]))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V1, vec_like V2, typename BinOp>
		requires (vec_size_v<V1> == vec_size_v<V2>)
		constexpr auto apply_multitype_make(const V1 &lhs,
											const V2 &rhs,
											const BinOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V1>;
			using out_scalar_t = binop_return_t<BinOp, vec_scalar_t<V1>, vec_scalar_t<V2>>;

			return [&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(lhs[Is], rhs[Is])... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V, dimensional_scalar U, typename BinOp>
		constexpr auto apply_multitype_make(const V &lhs,
											U rhs,
											const BinOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V>;
			using out_scalar_t = binop_return_t<BinOp, vec_scalar_t<V>, U>;

			return [&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(lhs[Is], rhs)... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V, dimensional_scalar U, typename BinOp>
		constexpr auto apply_multitype_make(U lhs,
											const V &rhs,
											const BinOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V>;
			using out_scalar_t = binop_return_t<BinOp, U, vec_scalar_t<V>>;

			return [&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(lhs, rhs[Is])... };
			}(std::make_index_sequence<C>{});
		}

		template <writable_vec_like V1, vec_like V2, typename BinOp>
		requires (vec_size_v<V1> == vec_size_v<V2>)
		constexpr void apply_unitype_modify(V1 &lhs,
											const V2 &rhs,
											const BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V1>, vec_scalar_t<V2>>;
			constexpr std::size_t C = vec_size_v<V1>;

			[&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs[Is]))... );
			}(std::make_index_sequence<C>{});
		}

		template <writable_vec_like V, dimensional_scalar U, typename BinOp>
		constexpr void apply_unitype_modify(V &lhs,
											U rhs,
											const BinOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V>, U>;
			constexpr std::size_t C = vec_size_v<V>;

			[&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(static_cast<ArgT>(lhs[Is]), static_cast<ArgT>(rhs))... );
			}(std::make_index_sequence<C>{});
		}

		template <writable_vec_like V1, vec_like V2, typename BinOp>
		requires (vec_size_v<V1> == vec_size_v<V2>)
		constexpr void apply_multitype_modify(V1 &lhs,
											  const V2 &rhs,
											  const BinOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V1>;

			[&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(lhs[Is], rhs[Is])... );
			}(std::make_index_sequence<C>{});
		}

		template <writable_vec_like V, dimensional_scalar U, typename BinOp>
		constexpr void apply_multitype_modify(V &lhs,
											  U rhs,
											  const BinOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V>;

			[&op, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				lhs.set( op(lhs[Is], rhs)... );
			}(std::make_index_sequence<C>{});
		}

		// ternary

		template <vec_like V1, vec_like V2, vec_like V3, typename TernOp>
		requires (vec_size_v<V1> == vec_size_v<V2>) && (vec_size_v<V1> == vec_size_v<V3>)
		constexpr auto apply_unitype_make(const V1 &x,
										  const V2 &y,
										  const V3 &z,
										  const TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V1>, vec_scalar_t<V2>, vec_scalar_t<V3>>;
			using out_scalar_t = ternop_return_t<TernOp, ArgT, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V1>;

			return [&op, &x, &y, &z]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>
				{ op(static_cast<ArgT>(x[Is]), static_cast<ArgT>(y[Is]), static_cast<ArgT>(z[Is]))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V1, vec_like V2, dimensional_scalar U, typename TernOp>
		requires (vec_size_v<V1> == vec_size_v<V2>)
		constexpr auto apply_unitype_make(const V1 &x,
										  const V2 &y,
										  U z,
										  const TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V1>, vec_scalar_t<V2>, U>;
			using out_scalar_t = ternop_return_t<TernOp, ArgT, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V1>;

			return [&op, &x, &y, &z]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(static_cast<ArgT>(x[Is]), static_cast<ArgT>(y[Is]), static_cast<ArgT>(z))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V, dimensional_scalar U, dimensional_scalar T, typename TernOp>
		constexpr auto apply_unitype_make(const V &x,
										  U y,
										  T z,
										  const TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<vec_scalar_t<V>, U, T>;
			using out_scalar_t = ternop_return_t<TernOp, ArgT, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V>;

			return [&op, &x, &y, &z]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(static_cast<ArgT>(x[Is]), static_cast<ArgT>(y), static_cast<ArgT>(z))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V, dimensional_scalar U, dimensional_scalar T, typename TernOp>
		constexpr auto apply_unitype_make(U x,
										  T y,
										  const V &z,
										  const TernOp &op) noexcept
		{
			using ArgT = std::common_type_t<T, U, vec_scalar_t<V>>;
			using out_scalar_t = ternop_return_t<TernOp, ArgT, ArgT, ArgT>;
			constexpr std::size_t C = vec_size_v<V>;

			return [&op, &x, &y, &z]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(static_cast<ArgT>(x), static_cast<ArgT>(y), static_cast<ArgT>(z[Is]))... };
			}(std::make_index_sequence<C>{});
		}

		template <vec_like V1, vec_like V2, vec_like V3, typename TernOp>
		requires (vec_size_v<V1> == vec_size_v<V2>) && (vec_size_v<V1> == vec_size_v<V3>)
		constexpr auto apply_multitype_make(const V1 &x,
											const V2 &y,
											const V3 &z,
											const TernOp &op) noexcept
		{
			constexpr std::size_t C = vec_size_v<V1>;
			using out_scalar_t = ternop_return_t<TernOp, vec_scalar_t<V1>, vec_scalar_t<V2>, vec_scalar_t<V3>>;

			return [&op, &x, &y, &z]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec_or_scalar_t<out_scalar_t, C>{ op(x[Is], y[Is], z[Is])... };
			}(std::make_index_sequence<C>{});
		}

		// dispatches a binary vec-vec operator call across the three broadcast shapes permitted by
		// ((vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)), where
		// V1 and V2 are either same-length, or V1 treated like a scalar, or V2 treated like a scalar.
		// for unitype case, the operator is applied to the common type of the two vector's scalar types.
		// for multitype case, the operator is applied to the two vector's actual scalar types.

		// for unitype case
		template <vec_like V1, vec_like V2, typename BinOp>
		requires (implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>> ||
				  implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>>) &&
				  ((vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1))
		constexpr auto apply_broadcast_unitype_make(const V1 &lhs, const V2 &rhs, const BinOp &op) noexcept
		{
			constexpr std::size_t C1 = vec_size_v<V1>;
			constexpr std::size_t C2 = vec_size_v<V2>;

			if constexpr (C1 == C2)
				return apply_unitype_make(lhs, rhs, op);
			else if constexpr (C1 == 1)
				return apply_unitype_make(lhs[0], rhs, op);
			else
				return apply_unitype_make(lhs, rhs[0], op);
		}

		// for multitype case
		template <vec_like V1, vec_like V2, typename BinOp>
		requires (implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>> ||
				  implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>>) &&
				  ((vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1))
		constexpr auto apply_broadcast_multitype_make(const V1 &lhs, const V2 &rhs, const BinOp &op) noexcept
		{
			constexpr std::size_t C1 = vec_size_v<V1>;
			constexpr std::size_t C2 = vec_size_v<V2>;

			if constexpr (C1 == C2)
				return apply_multitype_make(lhs, rhs, op);
			else if constexpr (C1 == 1)
				return apply_multitype_make(lhs[0], rhs, op);
			else if constexpr (C2 == 1)
				return apply_multitype_make(lhs, rhs[0], op);
		}
	}	// namespace machinery

	//
	// operators
	//

	// lambdas for operators
	namespace lambda_ops
	{
		constexpr inline auto plus_op =		[](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept					{ return lhs + rhs; };
		constexpr inline auto minus_op =	[](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept					{ return lhs - rhs; };
		constexpr inline auto times_op =	[](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept					{ return lhs * rhs; };
		constexpr inline auto div_op =		[](non_bool_scalar auto lhs, non_bool_scalar auto rhs) noexcept					{ return lhs / rhs; };
		constexpr inline auto modulus_op =	[](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs)
		{
			if (rhs == 0)
			{
				[[ unlikely ]] throw std::domain_error("(lhs % 0) is undefined");
			}

			[[ likely ]] return lhs % rhs;
		};
		constexpr inline auto bit_not_op =	[](numeric_integral_scalar auto arg) noexcept									{ return ~arg; };
		constexpr inline auto lshift_op =
			[]<numeric_integral_scalar T1, numeric_integral_scalar T2>(T1 lhs, T2 rhs) noexcept								{ return static_cast<T1>(lhs << rhs); };
		constexpr inline auto rshift_op =
			[]<numeric_integral_scalar T1, numeric_integral_scalar T2>(T1 lhs, T2 rhs) noexcept								{ return static_cast<T1>(lhs >> rhs); };
		constexpr inline auto and_op =		[](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept	{ return lhs & rhs; };
		constexpr inline auto or_op =		[](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept	{ return lhs | rhs; };
		constexpr inline auto xor_op =		[](numeric_integral_scalar auto lhs, numeric_integral_scalar auto rhs) noexcept	{ return lhs ^ rhs; };
		constexpr inline auto no_op =		[](non_bool_scalar auto arg) noexcept											{ return arg; };
		constexpr inline auto neg_op =		[](non_bool_scalar auto arg) noexcept											{ return -arg; };

	}	// namespace lambda_ops

	// binary operators +=, +

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator +=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::plus_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator +=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::plus_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, non_bool_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator +=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::plus_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			  non_bool_scalar<vec_scalar_t<V1>> && non_bool_scalar<vec_scalar_t<V2>> &&
			  (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator +(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::plus_op);
	}

	template <vec_like V, non_bool_scalar R>
	requires non_bool_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator +(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::plus_op);
	}

	template <vec_like V, non_bool_scalar L>
	requires non_bool_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	[[nodiscard]] constexpr auto operator +(L lhs,
											const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::plus_op);
	}

	// binary operators -=, -

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator -=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::minus_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator -=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::minus_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, non_bool_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator -=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::minus_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			  non_bool_scalar<vec_scalar_t<V1>> && non_bool_scalar<vec_scalar_t<V2>> &&
			  (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator -(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::minus_op);
	}

	template <vec_like V, non_bool_scalar R>
	requires non_bool_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator -(const V &lhs,
											R rhs) noexcept
			  {
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::minus_op);
	}

	template <vec_like V, non_bool_scalar L>
	requires non_bool_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	[[nodiscard]] constexpr auto operator -(L lhs,
											const V &rhs) noexcept
			 {
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::minus_op);
	}

	// binary operators *=, *

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator *=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::times_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator *=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::times_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, non_bool_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator *=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::times_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			  non_bool_scalar<vec_scalar_t<V1>> && non_bool_scalar<vec_scalar_t<V2>> &&
			  (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator *(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::times_op);
	}

	template <vec_like V, non_bool_scalar R>
	requires non_bool_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator *(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::times_op);
	}

	template <vec_like V, non_bool_scalar L>
	requires non_bool_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	[[nodiscard]] constexpr auto operator *(L lhs,
											const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::times_op);
	}

	// binary operators /=, /

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator /=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::div_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 non_bool_scalar<vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator /=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::div_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, non_bool_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && non_bool_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator /=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::div_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			  non_bool_scalar<vec_scalar_t<V1>> && non_bool_scalar<vec_scalar_t<V2>> &&
			  (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator /(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::div_op);
	}

	template <vec_like V, non_bool_scalar R>
	 requires non_bool_scalar<vec_scalar_t<V>> && 
			  (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator /(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::div_op);
	}

	template <vec_like V, non_bool_scalar L>
	requires non_bool_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	[[nodiscard]] constexpr auto operator /(L lhs,
											const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::div_op);
	}

	// binary operators %=, % -- uses c++ modulus operator rules

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator %=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::modulus_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator %=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::modulus_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, numeric_integral_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator %=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::modulus_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			  numeric_integral_scalar<vec_scalar_t<V1>> && numeric_integral_scalar<vec_scalar_t<V2>> &&
			  (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator %(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::modulus_op);
	}

	template <vec_like V, numeric_integral_scalar R>
	requires numeric_integral_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator %(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::modulus_op);
	}

	template <vec_like V, numeric_integral_scalar L>
	requires numeric_integral_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	[[nodiscard]] constexpr auto operator %(L lhs,
											const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::modulus_op);
	}

	// unary operator ~

	template <vec_like V>
	requires numeric_integral_scalar<vec_scalar_t<V>>
	[[nodiscard]] constexpr auto operator ~(const V &arg) noexcept
	{
		return machinery::apply_make(arg, lambda_ops::bit_not_op);
	}

	// binary operators <<=, <<

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator <<=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, lambda_ops::lshift_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator <<=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs[0], lambda_ops::lshift_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, numeric_integral_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator <<=(L &lhs,
											 R rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, lambda_ops::lshift_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			  numeric_integral_scalar<vec_scalar_t<V1>> && numeric_integral_scalar<vec_scalar_t<V2>> &&
			  (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator <<(const V1 &lhs,
											 const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_multitype_make(lhs, rhs, lambda_ops::lshift_op);
	}

	template <vec_like V, numeric_integral_scalar R>
	requires numeric_integral_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator <<(const V &lhs,
											 R rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, lambda_ops::lshift_op);
	}

	template <vec_like V, numeric_integral_scalar L>
	requires numeric_integral_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	 [[nodiscard]] constexpr auto operator <<(L lhs,
											  const V &rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, lambda_ops::lshift_op);
	}

	// binary operators >>=, >>

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator >>=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, lambda_ops::rshift_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator >>=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs[0], lambda_ops::rshift_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, numeric_integral_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<L>>
	constexpr vec_derived_t<L> &operator >>=(L &lhs,
											 R rhs) noexcept
	{
		machinery::apply_multitype_modify(lhs, rhs, lambda_ops::rshift_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			 numeric_integral_scalar<vec_scalar_t<V1>> && numeric_integral_scalar<vec_scalar_t<V2>> &&
			 (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1)
	[[nodiscard]] constexpr auto operator >>(const V1 &lhs,
											 const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_multitype_make(lhs, rhs, lambda_ops::rshift_op);
	}

	template <vec_like V, numeric_integral_scalar R>
	requires numeric_integral_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>)
	[[nodiscard]] constexpr auto operator >>(const V &lhs,
											 R rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, lambda_ops::rshift_op);
	}

	template <vec_like V, numeric_integral_scalar L>
	requires numeric_integral_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>)
	[[nodiscard]] constexpr auto operator >>(L lhs,
											 const V &rhs) noexcept
	{
		return machinery::apply_multitype_make(lhs, rhs, lambda_ops::rshift_op);
	}

	// binary operators &=, &

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>> &&
			 detail::same_sizeof<vec_scalar_t<L>, vec_scalar_t<R>>
	 constexpr vec_derived_t<L> &operator &=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::and_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>> &&
			 detail::same_sizeof<vec_scalar_t<L>, vec_scalar_t<R>>
	 constexpr vec_derived_t<L> &operator &=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::and_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, numeric_integral_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<L>> &&
			 detail::same_sizeof<vec_scalar_t<L>, R>
	constexpr vec_derived_t<L> &operator &=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::and_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			 numeric_integral_scalar<vec_scalar_t<V1>> && numeric_integral_scalar<vec_scalar_t<V2>> &&
			 (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1) &&
			 detail::same_sizeof<vec_scalar_t<V1>, vec_scalar_t<V2>>
	[[nodiscard]] constexpr auto operator &(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::and_op);
	}

	template <vec_like V, numeric_integral_scalar R>
	requires numeric_integral_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>) &&
			 detail::same_sizeof<vec_scalar_t<V>, R>
	[[nodiscard]] constexpr auto operator &(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::and_op);
	}

	template <vec_like V, numeric_integral_scalar L>
	requires numeric_integral_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>) &&
			 detail::same_sizeof<vec_scalar_t<V>, L>
	 [[nodiscard]] constexpr auto operator &(L lhs,
											 const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::and_op);
	}

	// binary operators |=, |

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>> &&
			 detail::same_sizeof<vec_scalar_t<L>, vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator |=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::or_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>> &&
			 detail::same_sizeof<vec_scalar_t<L>, vec_scalar_t<R>>
	 constexpr vec_derived_t<L> &operator |=(L &lhs,
											 const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::or_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, numeric_integral_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<L>> &&
			 detail::same_sizeof<vec_scalar_t<L>, R>
	constexpr vec_derived_t<L> &operator |=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::or_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			 numeric_integral_scalar<vec_scalar_t<V1>> && numeric_integral_scalar<vec_scalar_t<V2>> &&
			 (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1) &&
			 detail::same_sizeof<vec_scalar_t<V1>, vec_scalar_t<V2>>
	[[nodiscard]] constexpr auto operator |(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::or_op);
	}

	template <vec_like V, numeric_integral_scalar R>
	requires numeric_integral_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>) &&
			 detail::same_sizeof<vec_scalar_t<V>, R>
	[[nodiscard]] constexpr auto operator |(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::or_op);
	}

	template <vec_like V, numeric_integral_scalar L>
	requires numeric_integral_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>) &&
			 detail::same_sizeof<vec_scalar_t<V>, L>
	[[nodiscard]] constexpr auto operator |(L lhs,
											const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::or_op);
	}

	// binary operators ^=, ^

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> == vec_size_v<R>) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>> &&
			 detail::same_sizeof<vec_scalar_t<L>, vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator ^=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::xor_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, vec_like R>
	requires implicitly_convertible_to<vec_scalar_t<R>, vec_scalar_t<L>> && (vec_size_v<L> > 1) && (vec_size_v<R> == 1) &&
			 numeric_integral_scalar<vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<R>> &&
			 detail::same_sizeof<vec_scalar_t<L>, vec_scalar_t<R>>
	constexpr vec_derived_t<L> &operator ^=(L &lhs,
											const R &rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs[0], lambda_ops::xor_op);
		return lhs.as_derived();
	}

	template <writable_vec_like L, numeric_integral_scalar R>
	requires implicitly_convertible_to<R, vec_scalar_t<L>> && numeric_integral_scalar<vec_scalar_t<L>> &&
			 detail::same_sizeof<vec_scalar_t<L>, R>
	constexpr vec_derived_t<L> &operator ^=(L &lhs,
											R rhs) noexcept
	{
		machinery::apply_unitype_modify(lhs, rhs, lambda_ops::xor_op);
		return lhs.as_derived();
	}

	template <vec_like V1, vec_like V2>
	requires (implicitly_convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>> ||
			  implicitly_convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>>) &&
			 numeric_integral_scalar<vec_scalar_t<V1>> && numeric_integral_scalar<vec_scalar_t<V2>> &&
			 (vec_size_v<V1> == vec_size_v<V2> || vec_size_v<V1> == 1 || vec_size_v<V2> == 1) &&
			 detail::same_sizeof<vec_scalar_t<V1>, vec_scalar_t<V2>>
	[[nodiscard]] constexpr auto operator ^(const V1 &lhs,
											const V2 &rhs) noexcept
	{
		return machinery::apply_broadcast_unitype_make(lhs, rhs, lambda_ops::xor_op);
	}

	template <vec_like V, numeric_integral_scalar R>
	requires numeric_integral_scalar<vec_scalar_t<V>> && 
			 (implicitly_convertible_to<R, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, R>) &&
			 detail::same_sizeof<vec_scalar_t<V>, R>
	[[nodiscard]] constexpr auto operator ^(const V &lhs,
											R rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::xor_op);
	}

	template <vec_like V, numeric_integral_scalar L>
	requires numeric_integral_scalar<vec_scalar_t<V>> &&
			 (implicitly_convertible_to<L, vec_scalar_t<V>> || implicitly_convertible_to<vec_scalar_t<V>, L>) &&
			 detail::same_sizeof<vec_scalar_t<V>, L>
	[[nodiscard]] constexpr auto operator ^(L lhs,
											const V &rhs) noexcept
	{
		return machinery::apply_unitype_make(lhs, rhs, lambda_ops::xor_op);
	}

	// unary operator +

	template <vec_like V>
	requires non_bool_scalar<vec_scalar_t<V>>
	[[nodiscard]] constexpr auto operator +(const V &arg) noexcept
	{
		return machinery::apply_make(arg, lambda_ops::no_op);
	}

	// unary operator -

	template <vec_like V>
	requires non_bool_scalar<vec_scalar_t<V>>
	[[nodiscard]] constexpr auto operator -(const V &arg) noexcept
	{
		return machinery::apply_make(arg, lambda_ops::neg_op);
	}

	// unary operators ++

	// pre-increment
	template <writable_vec_like V>
	requires non_bool_scalar<vec_scalar_t<V>>
	constexpr vec_derived_t<V> &operator ++(V &arg) noexcept
	{
		arg += vec_scalar_t<V>(1);
		return arg.as_derived();
	}

	// post-increment
	template <writable_vec_like V>
	requires non_bool_scalar<vec_scalar_t<V>>
	constexpr auto operator ++(V &arg, int) noexcept
	{
		vec<vec_scalar_t<V>, vec_size_v<V>> value(arg);
		arg += vec_scalar_t<V>(1);
		return value;
	}

	// unary operators --

	// pre-decrement
	template <writable_vec_like V>
	requires non_bool_scalar<vec_scalar_t<V>>
	constexpr vec_derived_t<V> &operator --(V &arg) noexcept
	{
		arg -= vec_scalar_t<V>(1);
		return arg.as_derived();
	}

	// post-decrement
	template <writable_vec_like V>
	requires non_bool_scalar<vec_scalar_t<V>>
	constexpr auto operator --(V &arg, int) noexcept
	{
		vec<vec_scalar_t<V>, vec_size_v<V>> value(arg);
		arg -= vec_scalar_t<V>(1);
		return value;
	}

	//
	// get<> part of tuple protocol -- needed for structured bindings
	//

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr T & get(vec_storage<T, S> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr const T & get(const vec_storage<T, S> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	[[nodiscard]] constexpr auto && get(vec_storage<T, S> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	//

	template <int N, writable_vec_like V>
	requires (N >= 0) && (N < vec_size_v<V>)
	[[nodiscard]] constexpr vec_scalar_t<V> & get(V & arg) noexcept
	{
		return arg[N];
	}

	template <int N, vec_like V>
	requires (N >= 0) && (N < vec_size_v<V>)
	[[nodiscard]] constexpr const vec_scalar_t<V> & get(const V & arg) noexcept
	{
		return arg[N];
	}

	template <int N, writable_vec_like V>
	requires (N >= 0) && (N < vec_size_v<V>)
	[[nodiscard]] constexpr auto && get(V && arg) noexcept
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

		// lambdas for functions
		namespace lambda_ops
		{
			constexpr inline auto less_op =				[]<non_bool_scalar T>(T x, T y) noexcept { return (x < y); };
			constexpr inline auto less_equal_op =		[]<non_bool_scalar T>(T x, T y) noexcept { return (x <= y); };
			constexpr inline auto greater_op =			[]<non_bool_scalar T>(T x, T y) noexcept { return (x > y); };
			constexpr inline auto greater_equal_op =	[]<non_bool_scalar T>(T x, T y) noexcept { return (x >= y); };
			constexpr inline auto equal_op =			[]<non_bool_scalar T>(T x, T y) noexcept { return (x == y); };
			constexpr inline auto not_equal_op =		[]<non_bool_scalar T>(T x, T y) noexcept { return (x != y); };

			constexpr inline auto bool_equal_op =		[](bool x, bool y) noexcept	{ return x == y; };
			constexpr inline auto bool_not_equal_op =	[](bool x, bool y) noexcept	{ return x != y; };
			constexpr inline auto comp_not_op =			[](bool x) noexcept			{ return !x; };
			constexpr inline auto comp_and_op =			[](bool x, bool y) noexcept	{ return x && y; };
			constexpr inline auto comp_or_op =			[](bool x, bool y) noexcept	{ return x || y; };
			constexpr inline auto comp_xor_op =			[](bool x, bool y) noexcept	{ return x != y; };

		}	// namespace lambda_ops

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto lessThan(const V1 &x,
											  const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::less_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr bool lessThan(T x,
											  T y) noexcept
		{
			return lambda_ops::less_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto lessThanEqual(const V1 &x,
												   const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::less_equal_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr bool lessThanEqual(T x,
												   T y) noexcept
		{
			return lambda_ops::less_equal_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto greaterThan(const V1 &x,
												 const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::greater_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr bool greaterThan(T x,
												 T y) noexcept
		{
			return lambda_ops::greater_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto greaterThanEqual(const V1 &x,
													  const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::greater_equal_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr bool greaterThanEqual(T x,
													  T y) noexcept
		{
			return lambda_ops::greater_equal_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto equal(const V1 &x,
										   const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::equal_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr bool equal(T x,
										   T y) noexcept
		{
			return lambda_ops::equal_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto equal(const V1 &x,
										   const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::bool_equal_op);
		}

		[[nodiscard]] constexpr bool equal(bool x,
										   bool y) noexcept
		{
			return lambda_ops::bool_equal_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto notEqual(const V1 &x,
											  const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::not_equal_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr bool notEqual(T x,
											  T y) noexcept
		{
			return lambda_ops::not_equal_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto notEqual(const V1 &x,
											  const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::bool_not_equal_op);
		}

		[[nodiscard]] constexpr bool notEqual(bool x,
											  bool y) noexcept
		{
			return lambda_ops::bool_not_equal_op(x, y);
		}

		template <vec_like V>
		requires bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr bool any(const V &x) noexcept
		{
			constexpr std::size_t C = vec_size_v<V>;
			return [&x]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (x[Is] || ...);
			}(std::make_index_sequence<C>{});
		}

		[[nodiscard]] constexpr bool any(bool x) noexcept
		{
			return x;
		}

		template <vec_like V>
		requires bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr bool all(const V &x) noexcept
		{
			constexpr std::size_t C = vec_size_v<V>;
			return [&x]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (x[Is] && ...);
			}(std::make_index_sequence<C>{});
		}

		[[nodiscard]] constexpr bool all(bool x) noexcept
		{
			return x;
		}

		// not in GLSL
		template <vec_like V>
		requires bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr bool none(const V &x) noexcept
		{
			return !any(x);
		}

		[[nodiscard]] constexpr bool none(bool x) noexcept
		{
			return !x;
		}

		// c++ does not allow a function named not() as in GLSL, so this is our alternate name
		template <vec_like V>
		requires bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto compNot(const V &x) noexcept
		{
			return machinery::apply_make(x, lambda_ops::comp_not_op);
		}

		// c++ does not allow a function named not() as in GLSL, so this is our alternate name
		[[nodiscard]] constexpr bool compNot(bool x) noexcept
		{
			return lambda_ops::comp_not_op(x);
		}

		// not in GLSL
		template <vec_like V1, vec_like V2>
		requires bool_scalar<vec_scalar_t<V1>> && bool_scalar<vec_scalar_t<V2>> && (vec_size_v<V1> == vec_size_v<V2>)
		[[nodiscard]] constexpr auto compAnd(const V1 &x,
											 const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::comp_and_op);
		}

		[[nodiscard]] constexpr bool compAnd(bool x,
											 bool y) noexcept
		{
			return lambda_ops::comp_and_op(x, y);
		}

		// not in GLSL
		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto compOr(const V1 &x,
											const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::comp_or_op);
		}

		[[nodiscard]] constexpr bool compOr(bool x,
											bool y) noexcept
		{
			return lambda_ops::comp_or_op(x, y);
		}

		// not in GLSL
		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto compXor(const V1 &x,
											 const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::comp_xor_op);
		}

		[[nodiscard]] constexpr bool compXor(bool x,
											 bool y) noexcept
		{
			return lambda_ops::comp_xor_op(x, y);
		}

		//
		// 8.1 - angle and trigonometry
		//

		// lambdas for functions
		namespace lambda_ops
		{
			constexpr inline auto sin_op =		[](floating_point_scalar auto arg) noexcept				{ return std::sin(arg); };
			constexpr inline auto cos_op =		[](floating_point_scalar auto arg) noexcept				{ return std::cos(arg); };
			constexpr inline auto tan_op =		[](floating_point_scalar auto arg) noexcept				{ return std::tan(arg); };
			constexpr inline auto asin_op =		[](floating_point_scalar auto arg) noexcept				{ return std::asin(arg); };
			constexpr inline auto acos_op =		[](floating_point_scalar auto arg) noexcept				{ return std::acos(arg); };
			constexpr inline auto atan_op =		[](floating_point_scalar auto arg) noexcept				{ return std::atan(arg); };
			constexpr inline auto atan2_op =	[]<floating_point_scalar U>(U arg_y, U arg_x) noexcept	{ return std::atan2(arg_y, arg_x); };
			constexpr inline auto sinh_op =		[](floating_point_scalar auto arg) noexcept				{ return std::sinh(arg); };
			constexpr inline auto cosh_op =		[](floating_point_scalar auto arg) noexcept				{ return std::cosh(arg); };
			constexpr inline auto tanh_op =		[](floating_point_scalar auto arg) noexcept				{ return std::tanh(arg); };
			constexpr inline auto asinh_op =	[](floating_point_scalar auto arg) noexcept				{ return std::asinh(arg); };
			constexpr inline auto acosh_op =	[](floating_point_scalar auto arg) noexcept				{ return std::acosh(arg); };
			constexpr inline auto atanh_op =	[](floating_point_scalar auto arg) noexcept				{ return std::atanh(arg); };

		}	// namespace lambda_ops

		template <floating_point_scalar T>
		inline constexpr T degrees_per_radian_v = std::numbers::inv_pi_v<T> * T(180);

		template <floating_point_scalar T>
		inline constexpr T radians_per_degree_v = std::numbers::pi_v<T> / T(180);

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto radians(const V &deg) noexcept
		{
			return deg * radians_per_degree_v<vec_scalar_t<V>>;
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T radians(T deg) noexcept
		{
			return deg * radians_per_degree_v<T>;
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto degrees(const V &rad) noexcept
		{
			return rad * degrees_per_radian_v<vec_scalar_t<V>>;
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T degrees(T rad) noexcept
		{
			return rad * degrees_per_radian_v<T>;
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto sin(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::sin_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T sin(T arg) noexcept
		{
			return lambda_ops::sin_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto cos(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::cos_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T cos(T arg) noexcept
		{
			return lambda_ops::cos_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto tan(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::tan_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T tan(T arg) noexcept
		{
			return lambda_ops::tan_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto asin(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::asin_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T asin(T arg) noexcept
		{
			return lambda_ops::asin_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto acos(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::acos_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T acos(T arg) noexcept
		{
			return lambda_ops::acos_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto atan(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::atan_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T atan(T arg) noexcept
		{
			return lambda_ops::atan_op(arg);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] inline auto atan(const V1 &y,
									   const V2 &x) noexcept
		{
			return machinery::apply_unitype_make(y, x, lambda_ops::atan2_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T atan(T y,
									T x) noexcept
		{
			return lambda_ops::atan2_op(y, x);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto sinh(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::sinh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T sinh(T arg) noexcept
		{
			return lambda_ops::sinh_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto cosh(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::cosh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T cosh(T arg) noexcept
		{
			return lambda_ops::cosh_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto tanh(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::tanh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T tanh(T arg) noexcept
		{
			return lambda_ops::tanh_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto asinh(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::asinh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T asinh(T arg) noexcept
		{
			return lambda_ops::asinh_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto acosh(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::acosh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T acosh(T arg) noexcept
		{
			return lambda_ops::acosh_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto atanh(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::atanh_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T atanh(T arg) noexcept
		{
			return lambda_ops::atanh_op(arg);
		}

		//
		// 8.2 - exponential
		//

		// lambdas for functions
		namespace lambda_ops
		{
			constexpr inline auto pow_op =			[]<floating_point_scalar U>(U base, U exp) noexcept	{ return std::pow(base, exp); };
			constexpr inline auto exp_op =			[](floating_point_scalar auto arg) noexcept			{ return std::exp(arg); };
			constexpr inline auto log_op =			[](floating_point_scalar auto arg) noexcept			{ return std::log(arg); };
			constexpr inline auto exp2_op =			[](floating_point_scalar auto arg) noexcept			{ return std::exp2(arg); };
			constexpr inline auto log2_op =			[](floating_point_scalar auto arg) noexcept			{ return std::log2(arg); };
			constexpr inline auto sqrt_op =			[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::sqrt(arg); };
			constexpr inline auto fast_rsqrt_op =	[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::fast_rsqrt(arg); };
			constexpr inline auto rsqrt_op =		[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::rsqrt(arg); };

		}	// namespace lambda_ops

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] inline auto pow(const V1 &base,
									  const V2 &exp)
		{
			using T = vec_scalar_t<V1>;
			constexpr std::size_t C = vec_size_v<V1>;

			if (any(lessThan(base, vec<T, C>(0))))
			{
				[[ unlikely ]] throw std::invalid_argument("(base < 0) is UB");
			}
			else if (!all(compNot(compAnd(equal(base, vec<T, C>(0)), lessThanEqual(exp, vec<T, C>(0))))))
			{
				[[ unlikely ]] throw std::invalid_argument("(base == 0 && exp <= 0) is UB");
			}

			[[ likely ]] return machinery::apply_unitype_make(base, exp, lambda_ops::pow_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T pow(T base,
								   T exp)
		{
			if (base < T(0))
			{
				[[ unlikely ]] throw std::invalid_argument("(base < 0) is UB");
			}
			else if ((base == T(0) && exp <= T(0)))
			{
				[[ unlikely ]] throw std::invalid_argument("(base == 0 && exp <= 0) is UB");
			}

			[[ likely ]] return lambda_ops::pow_op(base, exp);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto exp(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::exp_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T exp(T arg) noexcept
		{
			return lambda_ops::exp_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto log(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::log_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T log(T arg) noexcept
		{
			return lambda_ops::log_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto exp2(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::exp2_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T exp2(T arg) noexcept
		{
			return lambda_ops::exp2_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] inline auto log2(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::log2_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T log2(T arg) noexcept
		{
			return lambda_ops::log2_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto sqrt(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::sqrt_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T sqrt(T arg) noexcept
		{
			return lambda_ops::sqrt_op(arg);
		}

		// not in GLSL

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto fast_inversesqrt(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::fast_rsqrt_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T fast_inversesqrt(T arg) noexcept
		{
			return lambda_ops::fast_rsqrt_op(arg);
		}

		// double specializations

		template <vec_like V>
		requires std::same_as<double, vec_scalar_t<V>>
		[[nodiscard]] constexpr auto inversesqrt(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::rsqrt_op);
		}

		[[nodiscard]] constexpr double inversesqrt(double arg) noexcept
		{
			return lambda_ops::rsqrt_op(arg);
		}

		// float specializations - cxcm::rsqrt(float) is 100% match with cxcm::fast_rsqrt(float)

		template <vec_like V>
		requires std::same_as<float, vec_scalar_t<V>>
		[[nodiscard]] constexpr auto inversesqrt(const V &arg) noexcept
		{
			return fast_inversesqrt(arg);
		}

		[[nodiscard]] constexpr float inversesqrt(float arg) noexcept
		{
			return fast_inversesqrt(arg);
		}

		//
		// 8.3 - common
		//

		// lambdas for functions
		namespace lambda_ops
		{
			constexpr inline auto abs_op =			[]<non_bool_scalar T>(T arg) noexcept				{ return detail::cxcm::abs(arg); };
			constexpr inline auto sign_op =			[]<non_bool_scalar T>(T arg) noexcept				{ return T(T(0) < arg) - T(arg < T(0)); };
			constexpr inline auto floor_op =		[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::floor(arg); };
			constexpr inline auto trunc_op =		[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::trunc(arg); };
			constexpr inline auto round_op =		[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::round(arg); };
			constexpr inline auto round_even_op =	[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::round_even(arg); };
			constexpr inline auto ceil_op =			[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::ceil(arg); };
			constexpr inline auto fract_op =		[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::fract(arg); };
			constexpr inline auto modf_op =			[]<floating_point_scalar T>(T x, T y) noexcept		{ return detail::cxcm::isinf(x) ? T(0) : (x - y); };
			constexpr inline auto min_op =			[]<non_bool_scalar T>(T x, T y) noexcept			{ return std::min(x ,y); };
			constexpr inline auto max_op =			[]<non_bool_scalar T>(T x, T y) noexcept			{ return std::max(x ,y); };
			constexpr inline auto mod_op =			[]<floating_point_scalar T>(T x, T y) noexcept
			{
				if (detail::cxcm::abs(y) <= std::numeric_limits<T>::epsilon())
				{
					[[ unlikely]] return std::numeric_limits<T>::quiet_NaN();
				}
				else
				{
					return x - y * detail::cxcm::floor(x / y);
				}
			 };

			constexpr inline auto clamp_op =		[]<non_bool_scalar T>(T x, T min_val, T max_val) noexcept
			{
				// this is confusing, because std::clamp is constexpr since it was introduced, so it should be usable in
				// constant evaluation, given constexpr variables. However, both gcc and MSVC fail to compile it in that
				// context, so we have to explicitly write out the logic here to make it work in constant evaluation contexts
				if (x < min_val)
				{
					return min_val;
				}
				else if (x > max_val)
				{
					return max_val;
				}
				else
				{
					return x;
				}
			};

			// https://stackoverflow.com/a/58648036
			constexpr inline auto mix1_op = []<floating_point_scalar T>(T x, T y, T a) noexcept
			{
				if (a <= T(0.5))
				{
					// closer to x
					return x + (y - x) * a;
				}
				else
				{
					// closer to y
					return y - (y - x) * (T(1.0) - a);
				}
			};
			constexpr inline auto mix2_op =							[]<dimensional_scalar T, bool_scalar B>(T x, T y, B a) noexcept		{ return a ? y : x; };
			constexpr inline auto step_op =							[]<floating_point_scalar T>(T edge, T x) noexcept					{ return ((x < edge) ? T(0) : T(1)); };
			constexpr inline auto smoothstep_op =					[]<floating_point_scalar T>(T edge0, T edge1, T x) noexcept
			{
				T t = clamp_op((x - edge0) / (edge1 - edge0), T(0), T(1));
				return t * t * (T(3) - T(2) * t);
			};
			constexpr inline auto isnan_op =						[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::isnan(arg); };
			constexpr inline auto isinf_op =						[](floating_point_scalar auto arg) noexcept			{ return detail::cxcm::isinf(arg); };
			constexpr inline auto float_bits_to_int_op =			[](float arg) noexcept								{ return std::bit_cast<int>(arg); };
			constexpr inline auto float_bits_to_uint_op =			[](float arg) noexcept								{ return std::bit_cast<unsigned int>(arg); };
			constexpr inline auto double_bits_to_long_long_op =		[](double arg) noexcept								{ return std::bit_cast<long long>(arg); };
			constexpr inline auto double_bits_to_ulong_long_op =	[](double arg) noexcept								{ return std::bit_cast<unsigned long long>(arg); };
			constexpr inline auto int_bits_to_float_op =			[](int arg) noexcept								{ return std::bit_cast<float>(arg); };
			constexpr inline auto uint_bits_to_float_op =			[](unsigned int arg) noexcept						{ return std::bit_cast<float>(arg); };
			constexpr inline auto long_long_bits_to_double_op =		[](long long arg) noexcept							{ return std::bit_cast<double>(arg); };
			constexpr inline auto ulong_long_bits_to_double_op =	[](unsigned long long arg) noexcept					{ return std::bit_cast<double>(arg); };
			constexpr inline auto fma_op =							[]<floating_point_scalar T>(T a, T b, T c) noexcept	{ return std::fma(a, b, c); };
			constexpr inline auto frexp_op =						[]<floating_point_scalar T>(T x, int &exp) noexcept	{ return std::frexp(x, &exp); };
			constexpr inline auto ldexp_op =						[]<floating_point_scalar T>(T x, int exp) noexcept	{ return std::ldexp(x, exp); };
			constexpr inline auto byteswap_op =						[]<numeric_integral_scalar T>(T x) noexcept
			{
#if defined(__cpp_lib_byteswap)
				return std::byteswap(x);
#else
				auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(x);
				std::ranges::reverse(value_representation);
				return std::bit_cast<T>(value_representation);
#endif
			};

		}	// namespace lambda_ops

		template <vec_like V>
		requires non_bool_scalar<vec_scalar_t<V>> && (!unsigned_scalar<vec_scalar_t<V>>)
		[[nodiscard]] constexpr auto abs(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::abs_op);
		}

		template <non_bool_scalar T>
		requires (!unsigned_scalar<T>)
		[[nodiscard]] constexpr T abs(T arg) noexcept
		{
			return lambda_ops::abs_op(arg);
		}

		template <vec_like V>
		requires non_bool_scalar<vec_scalar_t<V>> && (!unsigned_scalar<vec_scalar_t<V>>)
		[[nodiscard]] constexpr auto sign(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::sign_op);
		}

		template <non_bool_scalar T>
		requires (!unsigned_scalar<T>)
		[[nodiscard]] constexpr T sign(T arg) noexcept
		{
			return lambda_ops::sign_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto floor(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::floor_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T floor(T arg) noexcept
		{
			return lambda_ops::floor_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto trunc(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::trunc_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T trunc(T arg) noexcept
		{
			return lambda_ops::trunc_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto round(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::round_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T round(T arg) noexcept
		{
			return lambda_ops::round_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto roundEven(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::round_even_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T roundEven(T arg) noexcept
		{
			return lambda_ops::round_even_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto ceil(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::ceil_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T ceil(T arg) noexcept
		{
			return lambda_ops::ceil_op(arg);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto fract(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::fract_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T fract(T arg) noexcept
		{
			return lambda_ops::fract_op(arg);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto mod(const V1 &x,
										 const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::mod_op);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto mod(const V &x,
										 vec_scalar_t<V> y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::mod_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T mod(T x,
									  T y) noexcept
		{
			return lambda_ops::mod_op(x, y);
		}

		template <vec_like V1, writable_vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto modf(const V1 &arg,
										  V2 &i) noexcept
		{
			if constexpr (vec_size_v<V1> == 1)
				i.as_derived() = vec<vec_scalar_t<V1>, 1>(trunc(arg));
			else
				i.as_derived() = trunc(arg);

			return machinery::apply_unitype_make(arg, i, lambda_ops::modf_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T modf(T arg,
									   T &i) noexcept
		{
			i = trunc(arg);
			return lambda_ops::modf_op(arg, i);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto min(const V1 &x,
										 const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::min_op);
		}

		template <vec_like V>
		requires non_bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto min(const V &x,
										 vec_scalar_t<V> y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::min_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr T min(T x,
									  T y) noexcept
		{
			return lambda_ops::min_op(x, y);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto max(const V1 &x,
										 const V2 &y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::max_op);
		}

		template <vec_like V>
		requires non_bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto max(const V &x,
										 vec_scalar_t<V> y) noexcept
		{
			return machinery::apply_unitype_make(x, y, lambda_ops::max_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr T max(T x,
									  T y) noexcept
		{
			return lambda_ops::max_op(x, y);
		}

		template <vec_like V1, vec_like V2, vec_like V3>
		requires same_vec_shape<V1, V2, V3> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto clamp(const V1 &x,
										   const V2 &min_val,
										   const V3 &max_val)
		{
			return machinery::apply_unitype_make(x, min_val, max_val, lambda_ops::clamp_op);
		}

		template <vec_like V>
		requires non_bool_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto clamp(const V &x,
										   vec_scalar_t<V> min_val,
										   vec_scalar_t<V> max_val)
		{
			return machinery::apply_unitype_make(x, min_val, max_val, lambda_ops::clamp_op);
		}

		template <non_bool_scalar T>
		[[nodiscard]] constexpr T clamp(T x,
										T min_val,
										T max_val)
		{
			return lambda_ops::clamp_op(x, min_val, max_val);
		}

		template <vec_like V1, vec_like V2, vec_like V3>
		requires same_vec_shape<V1, V2, V3> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto mix(const V1 &x,
										 const V2 &y,
										 const V3 &a) noexcept
		{
			return machinery::apply_unitype_make(x, y, a, lambda_ops::mix1_op);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto mix(const V1 &x,
										 const V2 &y,
										 vec_scalar_t<V1> a) noexcept
		{
			return machinery::apply_unitype_make(x, y, a, lambda_ops::mix1_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T mix(T x,
									  T y,
									  T a) noexcept
		{
			return lambda_ops::mix1_op(x, y, a);
		}

		template <vec_like V1, vec_like V2, vec_like V3>
		requires same_vec_shape<V1, V2> && dimensional_scalar<vec_scalar_t<V1>> && bool_scalar<vec_scalar_t<V3>> && (vec_size_v<V1> == vec_size_v<V3>)
		[[nodiscard]] constexpr auto mix(const V1 &x,
										 const V2 &y,
										 const V3 &a) noexcept
		{
			return machinery::apply_multitype_make(x, y, a, lambda_ops::mix2_op);
		}

		template <dimensional_scalar T, bool_scalar B>
		[[nodiscard]] constexpr T mix(T x,
									  T y,
									  B a) noexcept
		{
			return lambda_ops::mix2_op(x, y, a);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto step(const V1 &edge,
										  const V2 &x) noexcept
		{
			return machinery::apply_unitype_make(edge, x, lambda_ops::step_op);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto step(vec_scalar_t<V> edge,
										  const V &x) noexcept
		{
			return machinery::apply_unitype_make(edge, x, lambda_ops::step_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T step(T edge,
									   T x) noexcept
		{
			return lambda_ops::step_op(edge, x);
		}

		template <vec_like V1, vec_like V2, vec_like V3>
		requires same_vec_shape<V1, V2, V3> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr auto smoothstep(const V1 &edge0,
												const V2 &edge1,
												const V3 &x)
		{
			if (any(greaterThanEqual(edge0, edge1)))
			{
				[[ unlikely ]] throw std::invalid_argument("(edge0 >= edge1) is UB");
			}
			[[ likely ]] return machinery::apply_unitype_make(edge0, edge1, x, lambda_ops::smoothstep_op);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto smoothstep(vec_scalar_t<V> edge0,
												vec_scalar_t<V> edge1,
												const V &x)
		{
			if (edge0 >= edge1)
			{
				[[ unlikely ]] throw std::invalid_argument("(edge0 >= edge1) is UB");
			}
			[[ likely ]] return machinery::apply_unitype_make(edge0, edge1, x, lambda_ops::smoothstep_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T smoothstep(T edge0,
											 T edge1,
											 T x)
		{
			if (edge0 >= edge1)
			{
				[[ unlikely ]] throw std::invalid_argument("(edge0 >= edge1) is UB");
			}
			[[ likely ]] return lambda_ops::smoothstep_op(edge0, edge1, x);
		}

		// MSVC has a problem when I try to implement this with vec_interface -- don't know about gcc or clang

		template <floating_point_scalar T, std::size_t C>
		[[nodiscard]] constexpr auto isnan(const vec<T, C> &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::isnan_op);
		}

		template <floating_point_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		[[nodiscard]] constexpr auto isnan(const swizzle_vec<T, S, C, Is...> &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::isnan_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr bool isnan(T arg) noexcept
		{
			return lambda_ops::isnan_op(arg);
		}

		// MSVC has a problem when I try to implement this with vec_interface -- don't know about gcc or clang

		template <floating_point_scalar T, std::size_t C>
		[[nodiscard]] constexpr auto isinf(const vec<T, C> &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::isinf_op);
		}

		template <floating_point_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		[[nodiscard]] constexpr auto isinf(const swizzle_vec<T, S, C, Is...> &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::isinf_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr bool isinf(T arg) noexcept
		{
			return lambda_ops::isinf_op(arg);
		}

		//
		// guarantee the byte sizes of these primitives
		//

		static_assert(sizeof(float) == sizeof(int), "requires IEEE-754 32-bit float on this platform");
		static_assert(sizeof(float) == sizeof(unsigned int), "requires IEEE-754 32-bit float on this platform");
		static_assert(sizeof(double) == sizeof(long long), "requires IEEE-754 64-bit double on this platform");
		static_assert(sizeof(double) == sizeof(unsigned long long), "requires IEEE-754 64-bit double on this platform");

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, float>
		[[nodiscard]] constexpr auto floatBitsToInt(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::float_bits_to_int_op);
		}

		[[nodiscard]] constexpr int floatBitsToInt(float arg) noexcept
		{
			return lambda_ops::float_bits_to_int_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, float>
		[[nodiscard]] constexpr auto floatBitsToUint(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::float_bits_to_uint_op);
		}

		[[nodiscard]] constexpr unsigned int floatBitsToUint(float arg) noexcept
		{
			return lambda_ops::float_bits_to_uint_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, double>
		[[nodiscard]] constexpr auto doubleBitsToLongLong(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::double_bits_to_long_long_op);
		}

		[[nodiscard]] constexpr long long doubleBitsToLongLong(double arg) noexcept
		{
			return lambda_ops::double_bits_to_long_long_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, double>
		[[nodiscard]] constexpr auto doubleBitsToUlongLong(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::double_bits_to_ulong_long_op);
		}

		[[nodiscard]] constexpr unsigned long long doubleBitsToUlongLong(double arg) noexcept
		{
			return lambda_ops::double_bits_to_ulong_long_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, int>
		[[nodiscard]] constexpr auto intBitsToFloat(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::int_bits_to_float_op);
		}

		[[nodiscard]] constexpr float intBitsToFloat(int arg) noexcept
		{
			return lambda_ops::int_bits_to_float_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, unsigned int>
		[[nodiscard]] constexpr auto uintBitsToFloat(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::uint_bits_to_float_op);
		}

		[[nodiscard]] constexpr float uintBitsToFloat(unsigned int arg) noexcept
		{
			return lambda_ops::uint_bits_to_float_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, long long>
		[[nodiscard]] constexpr auto longLongBitsToDouble(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::long_long_bits_to_double_op);
		}

		[[nodiscard]] constexpr double longLongBitsToDouble(long long arg) noexcept
		{
			return lambda_ops::long_long_bits_to_double_op(arg);
		}

		template <vec_like V>
		requires std::same_as<vec_scalar_t<V>, unsigned long long>
		[[nodiscard]] constexpr auto ulongLongBitsToDouble(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::ulong_long_bits_to_double_op);
		}

		[[nodiscard]] constexpr double ulongLongBitsToDouble(unsigned long long arg) noexcept
		{
			return lambda_ops::ulong_long_bits_to_double_op(arg);
		}

		template <vec_like V1, vec_like V2, vec_like V3>
		requires same_vec_shape<V1, V2, V3> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] inline auto fma(const V1 &a,
									  const V2 &b,
									  const V3 &c) noexcept
		{
			return machinery::apply_unitype_make(a, b, c, lambda_ops::fma_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T fma(T a,
								   T b,
								   T c) noexcept
		{
			return lambda_ops::fma_op(a, b, c);
		}

		template <vec_like V1, writable_vec_like V2>
		requires floating_point_scalar<vec_scalar_t<V1>> && std::same_as<int, vec_scalar_t<V2>> && (vec_size_v<V1> == vec_size_v<V2>)
		[[nodiscard]] inline auto frexp(const V1 &x,
										V2 &exp) noexcept
		{
			return [&x, &exp]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec<vec_scalar_t<V1>, vec_size_v<V1>>{lambda_ops::frexp_op(x[Is], exp[Is])...};
			}(std::make_index_sequence<vec_size_v<V1>>{});
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T frexp(T x,
									 int &exp) noexcept
		{
			return lambda_ops::frexp_op(x, exp);
		}

		template <vec_like V1, vec_like V2>
		requires floating_point_scalar<vec_scalar_t<V1>> && std::same_as<int, vec_scalar_t<V2>> && (vec_size_v<V1> == vec_size_v<V2>)
		[[nodiscard]] inline auto ldexp(const V1 &x,
										const V2 &exp) noexcept
		{
			return machinery::apply_multitype_make(x, exp, lambda_ops::ldexp_op);
		}

		template <floating_point_scalar T>
		[[nodiscard]] inline T ldexp(T x,
									 int exp) noexcept
		{
			return lambda_ops::ldexp_op(x, exp);
		}

		//
		// byteswap() for non-boolean typed vectors
		//

		// since dsga is designed for c++20, we can't use std::byteswap() from c++23
		template <vec_like V>
		requires numeric_integral_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto byteswap(const V &arg) noexcept
		{
			return machinery::apply_make(arg, lambda_ops::byteswap_op);
		}

		template <numeric_integral_scalar T>
		[[nodiscard]] constexpr T byteswap(T arg) noexcept
		{
			return lambda_ops::byteswap_op(arg);
		}

		//
		// to_underlying() for underlying enum value - not really a good fit for dsga, but a helpful function anyway
		//

		// since dsga is designed for c++20, we can't use std::to_underlying() from c++23
		template <typename E>
		requires std::is_enum_v<E>
		[[nodiscard]] constexpr std::underlying_type_t<E> to_underlying(E e) noexcept
		{
#if defined(__cpp_lib_to_underlying)
			return std::to_underlying(e);
#else
			return static_cast<std::underlying_type_t<E>>(e);
#endif
		}

		//
		// 8.4 is omitted
		//

		// not in GLSL -- dot() is just for floating point, innerProduct() does what dot() does, but it works on all non_bool_scalar types
		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && non_bool_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr vec_scalar_t<V1> innerProduct(const V1 &x,
															  const V2 &y) noexcept
		{
			return [&x, &y]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (... + (x[Is] * y[Is]));
			}(std::make_index_sequence<vec_size_v<V1>>{});
		}

		//
		// 8.5 - geometric
		//

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr vec_scalar_t<V1> dot(const V1 &x,
													 const V2 &y) noexcept
		{
			return [&x, &y]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (... + (x[Is] * y[Is]));
			}(std::make_index_sequence<vec_size_v<V1>>{});
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>> && (vec_size_v<V1> == 3)
		[[nodiscard]] constexpr vec<vec_scalar_t<V1>, 3> cross(const V1 &a,
															   const V2 &b) noexcept
		{
			return vec
			{
				(a[1] * b[2]) - (b[1] * a[2]),
				(a[2] * b[0]) - (b[2] * a[0]),
				(a[0] * b[1]) - (b[0] * a[1])
			};
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr vec_scalar_t<V> length(const V &x) noexcept
		{
			return detail::cxcm::sqrt(dot(x, x));
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>> && (vec_size_v<V> == 1)
		[[nodiscard]] constexpr vec_scalar_t<V> length(const V &x) noexcept
		{
			return detail::cxcm::abs(x[0]);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T length(const T &x) noexcept
		{
			return detail::cxcm::abs(x);
		}

		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr vec_scalar_t<V1> distance(const V1 &p0,
														  const V2 &p1) noexcept
		{
			return length(p0 - p1);
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T distance(T p0,
										   T p1) noexcept
		{
			return length(p0 - p1);
		}

		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>> && (vec_size_v<V> > 1)
		[[nodiscard]] constexpr vec<vec_scalar_t<V>, vec_size_v<V>> normalize(const V &x) noexcept
		{
			using T = vec_scalar_t<V>;

			auto len = length(x);
			if (len <= std::numeric_limits<T>::epsilon())
				return vec<T, vec_size_v<V>>{};		// if we are here, then x is a zero vector, so return a zero vector

			[[likely]] return x / len;
		}

		//
		// vec4 ftransform() omitted
		//
		
		template <vec_like V1, vec_like V2, vec_like V3>
		requires same_vec_shape<V1, V2, V3> && floating_point_scalar<vec_scalar_t<V1>> && (vec_size_v<V1> > 1)
		[[nodiscard]] constexpr vec<vec_scalar_t<V1>, vec_size_v<V1>> faceforward(const V1 &n,
																				  const V2 &i,
																				  const V3 &nref) noexcept
		{
			using T = vec_scalar_t<V1>;

			return (dot(nref, i) < T(0)) ? +n : -n;
		}

		// n must be normalized in order to achieve desired results
		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>> && (vec_size_v<V1> > 1)
		[[nodiscard]] constexpr vec<vec_scalar_t<V1>, vec_size_v<V1>> reflect(const V1 &i,
																			  const V2 &n) noexcept
		{
			using T = vec_scalar_t<V1>;

			return i - T(2) * dot(n, i) * n;
		}

		// i and n must be normalized in order to achieve desired results
		template <vec_like V1, vec_like V2>
		requires same_vec_shape<V1, V2> && floating_point_scalar<vec_scalar_t<V1>> && (vec_size_v<V1> > 1)
		[[nodiscard]] constexpr vec<vec_scalar_t<V1>, vec_size_v<V1>> refract(const V1 &i,
																			  const V2 &n,
																			  vec_scalar_t<V1> eta) noexcept
		{
			using T = vec_scalar_t<V1>;
			constexpr std::size_t C = vec_size_v<V1>;

			T k = T(1) - eta * eta * (T(1) - dot(n, i) * dot(n, i));

			if (k < T(0))
				return vec<T, C>{};

			[[likely]] return eta * i - (eta * dot(n, i) + detail::cxcm::sqrt(k)) * n;
		}

		//
		// 8.8 - 8.19 are omitted
		//

		//
		// runtime swizzle function -- if 1 < number of indexes <= 4, returns a stand-alone vec
		// as opposed to an swizzle_vec union data member. If number of indexes == 1, returns a scalar
		// value for that indexed value. return value is *not* bound to the lifetime of the input argument,
		// unlike how v.xyz is a member of v. It will throw if the index arguments are out of bounds (index
		// arguments must be < C) or if the number of index arguments are not in range 1 <= num args <= 4.
		//
		// Not in GLSL -- inspired by the Odin Programming Language.
		//

		template <vec_like V, typename Arg>
		requires std::convertible_to<Arg, std::size_t> && dimensional_scalar<vec_scalar_t<V>>
		inline vec_scalar_t<V> swizzle(const V &v, const Arg &index)
		{
			constexpr std::size_t C = vec_size_v<V>;

			bool index_valid = (static_cast<std::size_t>(index) < C);

			if (!index_valid)
			{
				[[ unlikely ]] throw std::out_of_range("swizzle() index out of range");
			}

			[[ likely ]] return v[static_cast<std::size_t>(index)];
		}

		template <vec_like V, typename ...Args>
		requires (std::convertible_to<Args, std::size_t> && ...) && (sizeof...(Args) > 1) && (sizeof...(Args) <= 4) &&
				 dimensional_scalar<vec_scalar_t<V>>
		inline vec<vec_scalar_t<V>, sizeof...(Args)> swizzle(const V &v, const Args &...Is)
		{
			using T = vec_scalar_t<V>;
			constexpr std::size_t C = vec_size_v<V>;

			bool indexes_valid = ((static_cast<std::size_t>(Is) < C) && ...);

			if (!indexes_valid)
			{
				[[ unlikely ]] throw std::out_of_range("swizzle() indexes out of range");
			}

			[[ likely ]] return vec<T, sizeof...(Args)>{ v[static_cast<std::size_t>(Is)]... };
		}

	}	// namespace functions

	//
	// component-wise equality operator for vectors, scalar boolean result: ==, != (thanks to c++20).
	// most vector equality/inequality testing should use free functions equal()/notEqual(), but
	// these have a scalar result and are useful for unit testing.
	//

	// implicitly_convertible_to does not work with bool, so need another function for that case if we want to support that (which we don't)
	template <vec_like V1, vec_like V2>
	requires dimensional_scalar<vec_scalar_t<V1>> && dimensional_scalar<vec_scalar_t<V2>> &&
			 (std::convertible_to<vec_scalar_t<V1>, vec_scalar_t<V2>> || std::convertible_to<vec_scalar_t<V2>, vec_scalar_t<V1>>) &&
			 (vec_size_v<V1> == vec_size_v<V2>)
	constexpr bool operator ==(const V1 &first,
							   const V2 &second) noexcept
	{
		using commontype = std::common_type_t<vec_scalar_t<V1>, vec_scalar_t<V2>>;
		constexpr std::size_t C = vec_size_v<V1>;

		return [&first, &second]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return ((static_cast<commontype>(first[Is]) == static_cast<commontype>(second[Is])) && ...);
		}(std::make_index_sequence<C>{});
	}

	// when Count == 1, treat it like a scalar value for equality comparison
	template <vec_like V, dimensional_scalar R>
	requires (std::convertible_to<R, vec_scalar_t<V>> || std::convertible_to<vec_scalar_t<V>, R>) &&
	dimensional_scalar<vec_scalar_t<V>> && (vec_size_v<V> == 1)
	constexpr bool operator ==(const V &first,
							   R second) noexcept
	{
		using commontype = std::common_type_t<vec_scalar_t<V>, R>;
		return (static_cast<commontype>(first[0]) == static_cast<commontype>(second));
	}

	namespace dm_detail
	{
		// create a column vector that is to be part of a diagonal matrix, where all elements
		// are 0 except for the element at index, with the value being diagonal_number.
		template <floating_point_scalar T, std::size_t C>
		requires mat_dimension<C>
		[[nodiscard]] constexpr auto diagonal_column(T diagonal_number, std::size_t index)
		{
			DSGA_ASSERT(index < C, "diagonal_column() index out of range");

			return [diagonal_number, index]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return vec<T, C>{((Is == index) ? diagonal_number : T(0)) ...};
			}(std::make_index_sequence<C>{});
		}

		// create a column vector that is to be part of a diagonal matrix, where all elements
		// are 0 except for the element at index, with the value being diagonal_vector[index].
		template <vec_like V>
		requires mat_dimension<vec_size_v<V>> && floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr auto diagonal_column(const V &diagonal_vector, std::size_t index)
		{
			using T = vec_scalar_t<V>;
			constexpr std::size_t C = vec_size_v<V>;

			DSGA_ASSERT(index < C, "diagonal_column() index out of range");

			return diagonal_column<T, C>(diagonal_vector[index], index);
		}
	}	// namespace dm_detail

	//
	// mat
	//

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	requires mat_dimension<C> && mat_dimension<R>
	struct mat
	{
		static constexpr std::size_t ComponentCount = C * R;

		// number of columns
		[[nodiscard]] static constexpr int length() noexcept					{ return C; }

		// number of rows
		[[nodiscard]] static constexpr int column_length() noexcept				{ return R; }

		// returns number of columns (row size), not number of elements
		// not required by spec, but more c++ container-like
		static constexpr std::integral_constant<std::size_t, C> size =			{};

		// returns number of rows
		// not required by spec, but more c++ container-like
		static constexpr std::integral_constant<std::size_t, R> column_size =	{};

		// data storage for matrix
		std::array<vec<T, R>, C> columns;

		// using directives related to storage
		using value_type = vec<T, R>;
		using iterator = std::array<vec<T, R>, C>::iterator;
		using const_iterator = std::array<vec<T, R>, C>::const_iterator;
		using reverse_iterator = std::array<vec<T, R>, C>::reverse_iterator;
		using const_reverse_iterator = std::array<vec<T, R>, C>::const_reverse_iterator;

		//
		// operator [] gets the column vector
		//

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr vec<T, R> &operator [](U index)
		{
			std::size_t idx = static_cast<std::size_t>(index);
			DSGA_ASSERT(idx < C, "index out of range");
			return columns[idx];
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const vec<T, R> &operator [](U index) const
		{
			std::size_t idx = static_cast<std::size_t>(index);
			DSGA_ASSERT(idx < C, "index out of range");
			return columns[idx];
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr vec<T, R> &at(U index)
		{
			std::size_t idx = static_cast<std::size_t>(index);
//			DSGA_ASSERT(idx < C, "index out of range");
			// use std::array::at() to get the column vector, which will throw if the index is out of range
			return columns.at(idx);
		}

		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr const vec<T, R> &at(U index) const
		{
			std::size_t idx = static_cast<std::size_t>(index);
//			DSGA_ASSERT(idx < C, "index out of range");
			// use std::array::at() to get the column vector, which will throw if the index is out of range
			return columns.at(idx);
		}

		// get a row of the matrix as a vector
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr vec<T, C> row(U row_index) const
		{
			std::size_t idx = static_cast<std::size_t>(row_index);
			DSGA_ASSERT(idx < R, "row index out of range");

			// for each column of the matrix, get a row component, and bundle
			// these components up into a vector that represents the row
			return[this, &idx]<std::size_t ...Is>(std::index_sequence<Is...>) -> vec<T, C>
			{
				return vec<T, C>{ columns[Is][idx]... };
			}(std::make_index_sequence<C>{});
		}

		// set a row of the matrix
		template <typename U, vec_like V>
		requires std::convertible_to<U, std::size_t> && std::convertible_to<vec_scalar_t<V>, T> && (vec_size_v<V> == C)
		constexpr void row(U row_index, const V &v)
		{
			std::size_t idx = static_cast<std::size_t>(row_index);
			DSGA_ASSERT(idx < R, "row index out of range");

			// for each column of the matrix, get a row component, and bundle
			// these components up into a vector that represents the row
			[this, &idx, &v]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((columns[Is][idx] = static_cast<T>(v[Is])),...);
			}(std::make_index_sequence<C>{});
		}

		// get a row of the matrix as a vector
		template <typename U>
		requires std::convertible_to<U, std::size_t>
		[[nodiscard]] constexpr vec<T, R> column(U column_index) const
		{
			std::size_t idx = static_cast<std::size_t>(column_index);
			DSGA_ASSERT(idx < C, "column index out of range");

			// columns are stored as vectors, so just return the column vector
			return columns[idx];
		}

		// set a column of the matrix
		template <typename U, vec_like V>
		requires std::convertible_to<U, std::size_t> && std::convertible_to<vec_scalar_t<V>, T> && (vec_size_v<V> == R)
		constexpr void column(U column_index, const V &v)
		{
			std::size_t idx = static_cast<std::size_t>(column_index);
			DSGA_ASSERT(idx < C, "column index out of range");

			// columns are stored as vectors, so just copy the vector
			columns[idx] = v;
		}

		// get the diagonal of the matrix as a vector
		[[nodiscard]] constexpr vec<T, C> diagonal() const requires (C == R)
		{
			// get the diagonal elements and bundle them into a vector that represents the diagonal
			return [this]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				return vec<T, C>{ columns[Is][Is]... };
			}(std::make_index_sequence<C>{});
		}

		// set the diagonal of the matrix
		template <typename U, vec_like V>
		requires std::convertible_to<vec_scalar_t<V>, T> && (C == R) && (vec_size_v<V> == C)
		constexpr void diagonal(const V &v)
		{
			// for each column of the matrix, get a row component, and bundle
			// these components up into a vector that represents the row
			[this, &v]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((columns[Is][Is] = static_cast<T>(v[Is])),...);
			}(std::make_index_sequence<C>{});
		}

		// get the trace of the matrix
		[[nodiscard]] constexpr T trace() const
		requires (C == R)
		{
			return diagonal().sum();
		}

		//
		// defaulted functions
		//

		constexpr mat() noexcept = default;
		constexpr ~mat() noexcept = default;

		constexpr mat(const mat &) noexcept = default;
		constexpr mat(mat &&) noexcept = default;
		constexpr mat &operator =(const mat &) & noexcept = default;
		constexpr mat &operator =(mat &&) & noexcept = default;

		//
		// constructors
		//

		// variadic constructor of scalar and vector arguments
		template <typename U, typename ... Args>
		requires (detail::valid_matrix_component<U, T>::value) && (detail::valid_matrix_component<Args, T>::value && ...) &&
				 detail::met_component_count<ComponentCount, U, Args...>
		explicit constexpr mat(const U &u, const Args & ...args) noexcept
			: columns{}
		{
			auto arg_tuple = detail::flatten_args_to_tuple(u, args...);
			[this, &arg_tuple]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				(([this, &arg_tuple]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
				{
					constexpr std::size_t Col = Is;
					columns.at(Col).set(std::get<Col * R + Js>(arg_tuple)...);
				}(std::make_index_sequence<R>{})), ...);
			}(std::make_index_sequence<C>{});
		}

		// diagonal constructor for square matrices
		template <typename U>
		requires std::convertible_to<U, T> && (C == R)
		explicit constexpr mat(U arg) noexcept
		{
			[this, arg] <std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				(((*this)[Is] = dm_detail::diagonal_column<T, C>(static_cast<T>(arg), Is)),...);
			}(std::make_index_sequence<C>{});
		}

		// implicit constructor from a matrix - uses implicitly convertible vector assignment
		template <floating_point_scalar U>
		requires implicitly_convertible_to<U, T>
		explicit(false) constexpr mat(const mat<U, C, R> &arg) noexcept
		{
			[this, &arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((columns.at(Is) = arg[Is]), ...);
			}(std::make_index_sequence<C>{});
		}

		// explicit constructor from a matrix
		template <floating_point_scalar U, std::size_t Cols, std::size_t Rows>
		explicit constexpr mat(const mat<U, Cols, Rows> &arg) noexcept
			: columns{}
		{
			[this, &arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				(([this, &arg]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
				{
					constexpr std::size_t Col = Is;
					((columns.at(Col)[Js] = static_cast<T>(arg[Col][Js])), ...);
				}(std::make_index_sequence<std::min(R, Rows)>{})), ...);
			}(std::make_index_sequence<std::min(C, Cols)>{});

			// for square matrix, extend identity diagonal as needed
			if constexpr (C == R)
			{
				[this]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
				{
					((columns.at(Is)[Is] = T(1.0)), ...);
				}(make_index_range<std::min(std::min(Cols, C), std::min(Rows, R)), C>{});
			}
		}

		//
		// assignment operators
		//

		template <floating_point_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr mat &operator =(const mat<U, C, R> &other) & noexcept
		{
			[this, &other]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((columns.at(Is) = other[Is]), ...);			// let vec do any type conversion if needed
			}(std::make_index_sequence<C>{});

			return *this;
		}

		constexpr void swap(mat &bm) noexcept										{ columns.swap(bm.columns); }

		// support for range-based for loop -- gives column vectors
		[[nodiscard]] constexpr		  iterator			begin() noexcept			{ return columns.begin(); }
		[[nodiscard]] constexpr const_iterator			begin() const noexcept		{ return columns.cbegin(); }
		[[nodiscard]] constexpr const_iterator			cbegin() const noexcept		{ return begin(); }
		[[nodiscard]] constexpr		  iterator			end() noexcept				{ return columns.end(); }
		[[nodiscard]] constexpr const_iterator			end() const noexcept		{ return columns.cend(); }
		[[nodiscard]] constexpr const_iterator			cend() const noexcept		{ return end(); }

		[[nodiscard]] constexpr		  reverse_iterator	rbegin() noexcept			{ return columns.rbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	rbegin() const noexcept		{ return columns.crbegin(); }
		[[nodiscard]] constexpr const_reverse_iterator	crbegin() const noexcept	{ return rbegin(); }
		[[nodiscard]] constexpr		  reverse_iterator	rend() noexcept				{ return columns.rend(); }
		[[nodiscard]] constexpr const_reverse_iterator	rend() const noexcept		{ return columns.crend(); }
		[[nodiscard]] constexpr const_reverse_iterator	crend() const noexcept		{ return rend(); }

	};	// struct mat

	// swap specialization
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr void swap(mat<T, C, R> &lhs, mat<T, C, R> &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	//
	// get<> part of tuple protocol -- needed for structured bindings
	//

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr vec<T, R> & get(mat<T, C, R> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr const vec<T, R> & get(const mat<T, C, R> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	[[nodiscard]] constexpr auto && get(mat<T, C, R> && arg) noexcept
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
		template <floating_point_scalar T, std::size_t C, std::size_t R>
		[[nodiscard]] constexpr mat<T, C, R> matrixCompMult(const mat<T, C, R> &lhs,
															const mat<T, C, R> &rhs) noexcept
		{
			return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				return mat<T, C, R>{ (lhs[Is] * rhs[Is])... };
			}(std::make_index_sequence<C>{});
		}

		// outerProduct() - matrix from a column vector times a row vector
		template <vec_like V1, vec_like V2>
		requires mat_dimension<vec_size_v<V1>> && mat_dimension<vec_size_v<V2>> &&
				 std::same_as<vec_scalar_t<V1>, vec_scalar_t<V2>> && floating_point_scalar<vec_scalar_t<V1>>
		[[nodiscard]] constexpr mat<vec_scalar_t<V1>, vec_size_v<V2>, vec_size_v<V1>> outerProduct(const V1 &lhs,
																								   const V2 &rhs) noexcept
		{
			using T = vec_scalar_t<V1>;
			constexpr std::size_t C1 = vec_size_v<V1>;
			constexpr std::size_t C2 = vec_size_v<V2>;

			mat<T, C2, C1> val;

			[&val, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((val[Is] = lhs * rhs[Is]), ...);
			}(std::make_index_sequence<C2>{});

			return val;
		}

		// transpose a matrix
		template <floating_point_scalar T, std::size_t C, std::size_t R>
		[[nodiscard]] constexpr mat<T, R, C> transpose(const mat<T, C, R> &arg) noexcept
		{
			mat<T, R, C> val;

			[&val, &arg] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((val[Is] = arg.row(Is)), ...);
			}(std::make_index_sequence<R>{});

			return val;
		}

		// determinant() - only on square matrices

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T determinant(const mat<T, 2, 2> &arg) noexcept
		{
			return 
				+ arg[0][0] * arg[1][1]
				- arg[0][1] * arg[1][0]
				;
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr T determinant(const mat<T, 3, 3> &arg) noexcept
		{
			// same results as dot(arg[0], cross(arg[1], arg[2]))
			return
				+ arg[0][0] * ( arg[1][1] * arg[2][2] - arg[1][2] * arg[2][1] ) 
				- arg[0][1] * ( arg[1][0] * arg[2][2] - arg[1][2] * arg[2][0] ) 
				+ arg[0][2] * ( arg[1][0] * arg[2][1] - arg[1][1] * arg[2][0] ) 
				;
		}

		// Source - https://stackoverflow.com/a/44446912
		// Posted by willnode, modified by community. See post 'Timeline' for change history
		// Retrieved 2026-04-27, License - CC BY-SA 4.0
		template <floating_point_scalar T>
		[[nodiscard]] constexpr T determinant(const mat<T, 4, 4> &m) noexcept
		{
			auto A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
			auto A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
			auto A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];

			auto A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
			auto A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
			auto A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];

			return
				+ m[0][0] * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223)
				- m[0][1] * (m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223)
				+ m[0][2] * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123)
				- m[0][3] * (m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123)
				;
		}

		// inverse() - only on square matrices

		template <floating_point_scalar T>
		[[nodiscard]] constexpr mat<T, 2, 2> inverse(const mat<T, 2, 2> &arg) noexcept
		{
			auto det = determinant(arg);
			if (abs(det) <= std::numeric_limits<T>::epsilon())
			{
				return mat<T, 2, 2>{};				// return a zero matrix if singular
			}

			// multiplying by reciprocal is faster than dividing each element by det
			det = T(1) / det;

			return mat<T, 2, 2>
			{
				det * +arg[1][1],
				det * -arg[0][1],
				det * -arg[1][0],
				det * +arg[0][0]
			};
		}

		template <floating_point_scalar T>
		[[nodiscard]] constexpr mat<T, 3, 3> inverse(const mat<T, 3, 3> &arg) noexcept
		{
			auto det = determinant(arg);
			if (abs(det) <= std::numeric_limits<T>::epsilon())
			{
				return mat<T, 3, 3>{};				// return a zero matrix if singular
			}

			// multiplying by reciprocal is faster than dividing each element by det
			det = T(1) / det;

			return mat<T, 3, 3>
			{
				det * +(arg[1][1] * arg[2][2] - arg[2][1] * arg[1][2]),
				det * -(arg[0][1] * arg[2][2] - arg[2][1] * arg[0][2]),
				det * +(arg[0][1] * arg[1][2] - arg[1][1] * arg[0][2]),
				det * -(arg[1][0] * arg[2][2] - arg[2][0] * arg[1][2]),
				det * +(arg[0][0] * arg[2][2] - arg[2][0] * arg[0][2]),
				det * -(arg[0][0] * arg[1][2] - arg[1][0] * arg[0][2]),
				det * +(arg[1][0] * arg[2][1] - arg[2][0] * arg[1][1]),
				det * -(arg[0][0] * arg[2][1] - arg[2][0] * arg[0][1]),
				det * +(arg[0][0] * arg[1][1] - arg[1][0] * arg[0][1])
			};
		}

		// Source - https://stackoverflow.com/a/44446912
		// Posted by willnode, modified by community. See post 'Timeline' for change history
		// Retrieved 2026-04-27, License - CC BY-SA 4.0
		template <floating_point_scalar T>
		[[nodiscard]] constexpr mat<T, 4, 4> inverse(const mat<T, 4, 4> &m) noexcept
		{
			auto A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
			auto A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
			auto A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];

			auto A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
			auto A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
			auto A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];

			auto A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
			auto A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
			auto A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];

			auto A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
			auto A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
			auto A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];

			auto A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
			auto A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
			auto A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];

			auto A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
			auto A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
			auto A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

			auto det =
				+ m[0][0] * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223)
				- m[0][1] * (m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223)
				+ m[0][2] * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123)
				- m[0][3] * (m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123);

			if (abs(det) <= std::numeric_limits<T>::epsilon())
			{
				return mat<T, 4, 4>{};				// return a zero matrix if singular
			}

			// multiplying by reciprocal is faster than dividing each element by det
			det = T(1) / det;

			return mat<T, 4, 4>
			{
				// m00 -m03
				det * +(m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223),
				det * -(m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223),
				det * +(m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213),
				det * -(m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212),

				// m10 -m13
				det * -(m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223),
				det * +(m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223),
				det * -(m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213),
				det * +(m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212),

				// m20 -m23
				det * +(m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123),
				det * -(m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123),
				det * +(m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113),
				det * -(m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112),

				// m30 -m33
				det * -(m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123),
				det * +(m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123),
				det * -(m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113),
				det * +(m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112),
			};
		}

		// not in glsl
		//
		// returns a skew symmetric matrix that can be used for computing the cross product. vector and matrix are 3D.
		//
		// cross(u, v) == cross_matrix(u) * v == u * cross_matrix(v)
		template <vec_like V>
		requires floating_point_scalar<vec_scalar_t<V>> && (vec_size_v<V> == 3)
		[[nodiscard]] constexpr mat<vec_scalar_t<V>, 3, 3> cross_matrix(const V &vec) noexcept
		{
			using T = vec_scalar_t<V>;

			return mat<T, 3, 3>{ T(0),	  vec[2], -vec[1],
								-vec[2],  T(0),	   vec[0],
								 vec[1], -vec[0],  T(0) };
		}

		// not in glsl
		//
		// returns a symmetric diagonal matrix (square) using the vector parameter as the diagonal values,
		// with all other elements being 0.
		//
		template <vec_like V>
		requires mat_dimension<vec_size_v<V>> && floating_point_scalar<vec_scalar_t<V>>
		[[nodiscard]] constexpr mat<vec_scalar_t<V>, vec_size_v<V>, vec_size_v<V>> diagonal_matrix(const V &vec) noexcept
		{
			using T = vec_scalar_t<V>;
			constexpr std::size_t C = vec_size_v<V>;

			mat<T, C, C> val;

			[&val, &vec] <std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((val[Is] = dm_detail::diagonal_column(vec, Is)),...);
			}(std::make_index_sequence<C>{});

			return val;
		}

		// not in glsl
		//
		// make an identity matrix
		template <floating_point_scalar T, std::size_t C>
		requires mat_dimension<C>
		[[nodiscard]] constexpr mat<T, C, C> identity_matrix() noexcept
		{
			return mat<T, C, C>(1);
		}

	}	// namespace functions

	//
	// matrix operators
	//

	// component-wise equality operator for matrices, scalar boolean result: ==, != (thanks to c++20)
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr bool operator ==(const mat<T, C, R> &lhs,
							   const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return ((lhs[Is] == rhs[Is]) && ...);
		}(std::make_index_sequence<C>{});
	}

	// unary operators

	// unary +
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr mat<T, C, R> operator +(const mat<T, C, R> &arg) noexcept
	{
		return mat<T, C, R>(arg);
	}

	// unary -
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr mat<T, C, R> operator -(const mat<T, C, R> &arg) noexcept
	{
		return [&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (-arg[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// pre-increment
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr mat<T, C, R> &operator ++(mat<T, C, R> &arg) noexcept
	{
		[&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((++arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return arg;
	}

	// post-increment
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr mat<T, C, R> operator ++(mat<T, C, R> &arg, int) noexcept
	{
		mat<T, C, R> value(arg);
		[&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((++arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return value;
	}

	// pre-decrement
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr mat<T, C, R> &operator --(mat<T, C, R> &arg) noexcept
	{
		[&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((--arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return arg;
	}

	// post-decrement
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	constexpr mat<T, C, R> operator --(mat<T, C, R> &arg, int) noexcept
	{
		mat<T, C, R> value(arg);
		[&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((--arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return value;
	}

	// operator + with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator +(const mat<T, C, R> &lhs,
													U rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] + static_cast<T>(rhs))... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator +(U lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (static_cast<T>(lhs) + rhs[Is])...};
		}(std::make_index_sequence<C>{});
	}

	// operator - with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator -(const mat<T, C, R> &lhs,
													U rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] - static_cast<T>(rhs))... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator -(U lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (static_cast<T>(lhs) - rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator * with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator *(const mat<T, C, R> &lhs,
													U rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] * static_cast<T>(rhs))... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator *(U lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (static_cast<T>(lhs) * rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator / with scalar

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator /(const mat<T, C, R> &lhs,
													U rhs) noexcept
	{
		auto reciprocal = T(1) / static_cast<T>(rhs);		// multiplying by reciprocal is faster than dividing each element by rhs
		return [&lhs, reciprocal] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] * reciprocal)... };
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	[[nodiscard]] constexpr mat<T, C, R> operator /(U lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (static_cast<T>(lhs) / rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator + with same size matrices

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr mat<T, C, R> operator +(const mat<T, C, R> &lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] + rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator - with same size matrices

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr mat<T, C, R> operator -(const mat<T, C, R> &lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] - rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// operator / with same size matrices

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr mat<T, C, R> operator /(const mat<T, C, R> &lhs,
													const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return mat<T, C, R>{ (lhs[Is] / rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	//
	// linear-algebriac binary ops
	//

	// matrix * (column) vector => (column) vector

	template <floating_point_scalar T, std::size_t C, std::size_t R, vec_like V>
	requires (vec_size_v<V> == C) && std::same_as<vec_scalar_t<V>, T>
	[[nodiscard]] constexpr vec<T, R> operator *(const mat<T, C, R> &lhs,
												 const V &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return (... + (lhs[Is] * rhs[Is]));
		}(std::make_index_sequence<C>{});
	}

	// (row) vector * matrix => (row) vector

	template <floating_point_scalar T, std::size_t C, std::size_t R, vec_like V>
	requires (vec_size_v<V> == R) && std::same_as<vec_scalar_t<V>, T>
	[[nodiscard]] constexpr vec<T, R> operator *(const V &lhs,
												 const mat<T, C, R> &rhs) noexcept
	{
		return [&lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return vec{ functions::dot(lhs, rhs[Is])... };
		}(std::make_index_sequence<C>{});
	}

	// matrix * matrix => matrix

	template <floating_point_scalar T, std::size_t C1, std::size_t R1, std::size_t C2, std::size_t R2>
	requires (C1 == R2)
	[[nodiscard]] constexpr mat<T, C2, R1> operator *(const mat<T, C1, R1> &lhs,
													  const mat<T, C2, R2> &rhs) noexcept
	{
		mat<T, C2, R1> val;

		[&val, &lhs, &rhs]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			 ((val[Is] = lhs * rhs[Is]), ...);
		}(std::make_index_sequence<C2>{});

		return val;
	}

	//
	// specialized using types
	//

	// boolean vectors
	using bscal = vec<bool, 1>;
	using bvec2 = vec<bool, 2>;
	using bvec3 = vec<bool, 3>;
	using bvec4 = vec<bool, 4>;

	// int vectors
	using iscal = vec<int, 1>;
	using ivec2 = vec<int, 2>;
	using ivec3 = vec<int, 3>;
	using ivec4 = vec<int, 4>;

	// unsigned int vectors
	using uscal = vec<unsigned, 1>;
	using uvec2 = vec<unsigned, 2>;
	using uvec3 = vec<unsigned, 3>;
	using uvec4 = vec<unsigned, 4>;

	// long long vectors (not in glsl)
	using llscal = vec<long long, 1>;
	using llvec2 = vec<long long, 2>;
	using llvec3 = vec<long long, 3>;
	using llvec4 = vec<long long, 4>;

	// unsigned long long vectors (not in glsl)
	using ullscal = vec<unsigned long long, 1>;
	using ullvec2 = vec<unsigned long long, 2>;
	using ullvec3 = vec<unsigned long long, 3>;
	using ullvec4 = vec<unsigned long long, 4>;

	// float vectors with out an 'f' prefix -- this is from glsl
	using scal = vec<float, 1>;
	using vec2 = vec<float, 2>;
	using vec3 = vec<float, 3>;
	using vec4 = vec<float, 4>;

	// also float vectors, but using the same naming convention as the other vectors do (not in glsl)
	using fscal = vec<float, 1>;
	using fvec2 = vec<float, 2>;
	using fvec3 = vec<float, 3>;
	using fvec4 = vec<float, 4>;

	// double vectors
	using dscal = vec<double, 1>;
	using dvec2 = vec<double, 2>;
	using dvec3 = vec<double, 3>;
	using dvec4 = vec<double, 4>;

	// float matrices
	using mat2x2 = mat<float, 2, 2>;
	using mat2x3 = mat<float, 2, 3>;
	using mat2x4 = mat<float, 2, 4>;
	using mat3x2 = mat<float, 3, 2>;
	using mat3x3 = mat<float, 3, 3>;
	using mat3x4 = mat<float, 3, 4>;
	using mat4x2 = mat<float, 4, 2>;
	using mat4x3 = mat<float, 4, 3>;
	using mat4x4 = mat<float, 4, 4>;

	using mat2 = mat<float, 2, 2>;
	using mat3 = mat<float, 3, 3>;
	using mat4 = mat<float, 4, 4>;

	// double matrices
	using dmat2x2 = mat<double, 2, 2>;
	using dmat2x3 = mat<double, 2, 3>;
	using dmat2x4 = mat<double, 2, 4>;
	using dmat3x2 = mat<double, 3, 2>;
	using dmat3x3 = mat<double, 3, 3>;
	using dmat3x4 = mat<double, 3, 4>;
	using dmat4x2 = mat<double, 4, 2>;
	using dmat4x3 = mat<double, 4, 3>;
	using dmat4x4 = mat<double, 4, 4>;

	using dmat2 = mat<double, 2, 2>;
	using dmat3 = mat<double, 3, 3>;
	using dmat4 = mat<double, 4, 4>;

	//
	// bring the vector and matrix free functions into the dsga namespace
	//
	using namespace functions;

	//
	// converting from external vector type or data to internal vector type
	//

	template <dimensional_scalar T, std::size_t S>
	requires dimensional_storage<T, S>
	[[nodiscard]] constexpr vec<T, S> to_vector(const std::array<T, S> &arg) noexcept
	{
		return [&arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return vec<T, S>{ arg[Is]... };
		}(std::make_index_sequence<S>{});
	}

	template <dimensional_scalar T, std::size_t S>
	requires dimensional_storage<T, S>
	[[nodiscard]] constexpr vec<T, S> to_vector(const T(&arg)[S]) noexcept
	{
		return [&arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return vec<T, S>{ arg[Is]... };
		}(std::make_index_sequence<S>{});
	}

	// converting from internal vector type to std::array

	template <vec_like V>
	requires dimensional_storage<vec_scalar_t<V>, vec_size_v<V>>
	[[nodiscard]] constexpr std::array<vec_scalar_t<V>, vec_size_v<V>> to_array(const V &arg) noexcept
	{
		using T = vec_scalar_t<V>;
		constexpr std::size_t C = vec_size_v<V>;

		return[&arg]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return std::array<T, C>{ arg[Is]... };
		}(std::make_index_sequence<C>{});
	}

	// converting from array to a mat

	template <std::size_t C, std::size_t R, floating_point_scalar T, std::size_t S>
	requires (mat_dimension<C> && mat_dimension<R>) && (C * R <= S)
	[[nodiscard]] constexpr mat<T, C, R> to_matrix(const std::array<T, S> &arg) noexcept
	{
		return [&arg]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return mat<T, C, R>(
				[&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
				{
					constexpr auto cols = Js;
					return vec<T, R>{ arg[cols * R + Is]... };
				}(std::make_index_sequence<R>{}) ...);
		}(std::make_index_sequence<C>{});
	}

	template <std::size_t C, std::size_t R, floating_point_scalar T, std::size_t S>
	requires (mat_dimension<C> && mat_dimension<R>) && (C * R <= S)
	[[nodiscard]] constexpr mat<T, C, R> to_matrix(const T(&arg)[S]) noexcept
	{
		return [&arg]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return mat<T, C, R>(
				[&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
				{
					constexpr auto cols = Js;
					return vec<T, R>{ arg[cols * R + Is]... };
				}(std::make_index_sequence<R>{}) ...);
		}(std::make_index_sequence<C>{});
	}

	// converting from internal matrix type to std::array

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	requires (mat_dimension<C> && mat_dimension<R>)
	[[nodiscard]] constexpr std::array<T, C * R> to_array(const mat<T, C, R> &arg) noexcept
	{
		auto matrix_tuple = [&arg]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return detail::flatten_args_to_tuple(arg[Is]...);
		}(std::make_index_sequence<C>{});

		return [&matrix_tuple]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return std::array<T, C * R>{ std::get<Js>(matrix_tuple)... };
		}(std::make_index_sequence<C * R>{});
	}

	//
	// deprecated aliases for backward compatibility
	//

	template <dimensional_scalar T, std::size_t S>
	using basic_vector [[deprecated("use vec instead")]] = vec<T, S>;

	template <floating_point_scalar T, std::size_t C, std::size_t R>
	using basic_matrix [[deprecated("use mat instead")]] = mat<T, C, R>;

	template <bool W, dimensional_scalar T, std::size_t Count, typename Derived>
	using vector_base [[deprecated("use vec_interface instead")]] = vec_interface<W, T, Count, Derived>;

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	using indexed_vector [[deprecated("use swizzle_vec instead")]] = swizzle_vec<T, Size, Count, Is...>;

	template <dimensional_scalar T, std::size_t Size>
	using storage_wrapper [[deprecated("use vec_storage instead")]] = vec_storage<T, Size>;

}	// namespace dsga

//
// tuple protocol for vec and swizzle_vec and vec_base -- supports structured bindings
//

template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::vec_storage<T, S>> : std::integral_constant<std::size_t, S>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::vec_storage<T, S>>
{
	using type = T;
};

template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::vec<T, S>> : std::integral_constant<std::size_t, S>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::vec<T, S>>
{
	using type = T;
};

template <dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_size<dsga::swizzle_vec<T, S, C, Is...>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_element<I, dsga::swizzle_vec<T, S, C, Is...>>
{
	using type = T;
};

template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_size<dsga::vec_interface<W, T, C, D>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_element<I, dsga::vec_interface<W, T, C, D>>
{
	using type = T;
};

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::tuple_size<dsga::mat<T, C, R>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::tuple_element<I, dsga::mat<T, C, R>>
{
	using type = dsga::vec<T, R>;
};

// closing include guard
#endif
