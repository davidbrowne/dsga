
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

#if defined(__cpp_lib_format)

#include <iostream>
#include <format>

//
// std::format interfaces
//

//
// Hexadecimal floating-point
//
// In c++17 we got floating-point literals, which are prefixed with either "0x" or "0X".
// This prefix does not work when reading them with std::from_chars(). However, std::to_chars()
// does NOT generate this prefix when generating floating-point hexadecimal characters,
// although std::printf() does generate the prefix. So std::to_chars() and std::from_chars()
// round-trip. However, std::strtod() and std::sscanf() DO require the prefix for reading
// floating-point hexadecimal. This means that std::strtod() and std::sscanf() do not round-
// trip with std::to_chars() for floating-point hexadecimal.
// 
// In addition, std::format() uses std::to_chars() under the hood, so for format specifiers
// 'a' and 'A', which create floating-point hexadecimals, the numbers are NOT prefixed, so
// they won't round-trip with std::strtod() or std::sscanf() either.
// 
// For ostreams and hexadecimal floating-point, if you use std::hexfloat, the output has the
// "0x" prefix. However, for istreams and hexadecimal floating-point, if you use std::hexfloat
// then it doesn't matter if the prefix is there or not, it is input properly with MSVC. The
// input doesn't work either way for gcc. Only the prefixed input works for clang (on Windows,
// not in Linux).
// 
// Given this background (as of 30-SEP-2023), if we want to have round-tripped I/O, we need to
// be consistent with the prefixes. If we want to go without a prefix, then for output we can
// use std::format and/or std::to_chars, and for input we can use std::from_chars. If we want
// to have the prefix, which is the form of the official hexadecimal floating-point literal,
// then for output we can use std::printf and/or ostreams with std::hexfloat and/or custom
// std::formatters, and for input we can use std:sscanf() or std::strtod(). istreams are
// currently undependable as a solution for portability, with different behavior depending on
// the platform and compiler.
// 
// The following std::formatter specializations provide the functionality to round-trip
// floating-point hexadecimals with std::from_chars if 'a' or 'A' is used. It is not worth
// the effort to manually handle issues with format specifiers '+', ' ', 'a', 'A', etc., just
// in order to prefix the hexadecimal floating-point output with a "0x" or "0X".
//


template<typename T, std::size_t N, typename CharT>
struct std::formatter<std::array<T, N>, CharT> : std::formatter<T, CharT>
{
	template <typename FormatContext>
	auto format(const std::array<T, N> &arr, FormatContext &ctx) const
	{
		if constexpr (N == 0)
		{
			return std::format_to(ctx.out(), "{{ }}");
		}
		else
		{
			std::format_to(ctx.out(), "{{ ");

			std::formatter<T, CharT>::format(arr[0], ctx);
			if constexpr (N > 1)
			{
				[&] <std::size_t ...Is>(std::index_sequence<Is...>)
				{
					((std::format_to(ctx.out(), ", "), std::formatter<T, CharT>::format(arr[Is], ctx)), ...);
				}(dsga::make_index_range<1, N>{});
			}

			std::format_to(ctx.out(), " }}");

			return ctx.out();
		}
	}
};

template<typename T, std::size_t Size, typename CharT>
struct std::formatter<dsga::storage_wrapper<T, Size>, CharT> : std::formatter<T, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::storage_wrapper<T, Size> &sw, FormatContext &ctx) const
	{
		std::format_to(ctx.out(), "{{ ");

		std::formatter<T, CharT>::format(sw[0], ctx);
		if constexpr (Size > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::format_to(ctx.out(), ", "), std::formatter<T, CharT>::format(sw[Is], ctx)), ...);
			}(dsga::make_index_range<1, Size>{});
		}

		std::format_to(ctx.out(), " }}");

		return ctx.out();
	}
};

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived, typename CharT>
struct std::formatter<dsga::vector_base<Writable, T, Count, Derived>, CharT> : std::formatter<T, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::vector_base<Writable, T, Count, Derived> &v, FormatContext &ctx) const
	{
		std::format_to(ctx.out(), "{{ ");

		std::formatter<T, CharT>::format(v[0], ctx);
		if constexpr (Count > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::format_to(ctx.out(), ", "), std::formatter<T, CharT>::format(v[Is], ctx)), ...);
			}(dsga::make_index_range<1, Count>{});
		}

		std::format_to(ctx.out(), " }}");

		return ctx.out();
	}
};

template <typename CharT, dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
struct std::formatter<dsga::indexed_vector<T, Size, Count, Is...>, CharT>
	: std::formatter<dsga::vector_base<dsga::writable_swizzle<Size, Count, Is...>, T, Count, dsga::indexed_vector<T, Size, Count, Is...>>, CharT>
{
};

template <dsga::dimensional_scalar T, std::size_t Size, typename CharT>
struct std::formatter<dsga::basic_vector<T, Size>, CharT>
	: std::formatter<dsga::vector_base<true, T, Size, dsga::basic_vector<T, Size>>, CharT>
{
};

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R, typename CharT>
struct std::formatter<dsga::basic_matrix<T, C, R>, CharT> : std::formatter<dsga::basic_vector<T, R>, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::basic_matrix<T, C, R> &m, FormatContext &ctx) const
	{
		std::format_to(ctx.out(), "[ ");

		std::formatter<dsga::basic_vector<T, R>, CharT>::format(m[0], ctx);
		if constexpr (C > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::format_to(ctx.out(), ", "), std::formatter<dsga::basic_vector<T, R>, CharT>::format(m[Is], ctx)), ...);
			}(dsga::make_index_range<1, C>{});
		}

		std::format_to(ctx.out(), " ]");

		return ctx.out();
	}
};

//
// helper functions
//

// std::from_chars does not like a leading '+' unless it is associated with the exponent.
// std::format uses std::to_chars beneath the hood. from_format_hexfloat_chars() handles cases such as:
//
//    double val = 1.5;
//    auto vanilla_float_hex = std::format("{:a}", val); =>  "1.8p+0"s
//    auto plus_float_hex =  std::format("{:+a}", val);  => "+1.8p+0"s
//    auto space_float_hex =  std::format("{: a}", val); => " 1.8p+0"s
template <dsga::floating_point_scalar T>
auto from_format_hexfloat_chars(std::string_view sv, T &val)
{
	int leading_char = 0;
	if (!sv.empty() && ((sv.front() == '+') || (sv.front() == ' ')))
		leading_char = 1;

	return std::from_chars(sv.data() + leading_char, sv.data() + sv.size(), val, std::chars_format::hex);
}

//
// test functions
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline void test_format_vector_base(const dsga::vector_base<Writable, T, Count, Derived> &v)
{
	// std::format interface
	std::cout << std::format("{}\n", v);

	if constexpr (dsga::floating_point_scalar<T>)
	{
		std::cout << std::format("{:10.5}\n", v);
		std::cout << std::format("{:a}\n", v);
	}
}

template <typename T, std::size_t Size>
inline void test_format_array(const std::array<T, Size> &arr)
{
	// std::format interface
	std::cout << std::format("{}\n", arr);

	if constexpr (std::is_floating_point_v<T>)
	{
		std::cout << std::format("{:10.5}\n", arr);
		std::cout << std::format("{:a}\n", arr);
	}
}

template <typename T, std::size_t Size>
inline void test_format_storage_wrapper(const dsga::storage_wrapper<T, Size> &sw)
{
	// std::format interface
	std::cout << std::format("{}\n", sw);

	if constexpr (std::is_floating_point_v<T>)
	{
		std::cout << std::format("{:10.5}\n", sw);
		std::cout << std::format("{:a}\n", sw);
	}
}

template <dsga::dimensional_scalar T, std::size_t Size>
inline void test_format_vector(const dsga::basic_vector<T, Size> &v)
{
	// std::format interface
	std::cout << std::format("{}\n", v);

	if constexpr (dsga::floating_point_scalar<T>)
	{
		std::cout << std::format("{:10.5}\n", v);
		std::cout << std::format("{:a}\n", v);
	}
}

template <dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
inline void test_format_indexed_vector(const dsga::indexed_vector<T, Size, Count, Is...> &v)
{
	// std::format interface
	std::cout << std::format("{}\n", v);

	if constexpr (dsga::floating_point_scalar<T>)
	{
		std::cout << std::format("{:10.5}\n", v);
		std::cout << std::format("{:a}\n", v);
	}
}

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
inline void test_format_matrix(const dsga::basic_matrix<T, C, R> &m)
{
	// std::format interface
	std::cout << std::format("{}\n", m);
	std::cout << std::format("{:10.5}\n", m);
	std::cout << std::format("{:A}\n", m);
}

#endif
