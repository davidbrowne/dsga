#pragma once

//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <limits>
#include <type_traits>
#include <concepts>
#include <cmath>

//
// ConstXpr CMath (cxcm)
//

// version info

constexpr inline int CXCM_MAJOR_VERSION = 0;
constexpr inline int CXCM_MINOR_VERSION = 1;
constexpr inline int CXCM_PATCH_VERSION = 3;

namespace cxcm
{

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

		template <std::floating_point T>
		constexpr T fabs(T value) noexcept
		{
			return abs(value);
		}

		//
		// trunc()
		//

		// this is the workhorse function for floor(), ceil(), and round().

		// rounds towards zero

		// works for double and long double, as long as:
		// std::numeric_limits<long double>::digits <= 64

		template <std::floating_point T>
		constexpr T trunc(T value) noexcept requires (std::numeric_limits<T>::digits <= 64)
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
		// sqrt()
		//

		namespace detail
		{
			//	By itself, converging_sqrt() over all the 32-bit floats gives:
			//		75% of the time gives same answer as std::sqrt()
			//		25% of the time gives answer within 1 ulp of std::sqrt()
			template <std::floating_point T>
			constexpr T converging_sqrt(T arg) noexcept
			{
				T current_value = arg;
				T previous_value = T(0);

				while (current_value != previous_value)
				{
					previous_value = current_value;
					current_value = (T(0.5) * current_value) + (T(0.5) * (arg / current_value));
				}

				return current_value;
			}

			// 2 refinements:
			// all floats: 71.05% same as reciprocal of std::sqrt(), 28.95% apparently within 1 ulp.
			// a double sample of (2^52, 2^52 + 2^31-1] 62.01% exact match, 37.99% apparently within 1 ulp
			// best for implementing rsqrt()
			//
			// 3 refinements:
			// all floats: 62.65% same as reciprocal of std::sqrt(), 37.35% apparently within 1 ulp.
			// a double sample of (2^52, 2^52 + 2^31-1] 50% exact match, 50% apparently within 1 ulp
			// best for implementing sqrt()
			template <std::floating_point T>
			constexpr T inverse_sqrt(T arg) noexcept
			{
				T current_value = T(1.0) / converging_sqrt(arg);

				current_value += T(0.5) * current_value * (T(1.0) - arg * current_value * current_value);					// first refinement
				current_value += T(0.5) * current_value * (T(1.0) - arg * current_value * current_value);					// second refinement

				current_value += T(0.5) * current_value * (T(1.0) - arg * current_value * current_value);					// third refinement
				return current_value;
			}
		}

		// square root
		//	This version with inverse_sqrt(), when used over all the 32-bit floats gives:
		//
		//	with 3 refinements:
		//		all floats: 76.5% same result as std::sqrt, 23.5% apparently within 1 ulp
		//		a double sample of (2^52, 2^52 + 2^31-1] 99.99923% exact match, 0.0007717% apparently within 1 ulp
		//
		template <std::floating_point T>
		constexpr T sqrt(T arg) noexcept
		{
			return arg * detail::inverse_sqrt(arg);
		}

		// float specialization - uses double internally - relied upon by rsqrt<T>() when [T = float]
		//		100% match with std::sqrt
		template <>
		constexpr float sqrt(float value) noexcept
		{
			return static_cast<float>(sqrt(static_cast<double>(value)));
		}

		// reciprocal of square root
		//	all floats (no specializations): 84.75% same result as reciprocal of std::sqrt(), 15.25% apparently within 1 ulp
		//	all floats (sqrt() specialization): 100% same result as reciprocal of std::sqrt()
		//	a double sample starting after 2^52 for INT_MAX next values: 99.99923% exact match, 0.0007717% apparently within 1 ulp
		//	a double sample starting after 1.25 for INT_MAX next values: 90.34% exact match, 9.66% apparently within 1 ulp
		//	a double sample starting after 123456.789 for INT_MAX next values: 86.84% exact match, 13.16% apparently within 1 ulp
		//	a double sample starting after 0.0 for INT_MAX next values: 86.17% exact match, 13.83% apparently within 1 ulp (11.5 hrs to calc this)
		//	a double sample starting before std::numeric_limits<double>::max() for INT_MAX prev values: 84.81% exact match, 15.19% apparently within 1 ulp (1.5 hrs to calc this)
		template <std::floating_point T>
		constexpr T rsqrt(T arg) noexcept
		{
			return T(1.0) / sqrt(arg);
		}

		// this specialization is not necessary given sqrt() float specialization and our simple generic rsqrt()
		//
		//// float specialization - uses double internally
		////		100% match with (1.0f / std::sqrt) when using sqrt() float specialization
		////		all floats (no specializations): 74.01% match with (1.0f / std::sqrt), 25.99% apparently within 1 ulp
		//template <>
		//constexpr float rsqrt(float value) noexcept
		//{
		//	return static_cast<float>(rsqrt(static_cast<double>(value)));
		//}

	} // namespace relaxed

	//
	// isnan()
	//

	// I've seen comments on the Microsoft/STL Github Issue that tracks where they are implementing
	// std::isnan for c++20, and said that on some compilers with various compiler switches, what
	// we are using here, (x != x) or !(x == x), can be optimized away, so this is not a good practice. So
	// microsoft is in the process of making this stuff constexpr, and they need to have a compiler
	// intrinsic to do it. https://github.com/microsoft/STL/issues/65#issuecomment-563886838

#if defined(_MSC_VER)
#pragma float_control(precise, on, push)
#endif

	template <std::floating_point T>
	constexpr bool isnan(T value) noexcept
	{
		return (value != value);
	}

#if defined(_MSC_VER)
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
			// constexpr_sqrt()
			//

			template <std::floating_point T>
			constexpr T constexpr_sqrt(T value) noexcept
			{
				// screen out unnecessary input

				// arg == +infinity or +/-0, return val unmodified
				// arg == NaN, return Nan
				if (!isnormal_or_subnormal(value))
					return value;

				// arg < 0, return NaN
				if (value < T(0.0))
					return std::numeric_limits<T>::quiet_NaN();

				return relaxed::sqrt(value);
			}

			//
			// constexpr_inverse_sqrt()
			//

			template <std::floating_point T>
			constexpr T constexpr_rsqrt(T value) noexcept
			{
				// screen out unnecessary input

				// arg == NaN, return Nan
				if (isnan(value))
					return value;

				// arg == +infinity , return 0
				if (value == std::numeric_limits<T>::infinity())
					return T(0.0);

				// arg == -infinity or +/-0, return Nan
				if (!isnormal_or_subnormal(value))
					return std::numeric_limits<T>::quiet_NaN();

				// arg <= 0, return NaN
				if (value <= T(0.0))
					return std::numeric_limits<T>::quiet_NaN();

				return relaxed::rsqrt(value);
			}

		} // namespace detail

		//
		// abs(), fabs()
		//

#if 1

		// absolute value

		template <std::floating_point T>
		constexpr T abs(T value) noexcept
		{
			if (!detail::isnormal_or_subnormal(value))
				return value;

			return relaxed::abs(value);
		}

#else

		// absolute value
		// better optimized, but rejects philosophy of leaving the details to relaxed namespace

		template <std::floating_point T>
		constexpr T abs(T value) noexcept
		{
			if (!detail::isnormal_or_subnormal(value) || (value > T(0)))
				return value;

			return -value;
		}

#endif

		template <std::floating_point T>
		constexpr T fabs(T value) noexcept
		{
			return abs(value);
		}


		//
		// trunc()
		//

#if !defined(CXCM_DISABLE_RUNTIME_OPTIMIZATIONS) && (defined(_DEBUG) || defined(_M_IX86))

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

#else

		// rounds towards zero

		template <std::floating_point T>
		constexpr T trunc(T value) noexcept
		{
			return detail::constexpr_trunc(value);
		}

#endif

		//
		// floor()
		//

#if !defined(CXCM_DISABLE_RUNTIME_OPTIMIZATIONS) && (defined(_DEBUG) || defined(_M_IX86))

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

#else

		// rounds towards negative infinity

		template <std::floating_point T>
		constexpr T floor(T value) noexcept
		{
			return detail::constexpr_floor(value);
		}

#endif

		//
		// ceil()
		//

#if !defined(CXCM_DISABLE_RUNTIME_OPTIMIZATIONS) && (defined(_DEBUG) || defined(_M_IX86))

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

#else

		// rounds towards positive infinity

		template <std::floating_point T>
		constexpr T ceil(T value) noexcept
		{
			return detail::constexpr_ceil(value);
		}

#endif

		//
		// round()
		//

#if !defined(CXCM_DISABLE_RUNTIME_OPTIMIZATIONS) && (defined(_DEBUG) || defined(_M_IX86))

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

#else

		// rounds to nearest integral position, halfway cases away from zero

		template <std::floating_point T>
		constexpr T round(T value) noexcept
		{
			return detail::constexpr_round(value);
		}

#endif

		//
		// sqrt()
		//

#if !defined(CXCM_DISABLE_RUNTIME_OPTIMIZATIONS) && (defined(_DEBUG) || defined(_M_IX86))

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

#else

		template <std::floating_point T>
		constexpr T sqrt(T value) noexcept
		{
			return detail::constexpr_sqrt(value);
		}

#endif

		//
		// rsqrt() - inverse square root
		//

#if !defined(CXCM_DISABLE_RUNTIME_OPTIMIZATIONS) && (defined(_DEBUG) || defined(_M_IX86))

		template <std::floating_point T>
		constexpr T rsqrt(T value) noexcept
		{
			if (std::is_constant_evaluated())
			{
				return detail::constexpr_rsqrt(value);
			}
			else
			{
				return T(1.0) / std::sqrt(value);
			}
		}

#else

		template <std::floating_point T>
		constexpr T rsqrt(T value) noexcept
		{
			return detail::constexpr_rsqrt(value);
		}

#endif

	} // namespace strict

} // namespace cxcm
