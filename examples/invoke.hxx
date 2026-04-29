
//          Copyright David Browne 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// invoking lambdas with vector, matrix, and scalar values
//

namespace dsga
{
	//
	// support for the invoke() function
	//
	namespace invoke_detail
	{
		template <typename T>
		struct is_dsga_matrix : std::false_type	{};

		template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
		struct is_dsga_matrix<dsga::basic_matrix<T, C, R>> : std::true_type	{};

		template <typename T>
		constexpr bool is_dsga_matrix_v = is_dsga_matrix<T>::value;

		template <typename T>
		concept can_index = requires(T t, int i)
		{
			t[i];
			t.size();
		};

		template <typename T>
		concept has_at = requires (T t, int i)
		{
			t.at(i);
		};

		template <typename T>
		struct indexer
		{
			indexer(const T &value, std::size_t size) noexcept : arg(value), count(size)
			{
			}

			const auto &operator [](std::size_t index)
			{
				// check for valid index based on return vector size
				if (index >= count)
				{
					[[ unlikely ]] throw std::out_of_range("index out of range");
				}

				// a matrix can technically index, but we want matrices
				// to be passed down whole and not just a column of a matrix
				if constexpr (can_index<T> && !is_dsga_matrix_v<T>)
				{
					// check for valid index based on indexable size
					if (index >= arg.size())
					{
						[[ unlikely ]] throw std::out_of_range("index out of range");
					}

					if constexpr (has_at<T>)
					{
						[[ likely ]] return arg.at(index);
					}
					else
					{
						[[ likely ]] return arg[index];
					}
				}
				else
				{
					[[ likely ]] return arg;
				}
			}

			private:
				const T &arg;
				std::size_t count;
		};
	}

	// not in GLSL
	// return a vector created by invoking an operation element-wise to a variable number of arguments that are either
	// dimensional_scalar or basic_matrix --  if an argument isn't a vector of dimensional_scalars of length C, it can
	// be a single scalar that will be used C times (the length of the return vector) -- similarly, if an argument is
	// a matrix, it will be used C times (the length of the return vector) -- usually the arguments will be vectors of
	// the same length C or a std::array or std::span of matrices of the same length C.
	//
	// the Op must return a dsga::dimensional_scalar type, since invoke() returns a dsga::basic_vector of the results.
	// this also means that C must be greater than 1 for vectors (no longer supporting length 1 vectors)
	template <std::size_t C, typename Op, typename ...Ts>
	requires (C >= 1) && (C <= 4)
	[[nodiscard]] constexpr auto invoke(Op &&op, Ts && ...args) noexcept(std::is_nothrow_invocable_v<Op, Ts...>)
	{
		auto op_invoke = [&op, &args...]([[ maybe_unused ]] std::size_t index)
		{
			return std::forward<Op>(op)(invoke_detail::indexer(std::forward<Ts>(args), C)[index] ...);
		};

		return [&op_invoke]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			// using "{" and "}" for the constructor evaluates arguments left-to-right
			return basic_vector{op_invoke(Is) ...};
		}(std::make_index_sequence<C>{});
	}
}
