
//          Copyright David Browne 2020-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// find the minimum positive angle between 2 vectors and/or indexed vectors (swizzles).
// 2D or 3D only.
// 
// see https://people.eecs.berkeley.edu/~wkahan/Mindless.pdf , section 12:Mangled Angles
//
// angle(x, y) := 2 arctan( ||x*||y|| - ||x||*y||/||x*||y|| + ||x||*y|| )
//

template <bool W1, dsga::floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C == 2) || (C == 3))
auto angle_between(const dsga::vector_base<W1, T, C, D1> &v1,
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

	if (numerator == T(0))							// no angle between vectors
		return T(0);
	else if (denominator == T(0))					// no length of sum of scaled vectors
		return std::numbers::pi_v<T>;

	// numerator / denominator == tan(angle/2)
	// atan(numerator / denominator) == angle/2
	// angle == 2 * atan(numerator / denominator)
	return T(2) * std::atan(numerator / denominator);
}
