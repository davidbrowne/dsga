
//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include "dsga.hxx"

#if defined(__cpp_lib_format) && defined(_MSC_VER)

#include <format>

//
// std::format interfaces
//

// helpful links
// https://www.modernescpp.com/index.php/extend-std-format-in-c-20-for-user-defined-types
// https://www.cppstories.com/2022/custom-stdformat-cpp20/

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
struct std::formatter<dsga::vector_base<Writable, T, Count, Derived>> : std::formatter<std::string_view>
{
	std::string value_format;
	constexpr auto parse(const std::format_parse_context &ctx)
	{
		value_format = "{:";
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}')
		{
			value_format += *pos;
			++pos;
		}
		value_format += "}";
		return pos;
	}

	auto format(const dsga::vector_base<Writable, T, Count, Derived> &v, std::format_context &ctx)
	{
		std::string fmts{};
		for (std::size_t i = 1; i < Count; ++i)
			fmts += ", " + value_format;

		return[&]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			return std::vformat_to(ctx.out(), std::format("{{{{ {}{} }}}}", value_format, fmts), std::make_format_args(v[Is]...));
		}(std::make_index_sequence<Count>{});
	}
};

template <dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
struct std::formatter<dsga::indexed_vector<T, Size, Count, Is...>> : std::formatter<std::string_view>
{
	std::string value_format;
	constexpr auto parse(const std::format_parse_context &ctx)
	{
		value_format = "{:";
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}')
		{
			value_format += *pos;
			++pos;
		}
		value_format += "}";
		return pos;
	}

	auto format(const dsga::indexed_vector<T, Size, Count, Is...> &v, std::format_context &ctx)
	{
		std::string fmts{};
		for (std::size_t i = 1; i < Count; ++i)
			fmts += ", " + value_format;

		return[&]<std::size_t ...Js>(std::index_sequence<Js...>)
		{
			return std::vformat_to(ctx.out(), std::format("{{{{ {}{} }}}}", value_format, fmts), std::make_format_args(v[Js]...));
		}(std::make_index_sequence<Count>{});
	}
};


template <typename T, std::size_t N>
struct std::formatter<std::array<T, N>>
{
	std::string value_format;
	constexpr auto parse(const std::format_parse_context &ctx)
	{
		value_format = "{:";
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}')
		{
			value_format += *pos;
			++pos;
		}
		value_format += "}";
		return pos;
	}

	auto format(const std::array<T, N> &val, std::format_context &ctx)
	{
		if constexpr (N == 0)
		{
			return std::format_to(ctx.out(), "{{ }}");
		}
		else
		{
			std::string fmts{};
			for (std::size_t i = 1; i < N; ++i)
				fmts += ", " + value_format;

			return[&]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				return std::vformat_to(ctx.out(), std::format("{{{{ {}{} }}}}", value_format, fmts), std::make_format_args(val[Is]...));
			}(std::make_index_sequence<N>{});
		}
	}
};


template <dsga::dimensional_scalar T, std::size_t Size>
struct std::formatter<dsga::basic_vector<T, Size>> : std::formatter<std::string_view>
{
	std::string value_format;
	constexpr auto parse(const std::format_parse_context &ctx)
	{
		value_format = "{:";
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}')
		{
			value_format += *pos;
			++pos;
		}
		value_format += "}";
		return pos;
	}

	auto format(const dsga::basic_vector<T, Size> &v, std::format_context &ctx)
	{
		std::string fmts{};
		for (std::size_t i = 1; i < Size; ++i)
			fmts += ", " + value_format;

		return[&]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			return std::vformat_to(ctx.out(), std::format("{{{{ {}{} }}}}", value_format, fmts), std::make_format_args(v[Is]...));
		}(std::make_index_sequence<Size>{});
	}
};

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::formatter<dsga::basic_matrix<T, C, R>> : std::formatter<std::string_view>
{
	std::string value_format;
	constexpr auto parse(const std::format_parse_context &ctx)
	{
		value_format = "{:";
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}')
		{
			value_format += *pos;
			++pos;
		}
		value_format += "}";
		return pos;
	}

	auto format(const dsga::basic_matrix<T, C, R> &m, std::format_context &ctx)
	{
		std::string fmts{};
		for (std::size_t i = 1; i < C; ++i)
			fmts += ", " + value_format;

		return[&]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			return std::vformat_to(ctx.out(), std::format("[ {}{} ]", value_format, fmts), std::make_format_args(m[Is]...));
		}(std::make_index_sequence<C>{});
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

//
// iostream interface
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline std::ostream &operator<<(std::ostream &o, const dsga::vector_base<Writable, T, Count, Derived> &v)
{
	o << "{ " << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << " }";
}

template <dsga::dimensional_scalar T, std::size_t Size>
inline std::ostream &operator<<(std::ostream &o, const dsga::basic_vector<T, Size> &v)
{
	o << "{ " << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << " }";
}

template <dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
inline std::ostream &operator<<(std::ostream &o, const dsga::indexed_vector<T, Size, Count, Is...> &v)
{
	o << "{ " << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << ", " << v[i];
	return o << " }";
}

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
inline std::ostream &operator<<(std::ostream &o, const dsga::basic_matrix<T, C, R> &m)
{
	o << "[ " << m[0];
	for (int i = 1; i < m.length(); ++i)
		o << ", " << m[i];
	return o << " ]";
}

//
// test functions
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline void test_ostream_vector_base(const dsga::vector_base<Writable, T, Count, Derived> &v)
{
	// iostream interface
	std::cout << v << '\n';
}

template <dsga::dimensional_scalar T, std::size_t Size>
inline void test_ostream_vector(const dsga::basic_vector<T, Size> &v)
{
	// iostream interface
	std::cout << v << '\n';
}

template <dsga::dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
inline void test_ostream_indexed_vector(const dsga::indexed_vector<T, Size, Count, Is...> &v)
{
	// iostream interface
	std::cout << v << '\n';
}

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
inline void test_ostream_matrix(const dsga::basic_matrix<T, C, R> &m)
{
	// iostream interface
	std::cout << m << '\n';
}
