#pragma once

//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

// opening include guard
#if !defined(DSGA_TOLERANCE_HXX)
#define DSGA_TOLERANCE_HXX

//
// performing tolerance checks
//

namespace dsga
{
	//
	// Basic tolerance comparisons:
	// 
	//	* within_tolerance(x, tolerance) - vectors/matrices treated as independent components for checks on x close to 0
	//	* within_tolerance(x, y, tolerance) - vectors/matrices treated as independent components for checks on x close to y
	//	* within_distance(x, y, tolerance) - vectors treated as a whole for Euclidean distance check on x and y
	//	* within_box(x, y, tolerance) - vectors/matrices treated as whole for orthogonal box check on x and y.
	//
	//	if you want component-wise checks, use "within_tolerance(x - y, tolerance)" for comparing 2 vectors.
	//	For matrices, since we can't have a matrix of bools, we return a vector of bools, where each value
	//	indicates whether a matrix column has all its elements within tolerance or not, and the tolerance
	//	vector elements (assuming that tolerance is not a scalar) are tolerances for each entire column.
	//

	// General Tolerance Checking - checks if one or more values is close to 0 within the tolerances.
	// Returns component-wise tolerance comparisons. The tolerance checking is a less-than-or-equal comparison, which implies
	// that the boundary is considered "within". Negative tolerances will be treated the same as non-negative tolerances,
	// i.e., we take the absolute value.

	template <floating_point_scalar T, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_tolerance(T x,
												  U tolerance) noexcept
	{
		return abs(x) <= static_cast<T>(abs(tolerance));
	}

	template <floating_point_scalar T, floating_point_scalar U, floating_point_scalar V>
	requires implicitly_convertible_to<U, T> && implicitly_convertible_to<V, T>
	[[nodiscard]] constexpr bool within_tolerance(T x,
												  U y,
												  V tolerance) noexcept
	{
		return abs(x - y) <= static_cast<T>(abs(tolerance));
	}

	template <bool W, floating_point_scalar T, std::size_t C, typename D, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr auto within_tolerance(const vec_interface<W, T, C, D> &x,
												  U tolerance) noexcept
	{
		if constexpr (C == 1)
		{
			return within_tolerance(x[0], tolerance);
		}
		else
		{
			return lessThanEqual(abs(x), vec<T, C>(static_cast<T>(abs(tolerance))));
		}
	}

	template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr auto within_tolerance(const vec_interface<W1, T, C, D1> &x,
												  const vec_interface<W2, T, C, D2> &y,
												  U tolerance) noexcept
	{
		if constexpr (C == 1)
		{
			return within_tolerance(x[0], y[0], tolerance);
		}
		else
		{
			return lessThanEqual(abs(x - y), vec<T, C>(static_cast<T>(abs(tolerance))));
		}
	}

	template <bool W1, floating_point_scalar T, std::size_t C1, typename D1, bool W2, floating_point_scalar U, std::size_t C2, typename D2>
	requires ((C1 == C2) || (C2 == 1)) && implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr auto within_tolerance(const vec_interface<W1, T, C1, D1> &x,
												  const vec_interface<W2, U, C2, D2> &tolerance) noexcept
	{
		if constexpr (C1 == C2)
		{
			if constexpr (C1 == 1 && C2 == 1)
			{
				return within_tolerance(x[0], tolerance[0]);
			}
			else
			{
				return lessThanEqual(abs(x), static_cast<vec<T, C2>>(abs(tolerance)));
			}
		}
		else		// (C2 == 1)
		{
			return within_tolerance(x, tolerance[0]);
		}
	}

	template <bool W1, floating_point_scalar T, std::size_t C1, typename D1, bool W2, std::size_t C2, typename D2, bool W3, typename D3>
	requires ((C1 == C2) || (C2 == 1))
	[[nodiscard]] constexpr auto within_tolerance(const vec_interface<W1, T, C1, D1> &x,
												  const vec_interface<W2, T, C1, D2> &y,
												  const vec_interface<W3, T, C2, D3> &tolerance) noexcept
	{
		if constexpr (C1 == C2)
		{
			if constexpr (C1 == 1 && C2 == 1)
			{
				return within_tolerance(x[0], y[0], tolerance[0]);
			}
			else
			{
				return lessThanEqual(abs(x - y), abs(tolerance));
			}
		}
		else		// (C2 == 1)
		{
			return within_tolerance(x - y, tolerance[0]);
		}
	}

	// Tolerance checks for matrices - column-wise checks for matrices, i.e., each column is treated as a vector.
	// returns a vector of boolean values, where each value indicates whether the corresponding column has all
	// its elements within tolerance or not.

	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr vec<bool, C> within_tolerance(const mat<T, C, R> &arg,
														  U tolerance) noexcept
	{
		return [&arg, &tolerance] <std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
		{
			return vec<bool, C>(all(within_tolerance(arg[Is], tolerance)) ...);
		}(std::make_index_sequence<C>{});
	}

	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr vec<bool, C> within_tolerance(const mat<T, C, R> &x,
														  const mat<T, C, R> &y,
														  U tolerance) noexcept
	{
		return [&x, &y, &tolerance] <std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
		{
			return vec<bool, C>(all(within_tolerance(x[Is], y[Is], tolerance)) ...);
		}(std::make_index_sequence<C>{});
	}

	template <bool W, floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U, typename D>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr vec<bool, C> within_tolerance(const mat<T, C, R> &arg,
														  const vec_interface<W, U, C, D> &tolerance) noexcept
	{
		return [&arg, &tolerance] <std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
		{
			return vec<bool, C>(all(within_tolerance(arg[Is], tolerance[Is])) ...);
		}(std::make_index_sequence<C>{});
	}

	template <bool W, floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U, typename D>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr vec<bool, C> within_tolerance(const mat<T, C, R> &x,
														  const mat<T, C, R> &y,
														  const vec_interface<W, U, C, D> &tolerance) noexcept
	{
		return [&x, &y, &tolerance] <std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
		{
			return vec<bool, C>(all(within_tolerance(x[Is], y[Is]), tolerance[Is]) ...);
		}(std::make_index_sequence<C>{});
	}

	// Euclidean distance check - imagine x in the center of a region (offset number line, circle, sphere, hypersphere)
	// whose diameter is 2 * tolerance, return whether y is also in the region. Negative tolerances will be treated the
	// same as non-negative tolerances, i.e., we take the absolute value. Uses less-than-or-equal comparison, which implies
	// that the boundary is considered "within".

	template <floating_point_scalar T, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_distance(T x,
												 T y,
												 U tolerance) noexcept
	{
		return abs(x - y) <= static_cast<T>(abs(tolerance));
	}

	template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_distance(const vec_interface<W1, T, C, D1> &x,
												 const vec_interface<W2, T, C, D2> &y,
												 U tolerance) noexcept
	{
		auto diff = x - y;
		return dot(diff, diff) <= static_cast<T>(tolerance * tolerance);
	}

	template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, floating_point_scalar U, typename D3>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_distance(const vec_interface<W1, T, C, D1> &x,
												 const vec_interface<W2, T, C, D2> &y,
												 const vec_interface<W3, U, 1, D3> &tolerance) noexcept
	{
		return within_distance(x, y, tolerance[0]);
	}

	// Tolerance-box check - imagine x in the center of an orthogonal region (offset number line, rectangle, box, hyperbox)
	// whose side lengths are 2 * tolerance, return whether y is also in the orthogonal region, i.e., are all the component
	// checks true. Negative tolerances will be treated the same as non-negative tolerances, i.e., we take the absolute value.
	// Uses less-than-or-equal comparison, which implies that the boundary is considered "within".

	template <floating_point_scalar T, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_box(T x,
											T y,
											U tolerance) noexcept
	{
		return within_tolerance(x, y, tolerance);
	}

	template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_box(const vec_interface<W1, T, C, D1> &x,
											const vec_interface<W2, T, C, D2> &y,
											U tolerance) noexcept
	{
		if constexpr (C == 1)
		{
			return within_box(x[0], y[0], tolerance);
		}
		else
		{
			return all(within_tolerance(x, y, tolerance));
		}
	}

	template <bool W1, floating_point_scalar T, std::size_t C1, typename D1, bool W2, typename D2, bool W3, floating_point_scalar U, std::size_t C2, typename D3>
	requires ((C1 == C2) || (C2 == 1)) && implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_box(const vec_interface<W1, T, C1, D1> &x,
											const vec_interface<W2, T, C1, D2> &y,
											const vec_interface<W3, U, C2, D3> &tolerance) noexcept
	{
		if constexpr (C1 == C2)
		{
			if constexpr (C1 == 1 && C2 == 1)
			{
				return within_box(x[0], y[0], tolerance[0]);
			}
			else
			{
				return all(within_tolerance(x, y, tolerance));
			}
		}
		else		// (C2 == 1)
		{
			return within_box(x, y, tolerance[0]);
		}
	}

	// Tolerance checks for matrices - returns a boolean value indicating whether all the
	// matrix elements are within the tolerance box .

	template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_box(const mat<T, C, R> &x,
											const mat<T, C, R> &y,
											U tolerance) noexcept
	{
		return all(within_tolerance(x, y, tolerance));
	}

	template <bool W, floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U, typename D>
	requires implicitly_convertible_to<U, T>
	[[nodiscard]] constexpr bool within_box(const mat<T, C, R> &x,
											const mat<T, C, R> &y,
											const vec_interface<W, U, C, D> &tolerance) noexcept
	{
		return all(within_tolerance(x, y, tolerance));
	}

}	// namespace dsga

// closing include guard
#endif
