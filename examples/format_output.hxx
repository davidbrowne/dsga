
//          Copyright David Browne 2020-2023.
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

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived, typename CharT>
struct std::formatter<dsga::vector_base<Writable, T, Count, Derived>, CharT> : std::formatter<T, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::vector_base<Writable, T, Count, Derived> &v, FormatContext &ctx) const
	{
		std::vformat_to(ctx.out(), "{{ ", std::make_format_args());
		std::formatter<T, CharT>::format(v[0], ctx);

		if constexpr (Count > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::vformat_to(ctx.out(), ", ", std::make_format_args()), std::formatter<T, CharT>::format(v[Is], ctx)), ...);
			}(dsga::make_index_range<1, Count>{});
		}

		std::vformat_to(ctx.out(), " }}", std::make_format_args());

		return ctx.out();
	}
};

template <typename CharT, dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
struct std::formatter<dsga::indexed_vector<T, Size, Count, Is...>, CharT> : std::formatter<T, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::indexed_vector<T, Size, Count, Is...> &v, FormatContext &ctx) const
	{
		std::vformat_to(ctx.out(), "{{ ", std::make_format_args());
		std::formatter<T, CharT>::format(v[0], ctx);

		if constexpr (Count > 1)
		{
			[&] <std::size_t ...Js>(std::index_sequence<Js...>)
			{
				((std::vformat_to(ctx.out(), ", ", std::make_format_args()), std::formatter<T, CharT>::format(v[Js], ctx)), ...);
			}(dsga::make_index_range<1, Count>{});
		}

		std::vformat_to(ctx.out(), " }}", std::make_format_args());

		return ctx.out();
	}
};

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
			std::vformat_to(ctx.out(), "{{ ", std::make_format_args());
			std::formatter<T, CharT>::format(arr[0], ctx);

			if constexpr (N > 1)
			{
				[&] <std::size_t ...Is>(std::index_sequence<Is...>)
				{
					((std::vformat_to(ctx.out(), ", ", std::make_format_args()), std::formatter<T, CharT>::format(arr[Is], ctx)), ...);
				}(dsga::make_index_range<1, N>{});
			}

			std::vformat_to(ctx.out(), " }}", std::make_format_args());

			return ctx.out();
		}
	}
};

template <dsga::dimensional_scalar T, std::size_t Size, typename CharT>
struct std::formatter<dsga::basic_vector<T, Size>, CharT> : std::formatter<T, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::basic_vector<T, Size> &v, FormatContext &ctx) const
	{
		std::vformat_to(ctx.out(), "{{ ", std::make_format_args());
		std::formatter<T, CharT>::format(v[0], ctx);

		if constexpr (Size > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::vformat_to(ctx.out(), ", ", std::make_format_args()), std::formatter<T, CharT>::format(v[Is], ctx)), ...);
			}(dsga::make_index_range<1, Size>{});
		}

		std::vformat_to(ctx.out(), " }}", std::make_format_args());

		return ctx.out();
	}
};

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R, typename CharT>
struct std::formatter<dsga::basic_matrix<T, C, R>, CharT> : std::formatter<dsga::basic_vector<T, R>, CharT>
{
	template <typename FormatContext>
	auto format(const dsga::basic_matrix<T, C, R> &m, FormatContext &ctx) const
	{
		std::vformat_to(ctx.out(), "[ ", std::make_format_args());
		std::formatter<dsga::basic_vector<T, R>, CharT>::format(m[0], ctx);

		if constexpr (C > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::vformat_to(ctx.out(), ", ", std::make_format_args()), std::formatter<dsga::basic_vector<T, R>, CharT>::format(m[Is], ctx)), ...);
			}(dsga::make_index_range<1, C>{});
		}

		std::vformat_to(ctx.out(), " ]", std::make_format_args());

		return ctx.out();
	}
};

//
// test functions
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline void test_format_vector_base(const dsga::vector_base<Writable, T, Count, Derived> &v)
{
	// std::format interface
	std::cout << std::format("{}\n", v);
	std::cout << std::format("{:10.5}\n", v);
}

template <typename T, std::size_t Size>
inline void test_format_array(const std::array<T, Size> &arr)
{
	// std::format interface
	std::cout << std::format("{}\n", arr);
	std::cout << std::format("{:10.5}\n", arr);
}

template <dsga::dimensional_scalar T, std::size_t Size>
inline void test_format_vector(const dsga::basic_vector<T, Size> &v)
{
	// std::format interface
	std::cout << std::format("{}\n", v);
	std::cout << std::format("{:10.5}\n", v);
}

template <dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
inline void test_format_indexed_vector(const dsga::indexed_vector<T, Size, Count, Is...> &v)
{
	// std::format interface
	std::cout << std::format("{}\n", v);
	std::cout << std::format("{:10.5}\n", v);
}

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
inline void test_format_matrix(const dsga::basic_matrix<T, C, R> &m)
{
	// std::format interface
	std::cout << std::format("{}\n", m);
	std::cout << std::format("{:10.5}\n", m);
}

#endif
