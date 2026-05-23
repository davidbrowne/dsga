#pragma once

//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// some of these functions come from Section 12 and 17 in the paper:
// https://people.eecs.berkeley.edu/~wkahan/MathH110/Cross.pdf
//

// get a 2D vector that is perpendicular (rotated 90 degrees counter-clockwise)
// to a 2D vector in the plane
template <dsga::floating_point_scalar T>
constexpr dsga::basic_vector<T, 2> get_perpendicular1(const dsga::basic_vector<T, 2> &some_vec) noexcept
{
	auto cos90 = 0.0f;
	auto sin90 = 1.0f;

	// rotation matrix -- components in column major order
	return dsga::basic_matrix<T, 2, 2>(cos90, sin90, -sin90, cos90) * some_vec;
}

// same as above, different implementation
template <dsga::floating_point_scalar T>
constexpr dsga::basic_vector<T, 2> get_perpendicular2(const dsga::basic_vector<T, 2> &some_vec) noexcept
{
	return dsga::basic_vector<T, 2>(-1, 1) * some_vec.yx;
}

// if p1 == p2 == p3, then there is a singularity -- we will have 0/0 problem, when real answer should be p1 or p2 or p3.
// the return value c is the center point of a circle inscribed in a triangle represented by the vertices p1, p2, and p3.
// a line segment from c to any of the vertices bisects the angles at the vertices.
// From Section 17, #9 in the paper
template <bool W1, dsga::floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr dsga::basic_vector<T, C> triangle_incenter(const dsga::vector_base<W1, T, C, D1> &p1,
													 const dsga::vector_base<W2, T, C, D2> &p2,
													 const dsga::vector_base<W3, T, C, D3> &p3) noexcept
{
	auto mag1 = dsga::distance(p2, p3);
	auto mag2 = dsga::distance(p3, p1);
	auto mag3 = dsga::distance(p1, p2);

	return (p1 * mag1 + p2 * mag2 + p3 * mag3) / (mag1 + mag2 + mag3);
}

// the return value c is the center point of the biggest sphere inscribed in a tetrahedron represented by the vertices p1,
// p2, p3, and the implicit origin. c is equidistant from the four planes of the triangle faces of the tetrahedron.
// From Section 17, #10 in the paper
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
// From #8 in the paper
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr dsga::basic_vector<T, 3> three_point_circle_center(const dsga::vector_base<W1, T, 3u, D1> &p1,
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
// From #8 in the paper
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr T three_point_circle_radius(const dsga::vector_base<W1, T, 3u, D1> &p1,
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

// helper function for projecting a point onto a line, paying attention to attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, typename L>
constexpr dsga::basic_vector<T, 3> project_to_line_helper(const L projection,
														  const dsga::vector_base<W1, T, 3u, D1> &point,
														  const dsga::vector_base<W2, T, 3u, D2> &p1,
														  const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	// line is zero length, so just return one of the endpoints
	if (p1 == p2)
		return p1;

	// use the endpoint that is closer to the point as the start of the projection, to attenuate roundoff
	auto hyp1 = point - p1;
	auto hyp2 = point - p2;
	if (dsga::dot(hyp1, hyp1) > dsga::dot(hyp2, hyp2))
	{
		// point is closer to p2
		return projection(hyp2, p2, p1);
	}
	else
	{
		// point is closer to p1
		return projection(hyp1, p1, p2);
	}
}

// gives closest projection point from point to a line made from line segment p1 <=> p2, paying attention to attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr dsga::basic_vector<T, 3> project_to_line1(const dsga::vector_base<W1, T, 3u, D1> &point,
													const dsga::vector_base<W2, T, 3u, D2> &p1,
													const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	constexpr auto projection = [](const auto &hyp, const auto &start, const auto &end) noexcept -> dsga::basic_vector<T, 3>
	{
		auto v = end - start;
		auto t = dsga::dot(hyp, v) / dsga::dot(v, v);
		return start + (t * v);
	};

	return project_to_line_helper(projection, point, p1, p2);
}

// alternate implementation of project_to_line()
// From Section 9, #4 and #6 in the paper, paying attention to attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr dsga::basic_vector<T, 3> project_to_line2(const dsga::vector_base<W1, T, 3u, D1> &point,
													const dsga::vector_base<W2, T, 3u, D2> &p1,
													const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	//Section 9, #4 in the paper
	constexpr auto projection = [](const auto &hyp, const auto &start, const auto &end) noexcept -> dsga::basic_vector<T, 3>
	{
		auto v = end - start;
		return start + dsga::outerProduct(v, v) * hyp / dsga::dot(v, v);
	};

	return project_to_line_helper(projection, point, p1, p2);
}

// gives minimum distance from point to a line made from line segment p1 <=> p2
// From simple vector addition/subtraction (see project_to_line1())
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr T distance_to_line1(const dsga::vector_base<W1, T, 3u, D1> &point,
							  const dsga::vector_base<W2, T, 3u, D2> &p1,
							  const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	auto hyp = point - p1;
	auto v1 = p2 - p1;
	auto t = dsga::dot(hyp, v1) / dsga::dot(v1, v1);

	return dsga::length(hyp - (t * v1));
}

// gives minimum distance from point to a line made from line segment p1 <=> p2, paying attention to attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3>
constexpr T distance_to_line2(const dsga::vector_base<W1, T, 3u, D1> &point,
							  const dsga::vector_base<W2, T, 3u, D2> &p1,
							  const dsga::vector_base<W3, T, 3u, D3> &p2) noexcept
{
	auto hyp1 = point - p1;
	auto hyp2 = point - p2;
	auto v1 = p2 - p1;
	auto v2 = p1 - p2;

	// start values for p1-based approached
	auto hyp = hyp1;
	auto v = v1;

	// if length of hyp2 is less than length of hyp1, do a p2-based approach
	if (dsga::dot(hyp1, hyp1) > dsga::dot(hyp2, hyp2))
	{
		hyp = hyp2;
		v = v2;
	}
	auto t = dsga::dot(hyp, v) / dsga::dot(v, v);

	return dsga::length(hyp - (t * v));
}

// helper function for projecting a point to a plane, paying attention to attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4, typename L>
constexpr dsga::basic_vector<T, 3> project_to_plane_helper(const L plane_projection,
														   const dsga::vector_base<W1, T, 3u, D1> &point,
														   const dsga::vector_base<W2, T, 3u, D2> &p1,
														   const dsga::vector_base<W3, T, 3u, D3> &p2,
														   const dsga::vector_base<W4, T, 3u, D4> &p3) noexcept
{
	auto v1 = p2 - p1;
	auto v2 = p3 - p2;
	auto v3 = p1 - p3;

	auto dist1 = dsga::dot(v1, v1);
	auto dist2 = dsga::dot(v2, v2);
	auto dist3 = dsga::dot(v3, v3);

	if (dist1 > dist2)
	{
		if (dist1 > dist3)
			return plane_projection(point, p1, p2, p3);
		else
			return plane_projection(point, p3, p1, p2);
	}
	else
	{
		if (dist2 > dist3)
			return plane_projection(point, p2, p3, p1);
		else
			return plane_projection(point, p3, p1, p2);
	}
}

// project a point in 3D space to the closest point on a plane, where plane defined by 3 CCW points, attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr dsga::basic_vector<T, 3> project_to_plane1(const dsga::vector_base<W1, T, 3u, D1> &point,
													 const dsga::vector_base<W2, T, 3u, D2> &p1,
													 const dsga::vector_base<W3, T, 3u, D3> &p2,
													 const dsga::vector_base<W4, T, 3u, D4> &p3) noexcept
{
	constexpr auto plane_projection = [](const auto &pt, const auto &pt1, const auto &pt2, const auto &pt3) noexcept
	{
		constexpr auto triangle_norm = [](const auto &u, const auto &v, const auto &w) noexcept { return dsga::cross((v - u), (w - u)); };
		auto N = triangle_norm(pt1, pt2, pt3);
		auto d = dsga::dot(N, pt1);
		return pt - ((dsga::dot(N, pt) - d) / dsga::dot(N, N)) * N;
	};

	return project_to_plane_helper(plane_projection, point, p1, p2, p3);
}

// project a point in 3D space to the closest point on a plane, where plane defined by 3 CCW points
// From #3 in the paper, attenuating roundoff
template <bool W1, dsga::floating_point_scalar T, typename D1, bool W2, typename D2, bool W3, typename D3, bool W4, typename D4>
constexpr dsga::basic_vector<T, 3> project_to_plane2(const dsga::vector_base<W1, T, 3u, D1> &point,
													 const dsga::vector_base<W2, T, 3u, D2> &p1,
													 const dsga::vector_base<W3, T, 3u, D3> &p2,
													 const dsga::vector_base<W4, T, 3u, D4> &p3) noexcept
{
	constexpr auto plane_projection = [](const auto &pt, const auto &pt1, const auto &pt2, const auto &pt3) noexcept
	{
		constexpr auto triangle_norm = [](const auto &u, const auto &v, const auto &w) noexcept { return dsga::cross((v - u), (w - u)); };
		auto p_val = triangle_norm(pt1, pt2, pt3);
		auto p_cross = dsga::cross_matrix(p_val);

		return pt1 - p_cross * p_cross * (pt - pt1) / dsga::dot(p_val, p_val);
	};

	return project_to_plane_helper(plane_projection, point, p1, p2, p3);
}
