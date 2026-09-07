#pragma once

//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// doctest stringification support for dsga types.
// Include this file AFTER doctest.h in test files.

#include "dsga.hxx"
#include <sstream>
#include <string>

namespace doctest
{
	template <typename T, std::size_t S>
	struct StringMaker<dsga::vec<T, S>>
	{
		static String convert(const dsga::vec<T, S> &v)
		{
			std::ostringstream oss;
			oss << "vec(";
			for (std::size_t i = 0; i < S; ++i)
			{
				if (i > 0) oss << ", ";
				oss << v[i];
			}
			oss << ")";
			return oss.str().c_str();
		}
	};

	template <typename T, std::size_t Size, std::size_t Count, std::size_t... Is>
	struct StringMaker<dsga::swizzle_vec<T, Size, Count, Is...>>
	{
		static String convert(const dsga::swizzle_vec<T, Size, Count, Is...> &v)
		{
			std::ostringstream oss;
			oss << "swizzle_vec(";
			for (std::size_t i = 0; i < Count; ++i)
			{
				if (i > 0) oss << ", ";
				oss << v[i];
			}
			oss << ")";
			return oss.str().c_str();
		}
	};
}
