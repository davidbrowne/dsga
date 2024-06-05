
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <functional>


// based on https://www.boost.org/doc/libs/1_83_0/libs/container_hash/doc/html/hash.html#notes_hash_combine
// but then, https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
// Mix 13
template <typename T>
constexpr std::size_t hash_combine(std::size_t seed, const T &t) noexcept
{
	std::size_t x = seed + 0x9e3779b9 + std::hash<T>{}(t);

	x ^= x >> 30;
	x *= 0xbf58476d1ce4e5b9;
	x ^= x >> 27;
	x *= 0x94d049bb133111eb;
	x ^= x >> 31;

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

template <bool M, dsga::dimensional_scalar T, std::size_t S>
struct std::hash<dsga::basic_view<M, T, S>>
{
	std::size_t operator()(const dsga::basic_view<M, T, S> &v) const noexcept
	{
		std::size_t seed = 0;

		[&] <std::size_t ...Js>(std::index_sequence<Js ...>)
		{
			((seed = hash_combine(seed, v[Js])), ...);
		}(std::make_index_sequence<S>{});

		return seed;
	}
};

template <dsga::dimensional_scalar T, std::size_t S>
struct std::hash<dsga::view_vector<T, S>>
{
	std::size_t operator()(const dsga::view_vector<T, S> &v) const noexcept
	{
		std::size_t seed = 0;

		[&] <std::size_t ...Js>(std::index_sequence<Js ...>)
		{
			((seed = hash_combine(seed, v[Js])), ...);
		}(std::make_index_sequence<S>{});

		return seed;
	}
};

template <bool M, dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::hash<dsga::indexed_vector<M, T, S, C, Is...>>
{
	std::size_t operator()(const dsga::indexed_vector<M, T, S, C, Is...> &v) const noexcept
	{
		std::size_t seed = 0;

		[&] <std::size_t ...Js>(std::index_sequence<Js ...>)
		{
			((seed = hash_combine(seed, v[Js])), ...);
		}(std::make_index_sequence<C>{});

		return seed;
	}
};

template <bool M, dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::hash<dsga::indexed_view<M, T, S, C, Is...>>
{
	std::size_t operator()(const dsga::indexed_view<M, T, S, C, Is...> &v) const noexcept
	{
		std::size_t seed = 0;

		[&] <std::size_t ...Js>(std::index_sequence<Js ...>)
		{
			((seed = hash_combine(seed, v[Js])), ...);
		}(std::make_index_sequence<C>{});

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
