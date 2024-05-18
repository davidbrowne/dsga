
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// performing tolerance checks
//

namespace dsga
{
	//
	// basic tolerance comparisons -- will assert if tolerance has any negative values.
	// these functions do not give component-wise results -- they return a single bool value about the situation.
	//

	// General Tolerance Checking - this checks if one or more values is close to 0 within a scalar or vector of tolerances.
	// Returns true if all the values are within tolerance.The tolerance checking is a less-than-or-equal comparison.
	// Tolerances need to be non-negative, or the function will assert. We use vector_base as the vector argument type so
	// as to cover both basic_vector and indexed_vector.

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
	// whose diameter is 2 * tolerance, return whether y is also in the region. Tolerances need to be non-negative,
	// or the function will assert. uses less than or equal comparison: if y is on region boundary, that counts as within.

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
	// whose side lengths are 2 * tolerance, return whether y is also in the orthogonal region. Tolerances need to be non-negative,
	// or the function will assert. uses less than or equal comparison: if y is on the region boundary, that counts as within.

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
