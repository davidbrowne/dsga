#pragma once

//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// quadratic bezier evaluator
//

namespace detail
{
	// recursive interpolation approach (de Casteljau's algorithm) to show off vector functions and swizzling
	template <dsga::vec_like V>
	requires (dsga::vec_size_v<V> == 3) && dsga::floating_point_scalar<dsga::vec_scalar_t<V>>
	constexpr dsga::vec_scalar_t<V> quadratic_recursive_bezier_ordinate_eval(const V &control_points, dsga::vec_scalar_t<V> t) noexcept
	{
		using T = dsga::vec_scalar_t<V>;

		// not sure of real type of control_points, so make a vec so we can swizzle
		auto quadratic_control_points = dsga::vec<T, 3>(control_points);

		auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
		return dsga::mix(linear_control_points.x, linear_control_points.y, t);
	}
}

// given 3 control points and a t value (hopefully in the [0, 1] interval), evaluate the quadratic bezier function
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::same_vec_shape<V1, V2, V3> && (dsga::vec_size_v<V1> > 1) && dsga::floating_point_scalar<dsga::vec_scalar_t<V1>>
constexpr dsga::vec<dsga::vec_scalar_t<V1>, dsga::vec_size_v<V1>> quadratic_recursive_bezier_eval(const V1 &p0,
																								  const V2 &p1,
																								  const V3 &p2,
																								  dsga::vec_scalar_t<V1> t) noexcept
{
	using T = dsga::vec_scalar_t<V1>;
	constexpr std::size_t C = dsga::vec_size_v<V1>;

	// a matrix will make it easier to get all the ordinate values in the rows
	dsga::mat<T, 3, C> coord_matrix(p0, p1, p2);

	// lambda pack wrapper
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::vec(detail::quadratic_recursive_bezier_ordinate_eval(coord_matrix.row(Is), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::same_vec_shape<V1, V2, V3> && (dsga::vec_size_v<V1> == 1) && dsga::floating_point_scalar<dsga::vec_scalar_t<V1>>
constexpr dsga::vec_scalar_t<V1> quadratic_recursive_bezier_eval(const V1 &p0,
																 const V2 &p1,
																 const V3 &p2,
																 dsga::vec_scalar_t<V1> t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return detail::quadratic_recursive_bezier_ordinate_eval(dsga::vec(p0[0], p1[0], p2[0]), t);
}

//
// cubic bezier evaluator
//

namespace detail
{
	// recursive interpolation approach (de Casteljau's algorithm) to show off vector functions and swizzling
	template <dsga::vec_like V>
	requires (dsga::vec_size_v<V> == 4) && dsga::floating_point_scalar<dsga::vec_scalar_t<V>>
	constexpr dsga::vec_scalar_t<V> cubic_recursive_bezier_ordinate_eval(const V &control_points, dsga::vec_scalar_t<V> t) noexcept
	{
		using T = dsga::vec_scalar_t<V>;

		// not sure of real type of control_points, so make a vec so we can swizzle
		auto cubic_control_points = dsga::vec<T, 4>(control_points);

		auto quadratic_control_points = dsga::mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
		auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
		return dsga::mix(linear_control_points.x, linear_control_points.y, t);
	}
}

// given 4 control points and a t value (hopefully in the [0, 1] interval), evaluate the cubic bezier function
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3, dsga::vec_like V4>
requires dsga::same_vec_shape<V1, V2, V3, V4> && (dsga::vec_size_v<V1> > 1) && dsga::floating_point_scalar<dsga::vec_scalar_t<V1>>
constexpr dsga::vec<dsga::vec_scalar_t<V1>, dsga::vec_size_v<V1>> cubic_recursive_bezier_eval(const V1 &p0,
																							  const V2 &p1,
																							  const V3 &p2,
																							  const V4 &p3,
																							  dsga::vec_scalar_t<V1> t) noexcept
{
	using T = dsga::vec_scalar_t<V1>;
	constexpr std::size_t C = dsga::vec_size_v<V1>;

	// a matrix will make it easier to get all the ordinate values in the rows
	auto coord_matrix = dsga::mat<T, 4, C>(p0, p1, p2, p3);

	// lambda pack wrapper
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::vec(detail::cubic_recursive_bezier_ordinate_eval(coord_matrix.row(Is), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3, dsga::vec_like V4>
requires dsga::same_vec_shape<V1, V2, V3, V4> && (dsga::vec_size_v<V1> == 1) && dsga::floating_point_scalar<dsga::vec_scalar_t<V1>>
constexpr dsga::vec_scalar_t<V1> cubic_recursive_bezier_eval(const V1 &p0,
															 const V2 &p1,
															 const V3 &p2,
															 const V4 &p3,
															 dsga::vec_scalar_t<V1> t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return detail::cubic_recursive_bezier_ordinate_eval(dsga::vec(p0[0], p1[0], p2[0], p3[0]), t);
}
