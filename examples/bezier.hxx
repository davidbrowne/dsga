
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// quadratic bezier evaluator
//

// recursive interpolation approach (de Casteljau algorithm) to show off vector functions and swizzling
template <bool W, dsga::floating_point_scalar T, typename D>
constexpr auto quadratic_bezier_ordinate_eval(const dsga::vector_base<W, T, 3, D> &control_points, T t) noexcept
{
	// not sure of real type of control_points, so make a basic_vector so we can swizzle
	auto quadratic_control_points = dsga::basic_vector<T, 3>(control_points);

	auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
	return dsga::mix(linear_control_points.x, linear_control_points.y, t);
}

// given 3 control points and a t value (hopefully in the [0, 1] interval), evaluate the quadratic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3>
requires (C > 1)
constexpr auto quadratic_bezier_eval(const dsga::vector_base<W1, T, C, D1> &p0,
									 const dsga::vector_base<W2, T, C, D2> &p1,
									 const dsga::vector_base<W3, T, C, D3> &p2,
									 T t) noexcept
{
	// a matrix will make it easier to get all the ordinate values in the rows
	dsga::basic_matrix<T, 3, C> coord_matrix(p0, p1, p2);

	// lambda pack wrapper
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::basic_vector<T, C>(quadratic_bezier_ordinate_eval(coord_matrix.row(Is), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <bool W1, dsga::floating_point_scalar T, typename D1,
	bool W2, typename D2, bool W3, typename D3>
constexpr auto quadratic_bezier_eval(const dsga::vector_base<W1, T, 1, D1> &p0,
									 const dsga::vector_base<W2, T, 1, D2> &p1,
									 const dsga::vector_base<W3, T, 1, D3> &p2,
									 T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return quadratic_bezier_ordinate_eval(dsga::basic_vector<T, 3>(p0[0], p1[0], p2[0]), t);
}

// recursive interpolation approach to show off vector functions and swizzling
template <bool W, dsga::floating_point_scalar T, typename D>
constexpr auto quadratic_bezier_ordinate_eval2(const dsga::vector_base<W, T, 3, D> &control_points, T t) noexcept
{
	auto t_complement = T(1) - t;

	return
		t_complement * t_complement * control_points[0] +
		T(2) * t * t_complement * control_points[1] +
		t * t * control_points[2];
}

// given 3 control points and a t value (hopefully in the [0, 1] interval), evaluate the quadratic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3>
	requires (C > 1)
constexpr auto quadratic_bezier_eval2(const dsga::vector_base<W1, T, C, D1> &p0,
									  const dsga::vector_base<W2, T, C, D2> &p1,
									  const dsga::vector_base<W3, T, C, D3> &p2,
									  T t) noexcept
{
	// a matrix will make it easier to get all the ordinate values in the rows
	dsga::basic_matrix<T, 3, C> coord_matrix(p0, p1, p2);

	// lambda pack wrapper
	return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::basic_vector<T, C>(quadratic_bezier_ordinate_eval2(coord_matrix.row(Is), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <bool W1, dsga::floating_point_scalar T, typename D1,
	bool W2, typename D2, bool W3, typename D3>
constexpr auto quadratic_bezier_eval2(const dsga::vector_base<W1, T, 1, D1> &p0,
									  const dsga::vector_base<W2, T, 1, D2> &p1,
									  const dsga::vector_base<W3, T, 1, D3> &p2,
									  T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return quadratic_bezier_ordinate_eval2(dsga::basic_vector<T, 3>(p0[0], p1[0], p2[0]), t);
}

//
// cubic bezier evaluator
//

// recursive interpolation approach (de Casteljau algorithm) to show off vector functions and swizzling
template <bool W, dsga::floating_point_scalar T, typename D>
constexpr auto cubic_bezier_ordinate_eval(const dsga::vector_base<W, T, 4, D> &control_points, T t) noexcept
{
	// not sure of real type of control_points, so make a basic_vector so we can swizzle
	auto cubic_control_points = dsga::basic_vector<T, 4>(control_points);

	auto quadratic_control_points = dsga::mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
	auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
	return dsga::mix(linear_control_points.x, linear_control_points.y, t);
}

// given 4 control points and a t value (hopefully in the [0, 1] interval), evaluate the cubic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
requires (C > 1)
constexpr auto cubic_bezier_eval(const dsga::vector_base<W1, T, C, D1> &p0,
								 const dsga::vector_base<W2, T, C, D2> &p1,
								 const dsga::vector_base<W3, T, C, D3> &p2,
								 const dsga::vector_base<W4, T, C, D4> &p3,
								 T t) noexcept
{
	// a matrix will make it easier to get all the ordinate values in the rows
	auto coord_matrix = dsga::basic_matrix<T, 4, C>(p0, p1, p2, p3);

	// lambda pack wrapper
	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::basic_vector<T, C>(cubic_bezier_ordinate_eval(coord_matrix.row(Is), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr auto cubic_bezier_eval(const dsga::vector_base<W1, T, 1, D1> &p0,
								 const dsga::vector_base<W2, T, 1, D2> &p1,
								 const dsga::vector_base<W3, T, 1, D3> &p2,
								 const dsga::vector_base<W4, T, 1, D4> &p3,
								 T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return cubic_bezier_ordinate_eval(dsga::basic_vector<T, 4>(p0[0], p1[0], p2[0], p3[0]), t);
}

template <bool W, dsga::floating_point_scalar T, typename D>
constexpr auto cubic_bezier_ordinate_eval2(const dsga::vector_base<W, T, 4, D> &control_points, T t) noexcept
{
	auto t_complement = T(1) - t;

	return
		t_complement * t_complement * t_complement * control_points[0] +
		T(3) * t * t_complement * t_complement * control_points[1] +
		T(3) * t * t * t_complement * control_points[2] +
		t * t * t * control_points[3];
}


// given 4 control points and a t value (hopefully in the [0, 1] interval), evaluate the cubic bezier function
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
	requires (C > 1)
constexpr auto cubic_bezier_eval2(const dsga::vector_base<W1, T, C, D1> &p0,
								  const dsga::vector_base<W2, T, C, D2> &p1,
								  const dsga::vector_base<W3, T, C, D3> &p2,
								  const dsga::vector_base<W4, T, C, D4> &p3,
								  T t) noexcept
{
	// a matrix will make it easier to get all the ordinate values in the rows
	auto coord_matrix = dsga::basic_matrix<T, 4, C>(p0, p1, p2, p3);

	// lambda pack wrapper
	return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		// evaluate the bezier function for each ordinate (i.e., row of control points)
		return dsga::basic_vector<T, C>(cubic_bezier_ordinate_eval2(coord_matrix.row(Is), t)...);
	}(std::make_index_sequence<C>{});
}

// same as previous function, but specializing for length 1 vector case
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr auto cubic_bezier_eval2(const dsga::vector_base<W1, T, 1, D1> &p0,
								  const dsga::vector_base<W2, T, 1, D2> &p1,
								  const dsga::vector_base<W3, T, 1, D3> &p2,
								  const dsga::vector_base<W4, T, 1, D4> &p3,
								  T t) noexcept
{
	// since the arguments are all length 1, we can't create a matrix and do the lambda pack wrapper,
	// so we directly invoke the evaluator for the one ordinate
	return cubic_bezier_ordinate_eval2(dsga::basic_vector<T, 4>(p0[0], p1[0], p2[0], p3[0]), t);
}
