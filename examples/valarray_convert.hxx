
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <valarray>

// fill vector from valarray

template <dsga::dimensional_scalar T, std::size_t S, typename U>
requires std::convertible_to<U, T>
void copy_to_vector(dsga::vec<T, S> &lhs, std::valarray<U> rhs)
{
	const std::size_t count = std::min(S, rhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

// fill valarray from vector

template <dsga::dimensional_scalar T, std::size_t S, typename U>
requires std::convertible_to<T, U>
void copy_from_vector(std::valarray<U> &lhs, const dsga::vec<T, S> &rhs)
{
	const std::size_t count = std::min(S, lhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

// create a valarray from a vector

template <dsga::dimensional_scalar T, std::size_t S>
std::valarray<T> to_valarray(const dsga::vec<T, S> &v)
{
	return[&]<std::size_t ...Is>(std::index_sequence<Is...>)
	{
		return std::valarray<T>{v[Is] ...};
	}(std::make_index_sequence<S>());
}
