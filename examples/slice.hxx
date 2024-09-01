//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// slice functions
//

namespace dsga
{

	// 
	// The slice() function gives you a contiguous view on a sub-range of the data object.
	// It works for arguments of type basic_vector, basic_view, and vector_view. It returns
	// a basic_view into the same data of the underlying vector/view. There is a template
	// argument that tells how many of the vector/view elements are in the sub-range. The
	// function also takes an offset argument on where in the vector/view to start the return
	// sub-range. If the combination of the length of the new basic_view and the offset are
	// illegal, i.e., they would lead to buffer overrun, then it will assert and throw an
	// exception.
	// 
	// The returned basic_view holds a pointer to the underlying data. It therefore has a
	// lifetime that depends on the vector/view argument. Make sure that the basic_view
	// that is returned ends its lifetime or use before the vector/view argument's lifetime
	// is over, otherwise it will have a dangling pointer.
	//
	// Not in GLSL
	//

	// basic_vector

	// delete rvalue reference version -- we don't want to have a dangling pointer
	template <std::size_t Length, dimensional_scalar T, std::size_t S>
	auto slice(basic_vector<T, S> &&v, std::size_t offset) = delete;

	// pass in a non-const vector
	template <std::size_t Length, dimensional_scalar T, std::size_t S>
	requires (Length <= S) && (0 < Length)
	auto slice(basic_vector<T, S> &v, std::size_t offset)
	{
		bool length_valid = Length <= S;
		bool offset_valid = offset < S;
		bool length_and_offset_valid = (offset + Length) <= S;
		dsga_constexpr_assert(length_valid, "Length is longer than vector length");
		dsga_constexpr_assert(offset_valid, "offset is longer than end index of vector");
		dsga_constexpr_assert(length_and_offset_valid, "Length + offset is longer than vector length");

		if (!length_valid || !offset_valid || !length_and_offset_valid)
			throw std::out_of_range("slice() Length with offset are out of range");

		return basic_view<true, T, Length>(v.data() + offset);
	}

	// pass in a const vector, create a non-const vector that is internally const
	template <std::size_t Length, dimensional_scalar T, std::size_t S>
	requires (Length <= S) && (0 < Length)
	auto slice(const basic_vector<T, S> &v, std::size_t offset)
	{
		bool length_valid = Length <= S;
		bool offset_valid = offset < S;
		bool length_and_offset_valid = (offset + Length) <= S;
		dsga_constexpr_assert(length_valid, "Length is longer than vector length");
		dsga_constexpr_assert(offset_valid, "offset is longer than end index of vector");
		dsga_constexpr_assert(length_and_offset_valid, "Length + offset is longer than vector length");

		if (!length_valid || !offset_valid || !length_and_offset_valid)
			throw std::out_of_range("slice() Length with offset are out of range");

		return basic_view<false, T, Length>(v.data() + offset);
	}

	// view_vector

	// delete rvalue reference version -- we don't want to have a dangling pointer
	template <std::size_t Length, dimensional_scalar T, std::size_t S>
	auto slice(view_vector<T, S> &&v, std::size_t offset) = delete;

	// pass in a non-const vector
	template <std::size_t Length, dimensional_scalar T, std::size_t S>
	requires (Length <= S) && (0 < Length)
	auto slice(view_vector<T, S> &v, std::size_t offset)
	{
		bool length_valid = Length <= S;
		bool offset_valid = offset < S;
		bool length_and_offset_valid = (offset + Length) <= S;
		dsga_constexpr_assert(length_valid, "Length is longer than vector length");
		dsga_constexpr_assert(offset_valid, "offset is longer than end index of vector");
		dsga_constexpr_assert(length_and_offset_valid, "Length + offset is longer than vector length");

		if (!length_valid || !offset_valid || !length_and_offset_valid)
			throw std::out_of_range("slice() Length with offset are out of range");

		return basic_view<true, T, Length>(v.data() + offset);
	}

	// pass in a const vector, create a non-const vector that is internally const
	template <std::size_t Length, dimensional_scalar T, std::size_t S>
	requires (Length <= S) && (0 < Length)
	auto slice(const view_vector<T, S> &v, std::size_t offset)
	{
		bool length_valid = Length <= S;
		bool offset_valid = offset < S;
		bool length_and_offset_valid = (offset + Length) <= S;
		dsga_constexpr_assert(length_valid, "Length is longer than vector length");
		dsga_constexpr_assert(offset_valid, "offset is longer than end index of vector");
		dsga_constexpr_assert(length_and_offset_valid, "Length + offset is longer than vector length");

		if (!length_valid || !offset_valid || !length_and_offset_valid)
			throw std::out_of_range("slice() Length with offset are out of range");

		return basic_view<false, T, Length>(v.data() + offset);
	}

	// basic_view

	// delete rvalue reference version -- we don't want to have a dangling pointer
	template <std::size_t Length, bool M, dimensional_scalar T, std::size_t S>
	auto slice(basic_view<M, T, S> &&v, std::size_t offset) = delete;

	// pass in a non-const view, but it might be internally const depending on M
	template <std::size_t Length, bool M, dimensional_scalar T, std::size_t S>
	requires (Length <= S) && (0 < Length)
	auto slice(basic_view<M, T, S> &v, std::size_t offset)
	{
		bool length_valid = Length <= S;
		bool offset_valid = offset < S;
		bool length_and_offset_valid = (offset + Length) <= S;
		dsga_constexpr_assert(length_valid, "Length is longer than view length");
		dsga_constexpr_assert(offset_valid, "offset is longer than end index of view");
		dsga_constexpr_assert(length_and_offset_valid, "Length + offset is longer than view length");

		if (!length_valid || !offset_valid || !length_and_offset_valid)
			throw std::out_of_range("slice() Length with offset are out of range");

		return basic_view<M, T, Length>(v.data() + offset);
	}

	// pass in a const view, create a non-const view that is internally const
	template <std::size_t Length, bool M, dimensional_scalar T, std::size_t S>
	requires (Length <= S) && (0 < Length)
	auto slice(const basic_view<M, T, S> &v, std::size_t offset)
	{
		bool length_valid = Length <= S;
		bool offset_valid = offset < S;
		bool length_and_offset_valid = (offset + Length) <= S;
		dsga_constexpr_assert(length_valid, "Length is longer than view length");
		dsga_constexpr_assert(offset_valid, "offset is longer than end index of view");
		dsga_constexpr_assert(length_and_offset_valid, "Length + offset is longer than view length");

		if (!length_valid || !offset_valid || !length_and_offset_valid)
			throw std::out_of_range("slice() Length with offset are out of range");

		return basic_view<false, T, Length>(v.data() + offset);
	}

}
