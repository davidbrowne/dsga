
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
using namespace dsga;

//
// find the minimum positive angle between 2 vectors and/or indexed vectors (swizzles).
// 2D or 3D only
//

template <bool W1, floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C == 2u) || (C == 3u))
auto angle_between(const vector_base<W1, T, C, D1> &v1,
				   const vector_base<W2, T, C, D2> &v2) noexcept
{
	auto v1_mag = length(v1);
	auto v2_mag = length(v2);
	auto numerator = length(v1 * v2_mag - v2 * v1_mag);
	auto denominator = length(v1 * v2_mag + v2 * v1_mag);

	if (numerator == T(0))
		return T(0);
	else if (denominator == T(0))
		return std::numbers::pi_v<T>;

	return T(2) * std::atan(numerator / denominator);
}
