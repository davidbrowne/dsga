
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <functional>


// based on https://www.boost.org/doc/libs/1_83_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
template <typename T>
constexpr std::size_t hash_combine(std::size_t seed, const T &t) noexcept
{
	std::size_t x = seed + 0x9e3779b9 + std::hash<T>{}(t);

	x ^= x >> 32;
	x *= 0xe9846af9b1a615d;
	x ^= x >> 32;
	x *= 0xe9846af9b1a615d;
	x ^= x >> 28;

	return x;
}

template <dsga::dimensional_scalar T, std::size_t S>
struct std::hash<dsga::basic_vector<T, S>>
{
	std::size_t operator()(const dsga::basic_vector<T, S> &v) const noexcept
	{
		std::size_t seed = 0;

		[&] <std::size_t ...Js>(std::index_sequence<Js ...>)
		{
			((seed = hash_combine(seed, v[Js])), ...);
		}(std::make_index_sequence<S>{});

		return seed;
	}
};

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::hash<dsga::basic_matrix<T, C, R>>
{
	std::size_t operator()(const dsga::basic_matrix<T, C, R> &m) const noexcept
	{
		std::size_t seed = 0;

		[&] <std::size_t ...Js>(std::index_sequence<Js ...>)
		{
			((seed = hash_combine(seed, m[Js])), ...);
		}(std::make_index_sequence<C>{});

		return seed;
	}
};
