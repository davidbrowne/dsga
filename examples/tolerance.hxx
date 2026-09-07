#pragma once

//          Copyright David Browne 2020-2026.
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

	template <vec_like V, floating_point_scalar U>
	requires implicitly_convertible_to<U, vec_scalar_t<V>> && floating_point_scalar<vec_scalar_t<V>> && floating_point_scalar<U>
	[[nodiscard]] constexpr auto within_tolerance(const V &x,
												  U tolerance) noexcept
	{
		using T = vec_scalar_t<V>;
		constexpr std::size_t C = vec_size_v<V>;

		if constexpr (C == 1)
		{
			return within_tolerance(x[0], tolerance);
		}
		else
		{
			return lessThanEqual(abs(x), vec<T, C>(static_cast<T>(abs(tolerance))));
		}
	}

	template <vec_like V1, vec_like V2, floating_point_scalar U>
	requires same_vec_shape<V1, V2> && implicitly_convertible_to<U, vec_scalar_t<V1>> &&
			 floating_point_scalar<vec_scalar_t<V1>> && floating_point_scalar<U>
	[[nodiscard]] constexpr auto within_tolerance(const V1 &x,
												  const V2 &y,
												  U tolerance) noexcept
	{
		using T = vec_scalar_t<V1>;
		constexpr std::size_t C = vec_size_v<V1>;

		if constexpr (C == 1)
		{
			return within_tolerance(x[0], y[0], tolerance);
		}
		else
		{
			return lessThanEqual(abs(x - y), vec<T, C>(static_cast<T>(abs(tolerance))));
		}
	}

	template <vec_like V1, vec_like V2, floating_point_scalar U>
	requires ((vec_size_v<V1> == vec_size_v<V2>) || (vec_size_v<V2> == 1)) && implicitly_convertible_to<U, vec_scalar_t<V1>> &&
			 floating_point_scalar<vec_scalar_t<V1>> && floating_point_scalar<vec_scalar_t<V2>>
	[[nodiscard]] constexpr auto within_tolerance(const V1 &x,
												  const V2 &tolerance) noexcept
	{
		using T = vec_scalar_t<V1>;
		constexpr std::size_t C1 = vec_size_v<V1>;
		constexpr std::size_t C2 = vec_size_v<V2>;

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

	template <vec_like V1, vec_like V2, vec_like V3>
	requires (vec_size_v<V1> == vec_size_v<V2>) && ((vec_size_v<V1> == vec_size_v<V3>) || (vec_size_v<V3> == 1)) &&
			 std::same_as<vec_scalar_t<V1>, vec_scalar_t<V2>> && std::same_as< vec_scalar_t<V1>, vec_scalar_t<V3>> &&
			 floating_point_scalar<vec_scalar_t<V1>>
	[[nodiscard]] constexpr auto within_tolerance(const V1 &x,
												  const V2 &y,
												  const V3 &tolerance) noexcept
	{
		constexpr std::size_t C1 = vec_size_v<V1>;
		constexpr std::size_t C2 = vec_size_v<V3>;

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

	template <vec_like V, floating_point_scalar T, std::size_t C, std::size_t R>
	requires implicitly_convertible_to<vec_scalar_t<V>, T> && floating_point_scalar<vec_scalar_t<V>> &&	(C == vec_size_v<V>)
	[[nodiscard]] constexpr vec<bool, C> within_tolerance(const mat<T, C, R> &arg,
														  const V &tolerance) noexcept
	{
		return [&arg, &tolerance] <std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
		{
			return vec<bool, C>(all(within_tolerance(arg[Is], tolerance[Is])) ...);
		}(std::make_index_sequence<C>{});
	}

	template <vec_like V, floating_point_scalar T, std::size_t C, std::size_t R>
	requires implicitly_convertible_to<vec_scalar_t<V>, T> && floating_point_scalar<vec_scalar_t<V>> && (C == vec_size_v<V>)
	[[nodiscard]] constexpr vec<bool, C> within_tolerance(const mat<T, C, R> &x,
														  const mat<T, C, R> &y,
														  const V &tolerance) noexcept
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

	template <vec_like V1, vec_like V2, vec_like V3>
	requires same_vec_shape<V1, V2> && implicitly_convertible_to<vec_scalar_t<V3>, vec_scalar_t<V1>> && (vec_size_v<V3> == 1) &&
			 floating_point_scalar<vec_scalar_t<V1>> && floating_point_scalar<vec_scalar_t<V3>>
	[[nodiscard]] constexpr bool within_distance(const V1 &x,
												 const V2 &y,
												 const V3 &tolerance) noexcept
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

	template <vec_like V1, vec_like V2, floating_point_scalar U>
	requires same_vec_shape<V1, V2> && implicitly_convertible_to<U, vec_scalar_t<V1>> && floating_point_scalar<vec_scalar_t<V1>>
	[[nodiscard]] constexpr bool within_box(const V1 &x,
											const V2 &y,
											U tolerance) noexcept
	{
		constexpr std::size_t C = vec_size_v<V1>;

		if constexpr (C == 1)
		{
			return within_box(x[0], y[0], tolerance);
		}
		else
		{
			return all(within_tolerance(x, y, tolerance));
		}
	}

	template <vec_like V1, vec_like V2, vec_like V3>
	requires same_vec_shape<V1, V2> && ((vec_size_v<V1> == vec_size_v<V3>) || (vec_size_v<V3> == 1)) &&
			 implicitly_convertible_to<vec_scalar_t<V3>, vec_scalar_t<V1>> &&
			 floating_point_scalar<vec_scalar_t<V1>> &&floating_point_scalar<vec_scalar_t<V3>>
	[[nodiscard]] constexpr bool within_box(const V1 &x,
											const V2 &y,
											const V3 &tolerance) noexcept
	{
		constexpr std::size_t C1 = vec_size_v<V1>;
		constexpr std::size_t C2 = vec_size_v<V3>;

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

	template <vec_like V, floating_point_scalar T, std::size_t C, std::size_t R>
	requires implicitly_convertible_to<vec_scalar_t<V>, T> && floating_point_scalar<vec_scalar_t<V>> && (C == vec_size_v<V>)
	[[nodiscard]] constexpr bool within_box(const mat<T, C, R> &x,
											const mat<T, C, R> &y,
											const V &tolerance) noexcept
	{
		return all(within_tolerance(x, y, tolerance));
	}

}	// namespace dsga
