
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// quadratic bezier evaluator
//

// recursive interpolation approach
template <bool W, dsga::floating_point_dimensional_scalar T, typename D>
constexpr auto quadratic_bezier_ordinate_eval(const dsga::vector_base<W, T, 3u, D> &control_points, T t) noexcept
{
	// so we can swizzle
	auto quadratic_control_points = dsga::basic_vector<T, 3u>(control_points);

	auto linear_control_points = mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
	return mix(linear_control_points.x, linear_control_points.y, t);
}

template <bool W1, dsga::floating_point_dimensional_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3>
requires (C > 1u)
constexpr auto quadratic_bezier_eval(const dsga::vector_base<W1, T, C, D1> &p0,
									 const dsga::vector_base<W2, T, C, D2> &p1,
									 const dsga::vector_base<W3, T, C, D3> &p2,
									 T t) noexcept
{
	dsga::basic_matrix<T, 4, C> coord_matrix(p0, p1, p2);

	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		return dsga::basic_vector<T, C>(quadratic_bezier_ordinate_eval(coord_matrix.template row<Is>(), t)...);
	}(std::make_index_sequence<C>{});
}

template <bool W1, dsga::floating_point_dimensional_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3>
constexpr auto quadratic_bezier_eval(const dsga::vector_base<W1, T, 1u, D1> &p0,
									 const dsga::vector_base<W2, T, 1u, D2> &p1,
									 const dsga::vector_base<W3, T, 1u, D3> &p2,
									 T t) noexcept
{
	return quadratic_bezier_ordinate_eval(dsga::basic_vector<T, 4u>(p0, p1, p2), t);
}

//
// cubic bezier evaluator
//

// recursive interpolation approach
template <bool W, dsga::floating_point_dimensional_scalar T, typename D>
constexpr auto cubic_bezier_ordinate_eval(const dsga::vector_base<W, T, 4u, D> &control_points, T t) noexcept
{
	// so we can swizzle
	auto cubic_control_points = dsga::basic_vector<T, 4u>(control_points);

	auto quadratic_control_points = mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
	auto linear_control_points = mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
	return mix(linear_control_points.x, linear_control_points.y, t);
}

template <bool W1, dsga::floating_point_dimensional_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
requires (C > 1u)
constexpr auto cubic_bezier_eval(const dsga::vector_base<W1, T, C, D1> &p0,
								 const dsga::vector_base<W2, T, C, D2> &p1,
								 const dsga::vector_base<W3, T, C, D3> &p2,
								 const dsga::vector_base<W4, T, C, D4> &p3,
								 T t) noexcept
{
	auto coord_matrix = dsga::basic_matrix<T, 4, C>(p0, p1, p2, p3);

	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		return dsga::basic_vector<T, C>(cubic_bezier_ordinate_eval(coord_matrix.template row<Is>(), t)...);
	}(std::make_index_sequence<C>{});
}

template <bool W1, dsga::floating_point_dimensional_scalar T, std::size_t C, typename D1,
	bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr auto cubic_bezier_eval(const dsga::vector_base<W1, T, 1u, D1> &p0,
								 const dsga::vector_base<W2, T, 1u, D2> &p1,
								 const dsga::vector_base<W3, T, 1u, D3> &p2,
								 const dsga::vector_base<W4, T, 1u, D4> &p3,
								 T t) noexcept
{
	return cubic_bezier_ordinate_eval(dsga::basic_vector<T, 4u>(p0, p1, p2, p3), t);
}
