
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <iostream>
#include <iomanip>
#include <limits>

//
// iostream interface
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline std::ostream &operator<<(std::ostream &o, const dsga::vec_interface<Writable, T, Count, Derived> &v)
{
	const Derived &derived = v.as_derived();

	// cache the state of the stream before we modify it
	const auto cached_flags = o.flags();
	const auto cached_precision = o.precision();

	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}
	else if constexpr (dsga::floating_point_scalar<T>)
	{
		o << std::setprecision(std::numeric_limits<double>::max_digits10);
	}

	o << "[" << derived[0];
	for (int i = 1; i < derived.length(); ++i)
		o << ", " << derived[i];
	o << "]";

	// restore the state of the stream back to what it originally was
	o.flags(cached_flags);
	o.precision(cached_precision);

	return o;
}

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
inline std::ostream &operator<<(std::ostream &o, const dsga::mat<T, C, R> &m)
{
	o << "[" << m[0];
	for (int i = 1; i < m.length(); ++i)
		o << ", " << m[i];
	o << "]";

	return o;
}
