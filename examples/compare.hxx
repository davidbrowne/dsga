
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"

//
// performing weighted comparisons
//

namespace dsga
{
	// default weights for comparison - x has priority over y, which is over z, which is over w
	template <std::size_t C>
	requires (C >= 1 && C <= 4)
	[[nodiscard]] constexpr auto default_comparison_weights() noexcept
	{
		constexpr auto weights = basic_vector<int, 4>(1, 3, 9, 27);				// reverse order
		return[&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
		{
			return basic_vector<int, C>(weights[C - Is - 1] ...);
		}(std::make_index_sequence<C>{});
	}

	namespace detail
	{
		// helper that evaluates a binary operation lambda
		template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2, typename BinOp>
		[[nodiscard]] constexpr auto binary_op(BinOp lambda,
											   const vector_base<W1, T1, C, D1> &lhs,
											   const vector_base<W2, T2, C, D2> &rhs) noexcept
		{
			return machinery::apply_multitype_make(lhs, rhs, lambda);
		}

		// comparison lambdas that return -1 for less than, 0 for equal, and 1 for greater than.
		// these are for the types that don't work with the sign() function
		constexpr inline auto unsigned_compare_op = [](unsigned_scalar auto lhs, unsigned_scalar auto rhs) noexcept -> int
		{
			return (lhs < rhs) ? -1 : ((lhs > rhs) ? 1 : 0);
		};

		constexpr inline auto signed_unsigned_compare_op = []<signed_scalar T1, unsigned_scalar T2>(T1 lhs, T2 rhs) noexcept -> int
		{
			return (lhs < 0) ? -1 : ((static_cast<unsigned long long>(lhs) < static_cast<unsigned long long>(rhs)) ? -1 : ((static_cast<unsigned long long>(lhs) > static_cast<unsigned long long>(rhs)) ? 1 : 0));
		};

		constexpr inline auto unsigned_signed_compare_op = []<unsigned_scalar T1, signed_scalar T2>(T1 lhs, T2 rhs) noexcept -> int
		{
			return (rhs < 0) ? 1 : ((static_cast<unsigned long long>(lhs) < static_cast<unsigned long long>(rhs)) ? -1 : ((static_cast<unsigned long long>(lhs) > static_cast<unsigned long long>(rhs)) ? 1 : 0));
		};

		template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2, bool W3, numeric_integral_scalar T3, typename D3>
		requires (signed_scalar<T3>)
		[[nodiscard]] constexpr auto compare_impl(const vector_base<W1, T1, C, D1> &x,
												  const vector_base<W2, T2, C, D2> &y,
												  const vector_base<W3, T3, C, D3> &weights) noexcept
		{
			if constexpr (floating_point_scalar<T1> && floating_point_scalar<T2>)
			{
				return functions::innerProduct(weights, basic_vector<T3, C>(functions::sign(x - y)));
			}
			else if constexpr (signed_scalar<T1> && signed_scalar<T2>)
			{
				return functions::innerProduct(weights, basic_vector<T3, C>(functions::sign(x - y)));
			}
			else if constexpr (unsigned_scalar<T1> && unsigned_scalar<T2>)
			{
				return functions::innerProduct(weights, basic_vector<T3, C>(binary_op(unsigned_compare_op, x, y)));
			}
			else if constexpr (signed_scalar<T1> && unsigned_scalar<T2>)
			{
				return functions::innerProduct(weights, basic_vector<T3, C>(binary_op(signed_unsigned_compare_op, x, y)));
			}
			else if constexpr (unsigned_scalar<T1> && signed_scalar<T2>)
			{
				return functions::innerProduct(weights, basic_vector<T3, C>(binary_op(unsigned_signed_compare_op, x, y)));
			}
			else
			{
				using commontype = std::common_type_t<T1, T2>;
				return compare_impl(static_cast<dsga::basic_vector<commontype, C>>(x.as_derived()), static_cast<dsga::basic_vector<commontype, C>>(y.as_derived()), weights);
			}
		}

		// interface function for three-way comparison operator for vectors, using default weighting
		template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
		[[nodiscard]] constexpr auto compare(const vector_base<W1, T1, C, D1> &x,
											 const vector_base<W2, T2, C, D2> &y) noexcept
		{
			constexpr auto weights = default_comparison_weights<C>();
			return compare_impl(x, y, weights);
		}

		// interface function for three-way comparison operator for vectors, using user-defined weighting
		template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2, bool W3, numeric_integral_scalar T3, typename D3>
		requires signed_scalar<T3>
		[[nodiscard]] constexpr auto compare(const vector_base<W1, T1, C, D1> &x,
											 const vector_base<W2, T2, C, D2> &y,
											 const vector_base<W3, T3, C, D3> &weights) noexcept
		{
			return compare_impl(x, y, weights);
		}
	}

	//
	// non-operator weighted comparison for vectors - suggest using default_comparison_weights() or a swizzle
	// of it that uses all the components. the weights show the priority of the dimensions in the comparison.
	// 
	// returns the ordering of the the arguments based on a signed integral value, calculated using the weights
	// applied to the differences between the pair-wise components via compare_impl(). It is similar but different
	// than std::lexicographical_compare_three_way(), where that function iterates over the components, and this
	// function operates on all the components at once to arrive at values to compare. It is lexicographical
	// if the absolute value of the weights is in decreasing order, such as using default_comparison_weights<>().
	// 
	// this custom weighted comparison or the default operator <=> comparisons are important for sorting
	// vectors for algorithms, .e.g., convex hull, min-max, etc. Using the default_comparison_weights<>()
	// (or a swizzle of them where every component is used once) will give you a total ordering in the sort
	// (assuming no comparison is std::unordered()).
	// 
	// This comparison uses exact data, not fuzzy equality within a tolerance.
	//

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2, bool W3, numeric_integral_scalar T3, typename D3>
	requires signed_scalar<T3>
	[[nodiscard]] constexpr auto weighted_compare(const vector_base<W1, T1, C, D1> &first,
												  const vector_base<W2, T2, C, D2> &second,
												  const vector_base<W3, T3, C, D3> &weights) noexcept
	{
		if constexpr (std::integral<T1> && std::integral<T2>)
		{
			if (auto comp = detail::compare(first, second, weights); comp < 0)
			{
				return std::strong_ordering::less;
			}
			else if (comp > 0)
			{
				return std::strong_ordering::greater;
			}
			else
			{
				return std::strong_ordering::equal;
			}
		}
		else if constexpr (std::floating_point<T1> && std::floating_point<T2>)
		{
			// if any component of either inputs is a NaN, then it is unordered
			if (functions::any(functions::isnan(first.as_derived())) || functions::any(functions::isnan(second.as_derived())))
				return std::partial_ordering::unordered;

			if (auto comp = detail::compare(first, second, weights); comp < 0)
			{
				return std::partial_ordering::less;
			}
			else if (comp > 0)
			{
				return std::partial_ordering::greater;
			}
			else
			{
				return std::partial_ordering::equivalent;
			}
		}
		else
		{
			using commontype = std::common_type_t<T1, T2>;
			return weighted_compare(static_cast<basic_vector<commontype, C>>(first), static_cast<basic_vector<commontype, C>>(second), weights);
		}
	}

	//
	// lexicographic-like comparisons for vectors -- x has highest priority, then y, then z, then w.
	// all components are compared up front though, so it doesn't just stop checking when it finds the
	// first component that compares as not equal/equivalent.
	//
	// not in GLSL
	//

	template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
	constexpr auto operator <=>(const vector_base<W1, T1, C, D1> &first,
								const vector_base<W2, T2, C, D2> &second) noexcept
	{
		constexpr auto weights = default_comparison_weights<C>();
		return weighted_compare(first, second, weights);
	}

	//
	// lexicographic comparisons of vectors in matrices.
	// it will stop checking after first vector comparison that is not equal/equivalent.
	//

	constexpr auto mat_vec_comp_op =
		[]<floating_point_scalar T1, std::size_t C, floating_point_scalar T2>(const basic_vector<T1, C> &v1, const basic_vector<T2, C> &v2)
	{
		return v1 <=> v2;
	};

	template <floating_point_scalar T1, std::size_t C, std::size_t R, floating_point_scalar T2>
	constexpr auto operator <=>(const basic_matrix<T1, C, R> &lhs,
								const basic_matrix<T2, C, R> &rhs) noexcept
	{
		return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), mat_vec_comp_op);
	}

}
