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
constexpr dsga::vec<T, 2> get_perpendicular1(const dsga::vec<T, 2> &some_vec) noexcept
{
	constexpr T cos90 = T(0);
	constexpr T sin90 = T(1);
	constexpr auto rot90 = dsga::mat<T, 2, 2>(cos90, sin90, -sin90, cos90);

	// rotation matrix -- components in column major order
	return rot90 * some_vec;
}

// same as above, different implementation
template <dsga::floating_point_scalar T>
constexpr dsga::vec<T, 2> get_perpendicular2(const dsga::vec<T, 2> &some_vec) noexcept
{
	constexpr auto adjust_vec = dsga::vec<T, 2>(-1, 1);
	return adjust_vec * some_vec.yx;
}

// if p1 == p2 == p3, then there is a singularity -- we will have 0/0 problem, when real answer should be p1 or p2 or p3.
// the return value c is the center point of a circle inscribed in a triangle represented by the vertices p1, p2, and p3.
// a line segment from c to any of the vertices bisects the angles at the vertices.
// From Section 17, #9 in the paper
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>>
constexpr dsga::vec<dsga::vec_scalar_t<V1>, dsga::vec_size_v<V1>> triangle_incenter(const V1 &p1,
																					const V2 &p2,
																					const V3 &p3) noexcept
{
	auto mag1 = dsga::distance(p2, p3);
	auto mag2 = dsga::distance(p3, p1);
	auto mag3 = dsga::distance(p1, p2);

	return (p1 * mag1 + p2 * mag2 + p3 * mag3) / (mag1 + mag2 + mag3);
}

// the return value c is the center point of the biggest sphere inscribed in a tetrahedron represented by the vertices p1,
// p2, p3, and the implicit origin. c is equidistant from the four planes of the triangle faces of the tetrahedron.
// From Section 17, #10 in the paper
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> tetrahedron_incenter(const V1 &p1,
																	const V2 &p2,
																	const V3 &p3)
{
	auto mag1 = dsga::length(dsga::cross(p2, p3));
	auto mag2 = dsga::length(dsga::cross(p3, p1));
	auto mag3 = dsga::length(dsga::cross(p1, p2));
	auto mag4 = dsga::length(dsga::cross(p2 - p1, p3 - p1));

	return (p1 * mag1 + p2 * mag2 + p3 * mag3) / (mag1 + mag2 + mag3 + mag4);
}

// find center of circle that goes through the three points
// From #8 in the paper
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> three_point_circle_center(const V1 &p1,
																		 const V2 &p2,
																		 const V3 &p3) noexcept
{
	using T = dsga::vec_scalar_t<V1>;

	auto v = p2 - p1;
	auto u = dsga::vec<T, 3u>(p2);
	auto w = p3 - p2;

	auto cross_term = dsga::cross(v, w);

	return u + T(0.5) * (dsga::dot(v, v) * dsga::outerProduct(w, w) - dsga::dot(w, w) * dsga::outerProduct(v, v)) * (v + w) / dsga::dot(cross_term, cross_term);
}

// find radius of circle that goes through the three points
// From #8 in the paper
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec_scalar_t<V1> three_point_circle_radius(const V1 &p1,
														   const V2 &p2,
														   const V3 &p3) noexcept
{
	using T = dsga::vec_scalar_t<V1>;

	auto v = p2 - p1;
	[[maybe_unused]] auto u = dsga::vec<T, 3u>(p2);
	auto w = p3 - p2;

	auto cross_term = dsga::cross(v, w);

	return T(0.5) * dsga::length(v) * dsga::length(w) * dsga::length(v + w) / dsga::length(cross_term);
}

// helper function for projecting a point onto a line, paying attention to attenuating roundoff
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3, typename L>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> project_to_line_helper(const L projection,
																	  const V1 &point,
																	  const V2 &p1,
																	  const V3 &p2) noexcept
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

// gives closest projection point from point to a line made from line segment p1 <=> p2, paying attention to attenuating roundoff.
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> project_to_line1(const V1 &point,
																const V2 &p1,
																const V3 &p2) noexcept
{
	using T = dsga::vec_scalar_t<V1>;

	constexpr auto projection = [](const auto &hyp, const auto &start, const auto &end) noexcept -> dsga::vec<T, 3>
	{
		auto v = end - start;
		auto t = dsga::dot(hyp, v) / dsga::dot(v, v);
		return start + (t * v);
	};

	return project_to_line_helper(projection, point, p1, p2);
}

// alternate implementation of project_to_line()
// From Section 9, #4 and #6 in the paper, paying attention to attenuating roundoff.
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> project_to_line2(const V1 &point,
																const V2 &p1,
																const V3 &p2) noexcept
{
	using T = dsga::vec_scalar_t<V1>;

	//Section 9, #4 in the paper
	constexpr auto projection = [](const auto &hyp, const auto &start, const auto &end) noexcept -> dsga::vec<T, 3>
	{
		auto v = end - start;
		return start + dsga::outerProduct(v, v) * hyp / dsga::dot(v, v);
	};

	return project_to_line_helper(projection, point, p1, p2);
}

// gives minimum distance from point to a line made from line segment p1 <=> p2
// hopefully attenuating roundoff, similar to project_to_line_helper()
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec_scalar_t<V1> distance_to_line(const V1 &point,
												  const V2 &p1,
												  const V3 &p2) noexcept
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

// helper function for projecting a point to a plane. See Section 9, #3 in the paper,
// for attenuating roundoff in 'z' for the second formula, which is mostly for project_to_plane2()
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3, dsga::vec_like V4, typename L>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> project_to_plane_helper(const L plane_projection,
																	   const V1 &point,
																	   const V2 &p1,
																	   const V3 &p2,
																	   const V4 &p3) noexcept
{
	auto dist1 = dsga::dot(p1, p1);
	auto dist2 = dsga::dot(p2, p2);
	auto dist3 = dsga::dot(p3, p3);

	if (dist1 > dist2)
	{
		if (dist2 > dist3)
			return plane_projection(point, p3, p1, p2);		// p3 is closest to origin, so use p3 as the start of the triangle
		else
			return plane_projection(point, p2, p3, p1);		// p2 is closest to origin, so use p2 as the start of the triangle
	}
	else
	{
		if (dist1 > dist3)
			return plane_projection(point, p3, p1, p2);		// p3 is closest to origin, so use p3 as the start of the triangle
		else
			return plane_projection(point, p1, p2, p3);		// p1 is closest to origin, so use p1 as the start of the triangle
	}
}

// gives scaled normal vector of triangle defined by three points, paying attention to attenuating roundoff.
// HOWEVER, this vector is not normalized
// See Section 9, #3 in the paper (for attenuating roundoff in 'p'), but using cross products instead of cross matrices
constexpr auto triangle_cross(const auto &u, const auto &v, const auto &w) noexcept
{
	auto v1 = v - u;									// no w in this equation, so use w as the point to subtract below
	auto v2 = w - v;									// no u in this equation, so use u as the point to subtract below
	auto v3 = u - w;									// no v in this equation, so use v as the point to subtract below

	auto dist1 = dsga::dot(v1, v1);
	auto dist2 = dsga::dot(v2, v2);
	auto dist3 = dsga::dot(v3, v3);

	// get max of v1, v2, and v3, and use that to choose what to use for u, v, and w in the cross product, to attenuate roundoff
	if (dist1 > dist2)
	{
		if (dist1 > dist3)
			return dsga::cross(v3, -v2);				// v1 is longest, so use w as the point to subtract
		else
			return dsga::cross(v2, -v1);				// v3 is longest, so use v as the point to subtract
	}
	else
	{
		if (dist2 > dist3)
			return dsga::cross(v1, -v3);				// v2 is longest, so use u as the point to subtract
		else
			return dsga::cross(v2, -v1);				// v3 is longest, so use v as the point to subtract
	}
}

// project a point in 3D space to the closest point on a plane, where plane defined by 3 CCW points, attenuating roundoff (?).
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3, dsga::vec_like V4>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> project_to_plane1(const V1 &point,
																 const V2 &p1,
																 const V3 &p2,
																 const V4 &p3) noexcept
{
	constexpr auto plane_projection = [](const auto &pt, const auto &pt1, const auto &pt2, const auto &pt3) noexcept
	{
		// note that pt1 is the point that is closest to the origin. not sure if that matters here.
		// N isn't normalized, but we account for the length in return statement.
		auto N = triangle_cross(pt1, pt2, pt3);
		return pt - ((dsga::dot(N, pt) - dsga::dot(N, pt1)) / dsga::dot(N, N)) * N;
	};

	return project_to_plane_helper(plane_projection, point, p1, p2, p3);
}

// project a point in 3D space to the closest point on a plane, where plane defined by 3 CCW points
// From Section 9, #3 in the paper, attenuating roundoff, except we are using cross products instead of cross matrices
template <dsga::vec_like V1, dsga::vec_like V2, dsga::vec_like V3, dsga::vec_like V4>
requires dsga::floating_point_scalar<dsga::vec_scalar_t<V1>> && (dsga::vec_size_v<V1> == 3)
constexpr dsga::vec<dsga::vec_scalar_t<V1>, 3> project_to_plane2(const V1 &point,
																 const V2 &p1,
																 const V3 &p2,
																 const V4 &p3) noexcept
{
	constexpr auto plane_projection = [](const auto &pt, const auto &pt1, const auto &pt2, const auto &pt3) noexcept
	{
		// note that pt1 is the point that is closest to the origin, via project_to_plane_helper()
		auto p = triangle_cross(pt1, pt2, pt3);

		// use pt1 to attenuate roundoff
		return pt1 - dsga::cross(p, dsga::cross(p, pt - pt1)) / dsga::dot(p, p);
	};

	return project_to_plane_helper(plane_projection, point, p1, p2, p3);
}
