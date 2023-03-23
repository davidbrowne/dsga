
//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <format>
#include <iostream>
#include "dsga.hxx"

//
// std::format interface
//

// adapted from https://www.modernescpp.com/index.php/extend-std-format-in-c-20-for-user-defined-types
template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
struct std::formatter<dsga::vector_base<Writable, T, Count, Derived>> : std::formatter<std::string_view>
{
	std::string value_format;
	constexpr auto parse(std::format_parse_context &ctx)
	{
		value_format = "{:";
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}')
		{
			value_format += *pos;
			++pos;
		}
		value_format += "}";
		return pos;  // expect `}` at this position, otherwise, it's error! exception!
	}

	auto format(const dsga::vector_base<Writable, T, Count, Derived> &v, format_context &ctx)
	{
		std::string temp;
		std::format_to(std::back_inserter(temp), "{}", "{ ");
		std::vformat_to(std::back_inserter(temp), value_format, std::make_format_args(v[0]));

		for (int i = 1; i < v.length(); ++i)
			std::vformat_to(std::back_inserter(temp), std::string_view(", " + value_format), std::make_format_args(v[i]));

		std::format_to(std::back_inserter(temp), "{}", " }");

		return std::formatter<string_view>::format(temp, ctx);
	}
};

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

//
// test function
//

template <bool Writable, dsga::dimensional_scalar T, std::size_t Count, typename Derived>
inline void test_print(const dsga::vector_base<Writable, T, Count, Derived> &v)
{
	// iostream interface
	std::cout << v << '\n';

	// std::format interface
	std::cout << std::format("{}\n", v);
	std::cout << std::format("{:10.5}\n", v);
}
