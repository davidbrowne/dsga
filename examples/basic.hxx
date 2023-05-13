
//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

// get a 2D vector that is perpendicular (rotated 90 degrees counter-clockwise)
// to a 2D vector in the plane
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular1(const dsga::basic_vector<T, 2> &some_vec) noexcept
{
	auto cos90 = 0.0f;
	auto sin90 = 1.0f;

	// rotation matrix -- components in column major order
	return dsga::basic_matrix<T, 2, 2>(cos90, sin90, -sin90, cos90) * some_vec;
}

// same as above, different implementation
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular2(const dsga::basic_vector<T, 2> &some_vec) noexcept
{
	return dsga::basic_vector<T, 2>(-1, 1) * some_vec.yx;
}

// if p1 == p2 == p3, then there is a singularity -- we will have 0/0 problem, when real answer should be p1 or p2 or p3.
// the return value c is the center point of a circle inscribed in a triangle represented by the vertices p1, p2, and p3.
// a line segment from c to any of the vertices bisects the angles at the vertices.
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr dsga::basic_vector<T, 3u> triangle_incenter(const dsga::vector_base<W1, T, 3u, D1> &p1,
													  const dsga::vector_base<W2, T, 3u, D2> &p2,
													  const dsga::vector_base<W3, T, 3u, D3> &p3)
{
	auto mag1 = dsga::distance(p2, p3);
	auto mag2 = dsga::distance(p3, p1);
	auto mag3 = dsga::distance(p1, p2);

	return (p1 * mag1 + p2 * mag2 + p3 * mag3) / (mag1 + mag2 + mag3);
}

// the return value c is the center point of the biggest sphere inscribed in a tetrahedron represented by the vertices p1,
// p2, p3, and the implicit origin. c is equidistant from the four planes of the triangle faces of the tetrahedron.
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr dsga::basic_vector<T, 3u> tetrahedron_incenter(const dsga::vector_base<W1, T, 3u, D1> &p1,
														 const dsga::vector_base<W2, T, 3u, D2> &p2,
														 const dsga::vector_base<W3, T, 3u, D3> &p3)
{
	auto mag1 = dsga::length(dsga::cross_matrix(p2) * p3);
	auto mag2 = dsga::length(dsga::cross_matrix(p3) * p1);
	auto mag3 = dsga::length(dsga::cross_matrix(p1) * p2);
	auto mag4 = dsga::length(dsga::cross_matrix(p2 - p1) * (p3 - p1));

	return (p1 * mag1 + p2 * mag2 + p3 * mag3) / (mag1 + mag2 + mag3 + mag4);
}

// find center of circle that goes through the three points
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr auto three_point_circle_center(const dsga::vector_base<W1, T, 3u, D1> &p1,
										 const dsga::vector_base<W2, T, 3u, D2> &p2,
										 const dsga::vector_base<W3, T, 3u, D3> &p3) noexcept
{
	auto v = p2 - p1;
	auto u = dsga::basic_vector<T, 3u>(p2);
	auto w = p3 - p2;

	//auto u = p1;
	//auto v = p2;
	//auto w = p3;

	auto cross_term = dsga::cross_matrix(v) * w;

	return u + T(0.5) * (dsga::dot(v, v) * dsga::outerProduct(w, w) - dsga::dot(w, w) * dsga::outerProduct(v, v)) * (v + w) / dsga::dot(cross_term, cross_term);
}

// find radius of circle that goes through the three points
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr auto three_point_circle_radius(const dsga::vector_base<W1, T, 3u, D1> &p1,
										 const dsga::vector_base<W2, T, 3u, D2> &p2,
										 const dsga::vector_base<W3, T, 3u, D3> &p3) noexcept
{
	auto v = p2 - p1;
	[[maybe_unused]] auto u = dsga::basic_vector<T, 3u>(p2);
	auto w = p3 - p2;

	//auto u = p1;
	//auto v = p2;
	//auto w = p3;

	auto cross_term = dsga::cross_matrix(v) * w;

	return T(0.5) * dsga::length(v) * dsga::length(w) * dsga::length(v + w) / dsga::length(cross_term);
}

// gives closest projection point from point to a line made from line segment p1 <=> p2
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr auto project_to_line1(const dsga::vector_base<W1, T, 3u, D1> &point,
								const dsga::vector_base<W2, T, 3u, D2> &p1,
								const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	auto hyp = point - p1;
	auto v1 = p2 - p1;
	auto t = dsga::dot(hyp, v1) / dsga::dot(v1, v1);

	return p1 + (t * v1);
}

// same as above, different implementation
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr auto project_to_line2(const dsga::vector_base<W1, T, 3u, D1> &point,
								const dsga::vector_base<W2, T, 3u, D2> &p1,
								const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	auto hyp = point - p1;
	auto v1 = p2 - p1;
	return p1 + dsga::outerProduct(v1, v1) * hyp / dsga::dot(v1, v1);
}

// same as above, different implementation, paying more attention to attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr auto project_to_line3(const dsga::vector_base<W1, T, 3u, D1> &point,
								const dsga::vector_base<W2, T, 3u, D2> &p1,
								const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	auto hyp1 = point - p1;
	auto v1 = p2 - p1;
	auto hyp2 = point - p2;
	auto v2 = p1 - p2;

	auto hyp = hyp1;
	auto v = v1;
	auto u = dsga::basic_vector(p1);

	if (dsga::dot(hyp1, hyp1) > dsga::dot(hyp2, hyp2))
	{
		hyp = hyp2;
		v = v2;
		u = dsga::basic_vector(p2);
	}

	return u + dsga::outerProduct(v, v) * hyp / dsga::dot(v, v);
}

// gives minimum distance from point to a line made from line segment p1 <=> p2
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr T distance_to_line(const dsga::vector_base<W1, T, 3u, D1> &point,
							 const dsga::vector_base<W2, T, 3u, D2> &p1,
							 const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	auto hyp = point - p1;
	auto v1 = p2 - p1;
	auto t = dsga::dot(hyp, v1) / dsga::dot(v1, v1);

	return dsga::length(hyp - (t * v1));
}

// project a point in 3D space to the closest point on a plane, where plane defined by 3 CCW points
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr auto project_to_plane1(const dsga::vector_base<W1, T, 3u, D1> &point,
								 const dsga::vector_base<W2, T, 3u, D2> &p1,
								 const dsga::vector_base<W3, T, 3u, D3> &p2,
								 const dsga::vector_base<W4, T, 3u, D4> &p3) noexcept
{
	auto p = [](const auto &u, auto &v, const auto &w) { return dsga::cross_matrix(v - u) * (w - u); };
	auto p_val = p(p1, p2, p3);
	auto p_cross = dsga::cross_matrix(p_val);

	return p1 - p_cross * p_cross * (point - p1) / dsga::dot(p_val, p_val);
}

// same as above, different implementation
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr auto project_to_plane2(const dsga::vector_base<W1, T, 3u, D1> &point,
								 const dsga::vector_base<W2, T, 3u, D2> &p1,
								 const dsga::vector_base<W3, T, 3u, D3> &p2,
								 const dsga::vector_base<W4, T, 3u, D4> &p3) noexcept
{
	auto triangle_norm = [](const auto &u, auto &v, const auto &w) { return dsga::cross_matrix(v - u) * (w - u); };
	auto N = triangle_norm(p1, p2, p3);
	auto d = dsga::dot(N, p1);

	return point - ((dsga::dot(N, point) - d) / dsga::dot(N, N)) * N;
}

#if ATTENUATE_ROUNDOFF
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr auto project_to_plane(const dsga::vector_base<W1, T, 3u, D1> &point,
								const dsga::vector_base<W2, T, 3u, D2> &p1,
								const dsga::vector_base<W3, T, 3u, D3> &p2,
								const dsga::vector_base<W4, T, 3u, D4> &p3) noexcept
{
	auto p = [](const auto &u, auto &v, const auto &w) { return dsga::cross_matrix(v - u) * (w - u); };
	auto delta_u = p3 - p2;
	auto delta_v = p1 - p3;
	auto delta_w = p2 - p1;

	auto md2u = dsga::dot(delta_u, delta_u);
	auto md2v = dsga::dot(delta_v, delta_v);
	auto md2w = dsga::dot(delta_w, delta_w);
	dsga::basic_vector<T, 3u> p_val{0};

	// for p_val, choose u that maximizes (v - w).length()
	if (md2u > md2v)
	{
		if (md2u > md2w)
			p_val = p(p1, p2, p3);
		else
			p_val = p(p3, p1, p2);
	}
	else
	{
		if (md2v > md2w)
			p_val = p(p2, p3, p1);
		else
			p_val = p(p3, p1, p2);
	}

	auto p_cross = dsga::cross_matrix(p_val);

	dsga::basic_vector<T, 3u> anchor{0};
	auto m2u = dsga::dot(p1, p1);
	auto m2v = dsga::dot(p2, p2);
	auto m2w = dsga::dot(p3, p3);

	// for anchor point, choose u that minimizes u.length()
	if (m2u < m2v)
	{
		if (m2u < m2w)
			anchor = p1;
		else
			anchor = p3;
	}
	else
	{
		if (m2u < m2v)
			anchor = p1;
		else
			anchor = p2;
	}

	return anchor - p_cross * p_cross * (point - anchor) / dsga::dot(p_val, p_val);
}
#endif
