
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <version>					// feature test macros

#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "dsga_doctest.hxx"

TEST_SUITE("test swizzling access")
{
	TEST_CASE("range-for loop (which requires) begin/end/deref(*)/prefix ++ interface")
	{
		using namespace dsga;
		ivec4	four(0, 1, 2, 3);
		[[ maybe_unused ]] ivec3	three(4, 5, 6);
		[[ maybe_unused ]] ivec2	two(7, 8);
		[[ maybe_unused ]] iscal	one(9);

		SUBCASE("range-for non-const dimension_data")
		{
			ivec4 four_dest(0);

			// for vec

			// recreate input one at a time
			// "int &" deduced for "auto &"
			for (std::size_t dest_indx = 0; auto & loop_var : four)
			{
				four_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(four, four_dest);
		}

		SUBCASE("range-for const dimension_data")
		{
			const ivec4 const_data(19, 28, 37, 46);
			ivec4 data_dest(0);

			// for vec

			// recreate input one at a time
			// "const int &" deduced for "auto &"
			for (std::size_t dest_indx = 0; auto & loop_var : const_data)
			{
				data_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(const_data, data_dest);
		}

		SUBCASE("range-for dimension_data allows modifying")
		{
			ivec4 mutable_data(0, 0, 0, 0);

			// for swizzle_vec

			// add index squared to whatever is accessed from the swizzle
			for (int dest_indx = 0; auto &loop_var : mutable_data)
			{
				loop_var += dest_indx * dest_indx;
				++dest_indx;
			}

			CHECK_EQ(mutable_data, ivec4(0, 1, 4, 9));
		}

		SUBCASE("range-for non-const swizzle_vec")
		{
			ivec4 non_const_data(55, 64, 73, 82);
			ivec4 data_dest(0);

			// for swizzle_vec

			// recreate input one at a time
			// "int &" deduced for "auto &"
			for (std::size_t dest_indx = 0; auto & loop_var : non_const_data.zwxy)
			{
				data_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(data_dest, ivec4(73, 82, 55, 64));
		}

		SUBCASE("range-for const swizzle_vec")
		{
			const ivec4 const_data(19, 28, 37, 46);
			ivec4 data_dest(0);

			// for swizzle_vec

			// recreate input one at a time
			// "const int &" deduced for "auto &"
			for (std::size_t dest_indx = 0; auto & loop_var : const_data.zwxy)
			{
				data_dest[dest_indx] = loop_var;
				++dest_indx;
			}

			CHECK_EQ(data_dest, ivec4(37, 46, 19, 28));
		}

		SUBCASE("range-for swizzle_vec allows aliasing")
		{
			ivec4 mutable_data(0, 0, 0, 0);

			// for swizzle_vec

			// add index to whatever is accessed from the swizzle
			for (int dest_indx = 0; auto &loop_var : mutable_data.wzyx)
			{
				loop_var += dest_indx;
				++dest_indx;
			}

			CHECK_EQ(mutable_data, ivec4(3, 2, 1, 0));
		}
	}

	TEST_CASE("index_vector iterators")
	{
		using namespace dsga;
		SUBCASE("forward and reverse iterator")
		{
			auto source = dsga::ivec4(11, 22, 33, 44).xyzw;
			dsga::ivec4 dest{};
			auto src_iter = source.crbegin();
			auto dest_iter = dest.yxwz.begin();
			while (src_iter != source.crend())
				*dest_iter++ = *src_iter++;

			CHECK_EQ(dest, dsga::ivec4(33, 44, 11, 22));
		}
	}
}
