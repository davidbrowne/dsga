
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include <version>					// feature test macros
using namespace dsga;

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// structs for demonstrating common initial sequence
struct array_storage_a
{
	std::array<double, 4> i;
};

struct array_storage_b
{
	std::array<double, 4> j;
};

struct faux_vector
{
	array_storage_b k;
};

struct faux_wrapper
{
	array_storage_a l;
};

TEST_SUITE("type traits tests")
{
	TEST_CASE("type traits for storage_wrapper")
	{
		using dwrap4 = dsga::storage_wrapper<true, double, 4u>;
		auto dwrap4_var = dwrap4{};

		CHECK_UNARY(std::is_standard_layout_v<dwrap4>);
		CHECK_UNARY(std::is_default_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_copyable_v<dwrap4>);
		CHECK_UNARY(std::is_trivial_v<dwrap4>);
		CHECK_UNARY(std::is_copy_assignable_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<dwrap4>);
		CHECK_UNARY(std::is_assignable_v<dwrap4 &, dwrap4 &>);
		CHECK_UNARY(std::is_assignable_v<dwrap4 &, dwrap4 &&>);
		CHECK_UNARY(std::is_assignable_v<dwrap4 &&, dwrap4 &>);
		CHECK_UNARY(std::is_assignable_v<dwrap4 &&, dwrap4 &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<dwrap4>);

		CHECK_UNARY(std::is_aggregate_v<dwrap4>);

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(dwrap4_var.begin())>);
		CHECK_UNARY(std::contiguous_iterator<decltype(dwrap4_var.cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<dwrap4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<dwrap4>);
		CHECK_UNARY(std::ranges::sized_range<dwrap4>);
		CHECK_UNARY_FALSE(std::ranges::view<dwrap4>);
		CHECK_UNARY(std::ranges::contiguous_range<dwrap4>);
		CHECK_UNARY(std::ranges::common_range<dwrap4>);
	}

	TEST_CASE("type traits for view_wrapper")
	{
		double backing[8]{};
		using dwrap4 = dsga::view_wrapper<true, double, 4u>;
		auto dwrap4_var = dwrap4{backing + 4};

		CHECK_UNARY(std::is_standard_layout_v<dwrap4>);
		CHECK_UNARY_FALSE(std::is_default_constructible_v<dwrap4>);
		CHECK_UNARY_FALSE(std::is_trivially_constructible_v<dwrap4>);
		CHECK_UNARY_FALSE(std::is_trivially_default_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<dwrap4>);
		CHECK_UNARY(std::is_trivially_copyable_v<dwrap4>);

#if !defined(__GNUC__)
		CHECK_UNARY_FALSE(std::is_trivial_v<dwrap4>);
#endif

		CHECK_UNARY_FALSE(std::is_copy_assignable_v<dwrap4>);
		CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<dwrap4>);
		CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<dwrap4>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dwrap4 &, dwrap4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dwrap4 &, dwrap4 &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dwrap4 &&, dwrap4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dwrap4 &&, dwrap4 &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<dwrap4>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<dwrap4>);

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(dwrap4_var.begin())>);
		CHECK_UNARY(std::contiguous_iterator<decltype(dwrap4_var.cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<dwrap4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<dwrap4>);
		CHECK_UNARY(std::ranges::sized_range<dwrap4>);
		CHECK_UNARY_FALSE(std::ranges::view<dwrap4>);
		CHECK_UNARY(std::ranges::contiguous_range<dwrap4>);
		CHECK_UNARY(std::ranges::common_range<dwrap4>);
	}

	TEST_CASE("type traits for vector_base")
	{
		using vb1 = vector_base<true, double, 4, dsga::basic_vector<double, 4>>;
		using vb2 = vector_base<true, double, 4, dsga::indexed_vector<true, double, 4, 4, 0, 1, 2, 3>>;
		using vb3 = vector_base<true, double, 4, dsga::basic_view<true, double, 4>>;
		using vb4 = vector_base<true, double, 4, dsga::indexed_view<true, double, 4, 4, 0, 1, 2, 3>>;

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(std::declval<vb1>().begin())>);
		CHECK_UNARY_FALSE(std::contiguous_iterator<decltype(std::declval<vb2>().cbegin())>);
		CHECK_UNARY(std::random_access_iterator<decltype(std::declval<vb2>().cbegin())>);

		CHECK_UNARY(std::contiguous_iterator<decltype(std::declval<vb3>().begin())>);
		CHECK_UNARY_FALSE(std::contiguous_iterator<decltype(std::declval<vb4>().cbegin())>);
		CHECK_UNARY(std::random_access_iterator<decltype(std::declval<vb4>().cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<vb1>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<vb1>);
		CHECK_UNARY(std::ranges::sized_range<vb1>);
		CHECK_UNARY_FALSE(std::ranges::view<vb1>);
		CHECK_UNARY(std::ranges::contiguous_range<vb1>);
		CHECK_UNARY(std::ranges::common_range<vb1>);

		CHECK_UNARY(std::ranges::range<vb2>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<vb2>);
		CHECK_UNARY(std::ranges::sized_range<vb2>);
		CHECK_UNARY_FALSE(std::ranges::view<vb2>);
		CHECK_UNARY(std::ranges::random_access_range<vb2>);
		CHECK_UNARY_FALSE(std::ranges::contiguous_range<vb2>);
		CHECK_UNARY(std::ranges::common_range<vb2>);

		CHECK_UNARY(std::ranges::range<vb3>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<vb3>);
		CHECK_UNARY(std::ranges::sized_range<vb3>);
		CHECK_UNARY_FALSE(std::ranges::view<vb3>);
		CHECK_UNARY(std::ranges::contiguous_range<vb3>);
		CHECK_UNARY(std::ranges::common_range<vb3>);

		CHECK_UNARY(std::ranges::range<vb4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<vb4>);
		CHECK_UNARY(std::ranges::sized_range<vb4>);
		CHECK_UNARY_FALSE(std::ranges::view<vb4>);
		CHECK_UNARY(std::ranges::random_access_range<vb4>);
		CHECK_UNARY_FALSE(std::ranges::contiguous_range<vb4>);
		CHECK_UNARY(std::ranges::common_range<vb4>);
	}

	TEST_CASE("type traits for basic_vector")
	{
		using dvec4 = dsga::basic_vector<double, 4u>;
		auto dvec4_var = dvec4{};

		CHECK_UNARY(std::is_standard_layout_v<dvec4>);
		CHECK_UNARY(std::is_default_constructible_v<dvec4>);
		CHECK_UNARY(std::is_trivially_constructible_v<dvec4>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<dvec4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<dvec4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<dvec4>);
		CHECK_UNARY(std::is_trivially_copyable_v<dvec4>);
		CHECK_UNARY(std::is_trivial_v<dscal>);
		CHECK_UNARY(std::is_trivial_v<dvec2>);
		CHECK_UNARY(std::is_trivial_v<dvec3>);
		CHECK_UNARY(std::is_trivial_v<dvec4>);
		CHECK_UNARY(std::is_copy_assignable_v<dvec4>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<dvec4>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<dvec4>);
		CHECK_UNARY(std::is_assignable_v<dvec4 &, dvec4 &>);
		CHECK_UNARY(std::is_assignable_v<dvec4 &, dvec4 &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dvec4 &&, dvec4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dvec4 &&, dvec4 &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<dvec4>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<dvec4>);

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(dvec4_var.begin())>);
		CHECK_UNARY(std::contiguous_iterator<decltype(dvec4_var.cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<dvec4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<dvec4>);
		CHECK_UNARY(std::ranges::sized_range<dvec4>);
		CHECK_UNARY_FALSE(std::ranges::view<dvec4>);
		CHECK_UNARY(std::ranges::contiguous_range<dvec4>);
		CHECK_UNARY(std::ranges::common_range<dvec4>);
	}

	TEST_CASE("type traits for basic_view")
	{
		double backing[8]{};
		using nvec4 = dsga::basic_view<true, double, 4u>;
		auto nvec4_var = nvec4(backing + 2);

		CHECK_UNARY(std::is_standard_layout_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_default_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_default_constructible_v<nvec4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<nvec4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<nvec4>);
		CHECK_UNARY(std::is_trivially_copyable_v<nvec4>);

#if !defined(__GNUC__)
		CHECK_UNARY_FALSE(std::is_trivial_v<nvec4>);
#endif

		CHECK_UNARY_FALSE(std::is_copy_assignable_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_assignable_v<nvec4 &, nvec4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<nvec4 &, nvec4 &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<nvec4 &&, nvec4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<nvec4 &&, nvec4 &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<nvec4>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<nvec4>);

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(nvec4_var.begin())>);
		CHECK_UNARY(std::contiguous_iterator<decltype(nvec4_var.cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<nvec4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<nvec4>);
		CHECK_UNARY(std::ranges::sized_range<nvec4>);
		CHECK_UNARY_FALSE(std::ranges::view<nvec4>);
		CHECK_UNARY(std::ranges::contiguous_range<nvec4>);
		CHECK_UNARY(std::ranges::common_range<nvec4>);
	}

	TEST_CASE("type traits for view_vector")
	{
		using nvec4 = dsga::view_vector<double, 4u>;
		auto nvec4_var = nvec4{};

		CHECK_UNARY_FALSE(std::is_standard_layout_v<nvec4>);
		CHECK_UNARY(std::is_default_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_default_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_copy_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_move_constructible_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_copyable_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivial_v<nvec4>);
		CHECK_UNARY(std::is_copy_assignable_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<nvec4>);
		CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<nvec4>);
		CHECK_UNARY(std::is_assignable_v<nvec4 &, nvec4 &>);
		CHECK_UNARY(std::is_assignable_v<nvec4 &, nvec4 &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<nvec4 &&, nvec4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<nvec4 &&, nvec4 &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<nvec4>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<nvec4>);

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(nvec4_var.begin())>);
		CHECK_UNARY(std::contiguous_iterator<decltype(nvec4_var.cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<nvec4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<nvec4>);
		CHECK_UNARY(std::ranges::sized_range<nvec4>);
		CHECK_UNARY_FALSE(std::ranges::view<nvec4>);
		CHECK_UNARY(std::ranges::contiguous_range<nvec4>);
		CHECK_UNARY(std::ranges::common_range<nvec4>);
	}

	TEST_CASE("type traits for indexed_vector")
	{
		using dswizzle1 = dsga::indexed_vector<true, double, 1u, 1u, 0u>;
		using dswizzle4 = dsga::indexed_vector<true, double, 4u, 4u, 0u, 1u, 2u, 3u>;

		CHECK_UNARY(std::is_standard_layout_v<dswizzle4>);
		CHECK_UNARY(std::is_default_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_copyable_v<dswizzle4>);
		CHECK_UNARY(std::is_trivial_v<dswizzle4>);
		CHECK_UNARY(std::is_copy_assignable_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<dswizzle4>);
		CHECK_UNARY(std::is_assignable_v<dswizzle4 &, dswizzle4 &>);
		CHECK_UNARY(std::is_assignable_v<dswizzle4 &, dswizzle4 &&>);

		// this is true for rvalue indexed_vector assigned from other indexed_vector, just not from vector_base.
		// this needs to be true for std::swap() on dsga::basic_vector to work -- dsga::indexed_vector must be std::is_move_assignable_v<>
		CHECK_UNARY(std::is_assignable_v<dswizzle4 &&, dswizzle4 &>);
		CHECK_UNARY(std::is_assignable_v<dswizzle4 &&, dswizzle4 &&>);

		CHECK_UNARY(std::is_trivially_destructible_v<dswizzle4>);

		CHECK_UNARY(std::is_aggregate_v<dswizzle1>);
		CHECK_UNARY(std::is_aggregate_v<dswizzle4>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<dswizzle4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<dswizzle4>);
		CHECK_UNARY(std::ranges::sized_range<dswizzle4>);
		CHECK_UNARY_FALSE(std::ranges::view<dswizzle4>);
		CHECK_UNARY(std::ranges::random_access_range<dswizzle4>);
		CHECK_UNARY_FALSE(std::ranges::contiguous_range<dswizzle4>);
		CHECK_UNARY(std::ranges::common_range<dswizzle4>);
	}

	TEST_CASE("type traits for indexed_view")
	{
		using dswizzle1 = dsga::indexed_view<true, double, 1u, 1u, 0u>;
		using dswizzle4 = dsga::indexed_view<true, double, 4u, 4u, 0u, 1u, 2u, 3u>;

		CHECK_UNARY(std::is_standard_layout_v<dswizzle4>);
		CHECK_UNARY_FALSE(std::is_default_constructible_v<dswizzle4>);
		CHECK_UNARY_FALSE(std::is_trivially_constructible_v<dswizzle4>);
		CHECK_UNARY_FALSE(std::is_trivially_default_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<dswizzle4>);
		CHECK_UNARY(std::is_trivially_copyable_v<dswizzle4>);

#if !defined(__GNUC__)
		CHECK_UNARY_FALSE(std::is_trivial_v<dswizzle4>);
#endif

		CHECK_UNARY_FALSE(std::is_copy_assignable_v<dswizzle4>);
		CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<dswizzle4>);
		CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<dswizzle4>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dswizzle4 &, dswizzle4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dswizzle4 &, dswizzle4 &&>);

		// this is true for rvalue indexed_vector assigned from other indexed_vector, just not from vector_base.
		// this needs to be true for std::swap() on dsga::basic_vector to work -- dsga::indexed_vector must be std::is_move_assignable_v<>
		CHECK_UNARY_FALSE(std::is_assignable_v<dswizzle4 &&, dswizzle4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dswizzle4 &&, dswizzle4 &&>);

		CHECK_UNARY(std::is_trivially_destructible_v<dswizzle4>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<dswizzle1>);
		CHECK_UNARY_FALSE(std::is_aggregate_v<dswizzle4>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<dswizzle4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<dswizzle4>);
		CHECK_UNARY(std::ranges::sized_range<dswizzle4>);
		CHECK_UNARY_FALSE(std::ranges::view<dswizzle4>);
		CHECK_UNARY(std::ranges::random_access_range<dswizzle4>);
		CHECK_UNARY_FALSE(std::ranges::contiguous_range<dswizzle4>);
		CHECK_UNARY(std::ranges::common_range<dswizzle4>);
	}

	TEST_CASE("type traits for indexed_vector_const_iterator")
	{
		using const_iter_t = dsga::indexed_vector_const_iterator<true, double, 4, 4, 3, 0, 1, 2>;

		CHECK_UNARY(std::is_standard_layout_v<const_iter_t>);
		CHECK_UNARY(std::is_default_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_copyable_v<const_iter_t>);
		CHECK_UNARY(std::is_trivial_v<const_iter_t>);
		CHECK_UNARY(std::is_copy_assignable_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<const_iter_t>);
		CHECK_UNARY(std::is_assignable_v<const_iter_t &, const_iter_t &>);
		CHECK_UNARY(std::is_assignable_v<const_iter_t &, const_iter_t &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<const_iter_t &&, const_iter_t &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<const_iter_t &&, const_iter_t &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<const_iter_t>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<const_iter_t>);

		CHECK_UNARY(std::random_access_iterator<const_iter_t>);
		CHECK_UNARY_FALSE(std::contiguous_iterator<const_iter_t>);
	}

	TEST_CASE("type traits for indexed_vector_iterator")
	{
		using iter_t = dsga::indexed_vector_iterator<true, double, 4, 4, 3, 0, 1, 2>;

		CHECK_UNARY(std::is_standard_layout_v<iter_t>);
		CHECK_UNARY(std::is_default_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_copyable_v<iter_t>);
		CHECK_UNARY(std::is_trivial_v<iter_t>);
		CHECK_UNARY(std::is_copy_assignable_v<iter_t>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<iter_t>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<iter_t>);
		CHECK_UNARY(std::is_assignable_v<iter_t &, iter_t &>);
		CHECK_UNARY(std::is_assignable_v<iter_t &, iter_t &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<iter_t &&, iter_t &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<iter_t &&, iter_t &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<iter_t>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<iter_t>);

		CHECK_UNARY(std::random_access_iterator<iter_t>);
		CHECK_UNARY_FALSE(std::contiguous_iterator<iter_t>);
	}

	TEST_CASE("type traits for indexed_view_const_iterator")
	{
		using const_iter_t = dsga::indexed_view_const_iterator<true, double, 4, 4, 3, 0, 1, 2>;

		CHECK_UNARY(std::is_standard_layout_v<const_iter_t>);
		CHECK_UNARY(std::is_default_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_copyable_v<const_iter_t>);
		CHECK_UNARY(std::is_trivial_v<const_iter_t>);
		CHECK_UNARY(std::is_copy_assignable_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<const_iter_t>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<const_iter_t>);
		CHECK_UNARY(std::is_assignable_v<const_iter_t &, const_iter_t &>);
		CHECK_UNARY(std::is_assignable_v<const_iter_t &, const_iter_t &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<const_iter_t &&, const_iter_t &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<const_iter_t &&, const_iter_t &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<const_iter_t>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<const_iter_t>);

		CHECK_UNARY(std::random_access_iterator<const_iter_t>);
		CHECK_UNARY_FALSE(std::contiguous_iterator<const_iter_t>);
	}

	TEST_CASE("type traits for indexed_view_iterator")
	{
		using iter_t = dsga::indexed_view_iterator<true, double, 4, 4, 3, 0, 1, 2>;

		CHECK_UNARY(std::is_standard_layout_v<iter_t>);
		CHECK_UNARY(std::is_default_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<iter_t>);
		CHECK_UNARY(std::is_trivially_copyable_v<iter_t>);
		CHECK_UNARY(std::is_trivial_v<iter_t>);
		CHECK_UNARY(std::is_copy_assignable_v<iter_t>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<iter_t>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<iter_t>);
		CHECK_UNARY(std::is_assignable_v<iter_t &, iter_t &>);
		CHECK_UNARY(std::is_assignable_v<iter_t &, iter_t &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<iter_t &&, iter_t &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<iter_t &&, iter_t &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<iter_t>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<iter_t>);

		CHECK_UNARY(std::random_access_iterator<iter_t>);
		CHECK_UNARY_FALSE(std::contiguous_iterator<iter_t>);
	}

	TEST_CASE("type traits for basic_matrix")
	{
		using dmat4 = dsga::basic_matrix<double, 4u, 4u>;
		auto dmat4_var = dmat4{};

		CHECK_UNARY(std::is_standard_layout_v<dmat4>);
		CHECK_UNARY(std::is_default_constructible_v<dmat4>);
		CHECK_UNARY(std::is_trivially_constructible_v<dmat4>);
		CHECK_UNARY(std::is_trivially_default_constructible_v<dmat4>);
		CHECK_UNARY(std::is_trivially_copy_constructible_v<dmat4>);
		CHECK_UNARY(std::is_trivially_move_constructible_v<dmat4>);
		CHECK_UNARY(std::is_trivially_copyable_v<dmat4>);
		CHECK_UNARY(std::is_trivial_v<dmat4>);
		CHECK_UNARY(std::is_copy_assignable_v<dmat4>);
		CHECK_UNARY(std::is_trivially_copy_assignable_v<dmat4>);
		CHECK_UNARY(std::is_trivially_move_assignable_v<dmat4>);
		CHECK_UNARY(std::is_assignable_v<dmat4 &, dmat4 &>);
		CHECK_UNARY(std::is_assignable_v<dmat4 &, dmat4 &&>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dmat4 &&, dmat4 &>);
		CHECK_UNARY_FALSE(std::is_assignable_v<dmat4 &&, dmat4 &&>);
		CHECK_UNARY(std::is_trivially_destructible_v<dmat4>);

		CHECK_UNARY_FALSE(std::is_aggregate_v<dmat4>);

		// iterator concepts
		CHECK_UNARY(std::contiguous_iterator<decltype(dmat4_var.begin())>);
		CHECK_UNARY(std::contiguous_iterator<decltype(dmat4_var.cbegin())>);

		// ranges concepts
		CHECK_UNARY(std::ranges::range<dmat4>);
		CHECK_UNARY_FALSE(std::ranges::borrowed_range<dmat4>);
		CHECK_UNARY(std::ranges::sized_range<dmat4>);
		CHECK_UNARY_FALSE(std::ranges::view<dmat4>);
		CHECK_UNARY(std::ranges::contiguous_range<dmat4>);
		CHECK_UNARY(std::ranges::common_range<dmat4>);
	}

	TEST_CASE("type traits for common initial sequence for anonymous union")
	{
		// for our vector and swizzling, we need to rely on union and the common initial sequence.
		// anything written in the union via a union member that shares a common initial sequence with
		// another union member can be referenced via any of the shared common initial sequence union
		// members, regardless of whether the member last wrote to the union or not. this is the exception
		// to the normal rule, where you can't read from a union member unless that was the last one that
		// wrote through to the union.
		//
		// for our vector and swizzling, we want all union data members to share the same common intial
		// sequence. it was unclear to me if we just want something there representing the type that isn't
		// really used for normal reference, but after having implemented this, it is essential to have this
		// or at least one member of the union to be the standard goto member. this is important if you want
		// to have constexpr instances of the class: you have to initialize one and only one member, and so
		// this first placeholder is a good choice. it is also how the class can internally modify the data.
		//
		// I have seen many unions that want to have the common intial sequence benefit and have the first
		// union member be a dummy member of the common type. the problem lies in that the other union
		// members use that type inside them, as they are often a struct with the data member in it. so when
		// this first thing is not in a struct, it should not be considered a part of the shared common
		// initial sequence. to make it be a part of it, then it too needs to be in a struct.
		//
		// this information comes from what I have gathered from these links:
		//
		// https://www.reddit.com/r/cpp_questions/comments/7ktrrj/language_lawyers_unions_and_common_initial/
		// https://stackoverflow.com/questions/43655657/union-common-initial-sequence-with-primitive
		// https://stackoverflow.com/questions/48209179/do-scalar-members-in-a-union-count-towards-the-common-initial-sequence
		// https://stackoverflow.com/questions/48058545/are-there-any-guarantees-for-unions-that-contain-a-wrapped-type-and-the-type-its

#if defined(__cpp_lib_is_layout_compatible)

		// proof that we are using the common initial sequence ***properly*** by introducing
		// dsga::base_wrapper<> for the anonymous union instead of just adding a std::array<>:

		CHECK_UNARY(std::is_corresponding_member(&array_storage_a::i, &array_storage_b::j));		// using two structs of the same type form
		CHECK_UNARY_FALSE(std::is_corresponding_member(&array_storage_a::i, &faux_vector::k));		// analogous to using std::array<> and dsga::indexed_vector<> at same level of anonymous union
		CHECK_UNARY(std::is_corresponding_member(&faux_wrapper::l, &faux_vector::k));	// analogous to using dsga::base_wrapper<> and dsga::indexed_vector<> at same level of anonymous union

		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 1>::store, &dsga::indexed_vector<true, int, 1, 1, 0>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 2>::store, &dsga::indexed_vector<true, int, 2, 2, 1, 0>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 3>::store, &dsga::indexed_vector<true, int, 3, 3, 2, 0, 1>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 4>::store, &dsga::indexed_vector<true, int, 4, 1, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 4>::store, &dsga::indexed_vector<true, int, 4, 2, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 4>::store, &dsga::indexed_vector<true, int, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::storage_wrapper<true, int, 4>::store, &dsga::indexed_vector<true, int, 4, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<true, int, 4, 1, 3>::base, &dsga::indexed_vector<true, int, 4, 2, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<true, int, 4, 1, 3>::base, &dsga::indexed_vector<true, int, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<true, int, 4, 1, 3>::base, &dsga::indexed_vector<true, int, 4, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<true, int, 4, 2, 3, 3>::base, &dsga::indexed_vector<true, int, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<true, int, 4, 2, 3, 3>::base, &dsga::indexed_vector<true, int, 4, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_vector<true, int, 4, 3, 3, 3, 3>::base, &dsga::indexed_vector<true, int, 4, 4, 3, 3, 3, 3>::base));

		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 1>::store, &dsga::indexed_view<true, int, 1, 1, 0>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 2>::store, &dsga::indexed_view<true, int, 2, 2, 1, 0>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 3>::store, &dsga::indexed_view<true, int, 3, 3, 2, 0, 1>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 4>::store, &dsga::indexed_view<true, int, 4, 1, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 4>::store, &dsga::indexed_view<true, int, 4, 2, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 4>::store, &dsga::indexed_view<true, int, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::view_wrapper<true, int, 4>::store, &dsga::indexed_view<true, int, 4, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_view<true, int, 4, 1, 3>::base, &dsga::indexed_view<true, int, 4, 2, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_view<true, int, 4, 1, 3>::base, &dsga::indexed_view<true, int, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_view<true, int, 4, 1, 3>::base, &dsga::indexed_view<true, int, 4, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_view<true, int, 4, 2, 3, 3>::base, &dsga::indexed_view<true, int, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_view<true, int, 4, 2, 3, 3>::base, &dsga::indexed_view<true, int, 4, 4, 3, 3, 3, 3>::base));
		CHECK_UNARY(std::is_corresponding_member(&dsga::indexed_view<true, int, 4, 3, 3, 3, 3>::base, &dsga::indexed_view<true, int, 4, 4, 3, 3, 3, 3>::base));

#endif
	}
}
