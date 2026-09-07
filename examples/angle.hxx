#pragma once

//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// find the minimum positive angle (in radians, from 0 to PI) between 2 vector types.
// 2D or 3D only.
// 
// see https://people.eecs.berkeley.edu/~wkahan/Mindless.pdf , section 12:Mangled Angles
//
// angle(x, y) := 2 arctan( ||x*||y|| - ||x||*y||/||x*||y|| + ||x||*y|| )
//

template <dsga::vec_like V1, dsga::vec_like V2>
requires dsga::same_vec_shape<V1, V2> && ((dsga::vec_size_v<V1> == 2) || (dsga::vec_size_v<V1> == 3)) && 
		 dsga::floating_point_scalar<dsga::vec_scalar_t<V1>>
dsga::vec_scalar_t<V1> angle_between(const V1 &v1,
									 const V2 &v2) noexcept
{
	using T = dsga::vec_scalar_t<V1>;

	// scale the vectors to be the same length.
	// this makes the sum and difference of the two vectors intersect perpendicularly,
	// allowing us to use some basic trig to solve.
	auto a = v1 * dsga::length(v2);
	auto b = v2 * dsga::length(v1);

	// numerator / denominator = tan(angle/2)
	auto numerator = dsga::length(a - b);			// difference: length of line perpendicular to denominator, connecting the scaled endpoints
	auto denominator = dsga::length(a + b);			// sum: length of line that is sum of scaled vectors

	// numerator / denominator == tan(angle/2)
	// atan(numerator / denominator) == atan2(numerator, denominator) == angle/2
	// angle == 2 * atan2(numerator, denominator)
	return T(2) * dsga::atan(numerator, denominator);
}
