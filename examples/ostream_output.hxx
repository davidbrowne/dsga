
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <iostream>

//
// iostream interface
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline std::ostream &operator<<(std::ostream &o, const dsga::vector_base<Writable, T, Count, Derived> &v)
{
	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}

	o << "[" << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << "]";
}

template <dsga::dimensional_scalar T, std::size_t Size>
inline std::ostream &operator<<(std::ostream &o, const dsga::basic_vector<T, Size> &v)
{
	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}

	o << "[" << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << "]";
}

template <bool M, dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
inline std::ostream &operator<<(std::ostream &o, const dsga::indexed_vector<M, T, Size, Count, Is...> &v)
{
	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}

	o << "[" << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << "]";
}

template <bool M, dsga::dimensional_scalar T, std::size_t Size>
inline std::ostream &operator<<(std::ostream &o, const dsga::basic_view<M, T, Size> &v)
{
	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}

	o << "[" << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << "]";
}

template <bool M, dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
inline std::ostream &operator<<(std::ostream &o, const dsga::indexed_view<M, T, Size, Count, Is...> &v)
{
	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}

	o << "[" << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << "]";
}

template <dsga::dimensional_scalar T, std::size_t Size>
inline std::ostream &operator<<(std::ostream &o, const dsga::view_vector<T, Size> &v)
{
	if constexpr (std::same_as<bool, T>)
	{
		o << std::boolalpha;
	}

	o << "[" << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << "]";
}

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
inline std::ostream &operator<<(std::ostream &o, const dsga::basic_matrix<T, C, R> &m)
{
	o << "[" << m[0];
	for (int i = 1; i < m.length(); ++i)
		o << ", " << m[i];
	return o << "]";
}
