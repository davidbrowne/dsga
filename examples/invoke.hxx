
//          Copyright David Browne 2024-2025.
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

		template <typename T, std::size_t C, std::size_t R>
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
			indexer(const T &val, std::size_t c) noexcept : t(val), C(c)
			{
			}

			auto &operator []([[ maybe_unused ]] std::size_t index)
			{
				// check for valid index based on return vector size
				if (index >= C)
				{
					throw std::out_of_range("index out of range");
				}

				// a matrix can technically index, but we want matrices
				// to be passed down whole and not just a column of a matrix
				if constexpr (can_index<T> && !is_dsga_matrix_v<T>)
				{
					// check for valid index based on indexable size
					if (index >= t.size())
					{
						throw std::out_of_range("index out of range");
					}

					if constexpr (has_at<T>)
					{
						return t.at(index);
					}
					else
					{
						return t[index];
					}
				}
				else
				{
					return t;
				}
			}

			private:
			const T &t;
			std::size_t C;
		};
	}

	// not in GLSL
	// return a vector created by invoking an operation element-wise to a variable number of arguments (there must be at least 1)
	// that are either dimensional_scalar or basic_matrix --  if an argument isn't a vector of dimensional_scalars of length C,
	// it can be a single scalar that will be used C times (the length of the return vector) -- similarly, if an argument is a
	// matrix, it will be used C times (the length of the return vector) -- usually the arguments will be vectors of the same
	// length C or a std::array or std::span of matrices of the same length C.
	//
	// the Op must return a dsga::dimensional_scalar type, since invoke() returns a dsga::basic_vector of the results.
	template <std::size_t C, typename Op, typename ...Ts>
	requires ((sizeof ...(Ts)) > 0) && (C > 1)
	[[nodiscard]] constexpr auto invoke(Op op, const Ts & ...args) noexcept
	{
		auto op_invoke = [&op, &args...](std::size_t index) { return op(invoke_detail::indexer(args, C)[index] ...); };

		return [&op_invoke]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			return basic_vector(op_invoke(Is) ...);
		}(std::make_index_sequence<C>{});
	}

	// not in GLSL
	// return a vector created by invoking an operation element-wise to a variable number of vectors (there must be at least 1)
	// that are all the same size, but might be of different types
	template <typename Op, std::size_t C, bool ...Ws, dimensional_scalar ...Ts, typename ...Ds>
	requires ((sizeof ...(Ts)) > 0) && (C > 1)
	[[nodiscard]] constexpr basic_vector<std::invoke_result_t<Op, Ts...>, C> invoke_on_vectors(Op op, const vector_base<Ws, Ts, C, Ds> & ...vectors) noexcept
	{
		return dsga::invoke<C>(op, vectors...);

//		auto op_invoke = [&op, &vectors ...](std::size_t index) { return op(vectors[index] ...); };
//
//		return [&op_invoke]<std::size_t ...Is>(std::index_sequence<Is...>)
//		{
//			return basic_vector<std::invoke_result_t<Op, Ts...>, C>{op_invoke(Is) ...};
//		}(std::make_index_sequence<C>{});
	}
}
