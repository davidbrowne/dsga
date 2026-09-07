#pragma once

//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <span>

// fill vectors from spans

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_scalar<U> && std::convertible_to<U, T>
constexpr void copy_to_vector(dsga::vec<T, S> &lhs, std::span<U, E> rhs) noexcept
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_scalar<U> && std::convertible_to<U, T>
constexpr void copy_to_vector(dsga::vec<T, S> &lhs, std::span<const U, E> rhs) noexcept
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_scalar<U> && std::convertible_to<U, T>
constexpr void copy_to_vector(dsga::vec<T, S> &lhs, std::span<U, E> rhs) noexcept
{
	const std::size_t count = std::min(S, rhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_scalar<U> && std::convertible_to<U, T>
constexpr void copy_to_vector(dsga::vec<T, S> &lhs, std::span<const U, E> rhs) noexcept
{
	const std::size_t count = std::min(S, rhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

// fill spans from vectors

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_scalar<U> && std::convertible_to<T, U>
constexpr void copy_from_vector(std::span<U, E> lhs, const dsga::vec<T, S> &rhs) noexcept
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_scalar<U> && std::convertible_to<T, U>
constexpr void copy_from_vector(std::span<U, E> lhs, const dsga::vec<T, S> &rhs) noexcept
{
	const std::size_t count = std::min(S, lhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

//
// create span from dsga::vec - can't create spans for dsga::swizzle_vec because not contiguous range,
// so a general purpose version for dsga::vec_interface won't compile either for dsga::swizzle_vec Derived classes
//

// non-const T span from non-const vector
template <dsga::dimensional_scalar T, std::size_t S>
[[nodiscard]] constexpr auto make_span(dsga::vec<T, S> &v) noexcept
{
//	return std::span<T, S>(v.begin(), v.end());
//	return std::span<T, S>(v.data(), v.size());
	return std::span<T, S>(v);
}

// const T span from const vector
template <dsga::dimensional_scalar T, std::size_t S>
[[nodiscard]] constexpr std::span<const T, S> make_span(const dsga::vec<T, S> &v) noexcept
{
//	return std::span<const T, S>(v.begin(), v.end());
//	return std::span<const T, S>(v.data(), v.size());
	return std::span<const T, S>(v);
}

// disallow creating spans from rvalues
template <dsga::dimensional_scalar T, std::size_t S>
auto make_span(dsga::vec<T, S> &&v) noexcept = delete;

// disallow creating spans from rvalues
template <dsga::dimensional_scalar T, std::size_t S>
auto make_span(const dsga::vec<T, S> &&v) noexcept = delete;

// const T span from either const or non-const vector
template <dsga::dimensional_scalar T, std::size_t S>
[[nodiscard]] constexpr std::span<const T, S> make_const_span(const dsga::vec<T, S> &v) noexcept
{
//	return std::span<const T, S>(v.begin(), v.end());
//	return std::span<const T, S>(v.data(), v.size());
	return std::span<const T, S>(v);
}

// disallow creating spans from rvalues
template <dsga::dimensional_scalar T, std::size_t S>
auto make_const_span(dsga::vec<T, S> &&v) noexcept = delete;

// disallow creating spans from rvalues
template <dsga::dimensional_scalar T, std::size_t S>
auto make_const_span(const dsga::vec<T, S> &&v) noexcept = delete;

//
// create span from dsga::mat
//

// non-const dsga::vec<T, C> span from non-const matrix
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
[[nodiscard]] constexpr std::span<dsga::vec<T, R>, C> make_span(dsga::mat<T, C, R> &m) noexcept
{
//	return std::span<dsga::vec<T, R>, C>(m.begin(), m.end());
//	return std::span<dsga::vec<T, R>, C>(m.data(), m.size());
	return std::span<dsga::vec<T, R>, C>(m);
}

// const dsga::vec<T, C> span from const matrix
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
[[nodiscard]] constexpr std::span<const dsga::vec<T, R>, C> make_span(const dsga::mat<T, C, R> &m) noexcept
{
//	return std::span<const dsga::vec<T, R>, C>(m.begin(), m.end());
//	return std::span<const dsga::vec<T, R>, C>(m.data(), m.size());
	return std::span<const dsga::vec<T, R>, C>(m);
}

// disallow creating spans from rvalues
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
auto make_span(dsga::mat<T, C, R> &&m) noexcept = delete;

// disallow creating spans from rvalues
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
auto make_span(const dsga::mat<T, C, R> &&m) noexcept = delete;

// const dsga::vec<T, C> span from either const or non-const matrix
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
[[nodiscard]] constexpr std::span<const dsga::vec<T, R>, C> make_const_span(const dsga::mat<T, C, R> &m) noexcept
{
//	return std::span<const dsga::vec<T, R>, C>(m.begin(), m.end());
//	return std::span<const dsga::vec<T, R>, C>(m.data(), m.size());
	return std::span<const dsga::vec<T, R>, C>(m);
}

// disallow creating spans from rvalues
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
auto make_const_span(dsga::mat<T, C, R> &&m) noexcept = delete;

// disallow creating spans from rvalues
template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
auto make_const_span(const dsga::mat<T, C, R> &&m) noexcept = delete;

//
// std::span to dsga::vec
//

// create a vec from parts of a span, the parts (indexes) described in a std::index_sequence
// use like:	make_vector(vector_input_span, std::make_index_sequence<4>{})
template <typename T, std::size_t E, std::size_t ...Is>
requires (E != std::dynamic_extent) && dsga::dimensional_scalar<std::remove_cvref_t<T>> && dsga::vec_dimension<sizeof...(Is)> && ((Is < E) && ...)
[[nodiscard]] constexpr auto make_vector(std::span<T, E> s, std::index_sequence<Is...> ) noexcept
{
	return dsga::vec<std::remove_cvref_t<T>, sizeof...(Is)>{ s[Is]... };
}

//
// std::span to dsga::mat
//

// create a mat from a span that holds at least the number of matrix elements.
// use like:	make_matrix<C, R>(matrix_input_span)
template <std::size_t C, std::size_t R, typename T, std::size_t E>
requires (E != std::dynamic_extent) && (((C >= 2u) && (C <= 4u)) && ((R >= 2u) && (R <= 4u))) && (C * R <= E) && dsga::floating_point_scalar<std::remove_cvref_t<T>>
[[nodiscard]] constexpr auto make_matrix(std::span<T, E> s) noexcept
{
	return [&]<std::size_t ...Js>(std::index_sequence <Js...>) noexcept
	{
		return dsga::mat<std::remove_cvref_t<T>, C, R>(
			[&]<std::size_t ...Is>(std::index_sequence <Is...>) noexcept
			{
				constexpr auto cols = Js;
				return dsga::vec<std::remove_cvref_t<T>, R>{ s[cols * R + Is]... };
			}(std::make_index_sequence<R>{}) ...);
	}(std::make_index_sequence<C>{});
}
