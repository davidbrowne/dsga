
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

template<typename T, std::size_t N, typename CharT>
struct std::formatter<std::array<T, N>, CharT> : std::formatter<T, CharT>
{
	constexpr auto parse(std::basic_format_parse_context<CharT> &ctx)
	{
		if constexpr (dsga::floating_point_scalar<T>)
		{
			for (auto it = ctx.begin(); !lead_hex && (it != ctx.end()); ++it)
			{
				CharT c = *it;

				if (c == 'a')
				{
					lead_hex = true;
					upper_hex = false;
				}
				else if (c == 'A')
				{
					lead_hex = true;
					upper_hex = true;
				}
			}
		}

		return std::formatter<T, CharT>::parse(ctx);
	}

	template <typename FormatContext>
	auto format(const std::array<T, N> &arr, FormatContext &ctx) const
	{
		if constexpr (N == 0)
		{
			return std::format_to(ctx.out(), "{{ }}");
		}
		else
		{
			auto prefix = [&]() { if (lead_hex) (upper_hex ? std::format_to(ctx.out(), "0X") : std::format_to(ctx.out(), "0x")); };

			std::format_to(ctx.out(), "{{ ");

			prefix();
			std::formatter<T, CharT>::format(arr[0], ctx);

			if constexpr (N > 1)
			{
				[&] <std::size_t ...Is>(std::index_sequence<Is...>)
				{
					((std::format_to(ctx.out(), ", "), prefix(), std::formatter<T, CharT>::format(arr[Is], ctx)), ...);
				}(dsga::make_index_range<1, N>{});
			}

			std::format_to(ctx.out(), " }}");

			return ctx.out();
		}
	}

	// set these appropriately if 'a' or 'A' was found during parsing for a floating_point_scalar type
	bool upper_hex = false;
	bool lead_hex = false;
};

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived, typename CharT>
struct std::formatter<dsga::vector_base<Writable, T, Count, Derived>, CharT> : std::formatter<T, CharT>
{
	constexpr auto parse(std::basic_format_parse_context<CharT> &ctx)
	{
		if constexpr (dsga::floating_point_scalar<T>)
		{
			for (auto it = ctx.begin(); !lead_hex && (it != ctx.end()); ++it)
			{
				CharT c = *it;

				if (c == 'a')
				{
					lead_hex = true;
					upper_hex = false;
				}
				else if (c == 'A')
				{
					lead_hex = true;
					upper_hex = true;
				}
			}
		}

		return std::formatter<T, CharT>::parse(ctx);
	}

	template <typename FormatContext>
	auto format(const dsga::vector_base<Writable, T, Count, Derived> &v, FormatContext &ctx) const
	{
		auto prefix = [&]() { if (lead_hex) (upper_hex ? std::format_to(ctx.out(), "0X") : std::format_to(ctx.out(), "0x")); };

		std::format_to(ctx.out(), "{{ ");

		prefix();
		std::formatter<T, CharT>::format(v[0], ctx);

		if constexpr (Count > 1)
		{
			[&] <std::size_t ...Is>(std::index_sequence<Is...>)
			{
				((std::format_to(ctx.out(), ", "), prefix(), std::formatter<T, CharT>::format(v[Is], ctx)), ...);
			}(dsga::make_index_range<1, Count>{});
		}

		std::format_to(ctx.out(), " }}");

		return ctx.out();
	}

	// set these appropriately if 'a' or 'A' was found during parsing for a floating_point_scalar type
	bool upper_hex = false;
	bool lead_hex = false;
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
