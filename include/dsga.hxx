//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// opening include guard
#if !defined(DSGA_DSGA_HXX)
#define DSGA_DSGA_HXX

#include <type_traits>				// requirements
#include <concepts>					// requirements
#include <array>					// underlying storage
#include <tuple>					// tuple interface for structured bindings, matrix variadic constructor
#include <algorithm>				// min()
#include <numbers>					// pi_v<>, inv_pi_v<>
#include <version>					// feature test macros
#include <limits>					// for cxcm
#include <cmath>					// for cxcm

#if defined(__cpp_lib_bit_cast)
#include <bit>						// bit_cast
#endif

//
// Data Structures for Geometric Algebra (dsga)
//

// version info

constexpr inline int DSGA_MAJOR_VERSION = 0;
constexpr inline int DSGA_MINOR_VERSION = 8;
constexpr inline int DSGA_PATCH_VERSION = 3;

namespace dsga
{
	namespace cxcm
	{
		// copyright for cxcm
		
		//          Copyright David Browne 2020-2023.
		// Distributed under the Boost Software License, Version 1.0.
		//    (See accompanying file LICENSE_1_0.txt or copy at
		//          https://www.boost.org/LICENSE_1_0.txt)

		constexpr inline int CXCM_MAJOR_VERSION = 0;
		constexpr inline int CXCM_MINOR_VERSION = 1;
		constexpr inline int CXCM_PATCH_VERSION = 9;

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

			// undefined behavior if (value == std::numeric_limits<T>::min())
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
				if (is_halfway)
					if (is_even)
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
				//	By itself, converging_sqrt() gives:
				//	0 ulps : 75%
				//	1 ulps : 25%
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

				// 3 refinements is best
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
			//	0 ulps : 75%
			//	1 ulps : 25%
			template <std::floating_point T>
			constexpr T sqrt(T value) noexcept
			{
				return detail::converging_sqrt(value);
			}

			// float specialization - uses double internally - relied upon by rsqrt<T>() when [T = float]
			//		100% match with std::sqrt
			template <>
			constexpr float sqrt(float value) noexcept
			{
				double val = value;
//				return static_cast<float>(val * detail::inverse_sqrt(val));
				return static_cast<float>(detail::converging_sqrt(val));
			}

			// reciprocal of square root
			//	0 ulps : ~83.3068913%
			//	1 ulps : ~15.8502949%
			//	2 ulps :  ~0.8428138%
			template <std::floating_point T>
			constexpr T rsqrt(T value) noexcept
			{
				return T(1.0) / (value * detail::inverse_sqrt(value));
			}

			// float specialization - uses double internally
			//		100% match with (1.0f / std::sqrt) when using sqrt() float specialization
			template <>
			constexpr float rsqrt(float value) noexcept
			{
				return 1.0f / sqrt(value);
			}

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

					// arg == +infinity or +/-0, return val unmodified
					// arg == NaN, return NaN
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

					// arg == NaN, return NaN
					if (isnan(value))
						return value;

					// arg == +infinity , return 0
					if (value == std::numeric_limits<T>::infinity())
						return T(0.0);

					// arg == -infinity or +/-0, return NaN
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
				return relaxed::abs(value);
			}

			template <std::floating_point T>
			constexpr T fabs(T value) noexcept
			{
				return abs(value);
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

			//
			// sqrt()
			//

	#if defined(DSGA_CXCM_CONSTEXPR_APPROXIMATIONS_ALLOWED)

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
			T sqrt(T value) noexcept
			{
				return std::sqrt(value);
			}

	#endif

			//
			// rsqrt() - inverse square root
			//

	#if defined(DSGA_CXCM_CONSTEXPR_APPROXIMATIONS_ALLOWED)

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
			T rsqrt(T value) noexcept
			{
				return T(1.0) / std::sqrt(value);
			}

	#endif

		} // namespace strict

	} // namespace cxcm

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//
	// helper template function to convert a std::index_sequence to a std::array.
	// can be used for indirect indexing.
	//

	template <std::size_t... Is>
	static constexpr std::array<std::size_t, sizeof...(Is)> make_sequence_array(std::index_sequence<Is...> /* dummy */)
	{
		return { Is... };
	}

	// plain undecorated arithmetic types
	template <typename T>
	concept dimensional_scalar = std::is_arithmetic_v<T> && std::same_as<T, std::remove_cvref_t<T>>;

	// plain undecorated integral types
	template <typename T>
	concept integral_dimensional_scalar = std::integral<T> && dimensional_scalar<T>;

	// plain undecorated floating point types
	template <typename T>
	concept floating_point_dimensional_scalar = std::floating_point<T> && dimensional_scalar<T>;

	// plain undecorated boolean type
	template <typename T>
	concept boolean_dimensional_scalar = std::same_as<bool, T> && dimensional_scalar<T>;

	// want the size to be between 1 and 4, inclusive
	template <std::size_t Size>
	concept dimensional_size = ((Size >= 1u) && (Size <= 4u));

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

	// common initial sequence wrapper with basic storage access -- forwards function calls to wrapped storage.
	// this struct is an aggregate
	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
	struct storage_wrapper
	{
		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical storage - indirection is same as physical contiguous order.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		dimensional_storage_t<T, Size> store;

		[[nodiscard]] constexpr int length() const noexcept					{ return static_cast<int>(Count); }
		[[nodiscard]] constexpr std::size_t	size() const noexcept			{ return Count; }

		// physically contiguous access to data
		constexpr T &operator [](std::size_t index) noexcept				{ return store[index]; }
		constexpr const T &operator [](std::size_t index) const noexcept	{ return store[index]; }

		constexpr T * data() noexcept										{ return store.data(); }
		[[nodiscard]] constexpr const T * data() const noexcept				{ return store.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		[[nodiscard]] constexpr auto sequence() const noexcept				{ return sequence_pack{}; }

		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void set(Args ...args) noexcept
		{
			[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args) noexcept
			{
				((store[Js] = static_cast<T>(same_args)),...);
			}(std::make_index_sequence<Count>{}, args...);
		}

		// support for range-for loop
		constexpr auto begin() noexcept							{ return store.begin(); }
		[[nodiscard]] constexpr auto begin() const	noexcept	{ return store.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const	noexcept	{ return store.cbegin(); }
		constexpr auto end() noexcept							{ return store.end(); }
		[[nodiscard]] constexpr auto end() const	noexcept	{ return store.cend(); }
		[[nodiscard]] constexpr auto cend() const	noexcept	{ return store.cend(); }

	};

	namespace detail
	{
		// the concepts will help indexed_vector determine if it can be assigned to, like an lvalue reference,
		// i.e., if all indexes are unique then it can be used as an lvalue reference, i.e., is writable to.

		// see if all the std::size_t index values are unique

		template <std::size_t ...Is>
		struct unique_indexes_impl;

		template <>
		struct unique_indexes_impl<> : std::true_type
		{
		};

		template <std::size_t Index>
		struct unique_indexes_impl<Index> : std::true_type
		{
		};

		template <std::size_t First, std::size_t ...Rest>
		struct unique_indexes_impl<First, Rest...>
		{
			static constexpr bool value = ((First != Rest) && ...) && unique_indexes_impl<Rest...>::value;
		};

		// all Index values must be in ranged [0, Size) -- not checking here about Size and number of Is

		template <std::size_t Size, std::size_t ...Is>
		struct valid_indexes_impl
		{
			static constexpr bool value = ((Is < Size) && ...);
		};

	}

	// concepts required to build concept for testing for writable swizzle indexes

	template <std::size_t ...Is>
	constexpr inline bool unique_indexes = (sizeof...(Is) > 0) && detail::unique_indexes_impl<Is...>::value;

	template <std::size_t Count, std::size_t ...Is>
	constexpr inline bool valid_index_count = (sizeof...(Is) == Count) && dimensional_size<Count>;

	template <std::size_t Size, std::size_t ...Is>
	constexpr inline bool valid_range_indexes = detail::valid_indexes_impl<Size, Is...>::value;

	// writable_swizzle can determine whether a particular indexed_vector can be used as an lvalue reference

	template <std::size_t Size, std::size_t Count, std::size_t ...Is>
	constexpr inline bool writable_swizzle = valid_index_count<Count, Is...> && unique_indexes<Is...> && valid_range_indexes<Size, Is...>;

	//
	// helper template functions to determine whether implicit conversions are allowed
	//

	template <typename T>
	concept non_bool_arithmetic = std::is_arithmetic_v<std::remove_cvref_t<T>> && !std::same_as<std::remove_cvref_t<T>, bool>;

	template <typename T, typename U>
	concept latter_same_as_common_type =
	requires
	{
		typename std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
		requires std::same_as<std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>, std::remove_cvref_t<U>>;
	};

	template <typename T, typename U>
	concept implicitly_convertible_to = non_bool_arithmetic<T> && non_bool_arithmetic<U> && latter_same_as_common_type<T, U>;

	template <typename T, typename U>
	concept same_sizeof = (sizeof(T) == sizeof(U));

	// there is no use for this enum, it is meant as FEO (For Exposition Only). we will separate domains by the names of the swizzle union
	// members we create, as opposed to using this enum class as a template parameter. we only intend to implement the xyzw swizzle accessors.
	// if we intend to implement the other swizzle mask sets, then values of this enum class would come in handy as a template parameter (can we
	// use those in NTTPs?), as we are not allowed to mix and match accessors from different mask sets.
	enum class swizzle_mask_sets
	{
		xyzw,						// spatial points and normals
		rgba,						// colors
		stpq						// texture coordinates
	};

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
	//		set() - relies on init() in Derived - access in logical order
	//		operator[] - relies on at() in Derived - access in logical order
	//		size() - relies on Count template parameter
	//		length() - relies on Count template parameter
	//		data() - relies on raw_data() in Derived - access in physical order
	//		sequence() - relies on make_sequence_pack() in Derived - the physical order to logical order mapping
	//		as_derived() - relies on Derived type - useful for returning references to Derived when you just have a vector_base
	//
	// https://yuml.me/diagram/scruffy/class/draw
	//
	//[vector_base;set();operator_brackets();data();sequence();length();size();as_derived()|Count (template parameter)]^[<<vector duck type>>basic_vector|anonymous union ]
	//[vector_base]^[<<vector duck type>>indexed_vector]
	//[<<vector duck type>>|init();at();raw_data();make_sequence_pack()]^-.-[basic_vector]
	//[<<vector duck type>>]^-.-[indexed_vector]
	//[basic_vector]++-*>[indexed_vector]
	//[basic_vector]++-1>[storage_wrapper]
	//
	template <bool Writable, dimensional_scalar T, std::size_t Count, typename Derived>
	requires dimensional_storage<T, Count>
	struct vector_base
	{
		// CRTP access to Derived class
		constexpr Derived &as_derived() noexcept requires Writable				{ return static_cast<Derived &>(*this); }
		[[nodiscard]] constexpr const Derived &as_derived() const noexcept		{ return static_cast<const Derived &>(*this); }

		// logically contiguous write access to all data that allows for self-assignment that works properly
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void set(Args ...args) noexcept								{ this->as_derived().init(args...); }

		// logically contiguous access to piecewise data as index goes from 0 to (Count - 1)
		constexpr T &operator [](std::size_t index) noexcept requires Writable	{ return this->as_derived().at(index); }
		constexpr const T &operator [](std::size_t index) const	noexcept		{ return this->as_derived().at(index); }

		// physically contiguous access via pointer
		constexpr T * data() noexcept requires Writable							{ return this->as_derived().raw_data(); }
		[[nodiscard]] constexpr const T * data() const noexcept					{ return this->as_derived().raw_data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous.
		// this is only really helpful if you use data() in your API, because operator [] already adjusts for sequencing.
		[[nodiscard]] constexpr auto sequence() const noexcept					{ return this->as_derived().make_sequence_pack(); }

		// number of accessible T elements - required by spec
		[[nodiscard]] constexpr int length() const noexcept						{ return static_cast<int>(Count); }

		// not required by spec, but more c++ container-like
		[[nodiscard]] constexpr std::size_t	size() const noexcept				{ return Count; }
	};

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

	// indexed_vector will act as a swizzle of a basic_vector. basic_vector relies on the anonymous union of indexed_vector data members.
	//
	// T is the type of the elements stored in the underlying storage
	// Size relates to the number of elements in the underlying storage, which informs the values the Is can hold
	// Count is the number of elements accessible in swizzle -- often works alongside with basic_vector's Size
	// Is... are the number of swizzlable values available -- there are Count of them, and their values are in the range:  0 <= Indexes < Size

	// we want indexed_vector (vector swizzles) to have length from 1 to 4 (1 is just a sneaky type of T swizzle) in order
	// to work with the basic_vector which also has these lengths. The number of indexes is the same as the Count, between 1 and 4.
	// The indexes are valid for indexing into the values in the storage which is Size big.

	template <typename T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	concept indexable = dimensional_storage<T, Size> && valid_index_count<Count, Is...> && valid_range_indexes<Size, Is...>;

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
	// iterators for indexed_vector so it can participate in range-for loop.
	// make sure that it doesn't out-live it's indexed_vector or there will be
	// a dangling pointer.
	//

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable<T, Size, Count, Is...>
	struct indexed_vector_const_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = const T *;
		using reference = const T &;

		constexpr static std::size_t begin_index = 0;
		constexpr static std::size_t end_index = Count;

		// the data
		const indexed_vector<T, Size, Count, Is ...> *mapper_ptr;
		std::size_t mapper_index;

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr indexed_vector_const_iterator(const indexed_vector<T, Size, Count, Is ...> &mapper, std::size_t index) noexcept
			: mapper_ptr(std::addressof(mapper)), mapper_index((index > Count) ? Count : index)
		{
		}

		constexpr indexed_vector_const_iterator() noexcept = default;
		constexpr indexed_vector_const_iterator(const indexed_vector_const_iterator &) noexcept = default;
		constexpr indexed_vector_const_iterator(indexed_vector_const_iterator &&) noexcept = default;
		constexpr indexed_vector_const_iterator &operator =(const indexed_vector_const_iterator &) noexcept = default;
		constexpr indexed_vector_const_iterator &operator =(indexed_vector_const_iterator &&) noexcept = default;
		constexpr ~indexed_vector_const_iterator() = default;
		constexpr bool operator ==(const indexed_vector_const_iterator &other) const noexcept = default;

		constexpr reference operator *() const noexcept
		{
			return (*mapper_ptr)[mapper_index];
		}

		constexpr indexed_vector_const_iterator &operator++() noexcept
		{
			if (mapper_index < Count)
				mapper_index++;
			return *this;
		}

		constexpr indexed_vector_const_iterator operator++(int) noexcept
		{
			indexed_vector_const_iterator temp = *this;
			if (mapper_index < Count)
				mapper_index++;
			return temp;
		}
	};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable <T, Size, Count, Is...>
	struct indexed_vector_iterator : indexed_vector_const_iterator<T, Size, Count, Is...>
	{
		// let base class do all the work
		using base_iter = indexed_vector_const_iterator<T, Size, Count, Is...>;

		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = T *;
		using reference = T &;
		using const_reference = const T &;

		constexpr static std::size_t begin_index = 0;
		constexpr static std::size_t end_index = Count;

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr indexed_vector_iterator(indexed_vector<T, Size, Count, Is ...> &mapper, std::size_t index) noexcept
			: base_iter(mapper, index)
		{
		}

		constexpr indexed_vector_iterator() noexcept = default;
		constexpr indexed_vector_iterator(const indexed_vector_iterator &) noexcept = default;
		constexpr indexed_vector_iterator(indexed_vector_iterator &&) noexcept = default;
		constexpr indexed_vector_iterator &operator =(const indexed_vector_iterator &) noexcept = default;
		constexpr indexed_vector_iterator &operator =(indexed_vector_iterator &&) noexcept = default;
		constexpr ~indexed_vector_iterator() = default;
		constexpr bool operator ==(const indexed_vector_iterator &other) const noexcept = default;

		constexpr reference operator *() const noexcept
		{
			return const_cast<reference>(base_iter::operator*());
		}

		constexpr indexed_vector_iterator &operator++() noexcept
		{
			base_iter::operator++();
			return *this;
		}

		constexpr indexed_vector_iterator operator++(int) noexcept
		{
			indexed_vector_iterator temp = *this;
			base_iter::operator++();
			return temp;
		}
	};

	// for swizzling 2D-4D parts of basic_vector
	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	requires indexable<T, Size, Count, Is...>
	struct indexed_vector<T, Size, Count, Is...>
		: vector_base<writable_swizzle<Size, Count, Is...>, T, Count, indexed_vector<T, Size, Count, Is...>>
	{
		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, Is...>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<Is...>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> base;

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			[&] <std::size_t ...Js>(std::index_sequence<Js ...> /* dummy */) noexcept
			{
				init( other[Js]... );
			}(std::make_index_sequence<Count>{});

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			[&] <std::size_t ...Js>(std::index_sequence<Js ...> /* dummy */) noexcept
			{
				init( other[Js]... );
			}(std::make_index_sequence<Count>{});

			return *this;
		}

		// support for range-for loop
		constexpr auto begin() noexcept requires Writable		{ return indexed_vector_iterator<T, Size, Count, Is...>(*this, 0u); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, 0u); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, 0u); }
		constexpr auto end() noexcept requires Writable			{ return indexed_vector_iterator<T, Size, Count, Is...>(*this, Count); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, Count); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, Count); }

		private:

			friend struct vector_base<Writable, T, Count, indexed_vector<T, Size, Count, Is...>>;

			// logically contiguous - used by set() for write access to data
			// allows for self-assignment without aliasing issues
			template <typename ... Args>
			requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
			constexpr void init(Args ...args) noexcept
			{
				((base[Is] = static_cast<T>(args)), ...);
			}

			// logically contiguous - used by operator [] for read/write access to data
			constexpr T &at(std::size_t index) noexcept requires Writable			{ return base[offsets[index]]; }

			// logically contiguous - used by operator [] for read access to data
			[[nodiscard]] constexpr const T &at(std::size_t index) const noexcept	{ return base[offsets[index]]; }

			// physically contiguous -- used by data() for read/write access to data
			constexpr T *raw_data() noexcept requires Writable						{ return base.data(); }

			// physically contiguous -- used by data() for read access to data
			[[nodiscard]] constexpr const T *raw_data() const noexcept				{ return base.data(); }

			// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
			[[nodiscard]] constexpr auto make_sequence_pack() const noexcept		{ return sequence_pack{}; }
	};

	// for swizzling 1D parts of basic_vector - like a scalar accessor
	template <dimensional_scalar T, std::size_t Size, std::size_t I>
	requires indexable<T, Size, 1u, I>
	struct indexed_vector<T, Size, 1u, I>
		: vector_base<writable_swizzle<Size, 1u, I>, T, 1u, indexed_vector<T, Size, 1u, I>>
	{
		// we have partial specialization, so can't use template parameter for Count number of logical storage elements
		static constexpr std::size_t Count = 1u;

		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, I>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<I>;

		// as an array
		static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> base;

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init( other[0u] );

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			init( other[0u] );

			return *this;
		}

		// scalar assignment
		// assignment for some scalar type that converts to T and is only for indexed_vector of [Size == 1]
		template <dimensional_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(U other) noexcept
		{
			init(other);

			return *this;
		}

		constexpr indexed_vector &operator =(T other) noexcept
		{
			init(other);

			return *this;
		}

		//
		// scalar conversion operators
		//

		// this is extremely important and is only for indexed_vector of [Count == 1]
		constexpr operator T() const noexcept
		{
			return base[I];
		}

		template <typename U>
		requires std::convertible_to<T, U>
		explicit constexpr operator U() const noexcept
		{
			return static_cast<U>(base[I]);
		}

		// support for range-for loop
		constexpr auto begin() noexcept requires Writable		{ return indexed_vector_iterator<T, Size, Count, I>(*this, 0u); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, 0u); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto end() noexcept requires Writable			{ return indexed_vector_iterator<T, Size, Count, I>(*this, Count); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, Count); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, Count); }

		private:

			friend struct vector_base<Writable, T, 1u, indexed_vector<T, Size, 1u, I>>;

			// logically contiguous - used by set() for write access to data
			// allows for self-assignment without aliasing issues
			template <typename U>
			requires Writable &&std::convertible_to<U, T>
			constexpr void init(U other) noexcept
			{
				base[I] = static_cast<T>(other);
			}

			// logically contiguous - used by operator [] for read/write access to data
			constexpr T &at(std::size_t index) noexcept requires Writable			{ return base[offsets[index]]; }

			// logically contiguous - used by operator [] for read access to data
			[[nodiscard]] constexpr const T &at(std::size_t index) const noexcept	{ return base[offsets[index]]; }

			// physically contiguous -- used by data() for read/write access to data
			constexpr T *raw_data() noexcept requires Writable						{ return base.data(); }

			// physically contiguous -- used by data() for read access to data
			[[nodiscard]] constexpr const T *raw_data() const noexcept				{ return base.data(); }

			// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
			[[nodiscard]] constexpr auto make_sequence_pack() const noexcept		{ return sequence_pack{}; }
	};


	// convenience using types for indexed_vector as members of basic_vector

	template <typename T, std::size_t Size, std::size_t I>
	using dexvec1 = indexed_vector<std::remove_cvref_t<T>, Size, 1u, I>;

	template <typename T, std::size_t Size, std::size_t ...Is>
	using dexvec2 = indexed_vector<std::remove_cvref_t<T>, Size, 2u, Is...>;

	template <typename T, std::size_t Size, std::size_t ...Is>
	using dexvec3 = indexed_vector<std::remove_cvref_t<T>, Size, 3u, Is...>;

	template <typename T, std::size_t Size, std::size_t ...Is>
	using dexvec4 = indexed_vector<std::remove_cvref_t<T>, Size, 4u, Is...>;

	// basic_matrix will act as the primary matrix class in this library.
	//
	// T is the type of the elements stored in the matrix
	// C is the number of elements in a column
	// R is the number of elements in a row

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
	requires (((C >= 2u) && (C <= 4u)) && ((R >= 2u) && (R <= 4u)))
	struct basic_matrix;

	namespace detail
	{

		// how many components can the item supply

		template <typename T>
		struct component_size;

		template <dimensional_scalar T, std::size_t C>
		struct component_size<basic_vector<T, C>>
		{
			static constexpr std::size_t value = C;
		};

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		struct component_size<indexed_vector<T, S, C, Is...>>
		{
			static constexpr std::size_t value = C;
		};

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		struct component_size<vector_base<W, T, C, D>>
		{
			static constexpr std::size_t value = C;
		};

		template <dimensional_scalar T>
		struct component_size<T>
		{
			static constexpr std::size_t value = 1u;
		};

		// the make sure Count and Args... are valid together w.r.t. component count.
		// Args is expected to be a combination of derived vector_base classes and dimensional_scalars.
		template <std::size_t Count, typename ...Args>
		struct component_match;

		// can't have 0 Args unless Count is 0
		template <std::size_t Count, typename ...Args>
		requires (sizeof...(Args) == 0u)
		struct component_match<Count, Args...>
		{
			static constexpr bool valid = (Count == 0u);
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
		requires (sizeof...(Args) > 0u) && (Count > 0u)
		struct component_match<Count, Args...>
		{
			// total number components in Args...
			static constexpr std::size_t value = (component_size<Args>::value + ... + 0);
			using tuple_pack = std::tuple<Args...>;

			// get the last Arg type in the pack
			using last_type = std::tuple_element_t<std::tuple_size_v<tuple_pack> -1u, tuple_pack>;

			// see what the component count is if we didn't use the last Arg type in pack
			static constexpr std::size_t previous_size = value - component_size<last_type>::value;

			// check the conditions that we need exactly all those Args and that they give us enough components.
			static constexpr bool valid = (previous_size < Count) && (value >= Count);
		};

		template <std::size_t Count, typename ...Args>
		inline constexpr bool component_match_v = component_match<Count, Args...>::valid;

		// do Args... supply the correct number of components for Count without having leftover Args
		template <std::size_t Count, typename ...Args>
		concept met_component_count = component_match_v<Count, Args...>;

		template <typename T>
		struct valid_component_source : std::false_type
		{
		};

		template <dimensional_scalar T>
		struct valid_component_source<T> : std::true_type
		{
		};

		template <std::size_t C, dimensional_scalar T>
		struct valid_component_source<basic_vector<T, C>> : std::true_type
		{
		};

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		struct valid_component_source<indexed_vector<T, S, C, Is...>> : std::true_type
		{
		};

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		struct valid_component_source<vector_base<W, T, C, D>> : std::true_type
		{
		};

		// create a tuple from a scalar

		auto to_tuple(dimensional_scalar auto arg) noexcept
		{
			return std::make_tuple(arg);
		}

		// create a tuple from a vector

		template <dimensional_scalar T, std::size_t C>
		constexpr auto to_tuple(const basic_vector<T, C> &arg) noexcept
		{
			return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::make_tuple(arg[Is]...);
			}(std::make_index_sequence<C>{});
		}

		template <dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		constexpr auto to_tuple(const indexed_vector<T, S, C, Is...> &arg) noexcept
		{
			return[&]<std::size_t ...Js>(std::index_sequence<Js...>) noexcept
			{
				return std::make_tuple(arg[Js]...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		constexpr auto to_tuple(const vector_base<W, T, C, D> &arg) noexcept
		{
			return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::make_tuple(arg[Is]...);
			}(std::make_index_sequence<C>{});
		}

		template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
		constexpr auto to_tuple(const basic_matrix<T, C, R> &arg) noexcept
		{
			return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return std::tuple_cat(to_tuple(arg[Is])...);
			}(std::make_index_sequence<C>{});
		}

		// flatten the Args out in a big tuple. Args is expected to be a combination of derived vector_base classes
		// and dimensional_scalars.
		template <typename ...Args>
		auto flatten_args_to_tuple(const Args & ...args) noexcept
		{
			return std::tuple_cat(to_tuple(args)...);
		}

		template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
		struct component_size<basic_matrix<T, C, R>>
		{
			static constexpr std::size_t value = C * R;
		};

		template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
		struct valid_component_source<basic_matrix<T, C, R>> : std::true_type
		{
		};
	}

	template <dimensional_scalar T>
	struct basic_vector<T, 1u> : vector_base<true, T, 1u, basic_vector<T, 1u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 1u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
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

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

		//
		// constructors
		//

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0u]) }
		{
		}

		template <typename U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(U value) noexcept
			: base{ static_cast<T>(value) }
		{
		}

		template <typename U>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(U value) noexcept
			: base{ static_cast<T>(value) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename ... Args>
		requires (detail::valid_component_source<Args>::value && ...) && detail::met_component_count<Size, Args...>
		explicit constexpr basic_vector(const Args & ...args) noexcept
		{
			auto arg_tuple = detail::flatten_args_to_tuple(args...);
			[&] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Size>{});
		}

		//
		// implicit assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u]);
			return *this;
		}

		template <typename U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(U value) noexcept
		{
			init(value);
			return *this;
		}

		//
		// scalar conversion operators
		//

		// this is extremely important and is only for basic_vector of [Size == 1]
		constexpr operator T() const noexcept
		{
			return base.store[0u];
		}

		template <typename U>
		requires std::convertible_to<T, U>
		explicit constexpr operator U() const noexcept
		{
			return static_cast<U>(base.store[0u]);
		}

		// support for range-for loop
		constexpr auto begin() noexcept							{ return base.store.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return base.store.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return base.store.cbegin(); }
		constexpr auto end() noexcept							{ return base.store.end(); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return base.store.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return base.store.cend(); }

		private:

			friend struct vector_base<Writable, T, 1u, basic_vector<T, 1u>>;

			//
			// data access
			//

			// logically and physically contiguous - used by set() for write access to data
			// allows for self-assignment without aliasing issues
			template <typename U>
			requires std::convertible_to<U, T>
			constexpr void init(U value) noexcept
			{
				base.store[0u] = static_cast<T>(value);
			}

			// logically and physically contiguous - used by operator [] for access to data
			constexpr T &at(std::size_t index) noexcept								{ return base.store[index]; }
			[[nodiscard]] constexpr const T &at(std::size_t index) const noexcept	{ return base.store[index]; }

			// physically contiguous -- used by data()
			constexpr T *raw_data() noexcept										{ return base.store.data(); }
			[[nodiscard]] constexpr const T *raw_data() const noexcept				{ return base.store.data(); }

			// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
			[[nodiscard]] constexpr auto make_sequence_pack() const noexcept		{ return sequence_pack{}; }
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 2u> : vector_base<true, T, 2u, basic_vector<T, 2u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 2u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
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

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

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
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename ... Args>
		requires (detail::valid_component_source<Args>::value && ...) && detail::met_component_count<Size, Args...>
		explicit constexpr basic_vector(const Args & ...args) noexcept
		{
			auto arg_tuple = detail::flatten_args_to_tuple(args...);
			[&] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Size>{});
		}

		//
		// assignment operator
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u]);
			return *this;
		}

		// support for range-for loop
		constexpr auto begin() noexcept							{ return base.store.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return base.store.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return base.store.cbegin(); }
		constexpr auto end() noexcept							{ return base.store.end(); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return base.store.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return base.store.cend(); }

		private:

			friend struct vector_base<Writable, T, 2u, basic_vector<T, 2u>>;

			//
			// data access
			//

			// logically and physically contiguous - used by set() for write access to data
			// allows for self-assignment without aliasing issues
			template <typename ...Args>
			requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
			constexpr void init(Args ...args) noexcept
			{
				[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args) noexcept
				{
					((base.store[Js] = static_cast<T>(same_args)), ...);
				}(std::make_index_sequence<Count>{}, args...);
			}

			// logically and physically contiguous - used by operator [] for access to data
			constexpr T &at(std::size_t index) noexcept								{ return base.store[index]; }
			[[nodiscard]] constexpr const T &at(std::size_t index) const noexcept	{ return base.store[index]; }

			// physically contiguous -- used by data()
			constexpr T *raw_data() noexcept										{ return base.store.data(); }
			[[nodiscard]] constexpr const T *raw_data() const noexcept				{ return base.store.data(); }

			// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
			[[nodiscard]] constexpr auto make_sequence_pack() const noexcept		{ return sequence_pack{}; }
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 3u> : vector_base<true, T, 3u, basic_vector<T, 3u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 3u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
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

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

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
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: base{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename ... Args>
		requires (detail::valid_component_source<Args>::value && ...) && detail::met_component_count<Size, Args...>
		explicit constexpr basic_vector(const Args & ...args) noexcept
		{
			auto arg_tuple = detail::flatten_args_to_tuple(args...);
			[&] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Size>{});
		}

		//
		// assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);
			return *this;
		}

		// support for range-for loop
		constexpr auto begin() noexcept							{ return base.store.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return base.store.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return base.store.cbegin(); }
		constexpr auto end() noexcept							{ return base.store.end(); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return base.store.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return base.store.cend(); }

		private:

			friend struct vector_base<Writable, T, 3u, basic_vector<T, 3u>>;

			//
			// data access
			//

			// logically and physically contiguous - used by set() for write access to data
			// allows for self-assignment without aliasing issues
			template <typename ...Args>
			requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
			constexpr void init(Args ...args) noexcept
			{
				[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args) noexcept
				{
					((base.store[Js] = static_cast<T>(same_args)), ...);
				}(std::make_index_sequence<Count>{}, args...);
			}

			// logically and physically contiguous - used by operator [] for access to data
			constexpr T &at(std::size_t index) noexcept								{ return base.store[index]; }
			[[nodiscard]] constexpr const T &at(std::size_t index) const noexcept	{ return base.store[index]; }

			// physically contiguous -- used by data()
			constexpr T *raw_data() noexcept										{ return base.store.data(); }
			[[nodiscard]] constexpr const T *raw_data() const noexcept				{ return base.store.data(); }

			// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
			[[nodiscard]] constexpr auto make_sequence_pack() const noexcept		{ return sequence_pack{}; }
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 4u> : vector_base<true, T, 4u, basic_vector<T, 4u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 4u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
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

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

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
		constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: base{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(other[3u]) }
		{
		}

		// variadic constructor of scalar and vector arguments
		template <typename ... Args>
		requires (detail::valid_component_source<Args>::value && ...) && detail::met_component_count<Size, Args...>
		explicit constexpr basic_vector(const Args & ...args) noexcept
		{
			auto arg_tuple = detail::flatten_args_to_tuple(args...);
			[&] <std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((base[Is] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<Size>{});
		}

		//
		// assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);
			return *this;
		}

		// support for range-for loop
		constexpr auto begin() noexcept							{ return base.store.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept		{ return base.store.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept	{ return base.store.cbegin(); }
		constexpr auto end() noexcept							{ return base.store.end(); }
		[[nodiscard]] constexpr auto end() const noexcept		{ return base.store.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept		{ return base.store.cend(); }

		private:

			friend struct vector_base<Writable, T, 4u, basic_vector<T, 4u>>;

			//
			// data access
			//

			// logically and physically contiguous - used by set() for write access to data
			// allows for self-assignment without aliasing issues
			template <typename ...Args>
			requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
			constexpr void init(Args ...args) noexcept
			{
				[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args) noexcept
				{
					((base.store[Js] = static_cast<T>(same_args)), ...);
				}(std::make_index_sequence<Count>{}, args...);
			}

			// logically and physically contiguous - used by operator [] for access to data
			constexpr T &at(std::size_t index) noexcept								{ return base.store[index]; }
			[[nodiscard]] constexpr const T &at(std::size_t index) const noexcept	{ return base.store[index]; }

			// physically contiguous -- used by data()
			constexpr T *raw_data() noexcept										{ return base.store.data(); }
			[[nodiscard]] constexpr const T *raw_data() const noexcept				{ return base.store.data(); }

			// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
			[[nodiscard]] constexpr auto make_sequence_pack() const noexcept		{ return sequence_pack{}; }
	};

	//
	// CTAD deduction guides
	//

	template <class T, class... U>
	basic_vector(T, U...) -> basic_vector<T, 1 + sizeof...(U)>;

	template <bool W, class T, std::size_t C, class D>
	basic_vector(const vector_base<W, T, C, D> &) -> basic_vector<T, C>;

	//
	// operators and compound assignment operators
	//

	namespace detail
	{
		// convert a parameter pack into a basic_vector

		template <typename ...Ts>
		requires dimensional_storage<std::common_type_t<Ts...>, sizeof...(Ts)>
			constexpr auto parameter_pack_to_vec(Ts ...args) noexcept
		{
			using ArgType = std::common_type_t<Ts...>;
			constexpr std::size_t Size = sizeof...(Ts);

			return basic_vector<ArgType, Size>{(static_cast<ArgType>(args))...};
		}

		// convert basic array types to a basic_vector

		template <typename T, std::size_t S, std::size_t ...Is>
		constexpr auto passthru_execute(std::index_sequence<Is...> /* dummy */,
										const std::array<T, S> &arg) noexcept
		{
			return basic_vector<T, S>(arg[Is]...);
		}

		template <typename T, std::size_t S, std::size_t ...Is>
		constexpr auto passthru_execute(std::index_sequence<Is...> /* dummy */,
										const T(&arg)[S]) noexcept
		{
			return basic_vector<T, S>(arg[Is]...);
		}

		// return types from executing lambdas on arguments of various types

		template <typename UnOp, dsga::dimensional_scalar T>
		using unary_op_return_t = decltype(UnOp()(std::declval<T>()));

		template <typename BinOp, dsga::dimensional_scalar T, dsga::dimensional_scalar U>
		using binary_op_return_t = decltype(BinOp()(std::declval<T>(), std::declval<U>()));

		template <typename TernOp, dsga::dimensional_scalar T, dsga::dimensional_scalar U, dsga::dimensional_scalar V>
		using ternary_op_return_t = decltype(TernOp()(std::declval<T>(), std::declval<U>(), std::declval<V>()));

		//
		// these rely on vector_base::operator[] to have dealt with indirection, if any, of derived vector types.
		//

		// perform the lambda action on components of vector_base arguments, returning a new basic_vector.
		// when Count == 1, treat it like a scalar value and return a scalar value.

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D, typename UnOp, std::size_t ...Is>
		constexpr auto unary_op_execute(std::index_sequence<Is...> /* dummy */,
										const vector_base<W, T, C, D> &arg,
										UnOp &lambda) noexcept
		{
			using ReturnType = unary_op_return_t<UnOp, T>;
			return basic_vector<ReturnType, C>(lambda(arg[Is])...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &lhs,
										 const vector_base<W2, T2, C, D2> &rhs,
										 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2>;
			using ReturnType = binary_op_return_t<BinOp, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs[Is]))...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W, T, C, D> &lhs,
										 U rhs,
										 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U>;
			using ReturnType = binary_op_return_t<BinOp, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs))...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 U lhs,
										 const vector_base<W, T, C, D> &rhs,
										 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U>;
			using ReturnType = binary_op_return_t<BinOp, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(lhs), static_cast<ArgType>(rhs[Is]))...);
		}

		//
		// don't cast the arguments to the lambda
		//

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													const vector_base<W1, T1, C, D1> &lhs,
													const vector_base<W2, T2, C, D2> &rhs,
													BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, T1, T2>, C>(lambda(lhs[Is], rhs[Is])...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													const vector_base<W, T, C, D> &lhs,
													U rhs,
													BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, T, U>, C>(lambda(lhs[Is], rhs)...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													U lhs,
													const vector_base<W, T, C, D> &rhs,
													BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, U, T>, C>(lambda(lhs, rhs[Is])...);
		}

		// perform the lambda action, setting the lhs vector_base to new values.
		// we need all the new values upfront before we set them. it is possible that
		// we are indexing into the same vector, which could give unexpected results if
		// we set values as we iterate. we need to set them all only after the new values
		// have all been gathered.

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		requires W1
		constexpr void binary_op_set(std::index_sequence<Is...> /* dummy */,
									 vector_base<W1, T1, C, D1> &lhs,
									 const vector_base<W2, T2, C, D2> &rhs,
									 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2>;
			lhs.set(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs[Is]))...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires W
		constexpr void binary_op_set(std::index_sequence<Is...> /* dummy */,
									 vector_base<W, T, C, D> &lhs,
									 U rhs,
									 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U>;
			lhs.set(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs))...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		requires W1
		constexpr void binary_op_set_no_convert(std::index_sequence<Is...> /* dummy */,
												vector_base<W1, T1, C, D1> &lhs,
												const vector_base<W2, T2, C, D2> &rhs,
												BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Is], rhs[Is])...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires W
		constexpr void binary_op_set_no_convert(std::index_sequence<Is...> /* dummy */,
												vector_base<W, T, C, D> &lhs,
												U rhs,
												BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Is], rhs)...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, bool W3, dsga::dimensional_scalar T3, typename D3,
			typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &x,
										 const vector_base<W2, T2, C, D2> &y,
										 const vector_base<W3, T3, C, D3> &z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2, T3>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x[Is]), static_cast<ArgType>(y[Is]), static_cast<ArgType>(z[Is]))...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, dsga::dimensional_scalar U, typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &x,
										 const vector_base<W2, T2, C, D2> &y,
										 U z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2, U>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x[Is]), static_cast<ArgType>(y[Is]), static_cast<ArgType>(z))...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, dsga::dimensional_scalar V, typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W, T, C, D> &x,
										 U y,
										 V z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U, V>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x[Is]), static_cast<ArgType>(y), static_cast<ArgType>(z))...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, dsga::dimensional_scalar V, typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 U x,
										 V y,
										 const vector_base<W, T, C, D> &z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U, V>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x), static_cast<ArgType>(y), static_cast<ArgType>(z[Is]))...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2,
			bool W3, dsga::dimensional_scalar T3, typename D3,
			typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													 const vector_base<W1, T1, C, D1> &x,
													 const vector_base<W2, T2, C, D2> &y,
													 const vector_base<W3, T3, C, D3> &z,
													 TernOp &lambda) noexcept
		{
			return basic_vector<ternary_op_return_t<TernOp, T1, T2, T3>, C>(lambda(x[Is], y[Is], z[Is])...);
		}

	}

	//
	// operators
	//

	// binary operators +=, +

	constexpr inline auto plus_op = [](auto lhs, auto rhs) noexcept { return lhs + rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator +=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator +=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], plus_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator +=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator +(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, plus_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, plus_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], plus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator +(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator +(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
	}

	// binary operators -=, -

	constexpr inline auto minus_op = [](auto lhs, auto rhs) noexcept { return lhs - rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator -=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator -=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], minus_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator -=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator -(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, minus_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, minus_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], minus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator -(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator -(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
	}

	// binary operators *=, *

	constexpr inline auto times_op = [](auto lhs, auto rhs) noexcept { return lhs * rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator *=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator *=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], times_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator *=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator *(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, times_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, times_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], times_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator *(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, times_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator *(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, times_op);
	}

	// binary operators /=, /

	constexpr inline auto div_op = [](auto lhs, auto rhs) noexcept { return lhs / rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator /=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator /=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], div_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator /=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator /(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, div_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, div_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], div_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator /(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, div_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator /(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, div_op);
	}

	// binary operators %=, %

	constexpr inline auto modulus_op = [](auto lhs, auto rhs) noexcept { return lhs % rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], modulus_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator %=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator %(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, modulus_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, modulus_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], modulus_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator %(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator %(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
	}

	// unary operator ~

	constexpr inline auto bit_not_op = [](auto arg) noexcept { return ~arg; };

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D>
	constexpr auto operator ~(const vector_base<W, T, C, D> &arg) noexcept
	{
		return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, bit_not_op);
	}

	// binary operators <<=, <<

	constexpr inline auto lshift_op = [](auto lhs, auto rhs) noexcept { return lhs << rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator <<=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator <<=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs[0u], lshift_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator <<=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator <<(const vector_base<W1, T1, C1, D1> &lhs,
							   const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs, lshift_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C2>{}, lhs[0u], rhs, lshift_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs[0u], lshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator <<(const vector_base<W, T, C, D> &lhs,
							   U rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator <<(U lhs,
							   const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	// binary operators >>=, >>

	constexpr inline auto rshift_op = [](auto lhs, auto rhs) noexcept { return lhs >> rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs[0u], rshift_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator >>=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator >>(const vector_base<W1, T1, C1, D1> &lhs,
							   const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs, rshift_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C2>{}, lhs[0u], rhs, rshift_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs[0u], rshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator >>(const vector_base<W, T, C, D> &lhs,
							   U rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator >>(U lhs,
							   const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
	}

	// binary operators &=, &

	constexpr inline auto and_op = [](auto lhs, auto rhs) noexcept { return lhs & rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator &=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator &=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], and_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator &=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator &(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, and_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, and_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], and_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator &(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, and_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator &(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, and_op);
	}

	// binary operators |=, |

	constexpr inline auto or_op = [](auto lhs, auto rhs) noexcept { return lhs | rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator |=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator |=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], or_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator |=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator |(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, or_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, or_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], or_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator |(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, or_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator |(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, or_op);
	}

	// binary operators ^=, ^

	constexpr inline auto xor_op = [](auto lhs, auto rhs) noexcept { return lhs ^ rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator ^=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator ^=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], xor_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator ^=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator ^(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, xor_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, xor_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], xor_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator ^(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator ^(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
	}

	// unary operator +

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires non_bool_arithmetic<T>
	constexpr auto operator +(const vector_base<W, T, C, D> &arg) noexcept
	{
		if constexpr (C > 1u)
			return basic_vector<T, C>(arg);					// no-op copy
		else
			return arg[0u];									// no-op scalar copy
	}

	// unary operator -

	constexpr inline auto neg_op = [](auto arg) noexcept { return -arg; };

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires non_bool_arithmetic<T>
	constexpr auto operator -(const vector_base<W, T, C, D> &arg) noexcept
	{
		return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, neg_op);
	}

	// unary operators ++

	// pre-increment
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto &operator ++(vector_base<W, T, C, D> &arg) noexcept
	{
		arg += T(1);
		return arg.as_derived();
	}

	// post-increment
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto operator ++(vector_base<W, T, C, D> &arg, int) noexcept
	{
		if constexpr (C > 1u)
		{
			basic_vector<T, C> value(arg);
			arg += T(1);
			return value;
		}
		else
		{
			T value = arg[0u];
			arg += T(1);
			return value;
		}
	}

	// unary operators --

	// pre-decrement
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto &operator --(vector_base<W, T, C, D> &arg) noexcept
	{
		arg -= T(1);
		return arg.as_derived();
	}

	// post-decrement
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto operator --(vector_base<W, T, C, D> &arg, int) noexcept
	{
		if constexpr (C > 1u)
		{
			basic_vector<T, C> value(arg);
			arg -= T(1);
			return value;
		}
		else
		{
			T value = arg[0u];
			arg -= T(1);
			return value;
		}
	}

	//
	// component-wise equality operator for vectors, scalar boolean result: ==, != (thanks to c++20).
	// most vector equality/inequality testing should use free functions equal()/notEqual(), but
	// these have a scalar result and are useful for unit testing.
	//

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
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

	// when Count == 1, treat it like a scalar value
	template <bool W, dimensional_scalar T, typename D, dimensional_scalar U>
	requires (std::same_as<T, bool> == std::same_as<U, bool>)
	constexpr bool operator ==(const vector_base<W, T, 1u, D> &first,
							   U second) noexcept
	{
		using CommonType = std::common_type_t<T, U>;
		return !std::isunordered(first[0u], second) && (static_cast<CommonType>(first[0u]) == static_cast<CommonType>(second));
	}

	//
	// get<> part of tuple interface -- needed for structured bindings
	//

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	constexpr auto && get(dsga::storage_wrapper<T, S> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	constexpr auto && get(const dsga::storage_wrapper<T, S> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	constexpr auto && get(dsga::storage_wrapper<T, S> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, dimensional_scalar T, std::size_t S>
	requires (N >= 0) && (N < S)
	constexpr auto && get(const dsga::storage_wrapper<T, S> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	//

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(dsga::vector_base<W, T, C, D> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(const dsga::vector_base<W, T, C, D> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(dsga::vector_base<W, T, C, D> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(const dsga::vector_base<W, T, C, D> && arg) noexcept
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
		// 8.1 - angle and trigonometry
		//

		template <std::floating_point T>
		inline constexpr T degrees_per_radian_v = std::numbers::inv_pi_v<T> * T(180);

		template <std::floating_point T>
		inline constexpr T radians_per_degree_v = std::numbers::pi_v<T> / T(180);

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto radians(const vector_base<W, T, C, D> &deg) noexcept
		{
			return deg * radians_per_degree_v<T>;
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto degrees(const vector_base<W, T, C, D> &rad) noexcept
		{
			return rad * degrees_per_radian_v<T>;
		}

		constexpr inline auto sin_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::sin(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto sin(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sin_op);
		}

		constexpr inline auto cos_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::cos(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto cos(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, cos_op);
		}

		constexpr inline auto tan_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::tan(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto tan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, tan_op);
		}

		constexpr inline auto asin_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::asin(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto asin(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, asin_op);
		}

		constexpr inline auto acos_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::acos(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto acos(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, acos_op);
		}

		constexpr inline auto atan_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::atan(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto atan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, atan_op);
		}

		constexpr inline auto atan2_op = []<floating_point_dimensional_scalar U>(U arg_y, U arg_x) noexcept { return std::atan2(arg_y, arg_x); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1,
		bool W2, typename D2>
		auto atan(const vector_base<W1, T, C, D1> &y,
				  const vector_base<W2, T, C, D2> &x) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, y, x, atan2_op);
		}

		constexpr inline auto sinh_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::sinh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto sinh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sinh_op);
		}

		constexpr inline auto cosh_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::cosh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto cosh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, cosh_op);
		}

		constexpr inline auto tanh_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::tanh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto tanh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, tanh_op);
		}

		constexpr inline auto asinh_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::asinh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto asinh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, asinh_op);
		}

		constexpr inline auto acosh_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::acosh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto acosh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, acosh_op);
		}

		constexpr inline auto atanh_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::atanh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto atanh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, atanh_op);
		}

		//
		// 8.2 - exponential
		//

		constexpr inline auto pow_op = []<floating_point_dimensional_scalar U>(U base, U exp) noexcept { return std::pow(base, exp); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		auto pow(const vector_base<W1, T, C, D1> &base,
				 const vector_base<W2, T, C, D2> &exp) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, base, exp, pow_op);
		}

		constexpr inline auto exp_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::exp(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto exp(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, exp_op);
		}

		constexpr inline auto log_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::log(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto log(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, log_op);
		}

		constexpr inline auto exp2_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::exp2(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto exp2(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, exp2_op);
		}

		constexpr inline auto log2_op = [](floating_point_dimensional_scalar auto arg) noexcept { return std::log2(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto log2(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, log2_op);
		}

		constexpr inline auto sqrt_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::sqrt(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto sqrt(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sqrt_op);
		}

		constexpr inline auto rsqrt_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::rsqrt(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto inversesqrt(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, rsqrt_op);
		}

		//
		// 8.3 - common
		//

		constexpr inline auto abs_op = []<dimensional_scalar T>(T arg) noexcept -> T { return cxcm::abs(arg); };

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		requires (!std::unsigned_integral<T>) && non_bool_arithmetic<T>
		constexpr auto abs(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, abs_op);
		}

		constexpr inline auto sign_op = []<dimensional_scalar T>(T arg) noexcept -> T { return T(T(T(0) < arg) - T(arg < T(0))); };

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		requires (!std::unsigned_integral<T>) && non_bool_arithmetic<T>
		constexpr auto sign(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sign_op);
		}

		constexpr inline auto floor_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::floor(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto floor(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, floor_op);
		}

		constexpr inline auto trunc_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::trunc(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto trunc(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, trunc_op);
		}

		constexpr inline auto round_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::round(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto round(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, round_op);
		}

		constexpr inline auto round_even_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::round_even(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto roundEven(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, round_even_op);
		}

		constexpr inline auto ceil_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::ceil(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto ceil(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, ceil_op);
		}

		constexpr inline auto fract_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::fract(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto fract(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, fract_op);
		}

		constexpr inline auto mod_op = []<floating_point_dimensional_scalar T>(T x, T y) noexcept { return x - y * cxcm::floor(x / y); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto mod(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, mod_op);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto mod(const vector_base<W, T, C, D> &x,
						   T y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, mod_op);
		}

		constexpr inline auto modf_op = []<floating_point_dimensional_scalar T>(T x, T y) noexcept { return x - y; };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires W2
		constexpr auto modf(const vector_base<W1, T, C, D1> &arg,
							vector_base<W2, T, C, D2> &i) noexcept
		{
			(*static_cast<D2 *>(&i)) = trunc(arg);
			return detail::binary_op_execute(std::make_index_sequence<C>{}, arg, i, modf_op);
		}

		constexpr inline auto min_op = []<floating_point_dimensional_scalar T>(T x, T y) noexcept { return y < x ? y : x; };

		template <bool W1, non_bool_arithmetic T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto min(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, min_op);
		}

		template <bool W, non_bool_arithmetic T, std::size_t C, typename D>
		constexpr auto min(const vector_base<W, T, C, D> &x,
						   T y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, min_op);
		}

		constexpr inline auto max_op = []<floating_point_dimensional_scalar T>(T x, T y) noexcept { return y < x ? x : y; };

		template <bool W1, non_bool_arithmetic T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto max(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, max_op);
		}

		template <bool W, non_bool_arithmetic T, std::size_t C, typename D>
		constexpr auto max(const vector_base<W, T, C, D> &x,
						   T y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, max_op);
		}

		constexpr inline auto clamp_op = []<dimensional_scalar T>(T x, T min_val, T max_val) noexcept { return std::min(std::max(x, min_val), max_val); };

		template <bool W1, non_bool_arithmetic T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		constexpr auto clamp(const vector_base<W1, T, C, D1> &x,
							 const vector_base<W2, T, C, D2> &min_val,
							 const vector_base<W3, T, C, D3> &max_val) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, min_val, max_val, clamp_op);
		}

		template <bool W, non_bool_arithmetic T, std::size_t C, typename D>
		constexpr auto clamp(const vector_base<W, T, C, D> &x,
							 T min_val,
							 T max_val) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, min_val, max_val, clamp_op);
		}

		constexpr inline auto mix1_op = []<floating_point_dimensional_scalar T>(T x, T y, T a) noexcept { return std::lerp(x, y, a); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		constexpr auto mix(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y,
						   const vector_base<W3, T, C, D3> &a) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, y, a, mix1_op);
		}

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto mix(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y,
						   T a) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, y, a, mix1_op);
		}

		constexpr inline auto mix2_op = []<dimensional_scalar T, boolean_dimensional_scalar B>(T x, T y, B a) noexcept -> T { return a ? y : x; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1,
			bool W2, typename D2, bool W3, boolean_dimensional_scalar B, typename D3>
		constexpr auto mix(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y,
						   const vector_base<W3, B, C, D3> &a) noexcept
		{
			return detail::ternary_op_execute_no_convert(std::make_index_sequence<C>{}, x, y, a, mix2_op);
		}

		constexpr inline auto step_op = []<floating_point_dimensional_scalar T>(T edge, T x) noexcept { return ((x < edge) ? T(0) : T(1)); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto step(const vector_base<W1, T, C, D1> &edge,
							const vector_base<W2, T, C, D2> &x) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, edge, x, step_op);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto step(T edge,
							const vector_base<W, T, C, D> &x) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, edge, x, step_op);
		}

		constexpr inline auto smoothstep_op = []<floating_point_dimensional_scalar T>(T edge0, T edge1, T x) noexcept
		{
			T t = clamp_op((x - edge0) / (edge1 - edge0), T(0), T(1));
			return t * t * (T(3) - T(2) * t);
		};

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		constexpr auto smoothstep(const vector_base<W1, T, C, D1> &edge0,
								  const vector_base<W2, T, C, D2> &edge1,
								  const vector_base<W3, T, C, D3> &x) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, edge0, edge1, x, smoothstep_op);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto smoothstep(T edge0,
								  T edge1,
								  const vector_base<W, T, C, D> &x) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, edge0, edge1, x, smoothstep_op);
		}

		constexpr inline auto isnan_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::isnan(arg); };

		// isnan() is defined for each derived type due to the way MSVC implemented their isnan() in <cmath>

		template <floating_point_dimensional_scalar T, std::size_t C>
		constexpr auto isnan(const basic_vector<T, C> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, isnan_op);
		}

		template <floating_point_dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		constexpr auto isnan(const indexed_vector<T, S, C, Is...> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, isnan_op);
		}

		constexpr inline auto isinf_op = [](floating_point_dimensional_scalar auto arg) noexcept { return cxcm::isinf(arg); };

		// isinf() is defined for each derived type due to the way MSVC implemented their isinf() in <cmath>

		template <floating_point_dimensional_scalar T, std::size_t C>
		constexpr auto isinf(const basic_vector<T, C> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, isinf_op);
		}

		template <floating_point_dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
		constexpr auto isinf(const indexed_vector<T, S, C, Is...> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, isinf_op);
		}

#if defined(__cpp_lib_bit_cast)

		constexpr inline auto float_bits_to_int_op = [](float arg) noexcept { return std::bit_cast<int>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto floatBitsToInt(const vector_base<W, float, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, float_bits_to_int_op);
		}

		constexpr inline auto float_bits_to_uint_op = [](float arg) noexcept { return std::bit_cast<unsigned int>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto floatBitsToUint(const vector_base<W, float, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, float_bits_to_uint_op);
		}

		constexpr inline auto double_bits_to_long_long_op = [](double arg) noexcept { return std::bit_cast<long long>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto doubleBitsToLongLong(const vector_base<W, double, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, double_bits_to_long_long_op);
		}

		constexpr inline auto double_bits_to_ulong_long_op = [](double arg) noexcept { return std::bit_cast<unsigned long long>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto doubleBitsToUlongLong(const vector_base<W, double, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, double_bits_to_ulong_long_op);
		}

		constexpr inline auto int_bits_to_float_op = [](int arg) noexcept { return std::bit_cast<float>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto intBitsToFloat(const vector_base<W, int, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, int_bits_to_float_op);
		}

		constexpr inline auto uint_bits_to_float_op = [](unsigned int arg) noexcept { return std::bit_cast<float>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto uintBitsToFloat(const vector_base<W, unsigned int, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, uint_bits_to_float_op);
		}

		constexpr inline auto long_long_bits_to_double_op = [](long long arg) noexcept { return std::bit_cast<double>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto longLongBitsToDouble(const vector_base<W, long long, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, long_long_bits_to_double_op);
		}

		constexpr inline auto ulong_long_bits_to_double_op = [](unsigned long long arg) noexcept { return std::bit_cast<double>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto ulongLongBitsToDouble(const vector_base<W, unsigned long long, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, ulong_long_bits_to_double_op);
		}

#endif

		constexpr inline auto fma_op = []<floating_point_dimensional_scalar T>(T a, T b, T c) noexcept { return std::fma(a, b, c); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		auto fma(const vector_base<W1, T, C, D1> &a,
				 const vector_base<W2, T, C, D2> &b,
				 const vector_base<W3, T, C, D3> &c) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, a, b, c, fma_op);
		}

		constexpr inline auto frexp_op = []<floating_point_dimensional_scalar T>(T x, int &exp) noexcept { return std::frexp(x, &exp); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires W2
		auto frexp(const vector_base<W1, T, C, D1> &x,
				   vector_base<W2, int, C, D2> &exp) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return basic_vector<T, C>(frexp_op(x[Is], exp[Is])...);
			}(std::make_index_sequence<C>{});
		}

		constexpr inline auto ldexp_op = []<floating_point_dimensional_scalar T>(T x, int exp) noexcept { return std::ldexp(x, exp); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		auto ldexp(const vector_base<W1, T, C, D1> &x,
				   const vector_base<W2, int, C, D2> &exp) noexcept
		{
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, x, exp, ldexp_op);
		}

		//
		// 8.4 is omitted
		//

		//
		// 8.5 - geometric
		//

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto dot(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return ((x[Is] * y[Is]) + ...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W1, floating_point_dimensional_scalar T, typename D1, bool W2, typename D2>
		constexpr auto cross(const vector_base<W1, T, 3u, D1> &x,
							 const vector_base<W2, T, 3u, D2> &y) noexcept
		{
			return basic_vector<T, 3u>((x[1] * y[2]) - (y[1] * x[2]),
									   (x[2] * y[0]) - (y[2] * x[0]),
									   (x[0] * y[1]) - (y[0] * x[1]));
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto length(const vector_base<W, T, C, D> &x) noexcept
		{
			return cxcm::sqrt(dot(x, x));
		}

		template <bool W, floating_point_dimensional_scalar T, typename D>
		constexpr auto length(const vector_base<W, T, 1u, D> &x) noexcept
		{
			return cxcm::abs(x[0u]);
		}

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto distance(const vector_base<W1, T, C, D1> &p0,
								const vector_base<W2, T, C, D2> &p1) noexcept
		{
			return length(p1 - p0);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto normalize(const vector_base<W, T, C, D> &x) noexcept
		{
			auto len = length(x);
			if (T(0.0) == len)
				return basic_vector<T, C>(std::numeric_limits<T>::quiet_NaN());

			[[likely]] return x / length(x);
		}

		template <bool W, floating_point_dimensional_scalar T, typename D>
		constexpr auto normalize(const vector_base<W, T, 1u, D> &x) noexcept
		{
			// can't normalize 0 -> 0/0
			if (T(0.0) == x[0u])
				return basic_vector<T, 1u>(std::numeric_limits<T>::quiet_NaN());

			[[likely]] return basic_vector<T, 1u>(T(1.0));
		}

		//
		// vec4 ftransform() omitted
		//
		
		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1,
			bool W2, typename D2, bool W3, typename D3>
		constexpr auto faceforward(const vector_base<W1, T, C, D1> &n,
								   const vector_base<W2, T, C, D2> &i,
								   const vector_base<W3, T, C, D3> &nref) noexcept
		{
			return (dot(nref, i) < T(0)) ? +n : -n;
		}

		// n must be normalized in order to achieve desired results
		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto reflect(const vector_base<W1, T, C, D1> &i,
							   const vector_base<W2, T, C, D2> &n) noexcept
		{
			return i - T(2) * dot(n, i) * n;
		}

		// i and n must be normalized in order to achieve desired results
		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto refract(const vector_base<W1, T, C, D1> &i,
							   const vector_base<W2, T, C, D2> &n,
							   T eta) noexcept
		{
			T k = T(1) - eta * eta * (T(1) - dot(n, i) * dot(n, i));

			if (k < T(0))
				return basic_vector<T, C>(T(0));

			return eta * i - (eta * dot(n, i) + cxcm::sqrt(k)) * n;
		}

		//
		// 8.7 - vector relational
		//

		constexpr inline auto less_op = []<dimensional_scalar T>(T x, T y) noexcept -> bool { return std::isless(x, y); };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto lessThan(const vector_base<W1, T, C, D1> &x,
								const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, less_op);
		}

		constexpr inline auto less_equal_op = []<dimensional_scalar T>(T x, T y) noexcept -> bool { return std::islessequal(x, y); };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto lessThanEqual(const vector_base<W1, T, C, D1> &x,
									 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, less_equal_op);
		}

		constexpr inline auto greater_op = []<dimensional_scalar T>(T x, T y) noexcept -> bool { return std::isgreater(x, y); };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto greaterThan(const vector_base<W1, T, C, D1> &x,
								   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, greater_op);
		}

		constexpr inline auto greater_equal_op = []<dimensional_scalar T>(T x, T y) noexcept -> bool { return std::isgreaterequal(x, y); };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto greaterThanEqual(const vector_base<W1, T, C, D1> &x,
								   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, greater_equal_op);
		}

		constexpr inline auto equal_op = []<dimensional_scalar T>(T x, T y) noexcept -> bool { return std::isunordered(x, y) ? false : x == y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto equal(const vector_base<W1, T, C, D1> &x,
							 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, equal_op);
		}

		constexpr inline auto bool_equal_op = [](bool x, bool y) noexcept -> bool { return x == y; };

		template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto equal(const vector_base<W1, bool, C, D1> &x,
							 const vector_base<W2, bool, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, bool_equal_op);
		}

		constexpr inline auto not_equal_op = []<dimensional_scalar T>(T x, T y) noexcept -> bool { return std::isunordered(x, y) ? true : x != y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto notEqual(const vector_base<W1, T, C, D1> &x,
								const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, not_equal_op);
		}

		constexpr inline auto bool_not_equal_op = [](bool x, bool y) noexcept -> bool { return x != y; };

		template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto notEqual(const vector_base<W1, bool, C, D1> &x,
								const vector_base<W2, bool, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, bool_not_equal_op);
		}

		template <bool W, std::size_t C, typename D>
		constexpr bool any(const vector_base<W, bool, C, D> &x) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (x[Is] || ...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W, std::size_t C, typename D>
		constexpr bool all(const vector_base<W, bool, C, D> &x) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return (x[Is] && ...);
			}(std::make_index_sequence<C>{});
		}

		// this is function is not in the glsl standard, same result as "!any(x)"
		template <bool W, std::size_t C, typename D>
		constexpr bool none(const vector_base<W, bool, C, D> &x) noexcept
		{
			return !any(x);
//			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
//			{
//				return !(x[Is] || ...);
//			}(std::make_index_sequence<C>{});
		}

		constexpr inline auto logical_not_op = [](bool x) noexcept -> bool { return !x; };

		// c++ is not allowing a function named not()
		template <bool W, std::size_t C, typename D>
		constexpr auto logicalNot(const vector_base<W, bool, C, D> &x) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, x, logical_not_op);
		}

		//
		// 8.8 - 8.19 are omitted
		//


		//
		// basic tolerance comparisons
		//

		// Euclidean distance check - strictly less than comparison, boundary is false

		template <std::floating_point T>
		constexpr bool within_distance(T x, T y, T tolerance) noexcept
		{
#if 1
			return cxcm::fabs(x - y) < tolerance;
#else
			auto delta = x - y;
			return (delta * delta) < (tolerance * tolerance);
#endif
		}

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
									   const vector_base<W2, T, C, D2> &y,
									   T tolerance) noexcept
		{
#if 0
			return distance(x - y) < tolerance;
#else
			auto direction_vector = x - y;
			return dot(direction_vector, direction_vector) < (tolerance * tolerance);
#endif
		}

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		requires non_bool_arithmetic<T>
		constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
									   const vector_base<W2, T, C, D2> &y,
									   const vector_base<W3, T, 1u, D3> &tolerance) noexcept
		{
			return within_distance(x, y, tolerance[0u]);
		}

		// tolerance-box component check - strictly less than comparison, boundary is false

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr bool within_box(const vector_base<W1, T, C, D1> &x,
								  const vector_base<W2, T, C, D2> &y,
								  T tolerance) noexcept
		{
			auto deltas = abs(x - y);
			auto comparison_vector = lessThan(deltas, decltype(deltas)(tolerance));
			return all(comparison_vector);
		}

		template <bool W1, dimensional_scalar T, std::size_t C1, typename D1,
			bool W2, typename D2, bool W3, std::size_t C2, typename D3>
		requires ((C1 == C2) || (C2 == 1u)) && non_bool_arithmetic<T>
		constexpr bool within_box(const vector_base<W1, T, C1, D1> &x,
								  const vector_base<W2, T, C1, D2> &y,
								  const vector_base<W3, T, C2, D3> &tolerance) noexcept
		{
			if constexpr (C1 == C2)
			{
				auto deltas = abs(x - y);
				auto comparison_vector = lessThan(deltas, tolerance);
				return all(comparison_vector);
			}
			else		// (C2 == 1u)
			{
				return within_box(x, y, tolerance[0u]);
			}
		}

	}

	//
	// matrices
	//

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
	requires (((C >= 2u) && (C <= 4u)) && ((R >= 2u) && (R <= 4u)))
	struct basic_matrix
	{
		static constexpr std::size_t ComponentCount = C * R;

		// number of columns
		[[nodiscard]] constexpr int length() const noexcept
		{
			return C;
		}

		// number of rows
		[[nodiscard]] constexpr int column_length() const noexcept
		{
			return R;
		}

		// returns number of columns (row size), not number of elements
		[[nodiscard]] constexpr std::size_t size() const noexcept
		{
			return C;
		}

		// returns number of rows
		[[nodiscard]] constexpr std::size_t column_size() const noexcept
		{
			return R;
		}

		std::array<basic_vector<T, R>, C> value;

		//
		// operator [] gets the column vector
		//

		constexpr basic_vector<T, R> &operator [](std::size_t index) noexcept
		{
			return value[index];
		}

		constexpr const basic_vector<T, R> &operator [](std::size_t index) const noexcept
		{
			return value[index];
		}

		// get a row of the matrix as a vector
		[[nodiscard]] constexpr basic_vector<T, C> row(std::size_t row_index) const noexcept
		{
			// for each column of the matrix, get a row component, and bundle
			// these components up into a vector that represents the row
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				return basic_vector<T, C>(value[Is][row_index]...);
			}(std::make_index_sequence<C>{});
		}

		//
		// defaulted functions
		//

		constexpr basic_matrix() noexcept = default;
		constexpr ~basic_matrix() noexcept = default;

		constexpr basic_matrix(const basic_matrix &) noexcept = default;
		constexpr basic_matrix(basic_matrix &&) noexcept = default;
		constexpr basic_matrix &operator =(const basic_matrix &) noexcept = default;
		constexpr basic_matrix &operator =(basic_matrix &&) noexcept = default;

		//
		// constructors
		//

		// variadic constructor of scalar and vector arguments
		template <typename ... Args>
		requires (detail::valid_component_source<Args>::value && ...) && detail::met_component_count<ComponentCount, Args...>
		explicit constexpr basic_matrix(const Args & ...args) noexcept
		{
			auto arg_tuple = detail::flatten_args_to_tuple(args...);
			[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				((value[Is / R][Is % R] = static_cast<T>(std::get<Is>(arg_tuple))), ...);
			}(std::make_index_sequence<ComponentCount>{});
		}

		// diagonal constructor for square matrices
		template <typename U>
		requires std::convertible_to<U, T> && (C == R)
		explicit constexpr basic_matrix(U arg) noexcept
		{
			for (std::size_t i = 0; i < C; ++i)
			{
				for (std::size_t j = 0; j < R; ++j)
				{
					value[i][j] = ((i == j) ? static_cast<T>(arg) : static_cast<T>(0));
				}
			}
		}

		// implicit constructor from a matrix
		template <floating_point_dimensional_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_matrix(const basic_matrix<U, C, R> &arg) noexcept
		{
			for (std::size_t i = 0; i < C; ++i)
			{
				value[i] = arg[i];
			}
		}

		// implicit constructor from a matrix
		template <floating_point_dimensional_scalar U, std::size_t Cols, std::size_t Rows>
		requires implicitly_convertible_to<U, T> && (Cols != C || Rows != R)
		constexpr basic_matrix(const basic_matrix<U, Cols, Rows> &arg) noexcept
		{
			for (std::size_t i = 0; i < C; ++i)
			{
				if (i >= Cols)
				{
					// not enough columns in arg
					for (std::size_t j = 0; j < R; ++j)
					{
						value[i][j] = (((i == j) && (C == R)) ? static_cast<T>(1) : static_cast<T>(0));
					}
				}
				else
				{
					for (std::size_t j = 0; j < R; ++j)
					{
						if (j >= Rows)
						{
							// not enough rows in arg
							value[i][j] = (((i == j) && (C == R)) ? static_cast<T>(1) : static_cast<T>(0));
						}
						else
						{
							value[i][j] = static_cast<T>(arg[i][j]);
						}
					}
				}
			}
		}

		// explicit constructor from a matrix
		template <floating_point_dimensional_scalar U, std::size_t Cols, std::size_t Rows>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>) && (Cols != C || Rows != R)
		explicit constexpr basic_matrix(const basic_matrix<U, Cols, Rows> &arg) noexcept
		{
			for (std::size_t i = 0; i < C; ++i)
			{
				if (i >= Cols)
				{
					// not enough columns in arg
					for (std::size_t j = 0; j < R; ++j)
					{
						value[i][j] = (((i == j) && (C == R)) ? static_cast<T>(1) : static_cast<T>(0));
					}
				}
				else
				{
					for (std::size_t j = 0; j < R; ++j)
					{
						if (j >= Rows)
						{
							// not enough rows in arg
							value[i][j] = (((i == j) && (C == R)) ? static_cast<T>(1) : static_cast<T>(0));
						}
						else
						{
							value[i][j] = static_cast<T>(arg[i][j]);
						}
					}
				}
			}
		}

		//
		// assignment operators
		//

		template <floating_point_dimensional_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_matrix &operator =(const basic_matrix<U, C, R> &other) noexcept
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>) noexcept
			{
				((value[Is] = other[Is]), ...);			// let basic_vector do any type conversion if needed
			}(std::make_index_sequence<C>{});

			return *this;
		}

		// pointer interface
		constexpr basic_vector<T, R> * data() noexcept								{ return value.data(); }
		[[nodiscard]] constexpr const basic_vector<T, R> * data() const noexcept	{ return value.data(); }

		// support for range-based for loop
		constexpr auto begin() noexcept												{ return value.begin(); }
		[[nodiscard]] constexpr auto begin() const noexcept							{ return value.cbegin(); }
		[[nodiscard]] constexpr auto cbegin() const noexcept						{ return value.cbegin(); }
		constexpr auto end() noexcept												{ return value.end(); }
		[[nodiscard]] constexpr auto end() const noexcept							{ return value.cend(); }
		[[nodiscard]] constexpr auto cend() const noexcept							{ return value.cend(); }
	};

	//
	// get<> part of tuple interface -- needed for structured bindings
	//

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	constexpr auto && get(dsga::basic_matrix<T, C, R> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	constexpr auto && get(const dsga::basic_matrix<T, C, R> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	constexpr auto && get(dsga::basic_matrix<T, C, R> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
	requires (N >= 0) && (N < C)
	constexpr auto && get(const dsga::basic_matrix<T, C, R> && arg) noexcept
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
		template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
		constexpr auto matrixCompMult(const basic_matrix<T, C, R> &lhs,
									  const basic_matrix<U, C, R> &rhs) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] * rhs[Is])...);
			}(std::make_index_sequence<C>{});
		}

		// outerProduct() - matrix from a column vector times a row vector
		template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
			bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
		requires (floating_point_dimensional_scalar<T1> || floating_point_dimensional_scalar<T2>) && ((C1 >= 2) && (C1 <= 4)) && ((C2 >= 2) && (C2 <= 4))
		constexpr auto outerProduct(const vector_base<W1, T1, C1, D1> &lhs,
									const vector_base<W2, T2, C2, D2> &rhs) noexcept
		{
			return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
			{
				return basic_matrix<std::common_type_t<T1, T2>, C1, C2>(
					[&]<std::size_t ...Is>(std::index_sequence <Is...>, auto row) noexcept
					{
						return basic_vector<std::common_type_t<T1, T2>, C1>((lhs[Is] * row)...);
					}(std::make_index_sequence<C1>{}, rhs[Js]) ...);
			}(std::make_index_sequence<C2>{});
		}

		// transpose a matrix
		template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
		constexpr auto transpose(const basic_matrix<T, C, R> &arg) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				return basic_matrix<T, R, C>(arg.row(Is)...);
			}(std::make_index_sequence<R>{});
		}

		// determinant() - only on square matrices

		// going for efficiency
		template <floating_point_dimensional_scalar T>
		constexpr auto determinant(const basic_matrix<T, 2u, 2u> &arg) noexcept
		{
			return 
				+ arg[0][0] * arg[1][1]
				- arg[0][1] * arg[1][0]
				;
		}

		// going for efficiency
		template <floating_point_dimensional_scalar T>
		constexpr auto determinant(const basic_matrix<T, 3u, 3u> &arg) noexcept
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

		template <floating_point_dimensional_scalar T>
		constexpr auto determinant(const basic_matrix<T, 4u, 4u> &arg) noexcept
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
		template <floating_point_dimensional_scalar T>
		constexpr auto inverse(const basic_matrix<T, 2u, 2u> &arg) noexcept
		{
			return basic_matrix<T, 2u, 2u>( arg[1][1], -arg[0][1],
										   -arg[1][0],  arg[0][0]) / determinant(arg);
		}

		// going for efficiency
		template <floating_point_dimensional_scalar T>
		constexpr auto inverse(const basic_matrix<T, 3u, 3u> &arg) noexcept
		{
			return basic_matrix<T, 3u, 3u>(
				+(arg[1][1] * arg[2][2] - arg[2][1] * arg[1][2]),
				-(arg[0][1] * arg[2][2] - arg[2][1] * arg[0][2]),
				+(arg[0][1] * arg[1][2] - arg[1][1] * arg[0][2]),
				-(arg[1][0] * arg[2][2] - arg[2][0] * arg[1][2]),
				+(arg[0][0] * arg[2][2] - arg[2][0] * arg[0][2]),
				-(arg[0][0] * arg[1][2] - arg[1][0] * arg[0][2]),
				+(arg[1][0] * arg[2][1] - arg[2][0] * arg[1][1]),
				-(arg[0][0] * arg[2][1] - arg[2][0] * arg[0][1]),
				+(arg[0][0] * arg[1][1] - arg[1][0] * arg[0][1])
				) / determinant(arg);
		}

		// going for efficiency
		template <floating_point_dimensional_scalar T>
		constexpr auto inverse(const basic_matrix<T, 4u, 4u> &arg) noexcept
		{
			return basic_matrix<T, 4u, 4u>(
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
				) / determinant(arg);
		}
	}

	//
	// matrix operators
	//

	// component-wise equality operator for matrices, scalar boolean result: ==, != (thanks to c++20)
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
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
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
	constexpr auto operator +(const basic_matrix<T, C, R> &arg) noexcept
	{
		return basic_matrix<T, C, R>(arg);
	}

	// unary -
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
	constexpr auto operator -(const basic_matrix<T, C, R> &arg) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<T, C, R>((-arg[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// pre-increment
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
	constexpr auto &operator ++(basic_matrix<T, C, R> &arg) noexcept
	{
		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((++arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return arg;
	}

	// post-increment
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
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
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
	constexpr auto &operator --(basic_matrix<T, C, R> &arg) noexcept
	{
		[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			((--arg[Is]),...);
		}(std::make_index_sequence<C>{});
		return arg;
	}

	// post-decrement
	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
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

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator +(const basic_matrix<T, C, R> &lhs,
							  U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>)
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] + rhs)...);
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator +(U lhs,
							  const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs + rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// operator - with scalar

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator -(const basic_matrix<T, C, R> &lhs,
							  U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] - rhs)...);
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator -(U lhs,
							  const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs - rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// operator * with scalar

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator *(const basic_matrix<T, C, R> &lhs,
							  U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] * rhs)...);
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator *(U lhs,
							  const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs * rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// operator / with scalar

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator /(const basic_matrix<T, C, R> &lhs,
							  U rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] / rhs)...);
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator /(U lhs,
							  const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs / rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// operator + with same size matrices

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator +(const basic_matrix<T, C, R> &lhs,
							  const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] + rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// operator - with same size matrices

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator -(const basic_matrix<T, C, R> &lhs,
							  const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] - rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// operator / with same size matrices

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R, floating_point_dimensional_scalar U>
	constexpr auto operator /(const basic_matrix<T, C, R> &lhs,
							  const basic_matrix<U, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_matrix<std::common_type_t<T, U>, C, R>((lhs[Is] / rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	//
	// linear-algebriac binary ops
	//

	// matrix * (column) vector => (column) vector

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R,
		bool W, dimensional_scalar U, typename D>
	constexpr auto operator *(const basic_matrix<T, C, R> &lhs,
							  const vector_base<W, U, C, D> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_vector<std::common_type_t<T, U>, R>(functions::dot(lhs.row(Is), rhs)...);
		}(std::make_index_sequence<R>{});
	}

	// (row) vector * matrix => (row) vector

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R,
		bool W, dimensional_scalar U, typename D>
	constexpr auto operator *(const vector_base<W, U, R, D> &lhs,
							  const basic_matrix<T, C, R> &rhs) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
		{
			return basic_vector<std::common_type_t<T, U>, C>(functions::dot(lhs, rhs[Is])...);
		}(std::make_index_sequence<C>{});
	}

	// matrix * matrix => matrix

	template <floating_point_dimensional_scalar T, std::size_t C, std::size_t R1, std::size_t C2>
	constexpr auto operator *(const basic_matrix<T, C, R1> &lhs,
							  const basic_matrix<T, C2, C> &rhs) noexcept
	{
		return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
		{
			return basic_matrix<T, C2, R1>(
				[&]<std::size_t ...Is>(std::index_sequence <Is...>, const auto &col) noexcept
				{
					return basic_vector<T, R1>(functions::dot(lhs.row(Is), col)...);
				}(std::make_index_sequence<R1>{}, rhs[Js]) ...);
		}(std::make_index_sequence<C2>{});
	}

	//
	// specialized using types
	//

	// boolean vectors
	using bscal = basic_vector<bool, 1u>;
	using bvec2 = basic_vector<bool, 2u>;
	using bvec3 = basic_vector<bool, 3u>;
	using bvec4 = basic_vector<bool, 4u>;

	// int vectors
	using iscal = basic_vector<int, 1u>;
	using ivec2 = basic_vector<int, 2u>;
	using ivec3 = basic_vector<int, 3u>;
	using ivec4 = basic_vector<int, 4u>;

	// unsigned int vectors
	using uscal = basic_vector<unsigned, 1u>;
	using uvec2 = basic_vector<unsigned, 2u>;
	using uvec3 = basic_vector<unsigned, 3u>;
	using uvec4 = basic_vector<unsigned, 4u>;

	// long long vectors (not in glsl)
	using llscal = basic_vector<long long, 1u>;
	using llvec2 = basic_vector<long long, 2u>;
	using llvec3 = basic_vector<long long, 3u>;
	using llvec4 = basic_vector<long long, 4u>;

	// unsigned long long vectors (not in glsl)
	using ullscal = basic_vector<unsigned long long, 1u>;
	using ullvec2 = basic_vector<unsigned long long, 2u>;
	using ullvec3 = basic_vector<unsigned long long, 3u>;
	using ullvec4 = basic_vector<unsigned long long, 4u>;

	// float vectors with out an 'f' prefix -- this is from glsl
	using scal = basic_vector<float, 1u>;
	using vec2 = basic_vector<float, 2u>;
	using vec3 = basic_vector<float, 3u>;
	using vec4 = basic_vector<float, 4u>;

	// also float vectors, but using the same naming convention as the other vectors do (not in glsl)
	using fscal = basic_vector<float, 1u>;
	using fvec2 = basic_vector<float, 2u>;
	using fvec3 = basic_vector<float, 3u>;
	using fvec4 = basic_vector<float, 4u>;

	// double vectors
	using dscal = basic_vector<double, 1u>;
	using dvec2 = basic_vector<double, 2u>;
	using dvec3 = basic_vector<double, 3u>;
	using dvec4 = basic_vector<double, 4u>;

	// float matrices
	using mat2x2 = basic_matrix<float, 2u, 2u>;
	using mat2x3 = basic_matrix<float, 2u, 3u>;
	using mat2x4 = basic_matrix<float, 2u, 4u>;
	using mat3x2 = basic_matrix<float, 3u, 2u>;
	using mat3x3 = basic_matrix<float, 3u, 3u>;
	using mat3x4 = basic_matrix<float, 3u, 4u>;
	using mat4x2 = basic_matrix<float, 4u, 2u>;
	using mat4x3 = basic_matrix<float, 4u, 3u>;
	using mat4x4 = basic_matrix<float, 4u, 4u>;

	using mat2 = basic_matrix<float, 2u, 2u>;
	using mat3 = basic_matrix<float, 3u, 3u>;
	using mat4 = basic_matrix<float, 4u, 4u>;

	// double matrices
	using dmat2x2 = basic_matrix<double, 2u, 2u>;
	using dmat2x3 = basic_matrix<double, 2u, 3u>;
	using dmat2x4 = basic_matrix<double, 2u, 4u>;
	using dmat3x2 = basic_matrix<double, 3u, 2u>;
	using dmat3x3 = basic_matrix<double, 3u, 3u>;
	using dmat3x4 = basic_matrix<double, 3u, 4u>;
	using dmat4x2 = basic_matrix<double, 4u, 2u>;
	using dmat4x3 = basic_matrix<double, 4u, 3u>;
	using dmat4x4 = basic_matrix<double, 4u, 4u>;

	using dmat2 = basic_matrix<double, 2u, 2u>;
	using dmat3 = basic_matrix<double, 3u, 3u>;
	using dmat4 = basic_matrix<double, 4u, 4u>;

	//
	// bring the vector and matrix free functions into the dsga namespace
	//
	using namespace functions;

}	// namespace dsga

//
// tuple inteface for basic_vector and indexed_vector and vec_base -- supports structured bindings
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

template <dsga::floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
struct std::tuple_size<dsga::basic_matrix<T, C, R>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, dsga::floating_point_dimensional_scalar T, std::size_t C, std::size_t R>
struct std::tuple_element<I, dsga::basic_matrix<T, C, R>>
{
	using type = dsga::basic_vector<T, R>;
};

// converting from external vector type or data to internal vector type

template <dsga::dimensional_scalar T, std::size_t S>
requires dsga::dimensional_storage<T, S>
constexpr auto to_vec(const std::array<T, S> &arg) noexcept
{
	return dsga::detail::passthru_execute(std::make_index_sequence<S>{}, arg);
}

template <dsga::dimensional_scalar T, std::size_t S>
requires dsga::dimensional_storage<T, S>
constexpr auto to_vec(const T (&arg)[S]) noexcept
{
	return dsga::detail::passthru_execute(std::make_index_sequence<S>{}, arg);
}

// converting from internal vector type to std::array

template <dsga::dimensional_scalar T, std::size_t S>
constexpr std::array<T, S> from_vec(const dsga::basic_vector<T, S> &arg) noexcept
{
	return arg.base.store;
}

template <bool W, typename T, std::size_t C, typename D>
constexpr std::array<T, C> from_vec(const dsga::vector_base<W, T, C, D> &arg) noexcept
{
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept -> std::array<T, C>
	{
		return { arg[Is]... };
	}(std::make_index_sequence<C>{});
}

// closing include guard
#endif
