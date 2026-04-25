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

template <bool W1, dsga::floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C == 2) || (C == 3))
T angle_between(const dsga::vector_base<W1, T, C, D1> &v1,
				const dsga::vector_base<W2, T, C, D2> &v2) noexcept
{
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
	return T(2) * std::atan2(numerator, denominator);
}

// using acos() for angle (in radians) is reportedly not as good as using atan() above in angle_between().
// acos() is pretty stable when input is near 0, but not great when close to 1 or -1
template <bool W1, dsga::floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C == 2) || (C == 3))
T vect_angle(const dsga::vector_base<W1, T, C, D1> &v1,
			 const dsga::vector_base<W2, T, C, D2> &v2) noexcept
{
	constexpr double tolerance = 1.25e-13;
	auto length_prod = (dsga::length(v1) * dsga::length(v2));

	// this is to prevent dividing by a number close to zero.
	if (length_prod <= tolerance)
		return 0.0;

	// from dot_product == length(v1) * length(v2) * cos(angle)
	// cos(angle) == dot_product / (length(v1) * length(v2))
	// angle == acos(cos(angle)) == acos(dot_product / (length(v1) * length(v2)))
	auto cos_value = dsga::clamp((dsga::dot(v1, v2) / length_prod), -1., 1.);
	return std::acos(cos_value);
}
