
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// find the minimum positive angle between 2 vectors and/or indexed vectors (swizzles).
// 2D or 3D only
//

template <bool W1, dsga::floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C == 2) || (C == 3))
auto angle_between(const dsga::vector_base<W1, T, C, D1> &v1,
				   const dsga::vector_base<W2, T, C, D2> &v2) noexcept
{
	auto a = v1 * dsga::length(v2);
	auto b = v2 * dsga::length(v1);
	auto numerator = dsga::length(a - b);
	auto denominator = dsga::length(a + b);

	if (numerator == T(0))
		return T(0);
	else if (denominator == T(0))
		return std::numbers::pi_v<T>;

	return T(2) * std::atan(numerator / denominator);
}
