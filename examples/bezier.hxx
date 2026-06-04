
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
	template <bool W, dsga::floating_point_scalar T, typename D>
	constexpr T quadratic_recursive_bezier_ordinate_eval(const dsga::vec_interface<W, T, 3, D> &control_points, T t) noexcept
	{
		// not sure of real type of control_points, so make a vec so we can swizzle
		auto quadratic_control_points = dsga::vec<T, 3>(control_points);

		auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
		return dsga::mix(linear_control_points.x, linear_control_points.y, t);
	}
}

// given 3 control points and a t value (hopefully in the [0, 1] interval), evaluate the quadratic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
requires (C > 1)
constexpr dsga::vec<T, C> quadratic_recursive_bezier_eval(const dsga::vec_interface<W1, T, C, D1> &p0,
																   const dsga::vec_interface<W2, T, C, D2> &p1,
																   const dsga::vec_interface<W3, T, C, D3> &p2,
																   T t) noexcept
{
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
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr T quadratic_recursive_bezier_eval(const dsga::vec_interface<W1, T, 1, D1> &p0,
											const dsga::vec_interface<W2, T, 1, D2> &p1,
											const dsga::vec_interface<W3, T, 1, D3> &p2,
											T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return detail::quadratic_recursive_bezier_ordinate_eval(dsga::vec(p0[0], p1[0], p2[0]), t);
}

namespace detail
{
	// Bernstein basis polynomial algorithm
	template <bool W, dsga::floating_point_scalar T, typename D>
	constexpr T quadratic_polynomial_bezier_ordinate_eval(const dsga::vec_interface<W, T, 3, D> &control_points, T t) noexcept
	{
		auto t_complement = T(1) - t;

		return
			t_complement * t_complement * control_points[0] +
			T(2) * t * t_complement * control_points[1] +
			t * t * control_points[2];
	}
}

// given 3 control points and a t value (hopefully in the [0, 1] interval), evaluate the quadratic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
requires (C > 1)
constexpr dsga::vec<T, C> quadratic_polynomial_bezier_eval(const dsga::vec_interface<W1, T, C, D1> &p0,
																	const dsga::vec_interface<W2, T, C, D2> &p1,
																	const dsga::vec_interface<W3, T, C, D3> &p2,
																	T t) noexcept
{
	// a matrix will make it easier to get all the ordinate values in the rows
	dsga::mat<T, 3, C> coord_matrix(p0, p1, p2);

	// lambda pack wrapper
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::vec(detail::quadratic_polynomial_bezier_ordinate_eval(dsga::vec(p0[Is], p1[Is], p2[Is]), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr T quadratic_polynomial_bezier_eval(const dsga::vec_interface<W1, T, 1, D1> &p0,
											 const dsga::vec_interface<W2, T, 1, D2> &p1,
											 const dsga::vec_interface<W3, T, 1, D3> &p2,
											 T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return detail::quadratic_polynomial_bezier_ordinate_eval(dsga::vec(p0[0], p1[0], p2[0]), t);
}

//
// cubic bezier evaluator
//

namespace detail
{
	// recursive interpolation approach (de Casteljau's algorithm) to show off vector functions and swizzling
	template <bool W, dsga::floating_point_scalar T, typename D>
	constexpr T cubic_recursive_bezier_ordinate_eval(const dsga::vec_interface<W, T, 4, D> &control_points, T t) noexcept
	{
		// not sure of real type of control_points, so make a vec so we can swizzle
		auto cubic_control_points = dsga::vec<T, 4>(control_points);

		auto quadratic_control_points = dsga::mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
		auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
		return dsga::mix(linear_control_points.x, linear_control_points.y, t);
	}
}

// given 4 control points and a t value (hopefully in the [0, 1] interval), evaluate the cubic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2,
	bool W3, typename D3, bool W4, typename D4>
requires (C > 1)
constexpr dsga::vec<T, C> cubic_recursive_bezier_eval(const dsga::vec_interface<W1, T, C, D1> &p0,
															   const dsga::vec_interface<W2, T, C, D2> &p1,
															   const dsga::vec_interface<W3, T, C, D3> &p2,
															   const dsga::vec_interface<W4, T, C, D4> &p3,
															   T t) noexcept
{
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
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr T cubic_recursive_bezier_eval(const dsga::vec_interface<W1, T, 1, D1> &p0,
										const dsga::vec_interface<W2, T, 1, D2> &p1,
										const dsga::vec_interface<W3, T, 1, D3> &p2,
										const dsga::vec_interface<W4, T, 1, D4> &p3,
										T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return detail::cubic_recursive_bezier_ordinate_eval(dsga::vec(p0[0], p1[0], p2[0], p3[0]), t);
}

namespace detail
{
	// Bernstein basis polynomial algorithm
	template <bool W, dsga::floating_point_scalar T, typename D>
	constexpr T cubic_polynomial_bezier_ordinate_eval(const dsga::vec_interface<W, T, 4, D> &control_points, T t) noexcept
	{
		auto t_complement = T(1) - t;

		return
			t_complement * t_complement * t_complement * control_points[0] +
			T(3) * t * t_complement * t_complement * control_points[1] +
			T(3) * t * t * t_complement * control_points[2] +
			t * t * t * control_points[3];
	}
}

// given 4 control points and a t value (hopefully in the [0, 1] interval), evaluate the cubic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2,
	bool W3, typename D3, bool W4, typename D4>
requires (C > 1)
constexpr dsga::vec<T, C> cubic_polynomial_bezier_eval(const dsga::vec_interface<W1, T, C, D1> &p0,
																const dsga::vec_interface<W2, T, C, D2> &p1,
																const dsga::vec_interface<W3, T, C, D3> &p2,
																const dsga::vec_interface<W4, T, C, D4> &p3,
																T t) noexcept
{
	// lambda pack wrapper
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::vec(detail::cubic_polynomial_bezier_ordinate_eval(dsga::vec(p0[Is], p1[Is], p2[Is], p3[Is]), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr T cubic_polynomial_bezier_eval(const dsga::vec_interface<W1, T, 1, D1> &p0,
										 const dsga::vec_interface<W2, T, 1, D2> &p1,
										 const dsga::vec_interface<W3, T, 1, D3> &p2,
										 const dsga::vec_interface<W4, T, 1, D4> &p3,
										 T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return detail::cubic_polynomial_bezier_ordinate_eval(dsga::vec(p0[0], p1[0], p2[0], p3[0]), t);
}
