//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// opening include guard
#if !defined(DSGA_DSGA_HXX)
#define DSGA_DSGA_HXX

#include <type_traits>				// requirements
#include <concepts>					// requirements
#include <array>					// underlying storage
#include <tuple>					// tuple interface for structured bindings
#include <algorithm>				// min()
#include <span>						// external types to/from vectors
#include <stdexcept>
#include <bit>						// bit_cast

//
// Data Structures for Geometric Algebra (dsga)
//

// version info

constexpr inline int DSGA_MAJOR_VERSION = 0;
constexpr inline int DSGA_MINOR_VERSION = 2;
constexpr inline int DSGA_PATCH_VERSION = 0;

namespace dsga
{
	// plain undecorated arithmetic types
	template <typename T>
	concept dimensional_scalar = std::is_arithmetic_v<T> && std::same_as<T, std::remove_cvref_t<T>>;

	// plain undecorated integral types
	template <typename T>
	concept integral_dimensional_scalar = std::integral<T> && dimensional_scalar<T>;

	// want the size to be between 1 and 4, inclusive
	template <std::size_t Size>
	concept dimensional_size = ((Size > 0u) && (Size <= 4u));

	// dimensional storage needs the arithmetic type and size restrictions
	template <typename ScalarType, std::size_t Size>
	concept dimensional_storage = dimensional_size<Size> && dimensional_scalar<ScalarType>;

	// we want dimensional_storage_t to have length from 1 to 4 (1 gives just a sneaky kind of ScalarType that can swizzle),
	// and the storage has to have room for all the data. We also need dimensional_storage_t to support operator[] to access
	// the data. It needs to also support iterators so we can use it in ranged-for loops, algorithms, etc.

	// the underlying storage for vector and indexed_vector types. originally this was to be a template parameter and
	// fairly generic, but that is a detail that can happen in a future version of this library. it makes things
	// much simpler to not have to pass this stuff around in template parameters.

	// as alluded to above, dimensional_storage_t has two roles: 1) the storage in a vector for each dimension of
	// the vector, and 2) the backing storage used by a swizzle of a vector (storage is in that vector), that
	// is used to index into as required by the swizzle.

	// this implementation uses std::array as the backing storage type.

	template <dimensional_scalar ScalarType, std::size_t Size>
	requires dimensional_storage<ScalarType, Size>
	using dimensional_storage_t = std::array<ScalarType, Size>;


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

	// common initial sequence wrapper with basic storage access -- forwards function calls to wrapped storage
	template <dimensional_scalar ScalarType, std::size_t Size>
	requires dimensional_storage<ScalarType, Size>
	struct storage_wrapper
	{
		dimensional_storage_t<ScalarType, Size> value;

		constexpr		std::size_t	size()							const	noexcept	{ return Size; }

		// physically contiguous access to data
		constexpr		ScalarType	&operator [](std::size_t index)			noexcept	{ return value[index]; }
		constexpr const ScalarType  &operator [](std::size_t index)	const	noexcept	{ return value[index]; }

		template <dimensional_scalar ...Args>
		requires (sizeof...(Args) == Size)
		constexpr		void		set(Args ...args)						noexcept	{ set_impl(std::make_index_sequence<Size>{}, args...); }

		// support for range-for loop
		constexpr		auto		begin()									noexcept	{ return value.begin(); }
		constexpr		auto		begin()							const	noexcept	{ return value.cbegin(); }
		constexpr		auto		cbegin()						const	noexcept	{ return value.cbegin(); }
		constexpr		auto		end()									noexcept	{ return value.end(); }
		constexpr		auto		end()							const	noexcept	{ return value.cend(); }
		constexpr		auto		cend()							const	noexcept	{ return value.cend(); }

		// details
		private:
			template <dimensional_scalar ...Args, std::size_t ...Is>
			requires (sizeof...(Args) == Size) && (sizeof...(Is) == Size)
			constexpr void set_impl(std::index_sequence<Is...> /* dummy */,
									Args ...args) noexcept { ((value[Is] = static_cast<ScalarType>(args)),...); }

	};


	namespace detail
	{
		// the concepts will help indexed_vector determine if it can be assigned to, like an lvalue reference,
		// i.e., if all indexes are unique then it can be used as an lvalue reference.

		// see if all the std::size_t index values are unique

		template <std::size_t ...Indexes>
		struct unique_indexes_impl;

		template <>
		struct unique_indexes_impl<> : std::true_type
		{
		};

		template <std::size_t Index>
		struct unique_indexes_impl<Index> : std::true_type
		{
		};

		template <std::size_t FirstIndex, std::size_t ...RestIndexes>
		struct unique_indexes_impl<FirstIndex, RestIndexes...>
		{
			static constexpr bool value = ((FirstIndex != RestIndexes) && ...) && unique_indexes_impl<RestIndexes...>::value;
		};

		// all Index values must be in ranged [0, Size) -- not checking here about Size and number of Indexes

		template <std::size_t Size, std::size_t ...Indexes>
		struct valid_indexes_impl
		{
			static constexpr bool value = ((Indexes < Size) && ...);
		};

	}

	// concepts required to build concept for testing for lvalue swizzle indexes

	template <std::size_t ...Indexes>
	concept unique_indexes = (sizeof...(Indexes) > 0) && detail::unique_indexes_impl<Indexes...>::value;

	template <std::size_t IndexCount, std::size_t ...Indexes>
	concept valid_index_count = (sizeof...(Indexes) == IndexCount) && (IndexCount > 0) && (IndexCount <= 4u);

	template <std::size_t Size, std::size_t ...Indexes>
	concept valid_range_indexes = detail::valid_indexes_impl<Size, Indexes...>::value;

	// lvalue_swizzle_indexes can determine whether a particular indexed_vector can be used as an lvalue reference

	template <std::size_t Size, std::size_t IndexCount, std::size_t ...Indexes>
	constexpr inline bool lvalue_swizzle_indexes = valid_index_count<IndexCount, Indexes...> && unique_indexes<Indexes...> && valid_range_indexes<Size, Indexes...>;

	//
	// helper template functions to determine whether implicit conversions are allowed
	//

	template <typename T>
	concept non_bool_arithmetic = std::is_arithmetic_v<std::remove_cvref_t<T>> && !std::same_as<std::remove_cvref_t<T>, bool>;

	template <typename T, typename U>
	concept latter_same_as_common_type =
	requires
	{
		typename std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
		requires std::same_as<std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>, std::remove_cvref_t<U>>;
	};

	template <typename T, typename U>
	concept implicitly_convertible_to = non_bool_arithmetic<T> && non_bool_arithmetic<U> && latter_same_as_common_type<T, U>;

	template <typename T, typename U>
	concept same_sizeof = (sizeof(T) == sizeof(U));


	// there is no use for this enum, it is meant as FEO (For Exposition Only). we will separate domains by the names of the swizzle union
	// members we create, as opposed to using this enum class as a template parameter. we only intend to implement the xyzw swizzle accessors.
	// if we intend to implement the other swizzle mask sets, then values of this enum class would come in handy as a template parameter (can we
	// use those in NTTPs?), as we are not allowed to mix and match accessors from different mask sets.
	enum class swizzle_mask_sets
	{
		xyzw,						// spatial points and normals
		rgba,						// colors
		stpq						// texture coordinates
	};

	// this is a CRTP struct that will help us with operators, compound assignment operators, and functions.
	//
	// It provides:
	// 
	// 		set() - relies on init() in Derived
	// 		operator[] - relies on at() in Derived
	// 		size() - relies on Count template parameter
	//
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires dimensional_storage<ScalarType, Count>
	struct vector_base
	{
		// CRTP access to Derived class
		constexpr		Derived		&as_derived()							noexcept	requires Writable	{ return static_cast<Derived &>(*this); }
		constexpr const Derived		&as_derived()					const	noexcept						{ return static_cast<const Derived &>(*this); }

		// logically contiguous write access to all data that allows for self-assignment that works properly
		template <dimensional_scalar ...Args>
		requires Writable && (sizeof...(Args) == Count)
		constexpr		void		set(Args ...args)						noexcept						{ this->as_derived().init(args...); }

		// logically contiguous access to piecewise data as index goes from 0 to (Count - 1)
		constexpr		ScalarType	&operator [](std::size_t index)			noexcept	requires Writable	{ return this->as_derived().at(index); }
		constexpr const	ScalarType	&operator [](std::size_t index) const	noexcept						{ return this->as_derived().at(index); }

		// number of accessible ScalarType elements
		constexpr		std::size_t	size()							const	noexcept						{ return Count; }
	};

	// basic_vector will act as the primary vector class in this library.
	//
	// Size is number of elements referencable in vector/storage
	// ScalarType is the type of the elements stored in the vector/storage

	template <dimensional_scalar ScalarType, std::size_t Size>
	requires dimensional_storage<ScalarType, Size>
	struct basic_vector;

	// indexed_vector will act as a swizzle of a basic_vector. basic_vector relies on the anonymous union of indexed_vector data members.
	//
	// Size relates to the number of elements in the underlying storage, which informs the values the Indexes can hold
	// ScalarType is the type of the elements stored in the underlying storage
	// IndexCount is the number of elements accessible in swizzle -- often works alongside with basic_vector's Size
	// Indexes... are the number of swizzlable values available -- there are IndexCount of them, and their values are in the range:  0 <= Indexes < Size

	// we want indexed_vector (vector swizzles) to have length from 1 to 4 (1 is just a sneaky type of ScalarType swizzle) in order
	// to work with the basic_vector which also has these lengths. The number of indexes is the same as the IndexCount, between 1 and 4.
	// The indexes are valid for indexing into the values in the storage which is Size big.

	template <typename ScalarType, std::size_t Size, std::size_t IndexCount, std::size_t ...Indexes>
	concept indexable = dimensional_storage<ScalarType, Size> && valid_index_count<IndexCount, Indexes...> && valid_range_indexes<Size, Indexes...>;

	template <typename ScalarType, std::size_t Size, std::size_t IndexCount, std::size_t ...Indexes>
	requires indexable <ScalarType, Size, IndexCount, Indexes...>
	struct indexed_vector;

	//
	// iterators for indexed_vector so it can participate in range-for loop
	//

	template <dimensional_scalar ScalarType, std::size_t Size, std::size_t IndexCount, std::size_t ... Indexes>
	requires indexable <ScalarType, Size, IndexCount, Indexes...>
	struct indexed_vector_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::forward_iterator_tag;
		using value_type = ScalarType;
		using difference_type = int;
		using pointer = ScalarType *;
		using reference = ScalarType &;

		constexpr static std::size_t begin_index = 0;
		constexpr static std::size_t end_index = IndexCount;

		// the data
		indexed_vector<ScalarType, Size, IndexCount, Indexes ...> *mapper_ptr;
		std::size_t mapper_index;

		// index == 0 is begin iterator
		// index == IndexCount is end iterator -- clamp index in [0, IndexCount] range
		constexpr indexed_vector_iterator(indexed_vector<ScalarType, Size, IndexCount, Indexes ...> &mapper, std::size_t index) noexcept
		{
			mapper_ptr = std::addressof(mapper);
			mapper_index = (index > IndexCount) ? IndexCount : index;			// std::size_t, so don't need lower bound check
		}

		indexed_vector_iterator(const indexed_vector_iterator &) noexcept = default;
		indexed_vector_iterator(indexed_vector_iterator &&) noexcept = default;
		indexed_vector_iterator &operator =(const indexed_vector_iterator &) noexcept = default;
		indexed_vector_iterator &operator =(indexed_vector_iterator &&) noexcept = default;

		constexpr reference operator *() noexcept
		{
			return (*mapper_ptr)[mapper_index];
		}

		constexpr value_type operator *() const noexcept
		{
			return (*mapper_ptr)[mapper_index];
		}

		constexpr indexed_vector_iterator& operator++() noexcept
		{
			if (mapper_index < IndexCount)
				mapper_index++;
			return *this;
		}

		constexpr indexed_vector_iterator operator++(int) noexcept
		{
			indexed_vector_iterator temp = *this;
			if (mapper_index < IndexCount)
				mapper_index++;
			return temp;
		}

		constexpr bool operator ==(const indexed_vector_iterator &other) const noexcept
		{
			return (this->mapper_ptr == other.mapper_ptr) && (this->mapper_index == other.mapper_index);
		}
	};

	template <dimensional_scalar ScalarType, std::size_t Size, std::size_t IndexCount, std::size_t ... Indexes>
	requires indexable <ScalarType, Size, IndexCount, Indexes...>
	struct indexed_vector_const_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::forward_iterator_tag;
		using value_type = ScalarType;
		using difference_type = int;
		using pointer = const ScalarType *;
		using reference = const ScalarType &;

		constexpr static std::size_t begin_index = 0;
		constexpr static std::size_t end_index = IndexCount;

		// the data
		const indexed_vector<ScalarType, Size, IndexCount, Indexes ...> *mapper_ptr;
		std::size_t mapper_index;

		// index == 0 is begin iterator
		// index == IndexCount is end iterator -- clamp index in [0, IndexCount] range
		constexpr indexed_vector_const_iterator(const indexed_vector<ScalarType, Size, IndexCount, Indexes ...> &mapper, std::size_t index) noexcept
		{
			mapper_ptr = std::addressof(mapper);
			mapper_index = (index > IndexCount) ? IndexCount : index;			// std::size_t, so don't need lower bound check
		}

		indexed_vector_const_iterator(const indexed_vector_const_iterator &) noexcept = default;
		indexed_vector_const_iterator(indexed_vector_const_iterator &&) noexcept = default;
		indexed_vector_const_iterator &operator =(const indexed_vector_const_iterator &) noexcept = default;
		indexed_vector_const_iterator &operator =(indexed_vector_const_iterator &&) noexcept = default;

		constexpr reference operator *() noexcept
		{
			return (*mapper_ptr)[mapper_index];
		}

		constexpr value_type operator *() const noexcept
		{
			return (*mapper_ptr)[mapper_index];
		}

	   constexpr indexed_vector_const_iterator& operator++() noexcept
		{
			if (mapper_index < IndexCount)
				mapper_index++;
			return *this;
		}

		constexpr indexed_vector_const_iterator operator++(int) noexcept
		{
			indexed_vector_const_iterator temp = *this;
			if (mapper_index < IndexCount)
				mapper_index++;
			return temp;
		}

		constexpr bool operator ==(const indexed_vector_const_iterator &other) const noexcept
		{
			return (this->mapper_ptr == other.mapper_ptr) && (this->mapper_index == other.mapper_index);
		}
	};

	// for swizzling 1D parts of dimension_data - like a scalar accessor
	template <dimensional_scalar ScalarType, std::size_t Size, std::size_t Index>
	struct indexed_vector<ScalarType, Size, 1u, Index>
		: vector_base<lvalue_swizzle_indexes<Size, 1u, Index>, ScalarType, 1u, indexed_vector<ScalarType, Size, 1u, Index>>
	{
		// we have partial specialization, so can't use template parameter for IndexCount
		// number of logical storage elements
		static constexpr std::size_t IndexCount = 1u;

		// can this be used as an lvalue
		static constexpr bool Writable = lvalue_swizzle_indexes<Size, 1u, Index>;

		// the underlying ordered storage sequence for this logical vector
		using access_sequence = std::index_sequence<Index>;

		// common initial sequence data
		dimensional_storage_t<ScalarType, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType>
		requires Writable
		constexpr void init(OtherScalarType value0) noexcept
		{
			value[Index] = static_cast<ScalarType>(value0);
		}

		// copy assignment
		template <bool OtherWritable, dimensional_scalar OtherScalarType, typename Derived>
		requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, OtherScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u]);

			return *this;
		}

		template <bool OtherWritable, typename Derived>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, ScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u]);

			return *this;
		}

		// scalar assignment
		// assignment for some scalar type that converts to our ScalarType type and is only for indexed_vector of [Size == 1]
		template <typename OtherScalarType>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr indexed_vector &operator =(const OtherScalarType other) noexcept
		{
			init(other);

			return *this;
		}

		constexpr indexed_vector &operator =(const ScalarType other) noexcept
		{
			init(other);

			return *this;
		}

		// scalar conversion operator
		// this is extremely important and is only for indexed_vector of [Size == 1]
		constexpr operator ScalarType() const noexcept
		{
			return value[Index];
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr ScalarType &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[Index];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const ScalarType &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[Index];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<ScalarType, IndexCount>() const noexcept
		{
			return basic_vector<ScalarType, IndexCount>(value[Index]);
		}

		template <dimensional_scalar OtherScalarType>
		requires (!std::same_as<ScalarType, OtherScalarType> && std::convertible_to<ScalarType, OtherScalarType>)
		explicit constexpr operator basic_vector<OtherScalarType, IndexCount>() const noexcept
		{
			return basic_vector<OtherScalarType, IndexCount>(static_cast<OtherScalarType>(value[Index]));
		}

		// support for range-for loop
		constexpr auto begin()				noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index>(*this, 0u); }
		constexpr auto begin()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index>(*this, 0u); }
		constexpr auto cbegin()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index>(*this, 0u); }
		constexpr auto end()				noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index>(*this, IndexCount); }
		constexpr auto end()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index>(*this, IndexCount); }
		constexpr auto cend()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index>(*this, IndexCount); }
	};

	// for swizzling 2D parts of dimension_data
	template <dimensional_scalar ScalarType, std::size_t Size, std::size_t Index0, std::size_t Index1>
	struct indexed_vector<ScalarType, Size, 2u, Index0, Index1>
		: vector_base<lvalue_swizzle_indexes<Size, 2u, Index0, Index1>, ScalarType, 2u, indexed_vector<ScalarType, Size, 2u, Index0, Index1>>
	{
		// we have partial specialization, so can't use template parameter for Size
		// number of logical storage elements
		static constexpr std::size_t IndexCount = 2u;

		// can this be used as an lvalue
		static constexpr bool Writable = lvalue_swizzle_indexes<Size, 2u, Index0, Index1>;

		// the underlying ordered storage sequence for this logical vector
		using access_sequence = std::index_sequence<Index0, Index1>;

		// common initial sequence data
		dimensional_storage_t<ScalarType, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType>
		requires Writable
		constexpr void init(OtherScalarType value0, OtherScalarType value1) noexcept
		{
			value[Index0] = static_cast<ScalarType>(value0);
			value[Index1] = static_cast<ScalarType>(value1);
		}

		// copy assignment
		template <bool OtherWritable, dimensional_scalar OtherScalarType, typename Derived>
		requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, OtherScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u], other[1u]);

			return *this;
		}

		template <bool OtherWritable, typename Derived>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, ScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u], other[1u]);

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr ScalarType &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[Index0];
				case 1u:
					return value[Index1];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const ScalarType &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[Index0];
				case 1u:
					return value[Index1];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<ScalarType, IndexCount>() const noexcept
		{
			return basic_vector<ScalarType, IndexCount>(value[Index0], value[Index1]);
		}

		template <dimensional_scalar OtherScalarType>
		requires (!std::same_as<ScalarType, OtherScalarType> && std::convertible_to<ScalarType, OtherScalarType>)
		explicit constexpr operator basic_vector<OtherScalarType, IndexCount>() const noexcept
		{
			return basic_vector<OtherScalarType, IndexCount>(static_cast<OtherScalarType>(value[Index0]),
															 static_cast<OtherScalarType>(value[Index1]));
		}

		// support for range-for loop
		constexpr auto begin()				noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index0, Index1>(*this, 0u); }
		constexpr auto begin()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1>(*this, 0u); }
		constexpr auto cbegin()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1>(*this, 0u); }
		constexpr auto end()				noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index0, Index1>(*this, IndexCount); }
		constexpr auto end()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1>(*this, IndexCount); }
		constexpr auto cend()		const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1>(*this, IndexCount); }
	};

	// for swizzling 3D parts of dimension_data
	template <dimensional_scalar ScalarType, std::size_t Size, std::size_t Index0, std::size_t Index1, std::size_t Index2>
	struct indexed_vector<ScalarType, Size, 3u, Index0, Index1, Index2>
		: vector_base<lvalue_swizzle_indexes<Size, 3u, Index0, Index1, Index2>, ScalarType, 3u, indexed_vector<ScalarType, Size, 3u, Index0, Index1, Index2>>
	{
		// we have partial specialization, so can't use template parameter for Size
		// number of logical storage elements
		static constexpr std::size_t IndexCount = 3u;

		// can this be used as an lvalue
		static constexpr bool Writable = lvalue_swizzle_indexes<Size, 3u, Index0, Index1, Index2>;

		// the underlying ordered storage sequence for this logical vector
		using access_sequence = std::index_sequence<Index0, Index1, Index2>;

		// common initial sequence data
		dimensional_storage_t<ScalarType, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType>
		requires Writable
		constexpr void init(OtherScalarType value0, OtherScalarType value1, OtherScalarType value2) noexcept
		{
			value[Index0] = static_cast<ScalarType>(value0);
			value[Index1] = static_cast<ScalarType>(value1);
			value[Index2] = static_cast<ScalarType>(value2);
		}

		// copy assignment
		template <bool OtherWritable, dimensional_scalar OtherScalarType, typename Derived>
		requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, OtherScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);

			return *this;
		}

		template <bool OtherWritable, typename Derived>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, ScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr ScalarType &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[Index0];
				case 1u:
					return value[Index1];
				case 2u:
					return value[Index2];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const ScalarType &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[Index0];
				case 1u:
					return value[Index1];
				case 2u:
					return value[Index2];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<ScalarType, IndexCount>() const noexcept
		{
			return basic_vector<ScalarType, IndexCount>(value[Index0], value[Index1], value[Index2]);
		}

		template <dimensional_scalar OtherScalarType>
		requires (!std::same_as<ScalarType, OtherScalarType> && std::convertible_to<ScalarType, OtherScalarType>)
		explicit constexpr operator basic_vector<OtherScalarType, IndexCount>() const noexcept
		{
			return basic_vector<OtherScalarType, IndexCount>(static_cast<OtherScalarType>(value[Index0]),
															 static_cast<OtherScalarType>(value[Index1]),
															 static_cast<OtherScalarType>(value[Index2]));
		}

		// support for range-for loop
		constexpr auto begin()			noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2>(*this, 0u); }
		constexpr auto begin()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2>(*this, 0u); }
		constexpr auto cbegin()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2>(*this, 0u); }
		constexpr auto end()			noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2>(*this, IndexCount); }
		constexpr auto end()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2>(*this, IndexCount); }
		constexpr auto cend()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2>(*this, IndexCount); }
	};

	// for swizzling 4D parts of dimension_data
	template <dimensional_scalar ScalarType, std::size_t Size, std::size_t Index0, std::size_t Index1, std::size_t Index2, std::size_t Index3>
	struct indexed_vector<ScalarType, Size, 4u, Index0, Index1, Index2, Index3>
		: vector_base<lvalue_swizzle_indexes<Size, 4u, Index0, Index1, Index2, Index3>, ScalarType, 4u, indexed_vector<ScalarType, Size, 4u, Index0, Index1, Index2, Index3>>
	{
		// we have partial specialization, so can't use template parameter for Size
		// number of logical storage elements
		static constexpr std::size_t IndexCount = 4u;

		// can this be used as an lvalue
		static constexpr bool Writable = lvalue_swizzle_indexes<Size, 4u, Index0, Index1, Index2, Index3>;

		// the underlying ordered storage sequence for this logical vector
		using access_sequence = std::index_sequence<Index0, Index1, Index2, Index3>;

		// common initial sequence data
		dimensional_storage_t<ScalarType, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType>
		requires Writable
		constexpr void init(OtherScalarType value0, OtherScalarType value1, OtherScalarType value2, OtherScalarType value3) noexcept
		{
			value[Index0] = static_cast<ScalarType>(value0);
			value[Index1] = static_cast<ScalarType>(value1);
			value[Index2] = static_cast<ScalarType>(value2);
			value[Index3] = static_cast<ScalarType>(value3);
		}

		// copy assignment
		template <bool OtherWritable, dimensional_scalar OtherScalarType, typename Derived>
		requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, OtherScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);

			return *this;
		}

		template <bool OtherWritable, typename Derived>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<OtherWritable, ScalarType, IndexCount, Derived> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr ScalarType &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[Index0];
				case 1u:
					return value[Index1];
				case 2u:
					return value[Index2];
				case 3u:
					return value[Index3];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const ScalarType &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[Index0];
				case 1u:
					return value[Index1];
				case 2u:
					return value[Index2];
				case 3u:
					return value[Index3];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<ScalarType, IndexCount>() const noexcept
		{
			return basic_vector<ScalarType, IndexCount>(value[Index0], value[Index1], value[Index2], value[Index3]);
		}

		template <dimensional_scalar OtherScalarType>
		requires (!std::same_as<ScalarType, OtherScalarType> && std::convertible_to<ScalarType, OtherScalarType>)
		explicit constexpr operator basic_vector<OtherScalarType, IndexCount>() const noexcept
		{
			return basic_vector<OtherScalarType, IndexCount>(static_cast<OtherScalarType>(value[Index0]),
															 static_cast<OtherScalarType>(value[Index1]),
															 static_cast<OtherScalarType>(value[Index2]),
															 static_cast<OtherScalarType>(value[Index3]));
		}

		// support for range-for loop
		constexpr auto begin()			noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2, Index3>(*this, 0u); }
		constexpr auto begin()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2, Index3>(*this, 0u); }
		constexpr auto cbegin()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2, Index3>(*this, 0u); }
		constexpr auto end()			noexcept	requires Writable	{ return indexed_vector_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2, Index3>(*this, IndexCount); }
		constexpr auto end()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2, Index3>(*this, IndexCount); }
		constexpr auto cend()	const	noexcept						{ return indexed_vector_const_iterator<ScalarType, Size, IndexCount, Index0, Index1, Index2, Index3>(*this, IndexCount); }
	};


	// convenience using types for indexed_vector as members of basic_vector

	template <typename ScalarType, std::size_t Size, std::size_t Index>
	using index1_vector = indexed_vector<std::remove_cvref_t<ScalarType>, Size, 1u, Index>;

	template <typename ScalarType, std::size_t Size, std::size_t Index0, std::size_t Index1>
	using index2_vector = indexed_vector<std::remove_cvref_t<ScalarType>, Size, 2u, Index0, Index1>;

	template <typename ScalarType, std::size_t Size, std::size_t Index0, std::size_t Index1, std::size_t Index2>
	using index3_vector = indexed_vector<std::remove_cvref_t<ScalarType>, Size, 3u, Index0, Index1, Index2>;

	template <typename ScalarType, std::size_t Size, std::size_t Index0, std::size_t Index1, std::size_t Index2, std::size_t Index3>
	using index4_vector = indexed_vector<std::remove_cvref_t<ScalarType>, Size, 4u, Index0, Index1, Index2, Index3>;


	template <dimensional_scalar ScalarType>
	struct basic_vector<ScalarType, 1u> : vector_base<true, ScalarType, 1u, basic_vector<ScalarType, 1u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 1u;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		// the underlying ordered storage sequence for this physical vector
		using access_sequence = std::make_index_sequence<Size>;

		union
		{
			storage_wrapper<ScalarType, Size>				store;

			index1_vector<ScalarType, Size, 0>				x;				// can modify as lvalue

			index2_vector<ScalarType, Size, 0, 0>			xx;

			index3_vector<ScalarType, Size, 0, 0, 0>		xxx;

			index4_vector<ScalarType, Size, 0, 0, 0, 0>		xxxx;
		};

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

		//
		// constructors
		//

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector(const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(other[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires (!implicitly_convertible_to<OtherScalarType, ScalarType> && std::convertible_to<OtherScalarType, ScalarType>)
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(other[0u]);
		}

		template <typename OtherScalarType>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector(OtherScalarType value) noexcept
			: store()
		{
			init(value);
		}

		template <typename OtherScalarType>
		requires (!implicitly_convertible_to<OtherScalarType, ScalarType> && std::convertible_to<OtherScalarType, ScalarType>)
		explicit constexpr basic_vector(OtherScalarType value) noexcept
			: store()
		{
			init(value);
		}

		//
		// implicit assignment operators
		//

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector &operator =(const vector_base<Writable, OtherScalarType, Size, Derived> &other) noexcept
		{
			init(other[0u]);
			return *this;
		}

		template <typename OtherScalarType>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector &operator =(OtherScalarType value) noexcept
		{
			init(value);
			return *this;
		}

		//
		// conversion operators
		//

		// this is extremely important and is only for basic_vector of [Size == 1]

		template <typename OtherScalarType>
		requires implicitly_convertible_to<ScalarType, OtherScalarType>
		constexpr operator OtherScalarType() const noexcept
		{
			return static_cast<OtherScalarType>(store.value[0u]);
		}

		template <typename OtherScalarType>
		requires (!implicitly_convertible_to<ScalarType, OtherScalarType>) && std::convertible_to<ScalarType, OtherScalarType>
		explicit constexpr operator OtherScalarType() const noexcept
		{
			return static_cast<OtherScalarType>(store.value[0u]);
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType>
		constexpr void init(OtherScalarType value) noexcept
		{
			store.value[0u] = static_cast<ScalarType>(value);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		ScalarType	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	ScalarType	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	template <dimensional_scalar ScalarType>
	struct basic_vector<ScalarType, 2u> : vector_base<true, ScalarType, 2u, basic_vector<ScalarType, 2u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 2u;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		// the underlying ordered storage sequence for this physical vector
		using access_sequence = std::make_index_sequence<Size>;

		union
		{
			storage_wrapper<ScalarType, Size>				store;

			index1_vector<ScalarType, Size, 0>				x;				// can modify as lvalue
			index1_vector<ScalarType, Size, 1>				y;				// can modify as lvalue

			index2_vector<ScalarType, Size, 0, 0>			xx;
			index2_vector<ScalarType, Size, 0, 1>			xy;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 0>			yx;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 1>			yy;

			index3_vector<ScalarType, Size, 0, 0, 0>		xxx;
			index3_vector<ScalarType, Size, 0, 0, 1>		xxy;
			index3_vector<ScalarType, Size, 0, 1, 0>		xyx;
			index3_vector<ScalarType, Size, 0, 1, 1>		xyy;
			index3_vector<ScalarType, Size, 1, 0, 0>		yxx;
			index3_vector<ScalarType, Size, 1, 0, 1>		yxy;
			index3_vector<ScalarType, Size, 1, 1, 0>		yyx;
			index3_vector<ScalarType, Size, 1, 1, 1>		yyy;

			index4_vector<ScalarType, Size, 0, 0, 0, 0>		xxxx;
			index4_vector<ScalarType, Size, 0, 0, 0, 1>		xxxy;
			index4_vector<ScalarType, Size, 0, 0, 1, 0>		xxyx;
			index4_vector<ScalarType, Size, 0, 0, 1, 1>		xxyy;
			index4_vector<ScalarType, Size, 0, 1, 0, 0>		xyxx;
			index4_vector<ScalarType, Size, 0, 1, 0, 1>		xyxy;
			index4_vector<ScalarType, Size, 0, 1, 1, 0>		xyyx;
			index4_vector<ScalarType, Size, 0, 1, 1, 1>		xyyy;
			index4_vector<ScalarType, Size, 1, 0, 0, 0>		yxxx;
			index4_vector<ScalarType, Size, 1, 0, 0, 1>		yxxy;
			index4_vector<ScalarType, Size, 1, 0, 1, 0>		yxyx;
			index4_vector<ScalarType, Size, 1, 0, 1, 1>		yxyy;
			index4_vector<ScalarType, Size, 1, 1, 0, 0>		yyxx;
			index4_vector<ScalarType, Size, 1, 1, 0, 1>		yyxy;
			index4_vector<ScalarType, Size, 1, 1, 1, 0>		yyyx;
			index4_vector<ScalarType, Size, 1, 1, 1, 1>		yyyy;
		};

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

		//
		// constructors
		//

		template <typename OtherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType>
		explicit constexpr basic_vector(OtherScalarType value) noexcept
			: store()
		{
			init(value, value);
		}

		template <typename OtherScalarType0, typename OtherScalarType1>
		requires std::convertible_to<OtherScalarType0, ScalarType> && std::convertible_to <OtherScalarType1, ScalarType>
		explicit constexpr basic_vector(OtherScalarType0 xvalue, OtherScalarType1 yvalue) noexcept
			: store()
		{
			init(xvalue, yvalue);
		}

		template <typename OtherScalarType0, bool Writable, dimensional_scalar OtherScalarType1, std::size_t Count, typename Derived>
		requires std::convertible_to<OtherScalarType0, ScalarType> && std::convertible_to<OtherScalarType1, ScalarType>
		explicit constexpr basic_vector(OtherScalarType0 xvalue, const vector_base<Writable, OtherScalarType1, Count, Derived> &yvalue_source) noexcept
			: store()
		{
			init(xvalue, yvalue_source[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType> && (Count >= Size)
		constexpr basic_vector(const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(other[0u], other[1u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires (!implicitly_convertible_to<OtherScalarType, ScalarType> && std::convertible_to<OtherScalarType, ScalarType> && (Count >= Size))
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(other[0u], other[1u]);
		}

		//
		// assignment operator
		//

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector &operator =(const vector_base<Writable, OtherScalarType, Size, Derived> &other) noexcept
		{
			init(other[0u], other[1u]);
			return *this;
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType0, typename OtherScalarType1>
		constexpr void init(OtherScalarType0 value0, OtherScalarType1 value1) noexcept
		{
			store.value[0u] = static_cast<ScalarType>(value0);
			store.value[1u] = static_cast<ScalarType>(value1);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		ScalarType	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	ScalarType	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	template <dimensional_scalar ScalarType>
	struct basic_vector<ScalarType, 3u> : vector_base<true, ScalarType, 3u, basic_vector<ScalarType, 3u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 3u;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		// the underlying ordered storage sequence for this physical vector
		using access_sequence = std::make_index_sequence<Size>;

		union
		{
			storage_wrapper<ScalarType, Size>				store;

			index1_vector<ScalarType, Size, 0>				x;				// can modify as lvalue
			index1_vector<ScalarType, Size, 1>				y;				// can modify as lvalue
			index1_vector<ScalarType, Size, 2>				z;				// can modify as lvalue

			index2_vector<ScalarType, Size, 0, 0>			xx;
			index2_vector<ScalarType, Size, 0, 1>			xy;				// can modify as lvalue
			index2_vector<ScalarType, Size, 0, 2>			xz;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 0>			yx;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 1>			yy;
			index2_vector<ScalarType, Size, 1, 2>			yz;				// can modify as lvalue
			index2_vector<ScalarType, Size, 2, 0>			zx;				// can modify as lvalue
			index2_vector<ScalarType, Size, 2, 1>			zy;				// can modify as lvalue
			index2_vector<ScalarType, Size, 2, 2>			zz;

			index3_vector<ScalarType, Size, 0, 0, 0>		xxx;
			index3_vector<ScalarType, Size, 0, 0, 1>		xxy;
			index3_vector<ScalarType, Size, 0, 0, 2>		xxz;
			index3_vector<ScalarType, Size, 0, 1, 0>		xyx;
			index3_vector<ScalarType, Size, 0, 1, 1>		xyy;
			index3_vector<ScalarType, Size, 0, 1, 2>		xyz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 2, 0>		xzx;
			index3_vector<ScalarType, Size, 0, 2, 1>		xzy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 2, 2>		xzz;
			index3_vector<ScalarType, Size, 1, 0, 0>		yxx;
			index3_vector<ScalarType, Size, 1, 0, 1>		yxy;
			index3_vector<ScalarType, Size, 1, 0, 2>		yxz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 1, 0>		yyx;
			index3_vector<ScalarType, Size, 1, 1, 1>		yyy;
			index3_vector<ScalarType, Size, 1, 1, 2>		yyz;
			index3_vector<ScalarType, Size, 1, 2, 0>		yzx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 2, 1>		yzy;
			index3_vector<ScalarType, Size, 1, 2, 2>		yzz;
			index3_vector<ScalarType, Size, 2, 0, 0>		zxx;
			index3_vector<ScalarType, Size, 2, 0, 1>		zxy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 0, 2>		zxz;
			index3_vector<ScalarType, Size, 2, 1, 0>		zyx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 1, 1>		zyy;
			index3_vector<ScalarType, Size, 2, 1, 2>		zyz;
			index3_vector<ScalarType, Size, 2, 2, 0>		zzx;
			index3_vector<ScalarType, Size, 2, 2, 1>		zzy;
			index3_vector<ScalarType, Size, 2, 2, 2>		zzz;

			index4_vector<ScalarType, Size, 0, 0, 0, 0>		xxxx;
			index4_vector<ScalarType, Size, 0, 0, 0, 1>		xxxy;
			index4_vector<ScalarType, Size, 0, 0, 0, 2>		xxxz;
			index4_vector<ScalarType, Size, 0, 0, 1, 0>		xxyx;
			index4_vector<ScalarType, Size, 0, 0, 1, 1>		xxyy;
			index4_vector<ScalarType, Size, 0, 0, 1, 2>		xxyz;
			index4_vector<ScalarType, Size, 0, 0, 2, 0>		xxzx;
			index4_vector<ScalarType, Size, 0, 0, 2, 1>		xxzy;
			index4_vector<ScalarType, Size, 0, 0, 2, 2>		xxzz;
			index4_vector<ScalarType, Size, 0, 1, 0, 0>		xyxx;
			index4_vector<ScalarType, Size, 0, 1, 0, 1>		xyxy;
			index4_vector<ScalarType, Size, 0, 1, 0, 2>		xyxz;
			index4_vector<ScalarType, Size, 0, 1, 1, 0>		xyyx;
			index4_vector<ScalarType, Size, 0, 1, 1, 1>		xyyy;
			index4_vector<ScalarType, Size, 0, 1, 1, 2>		xyyz;
			index4_vector<ScalarType, Size, 0, 1, 2, 0>		xyzx;
			index4_vector<ScalarType, Size, 0, 1, 2, 1>		xyzy;
			index4_vector<ScalarType, Size, 0, 1, 2, 2>		xyzz;
			index4_vector<ScalarType, Size, 0, 2, 0, 0>		xzxx;
			index4_vector<ScalarType, Size, 0, 2, 0, 1>		xzxy;
			index4_vector<ScalarType, Size, 0, 2, 0, 2>		xzxz;
			index4_vector<ScalarType, Size, 0, 2, 1, 0>		xzyx;
			index4_vector<ScalarType, Size, 0, 2, 1, 1>		xzyy;
			index4_vector<ScalarType, Size, 0, 2, 1, 2>		xzyz;
			index4_vector<ScalarType, Size, 0, 2, 2, 0>		xzzx;
			index4_vector<ScalarType, Size, 0, 2, 2, 1>		xzzy;
			index4_vector<ScalarType, Size, 0, 2, 2, 2>		xzzz;
			index4_vector<ScalarType, Size, 1, 0, 0, 0>		yxxx;
			index4_vector<ScalarType, Size, 1, 0, 0, 1>		yxxy;
			index4_vector<ScalarType, Size, 1, 0, 0, 2>		yxxz;
			index4_vector<ScalarType, Size, 1, 0, 1, 0>		yxyx;
			index4_vector<ScalarType, Size, 1, 0, 1, 1>		yxyy;
			index4_vector<ScalarType, Size, 1, 0, 1, 2>		yxyz;
			index4_vector<ScalarType, Size, 1, 0, 2, 0>		yxzx;
			index4_vector<ScalarType, Size, 1, 0, 2, 1>		yxzy;
			index4_vector<ScalarType, Size, 1, 0, 2, 2>		yxzz;
			index4_vector<ScalarType, Size, 1, 1, 0, 0>		yyxx;
			index4_vector<ScalarType, Size, 1, 1, 0, 1>		yyxy;
			index4_vector<ScalarType, Size, 1, 1, 0, 2>		yyxz;
			index4_vector<ScalarType, Size, 1, 1, 1, 0>		yyyx;
			index4_vector<ScalarType, Size, 1, 1, 1, 1>		yyyy;
			index4_vector<ScalarType, Size, 1, 1, 1, 2>		yyyz;
			index4_vector<ScalarType, Size, 1, 1, 2, 0>		yyzx;
			index4_vector<ScalarType, Size, 1, 1, 2, 1>		yyzy;
			index4_vector<ScalarType, Size, 1, 1, 2, 2>		yyzz;
			index4_vector<ScalarType, Size, 1, 2, 0, 0>		yzxx;
			index4_vector<ScalarType, Size, 1, 2, 0, 1>		yzxy;
			index4_vector<ScalarType, Size, 1, 2, 0, 2>		yzxz;
			index4_vector<ScalarType, Size, 1, 2, 1, 0>		yzyx;
			index4_vector<ScalarType, Size, 1, 2, 1, 1>		yzyy;
			index4_vector<ScalarType, Size, 1, 2, 1, 2>		yzyz;
			index4_vector<ScalarType, Size, 1, 2, 2, 0>		yzzx;
			index4_vector<ScalarType, Size, 1, 2, 2, 1>		yzzy;
			index4_vector<ScalarType, Size, 1, 2, 2, 2>		yzzz;
			index4_vector<ScalarType, Size, 2, 0, 0, 0>		zxxx;
			index4_vector<ScalarType, Size, 2, 0, 0, 1>		zxxy;
			index4_vector<ScalarType, Size, 2, 0, 0, 2>		zxxz;
			index4_vector<ScalarType, Size, 2, 0, 1, 0>		zxyx;
			index4_vector<ScalarType, Size, 2, 0, 1, 1>		zxyy;
			index4_vector<ScalarType, Size, 2, 0, 1, 2>		zxyz;
			index4_vector<ScalarType, Size, 2, 0, 2, 0>		zxzx;
			index4_vector<ScalarType, Size, 2, 0, 2, 1>		zxzy;
			index4_vector<ScalarType, Size, 2, 0, 2, 2>		zxzz;
			index4_vector<ScalarType, Size, 2, 1, 0, 0>		zyxx;
			index4_vector<ScalarType, Size, 2, 1, 0, 1>		zyxy;
			index4_vector<ScalarType, Size, 2, 1, 0, 2>		zyxz;
			index4_vector<ScalarType, Size, 2, 1, 1, 0>		zyyx;
			index4_vector<ScalarType, Size, 2, 1, 1, 1>		zyyy;
			index4_vector<ScalarType, Size, 2, 1, 1, 2>		zyyz;
			index4_vector<ScalarType, Size, 2, 1, 2, 0>		zyzx;
			index4_vector<ScalarType, Size, 2, 1, 2, 1>		zyzy;
			index4_vector<ScalarType, Size, 2, 1, 2, 2>		zyzz;
			index4_vector<ScalarType, Size, 2, 2, 0, 0>		zzxx;
			index4_vector<ScalarType, Size, 2, 2, 0, 1>		zzxy;
			index4_vector<ScalarType, Size, 2, 2, 0, 2>		zzxz;
			index4_vector<ScalarType, Size, 2, 2, 1, 0>		zzyx;
			index4_vector<ScalarType, Size, 2, 2, 1, 1>		zzyy;
			index4_vector<ScalarType, Size, 2, 2, 1, 2>		zzyz;
			index4_vector<ScalarType, Size, 2, 2, 2, 0>		zzzx;
			index4_vector<ScalarType, Size, 2, 2, 2, 1>		zzzy;
			index4_vector<ScalarType, Size, 2, 2, 2, 2>		zzzz;
		};

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

		//
		// constructors
		//

		template <typename OtherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType>
		explicit constexpr basic_vector(OtherScalarType value) noexcept
			: store()
		{
			init(value, value, value);
		}

		template <typename OtherScalarType0, typename OtherScalarType1, typename OtherScalarType2>
		requires std::convertible_to<OtherScalarType0, ScalarType> && std::convertible_to<OtherScalarType1, ScalarType> && std::convertible_to<OtherScalarType2, ScalarType>
		explicit constexpr basic_vector(OtherScalarType0 xvalue,
										OtherScalarType1 yvalue,
										OtherScalarType2 zvalue) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue);
		}

		template <typename OtherScalarType0, typename OtherScalarType1, bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires std::convertible_to<OtherScalarType0, ScalarType> && std::convertible_to<OtherScalarType1, ScalarType> && std::convertible_to<OtherScalarType, ScalarType>
		explicit constexpr basic_vector(OtherScalarType0 xvalue,
										OtherScalarType1 yvalue,
										const vector_base<Writable, OtherScalarType, Count, Derived> &zvalue_source) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue_source[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType> && (Count >= Size)
		constexpr basic_vector(const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires (!implicitly_convertible_to<OtherScalarType, ScalarType> && std::convertible_to<OtherScalarType, ScalarType> && (Count >= Size))
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived, typename YetAnotherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType>
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, 2u, Derived> &other,
										YetAnotherScalarType yet_another) noexcept
			: store()
		{
			init(other[0u], other[1u], yet_another);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived,
			bool YetAnotherWritable, typename YetAnotherScalarType, std::size_t Count, typename YetAnotherDerived>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType>
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, 2u, Derived> &other,
										const vector_base<YetAnotherWritable, YetAnotherScalarType, Count, YetAnotherDerived> &yet_another) noexcept
			: store()
		{
			init(other[0u], other[1u], yet_another[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived, typename YetAnotherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType> && (Count >= 2)
		explicit constexpr basic_vector(YetAnotherScalarType yet_another,
										const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(yet_another, other[0u], other[1u]);
		}

		//
		// assignment operators
		//

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector &operator =(const vector_base<Writable, OtherScalarType, Size, Derived> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);
			return *this;
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType0, typename OtherScalarType1, typename OtherScalarType2>
		constexpr void init(OtherScalarType0 value0, OtherScalarType1 value1, OtherScalarType2 value2) noexcept
		{
			store.value[0u] = static_cast<ScalarType>(value0);
			store.value[1u] = static_cast<ScalarType>(value1);
			store.value[2u] = static_cast<ScalarType>(value2);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		ScalarType	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	ScalarType	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	template <dimensional_scalar ScalarType>
	struct basic_vector<ScalarType, 4u> : vector_base<true, ScalarType, 4u, basic_vector<ScalarType, 4u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 4u;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		// the underlying ordered storage sequence for this physical vector
		using access_sequence = std::make_index_sequence<Size>;

		union
		{
			storage_wrapper<ScalarType, Size>				store;

			index1_vector<ScalarType, Size, 0>				x;				// can modify as lvalue
			index1_vector<ScalarType, Size, 1>				y;				// can modify as lvalue
			index1_vector<ScalarType, Size, 2>				z;				// can modify as lvalue
			index1_vector<ScalarType, Size, 3>				w;				// can modify as lvalue

			index2_vector<ScalarType, Size, 0, 0>			xx;
			index2_vector<ScalarType, Size, 0, 1>			xy;				// can modify as lvalue
			index2_vector<ScalarType, Size, 0, 2>			xz;				// can modify as lvalue
			index2_vector<ScalarType, Size, 0, 3>			xw;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 0>			yx;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 1>			yy;
			index2_vector<ScalarType, Size, 1, 2>			yz;				// can modify as lvalue
			index2_vector<ScalarType, Size, 1, 3>			yw;				// can modify as lvalue
			index2_vector<ScalarType, Size, 2, 0>			zx;				// can modify as lvalue
			index2_vector<ScalarType, Size, 2, 1>			zy;				// can modify as lvalue
			index2_vector<ScalarType, Size, 2, 2>			zz;
			index2_vector<ScalarType, Size, 2, 3>			zw;				// can modify as lvalue
			index2_vector<ScalarType, Size, 3, 0>			wx;				// can modify as lvalue
			index2_vector<ScalarType, Size, 3, 1>			wy;				// can modify as lvalue
			index2_vector<ScalarType, Size, 3, 2>			wz;				// can modify as lvalue
			index2_vector<ScalarType, Size, 3, 3>			ww;

			index3_vector<ScalarType, Size, 0, 0, 0>		xxx;
			index3_vector<ScalarType, Size, 0, 0, 1>		xxy;
			index3_vector<ScalarType, Size, 0, 0, 2>		xxz;
			index3_vector<ScalarType, Size, 0, 0, 3>		xxw;
			index3_vector<ScalarType, Size, 0, 1, 0>		xyx;
			index3_vector<ScalarType, Size, 0, 1, 1>		xyy;
			index3_vector<ScalarType, Size, 0, 1, 2>		xyz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 1, 3>		xyw;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 2, 0>		xzx;
			index3_vector<ScalarType, Size, 0, 2, 1>		xzy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 2, 2>		xzz;
			index3_vector<ScalarType, Size, 0, 2, 3>		xzw;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 3, 0>		xwx;
			index3_vector<ScalarType, Size, 0, 3, 1>		xwy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 3, 2>		xwz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 0, 3, 3>		xww;
			index3_vector<ScalarType, Size, 1, 0, 0>		yxx;
			index3_vector<ScalarType, Size, 1, 0, 1>		yxy;
			index3_vector<ScalarType, Size, 1, 0, 2>		yxz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 0, 3>		yxw;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 1, 0>		yyx;
			index3_vector<ScalarType, Size, 1, 1, 1>		yyy;
			index3_vector<ScalarType, Size, 1, 1, 2>		yyz;
			index3_vector<ScalarType, Size, 1, 1, 3>		yyw;
			index3_vector<ScalarType, Size, 1, 2, 0>		yzx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 2, 1>		yzy;
			index3_vector<ScalarType, Size, 1, 2, 2>		yzz;
			index3_vector<ScalarType, Size, 1, 2, 3>		yzw;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 3, 0>		ywx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 3, 1>		ywy;
			index3_vector<ScalarType, Size, 1, 3, 2>		ywz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 1, 3, 3>		yww;
			index3_vector<ScalarType, Size, 2, 0, 0>		zxx;
			index3_vector<ScalarType, Size, 2, 0, 1>		zxy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 0, 2>		zxz;
			index3_vector<ScalarType, Size, 2, 0, 3>		zxw;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 1, 0>		zyx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 1, 1>		zyy;
			index3_vector<ScalarType, Size, 2, 1, 2>		zyz;
			index3_vector<ScalarType, Size, 2, 1, 3>		zyw;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 2, 0>		zzx;
			index3_vector<ScalarType, Size, 2, 2, 1>		zzy;
			index3_vector<ScalarType, Size, 2, 2, 2>		zzz;
			index3_vector<ScalarType, Size, 2, 2, 3>		zzw;
			index3_vector<ScalarType, Size, 2, 3, 0>		zwx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 3, 1>		zwy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 2, 3, 2>		zwz;
			index3_vector<ScalarType, Size, 2, 3, 3>		zww;
			index3_vector<ScalarType, Size, 3, 0, 0>		wxx;
			index3_vector<ScalarType, Size, 3, 0, 1>		wxy;			// can modify as lvalue
			index3_vector<ScalarType, Size, 3, 0, 2>		wxz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 3, 0, 3>		wxw;
			index3_vector<ScalarType, Size, 3, 1, 0>		wyx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 3, 1, 1>		wyy;
			index3_vector<ScalarType, Size, 3, 1, 2>		wyz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 3, 1, 3>		wyw;
			index3_vector<ScalarType, Size, 3, 2, 0>		wzx;			// can modify as lvalue
			index3_vector<ScalarType, Size, 3, 2, 1>		wzy;
			index3_vector<ScalarType, Size, 3, 2, 2>		wzz;			// can modify as lvalue
			index3_vector<ScalarType, Size, 3, 2, 3>		wzw;
			index3_vector<ScalarType, Size, 3, 3, 0>		wwx;
			index3_vector<ScalarType, Size, 3, 3, 1>		wwy;
			index3_vector<ScalarType, Size, 3, 3, 2>		wwz;
			index3_vector<ScalarType, Size, 3, 3, 3>		www;

			index4_vector<ScalarType, Size, 0, 0, 0, 0>		xxxx;
			index4_vector<ScalarType, Size, 0, 0, 0, 1>		xxxy;
			index4_vector<ScalarType, Size, 0, 0, 0, 2>		xxxz;
			index4_vector<ScalarType, Size, 0, 0, 0, 3>		xxxw;
			index4_vector<ScalarType, Size, 0, 0, 1, 0>		xxyx;
			index4_vector<ScalarType, Size, 0, 0, 1, 1>		xxyy;
			index4_vector<ScalarType, Size, 0, 0, 1, 2>		xxyz;
			index4_vector<ScalarType, Size, 0, 0, 1, 3>		xxyw;
			index4_vector<ScalarType, Size, 0, 0, 2, 0>		xxzx;
			index4_vector<ScalarType, Size, 0, 0, 2, 1>		xxzy;
			index4_vector<ScalarType, Size, 0, 0, 2, 2>		xxzz;
			index4_vector<ScalarType, Size, 0, 0, 2, 3>		xxzw;
			index4_vector<ScalarType, Size, 0, 0, 3, 0>		xxwx;
			index4_vector<ScalarType, Size, 0, 0, 3, 1>		xxwy;
			index4_vector<ScalarType, Size, 0, 0, 3, 2>		xxwz;
			index4_vector<ScalarType, Size, 0, 0, 3, 3>		xxww;
			index4_vector<ScalarType, Size, 0, 1, 0, 0>		xyxx;
			index4_vector<ScalarType, Size, 0, 1, 0, 1>		xyxy;
			index4_vector<ScalarType, Size, 0, 1, 0, 2>		xyxz;
			index4_vector<ScalarType, Size, 0, 1, 0, 3>		xyxw;
			index4_vector<ScalarType, Size, 0, 1, 1, 0>		xyyx;
			index4_vector<ScalarType, Size, 0, 1, 1, 1>		xyyy;
			index4_vector<ScalarType, Size, 0, 1, 1, 2>		xyyz;
			index4_vector<ScalarType, Size, 0, 1, 1, 3>		xyyw;
			index4_vector<ScalarType, Size, 0, 1, 2, 0>		xyzx;
			index4_vector<ScalarType, Size, 0, 1, 2, 1>		xyzy;
			index4_vector<ScalarType, Size, 0, 1, 2, 2>		xyzz;
			index4_vector<ScalarType, Size, 0, 1, 2, 3>		xyzw;			// can modify as lvalue
			index4_vector<ScalarType, Size, 0, 1, 3, 0>		xywx;
			index4_vector<ScalarType, Size, 0, 1, 3, 1>		xywy;
			index4_vector<ScalarType, Size, 0, 1, 3, 2>		xywz;			// can modify as lvalue
			index4_vector<ScalarType, Size, 0, 1, 3, 3>		xyww;
			index4_vector<ScalarType, Size, 0, 2, 0, 0>		xzxx;
			index4_vector<ScalarType, Size, 0, 2, 0, 1>		xzxy;
			index4_vector<ScalarType, Size, 0, 2, 0, 2>		xzxz;
			index4_vector<ScalarType, Size, 0, 2, 0, 3>		xzxw;
			index4_vector<ScalarType, Size, 0, 2, 1, 0>		xzyx;
			index4_vector<ScalarType, Size, 0, 2, 1, 1>		xzyy;
			index4_vector<ScalarType, Size, 0, 2, 1, 2>		xzyz;
			index4_vector<ScalarType, Size, 0, 2, 1, 3>		xzyw;			// can modify as lvalue
			index4_vector<ScalarType, Size, 0, 2, 2, 0>		xzzx;
			index4_vector<ScalarType, Size, 0, 2, 2, 1>		xzzy;
			index4_vector<ScalarType, Size, 0, 2, 2, 2>		xzzz;
			index4_vector<ScalarType, Size, 0, 2, 2, 3>		xzzw;
			index4_vector<ScalarType, Size, 0, 2, 3, 0>		xzwx;
			index4_vector<ScalarType, Size, 0, 2, 3, 1>		xzwy;			// can modify as lvalue
			index4_vector<ScalarType, Size, 0, 2, 3, 2>		xzwz;
			index4_vector<ScalarType, Size, 0, 2, 3, 3>		xzww;
			index4_vector<ScalarType, Size, 0, 3, 0, 0>		xwxx;
			index4_vector<ScalarType, Size, 0, 3, 0, 1>		xwxy;
			index4_vector<ScalarType, Size, 0, 3, 0, 2>		xwxz;
			index4_vector<ScalarType, Size, 0, 3, 0, 3>		xwxw;
			index4_vector<ScalarType, Size, 0, 3, 1, 0>		xwyx;
			index4_vector<ScalarType, Size, 0, 3, 1, 1>		xwyy;
			index4_vector<ScalarType, Size, 0, 3, 1, 2>		xwyz;			// can modify as lvalue
			index4_vector<ScalarType, Size, 0, 3, 1, 3>		xwyw;
			index4_vector<ScalarType, Size, 0, 3, 2, 0>		xwzx;
			index4_vector<ScalarType, Size, 0, 3, 2, 1>		xwzy;			// can modify as lvalue
			index4_vector<ScalarType, Size, 0, 3, 2, 2>		xwzz;
			index4_vector<ScalarType, Size, 0, 3, 2, 3>		xwzw;
			index4_vector<ScalarType, Size, 0, 3, 3, 0>		xwwx;
			index4_vector<ScalarType, Size, 0, 3, 3, 1>		xwwy;
			index4_vector<ScalarType, Size, 0, 3, 3, 2>		xwwz;
			index4_vector<ScalarType, Size, 0, 3, 3, 3>		xwww;
			index4_vector<ScalarType, Size, 1, 0, 0, 0>		yxxx;
			index4_vector<ScalarType, Size, 1, 0, 0, 1>		yxxy;
			index4_vector<ScalarType, Size, 1, 0, 0, 2>		yxxz;
			index4_vector<ScalarType, Size, 1, 0, 0, 3>		yxxw;
			index4_vector<ScalarType, Size, 1, 0, 1, 0>		yxyx;
			index4_vector<ScalarType, Size, 1, 0, 1, 1>		yxyy;
			index4_vector<ScalarType, Size, 1, 0, 1, 2>		yxyz;
			index4_vector<ScalarType, Size, 1, 0, 1, 3>		yxyw;
			index4_vector<ScalarType, Size, 1, 0, 2, 0>		yxzx;
			index4_vector<ScalarType, Size, 1, 0, 2, 1>		yxzy;
			index4_vector<ScalarType, Size, 1, 0, 2, 2>		yxzz;
			index4_vector<ScalarType, Size, 1, 0, 2, 3>		yxzw;			// can modify as lvalue
			index4_vector<ScalarType, Size, 1, 0, 3, 0>		yxwx;
			index4_vector<ScalarType, Size, 1, 0, 3, 1>		yxwy;
			index4_vector<ScalarType, Size, 1, 0, 3, 2>		yxwz;			// can modify as lvalue
			index4_vector<ScalarType, Size, 1, 0, 3, 3>		yxww;
			index4_vector<ScalarType, Size, 1, 1, 0, 0>		yyxx;
			index4_vector<ScalarType, Size, 1, 1, 0, 1>		yyxy;
			index4_vector<ScalarType, Size, 1, 1, 0, 2>		yyxz;
			index4_vector<ScalarType, Size, 1, 1, 0, 3>		yyxw;
			index4_vector<ScalarType, Size, 1, 1, 1, 0>		yyyx;
			index4_vector<ScalarType, Size, 1, 1, 1, 1>		yyyy;
			index4_vector<ScalarType, Size, 1, 1, 1, 2>		yyyz;
			index4_vector<ScalarType, Size, 1, 1, 1, 3>		yyyw;
			index4_vector<ScalarType, Size, 1, 1, 2, 0>		yyzx;
			index4_vector<ScalarType, Size, 1, 1, 2, 1>		yyzy;
			index4_vector<ScalarType, Size, 1, 1, 2, 2>		yyzz;
			index4_vector<ScalarType, Size, 1, 1, 2, 3>		yyzw;
			index4_vector<ScalarType, Size, 1, 1, 3, 0>		yywx;
			index4_vector<ScalarType, Size, 1, 1, 3, 1>		yywy;
			index4_vector<ScalarType, Size, 1, 1, 3, 2>		yywz;
			index4_vector<ScalarType, Size, 1, 1, 3, 3>		yyww;
			index4_vector<ScalarType, Size, 1, 2, 0, 0>		yzxx;
			index4_vector<ScalarType, Size, 1, 2, 0, 1>		yzxy;
			index4_vector<ScalarType, Size, 1, 2, 0, 2>		yzxz;
			index4_vector<ScalarType, Size, 1, 2, 0, 3>		yzxw;			// can modify as lvalue
			index4_vector<ScalarType, Size, 1, 2, 1, 0>		yzyx;
			index4_vector<ScalarType, Size, 1, 2, 1, 1>		yzyy;
			index4_vector<ScalarType, Size, 1, 2, 1, 2>		yzyz;
			index4_vector<ScalarType, Size, 1, 2, 1, 3>		yzyw;
			index4_vector<ScalarType, Size, 1, 2, 2, 0>		yzzx;
			index4_vector<ScalarType, Size, 1, 2, 2, 1>		yzzy;
			index4_vector<ScalarType, Size, 1, 2, 2, 2>		yzzz;
			index4_vector<ScalarType, Size, 1, 2, 2, 3>		yzzw;
			index4_vector<ScalarType, Size, 1, 2, 3, 0>		yzwx;			// can modify as lvalue
			index4_vector<ScalarType, Size, 1, 2, 3, 1>		yzwy;
			index4_vector<ScalarType, Size, 1, 2, 3, 2>		yzwz;
			index4_vector<ScalarType, Size, 1, 2, 3, 3>		yzww;
			index4_vector<ScalarType, Size, 1, 3, 0, 0>		ywxx;
			index4_vector<ScalarType, Size, 1, 3, 0, 1>		ywxy;
			index4_vector<ScalarType, Size, 1, 3, 0, 2>		ywxz;			// can modify as lvalue
			index4_vector<ScalarType, Size, 1, 3, 0, 3>		ywxw;
			index4_vector<ScalarType, Size, 1, 3, 1, 0>		ywyx;
			index4_vector<ScalarType, Size, 1, 3, 1, 1>		ywyy;
			index4_vector<ScalarType, Size, 1, 3, 1, 2>		ywyz;
			index4_vector<ScalarType, Size, 1, 3, 1, 3>		ywyw;
			index4_vector<ScalarType, Size, 1, 3, 2, 0>		ywzx;			// can modify as lvalue
			index4_vector<ScalarType, Size, 1, 3, 2, 1>		ywzy;
			index4_vector<ScalarType, Size, 1, 3, 2, 2>		ywzz;
			index4_vector<ScalarType, Size, 1, 3, 2, 3>		ywzw;
			index4_vector<ScalarType, Size, 1, 3, 3, 0>		ywwx;
			index4_vector<ScalarType, Size, 1, 3, 3, 1>		ywwy;
			index4_vector<ScalarType, Size, 1, 3, 3, 2>		ywwz;
			index4_vector<ScalarType, Size, 1, 3, 3, 3>		ywww;
			index4_vector<ScalarType, Size, 2, 0, 0, 0>		zxxx;
			index4_vector<ScalarType, Size, 2, 0, 0, 1>		zxxy;
			index4_vector<ScalarType, Size, 2, 0, 0, 2>		zxxz;
			index4_vector<ScalarType, Size, 2, 0, 0, 3>		zxxw;
			index4_vector<ScalarType, Size, 2, 0, 1, 0>		zxyx;
			index4_vector<ScalarType, Size, 2, 0, 1, 1>		zxyy;
			index4_vector<ScalarType, Size, 2, 0, 1, 2>		zxyz;
			index4_vector<ScalarType, Size, 2, 0, 1, 3>		zxyw;			// can modify as lvalue
			index4_vector<ScalarType, Size, 2, 0, 2, 0>		zxzx;
			index4_vector<ScalarType, Size, 2, 0, 2, 1>		zxzy;
			index4_vector<ScalarType, Size, 2, 0, 2, 2>		zxzz;
			index4_vector<ScalarType, Size, 2, 0, 2, 3>		zxzw;
			index4_vector<ScalarType, Size, 2, 0, 3, 0>		zxwx;
			index4_vector<ScalarType, Size, 2, 0, 3, 1>		zxwy;			// can modify as lvalue
			index4_vector<ScalarType, Size, 2, 0, 3, 2>		zxwz;
			index4_vector<ScalarType, Size, 2, 0, 3, 3>		zxww;
			index4_vector<ScalarType, Size, 2, 1, 0, 0>		zyxx;
			index4_vector<ScalarType, Size, 2, 1, 0, 1>		zyxy;
			index4_vector<ScalarType, Size, 2, 1, 0, 2>		zyxz;
			index4_vector<ScalarType, Size, 2, 1, 0, 3>		zyxw;			// can modify as lvalue
			index4_vector<ScalarType, Size, 2, 1, 1, 0>		zyyx;
			index4_vector<ScalarType, Size, 2, 1, 1, 1>		zyyy;
			index4_vector<ScalarType, Size, 2, 1, 1, 2>		zyyz;
			index4_vector<ScalarType, Size, 2, 1, 1, 3>		zyyw;
			index4_vector<ScalarType, Size, 2, 1, 2, 0>		zyzx;
			index4_vector<ScalarType, Size, 2, 1, 2, 1>		zyzy;
			index4_vector<ScalarType, Size, 2, 1, 2, 2>		zyzz;
			index4_vector<ScalarType, Size, 2, 1, 2, 3>		zyzw;
			index4_vector<ScalarType, Size, 2, 1, 3, 0>		zywx;			// can modify as lvalue
			index4_vector<ScalarType, Size, 2, 1, 3, 1>		zywy;
			index4_vector<ScalarType, Size, 2, 1, 3, 2>		zywz;
			index4_vector<ScalarType, Size, 2, 1, 3, 3>		zyww;
			index4_vector<ScalarType, Size, 2, 2, 0, 0>		zzxx;
			index4_vector<ScalarType, Size, 2, 2, 0, 1>		zzxy;
			index4_vector<ScalarType, Size, 2, 2, 0, 2>		zzxz;
			index4_vector<ScalarType, Size, 2, 2, 0, 3>		zzxw;
			index4_vector<ScalarType, Size, 2, 2, 1, 0>		zzyx;
			index4_vector<ScalarType, Size, 2, 2, 1, 1>		zzyy;
			index4_vector<ScalarType, Size, 2, 2, 1, 2>		zzyz;
			index4_vector<ScalarType, Size, 2, 2, 1, 3>		zzyw;
			index4_vector<ScalarType, Size, 2, 2, 2, 0>		zzzx;
			index4_vector<ScalarType, Size, 2, 2, 2, 1>		zzzy;
			index4_vector<ScalarType, Size, 2, 2, 2, 2>		zzzz;
			index4_vector<ScalarType, Size, 2, 2, 2, 3>		zzzw;
			index4_vector<ScalarType, Size, 2, 2, 3, 0>		zzwx;
			index4_vector<ScalarType, Size, 2, 2, 3, 1>		zzwy;
			index4_vector<ScalarType, Size, 2, 2, 3, 2>		zzwz;
			index4_vector<ScalarType, Size, 2, 2, 3, 3>		zzww;
			index4_vector<ScalarType, Size, 2, 3, 0, 0>		zwxx;
			index4_vector<ScalarType, Size, 2, 3, 0, 1>		zwxy;			// can modify as lvalue
			index4_vector<ScalarType, Size, 2, 3, 0, 2>		zwxz;
			index4_vector<ScalarType, Size, 2, 3, 0, 3>		zwxw;
			index4_vector<ScalarType, Size, 2, 3, 1, 0>		zwyx;			// can modify as lvalue
			index4_vector<ScalarType, Size, 2, 3, 1, 1>		zwyy;
			index4_vector<ScalarType, Size, 2, 3, 1, 2>		zwyz;
			index4_vector<ScalarType, Size, 2, 3, 1, 3>		zwyw;
			index4_vector<ScalarType, Size, 2, 3, 2, 0>		zwzx;
			index4_vector<ScalarType, Size, 2, 3, 2, 1>		zwzy;
			index4_vector<ScalarType, Size, 2, 3, 2, 2>		zwzz;
			index4_vector<ScalarType, Size, 2, 3, 2, 3>		zwzw;
			index4_vector<ScalarType, Size, 2, 3, 3, 0>		zwwx;
			index4_vector<ScalarType, Size, 2, 3, 3, 1>		zwwy;
			index4_vector<ScalarType, Size, 2, 3, 3, 2>		zwwz;
			index4_vector<ScalarType, Size, 2, 3, 3, 3>		zwww;
			index4_vector<ScalarType, Size, 3, 0, 0, 0>		wxxx;
			index4_vector<ScalarType, Size, 3, 0, 0, 1>		wxxy;
			index4_vector<ScalarType, Size, 3, 0, 0, 2>		wxxz;
			index4_vector<ScalarType, Size, 3, 0, 0, 3>		wxxw;
			index4_vector<ScalarType, Size, 3, 0, 1, 0>		wxyx;
			index4_vector<ScalarType, Size, 3, 0, 1, 1>		wxyy;
			index4_vector<ScalarType, Size, 3, 0, 1, 2>		wxyz;			// can modify as lvalue
			index4_vector<ScalarType, Size, 3, 0, 1, 3>		wxyw;
			index4_vector<ScalarType, Size, 3, 0, 2, 0>		wxzx;
			index4_vector<ScalarType, Size, 3, 0, 2, 1>		wxzy;			// can modify as lvalue
			index4_vector<ScalarType, Size, 3, 0, 2, 2>		wxzz;
			index4_vector<ScalarType, Size, 3, 0, 2, 3>		wxzw;
			index4_vector<ScalarType, Size, 3, 0, 3, 0>		wxwx;
			index4_vector<ScalarType, Size, 3, 0, 3, 1>		wxwy;
			index4_vector<ScalarType, Size, 3, 0, 3, 2>		wxwz;
			index4_vector<ScalarType, Size, 3, 0, 3, 3>		wxww;
			index4_vector<ScalarType, Size, 3, 1, 0, 0>		wyxx;
			index4_vector<ScalarType, Size, 3, 1, 0, 1>		wyxy;
			index4_vector<ScalarType, Size, 3, 1, 0, 2>		wyxz;			// can modify as lvalue
			index4_vector<ScalarType, Size, 3, 1, 0, 3>		wyxw;
			index4_vector<ScalarType, Size, 3, 1, 1, 0>		wyyx;
			index4_vector<ScalarType, Size, 3, 1, 1, 1>		wyyy;
			index4_vector<ScalarType, Size, 3, 1, 1, 2>		wyyz;
			index4_vector<ScalarType, Size, 3, 1, 1, 3>		wyyw;
			index4_vector<ScalarType, Size, 3, 1, 2, 0>		wyzx;			// can modify as lvalue
			index4_vector<ScalarType, Size, 3, 1, 2, 1>		wyzy;
			index4_vector<ScalarType, Size, 3, 1, 2, 2>		wyzz;
			index4_vector<ScalarType, Size, 3, 1, 2, 3>		wyzw;
			index4_vector<ScalarType, Size, 3, 1, 3, 0>		wywx;
			index4_vector<ScalarType, Size, 3, 1, 3, 1>		wywy;
			index4_vector<ScalarType, Size, 3, 1, 3, 2>		wywz;
			index4_vector<ScalarType, Size, 3, 1, 3, 3>		wyww;
			index4_vector<ScalarType, Size, 3, 2, 0, 0>		wzxx;
			index4_vector<ScalarType, Size, 3, 2, 0, 1>		wzxy;			// can modify as lvalue
			index4_vector<ScalarType, Size, 3, 2, 0, 2>		wzxz;
			index4_vector<ScalarType, Size, 3, 2, 0, 3>		wzxw;
			index4_vector<ScalarType, Size, 3, 2, 1, 0>		wzyx;			// can modify as lvalue
			index4_vector<ScalarType, Size, 3, 2, 1, 1>		wzyy;
			index4_vector<ScalarType, Size, 3, 2, 1, 2>		wzyz;
			index4_vector<ScalarType, Size, 3, 2, 1, 3>		wzyw;
			index4_vector<ScalarType, Size, 3, 2, 2, 0>		wzzx;
			index4_vector<ScalarType, Size, 3, 2, 2, 1>		wzzy;
			index4_vector<ScalarType, Size, 3, 2, 2, 2>		wzzz;
			index4_vector<ScalarType, Size, 3, 2, 2, 3>		wzzw;
			index4_vector<ScalarType, Size, 3, 2, 3, 0>		wzwx;
			index4_vector<ScalarType, Size, 3, 2, 3, 1>		wzwy;
			index4_vector<ScalarType, Size, 3, 2, 3, 2>		wzwz;
			index4_vector<ScalarType, Size, 3, 2, 3, 3>		wzww;
			index4_vector<ScalarType, Size, 3, 3, 0, 0>		wwxx;
			index4_vector<ScalarType, Size, 3, 3, 0, 1>		wwxy;
			index4_vector<ScalarType, Size, 3, 3, 0, 2>		wwxz;
			index4_vector<ScalarType, Size, 3, 3, 0, 3>		wwxw;
			index4_vector<ScalarType, Size, 3, 3, 1, 0>		wwyx;
			index4_vector<ScalarType, Size, 3, 3, 1, 1>		wwyy;
			index4_vector<ScalarType, Size, 3, 3, 1, 2>		wwyz;
			index4_vector<ScalarType, Size, 3, 3, 1, 3>		wwyw;
			index4_vector<ScalarType, Size, 3, 3, 2, 0>		wwzx;
			index4_vector<ScalarType, Size, 3, 3, 2, 1>		wwzy;
			index4_vector<ScalarType, Size, 3, 3, 2, 2>		wwzz;
			index4_vector<ScalarType, Size, 3, 3, 2, 3>		wwzw;
			index4_vector<ScalarType, Size, 3, 3, 3, 0>		wwwx;
			index4_vector<ScalarType, Size, 3, 3, 3, 1>		wwwy;
			index4_vector<ScalarType, Size, 3, 3, 3, 2>		wwwz;
			index4_vector<ScalarType, Size, 3, 3, 3, 3>		wwww;
		};

		//
		// defaulted functions
		//

		constexpr basic_vector() noexcept = default;
		constexpr ~basic_vector() noexcept = default;

		constexpr basic_vector(const basic_vector &) noexcept = default;
		constexpr basic_vector(basic_vector &&) noexcept = default;
		constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
		constexpr basic_vector &operator =(basic_vector &&) noexcept = default;

		//
		// constructors
		//

		template <typename OtherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType>
		explicit constexpr basic_vector(OtherScalarType value) noexcept
			: store()
		{
			init(value, value, value, value);
		}

		template <typename OtherScalarType0, typename OtherScalarType1, typename OtherScalarType2, typename OtherScalarType3>
		requires
			std::convertible_to<OtherScalarType0, ScalarType> && std::convertible_to<OtherScalarType1, ScalarType> &&
			std::convertible_to<OtherScalarType2, ScalarType> && std::convertible_to<OtherScalarType3, ScalarType>
		explicit constexpr basic_vector(OtherScalarType0 xvalue,
										OtherScalarType1 yvalue,
										OtherScalarType2 zvalue,
										OtherScalarType3 wvalue) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue, wvalue);
		}

		template <typename OtherScalarType0, typename OtherScalarType1, typename OtherScalarType2,
			bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived>
		requires
			std::convertible_to<OtherScalarType0, ScalarType> && std::convertible_to<OtherScalarType1, ScalarType> &&
			std::convertible_to<OtherScalarType2, ScalarType> && std::convertible_to<OtherScalarType, ScalarType>
		explicit constexpr basic_vector(OtherScalarType0 xvalue,
										OtherScalarType1 yvalue,
										OtherScalarType2 zvalue,
										const vector_base<Writable, OtherScalarType, Count, Derived> &wvalue_source) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue, wvalue_source[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector(const vector_base<Writable, OtherScalarType, Size, Derived> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], other[3u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived>
		requires (!implicitly_convertible_to<OtherScalarType, ScalarType> && std::convertible_to<OtherScalarType, ScalarType>)
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, Size, Derived> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], other[3u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived, typename YetAnotherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType>
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, 3u, Derived> &other,
										YetAnotherScalarType yet_another) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], yet_another);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived,
			bool YetAnotherWritable, dimensional_scalar YetAnotherScalarType, std::size_t Count, typename YetAnotherDerived>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType>
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, 3u, Derived> &other,
										const vector_base<YetAnotherWritable, YetAnotherScalarType, Count, YetAnotherDerived> &wvalue_source) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], wvalue_source[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived, typename YetAnotherScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType> && (Count >= 3)
		explicit constexpr basic_vector(YetAnotherScalarType yet_another,
										const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(yet_another, other[0u], other[1u], other[2u]);
		}

		template <bool FirstWritable, dimensional_scalar FirstScalarType, typename FirstDerived,
			bool SecondWritable, dimensional_scalar SecondScalarType, std::size_t SecondCount, typename SecondDerived>
		requires std::convertible_to<FirstScalarType, ScalarType> && std::convertible_to<SecondScalarType, ScalarType> && (SecondCount >= 2)
		explicit constexpr basic_vector(const vector_base<FirstWritable, FirstScalarType, 2u, FirstDerived> &first,
										const vector_base<SecondWritable, SecondScalarType, SecondCount, SecondDerived> &second) noexcept
			: store()
		{
			init(first[0u], first[1u], second[0u], second[1u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived,  typename FirstScalarType, typename SecondScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<FirstScalarType, ScalarType> && std::convertible_to<SecondScalarType, ScalarType>
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, 2u, Derived> &other,
										FirstScalarType first,
										SecondScalarType second) noexcept
			: store()
		{
			init(other[0u], other[1u], first, second);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived, typename FirstScalarType,
			bool YetAnotherWritable, dimensional_scalar YetAnotherScalarType, std::size_t Count, typename YetAnotherDerived>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<FirstScalarType, ScalarType> && std::convertible_to<YetAnotherScalarType, ScalarType>
		explicit constexpr basic_vector(const vector_base<Writable, OtherScalarType, 2u, Derived> &other,
										FirstScalarType first,
										const vector_base<YetAnotherWritable, YetAnotherScalarType, Count, YetAnotherDerived> &wvalue_source) noexcept
			: store()
		{
			init(other[0u], other[1u], first, wvalue_source[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived, typename FirstScalarType, typename SecondScalarType>
		requires std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<FirstScalarType, ScalarType> && std::convertible_to<SecondScalarType, ScalarType>
		explicit constexpr basic_vector(FirstScalarType first,
										const vector_base<Writable, OtherScalarType, 2u, Derived> &other,
										SecondScalarType second) noexcept
			: store()
		{
			init(first, other[0u], other[1u], second);
		}

		template <	bool Writable, dimensional_scalar OtherScalarType, typename Derived,
					typename FirstScalarType,
					bool YetAnotherWritable, dimensional_scalar YetAnotherScalarType, std::size_t Count, typename YetAnotherDerived>
		requires	std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<FirstScalarType, ScalarType> &&
					std::convertible_to<YetAnotherScalarType, ScalarType>
		explicit constexpr basic_vector(FirstScalarType first,
										const vector_base<Writable, OtherScalarType, 2u, Derived> &other,
										const vector_base<YetAnotherWritable, YetAnotherScalarType, Count, YetAnotherDerived> &wvalue_source) noexcept
			: store()
		{
			init(first, other[0u], other[1u], wvalue_source[0u]);
		}

		template <bool Writable, dimensional_scalar OtherScalarType, std::size_t Count, typename Derived,
				  typename FirstScalarType, typename SecondScalarType>
		requires	std::convertible_to<OtherScalarType, ScalarType> && std::convertible_to<FirstScalarType, ScalarType> &&
					std::convertible_to<SecondScalarType, ScalarType>  && (Count >= 2)
		explicit constexpr basic_vector(FirstScalarType first,
										SecondScalarType second,
										const vector_base<Writable, OtherScalarType, Count, Derived> &other) noexcept
			: store()
		{
			init(first, second, other[0u], other[1u]);
		}

		//
		// assignment operators
		//

		template <bool Writable, dimensional_scalar OtherScalarType, typename Derived>
		requires implicitly_convertible_to<OtherScalarType, ScalarType>
		constexpr basic_vector &operator =(const vector_base<Writable, OtherScalarType, Size, Derived> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);
			return *this;
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename OtherScalarType0, typename OtherScalarType1, typename OtherScalarType2, typename OtherScalarType3>
		constexpr void init(OtherScalarType0 value0, OtherScalarType1 value1, OtherScalarType2 value2, OtherScalarType3 value3) noexcept
		{
			store.value[0u] = static_cast<ScalarType>(value0);
			store.value[1u] = static_cast<ScalarType>(value1);
			store.value[2u] = static_cast<ScalarType>(value2);
			store.value[3u] = static_cast<ScalarType>(value3);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		ScalarType	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	ScalarType	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	//
	// operators and compound assignment operators
	//

	namespace detail
	{
		// convert a parameter pack into a basic_vector vector

		template <typename ...Ts>
		requires dimensional_storage<std::common_type_t<Ts...>, sizeof...(Ts)>
			constexpr auto parameter_pack_to_vec(Ts ...args) noexcept
		{
			using ArgType = std::common_type_t<Ts...>;
			constexpr std::size_t Size = sizeof...(Ts);

			return basic_vector<ArgType, Size>{(static_cast<ArgType>(args))...};
		}

		// convert basic array types to a basic_vector vector

		template <typename ScalarType, std::size_t Size, std::size_t ...Indexes>
		constexpr auto passthru_execute(std::index_sequence<Indexes...> /* dummy */,
										const std::array<ScalarType, Size> &arg) noexcept
		{
			return basic_vector<ScalarType, Size>(arg[Indexes]...);
		}

		template <typename ScalarType, std::size_t Size, std::size_t ...Indexes>
		constexpr auto passthru_execute(std::index_sequence<Indexes...> /* dummy */,
										const ScalarType(&arg)[Size]) noexcept
		{
			return basic_vector<ScalarType, Size>(arg[Indexes]...);
		}

		// return types from executing lambdas on arguments of various types

		template <typename UnOp, dsga::dimensional_scalar ScalarType>
		using unary_op_return_t = decltype(UnOp()(std::declval<ScalarType>()));

		template <typename BinOp, dsga::dimensional_scalar ScalarType, dsga::dimensional_scalar OtherScalarType>
		using binary_op_return_t = decltype(BinOp()(std::declval<ScalarType>(), std::declval<OtherScalarType>()));

		// perform the lambda action on components of vector_base arguments, returning a new basic_vector vector

		template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename UnOp, std::size_t ...Indexes>
		constexpr auto unary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										const vector_base<Writable, ScalarType, Count, Derived> &arg,
										UnOp &lambda) noexcept
		{
			return basic_vector<unary_op_return_t<UnOp, ScalarType>, Count>(lambda(arg[Indexes])...);
		}

		// when Count == 1, treat it like a scalar value
		template <bool Writable, dimensional_scalar ScalarType, typename Derived, typename UnOp, std::size_t ...Indexes>
		constexpr auto unary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										const vector_base<Writable, ScalarType, 1u, Derived> &arg,
										UnOp &lambda) noexcept
		{
			return static_cast<unary_op_return_t<UnOp, ScalarType>>(lambda(arg[0u]));
		}

		template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
			bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived, typename BinOp, std::size_t ...Indexes>
		constexpr auto binary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										 const vector_base<Writable, ScalarType, Count, Derived> &lhs,
										 const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs,
										 BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, ScalarType, OtherScalarType>, Count>(lambda(lhs[Indexes], rhs[Indexes])...);
		}

		template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
			dimensional_scalar OtherScalarType, typename BinOp, std::size_t ...Indexes>
		constexpr auto binary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										 const vector_base<Writable, ScalarType, Count, Derived> &lhs,
										 OtherScalarType rhs,
										 BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, ScalarType, OtherScalarType>, Count>(lambda(lhs[Indexes], rhs)...);
		}

		template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
			dimensional_scalar OtherScalarType, typename BinOp, std::size_t ...Indexes>
		constexpr auto binary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										 OtherScalarType lhs,
										 const vector_base<Writable, ScalarType, Count, Derived> &rhs,
										 BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, OtherScalarType, ScalarType>, Count>(lambda(lhs, rhs[Indexes])...);
		}

		// when Count == 1, treat it like a scalar value
		template <bool Writable, dimensional_scalar ScalarType, typename Derived,
			bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived, typename BinOp, std::size_t ...Indexes>
		constexpr auto binary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										 const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
										 const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs,
										 BinOp &lambda) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, ScalarType, OtherScalarType>>(lambda(lhs[0u], rhs[0u]));
		}

		// when Count == 1, treat it like a scalar value
		template <bool Writable, dimensional_scalar ScalarType, typename Derived,
			dimensional_scalar OtherScalarType, typename BinOp, std::size_t ...Indexes>
		constexpr auto binary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										 const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
										 OtherScalarType rhs,
										 BinOp &lambda) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, ScalarType, OtherScalarType>>(lambda(lhs[0u], rhs));
		}

		// when Count == 1, treat it like a scalar value
		template <bool Writable, dimensional_scalar ScalarType, typename Derived,
			dimensional_scalar OtherScalarType, typename BinOp, std::size_t ...Indexes>
		constexpr auto binary_op_execute(std::index_sequence<Indexes...> /* dummy */,
										 OtherScalarType lhs,
										 const vector_base<Writable, ScalarType, 1u, Derived> &rhs,
										 BinOp &lambda) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, OtherScalarType, ScalarType>>(lambda(lhs, rhs[0u]));
		}

		// perform the lambda action, setting the lhs vector_base to new values

		template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
			bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived, typename BinOp, std::size_t ...Indexes>
		requires Writable
		constexpr void binary_op_set(std::index_sequence<Indexes...> /* dummy */,
									 vector_base<Writable, ScalarType, Count, Derived> &lhs,
									 const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs,
									 BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Indexes], rhs[Indexes])...);
		}

		template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
			dimensional_scalar OtherScalarType, typename BinOp, std::size_t ...Indexes>
		requires Writable
		constexpr void binary_op_set(std::index_sequence<Indexes...> /* dummy */,
									 vector_base<Writable, ScalarType, Count, Derived> &lhs,
									 OtherScalarType rhs,
									 BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Indexes], rhs)...);
		}

	}

	//
	// operators
	//

	// binary operators +=, +

	constexpr inline auto plus_op = [](auto lhs, auto rhs) { return lhs + rhs; };

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator +=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, dimensional_scalar OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator +=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator +(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, plus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator +(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, plus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator +(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], plus_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator +(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, plus_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator +(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, plus_op);
	}

	// binary operators -=, -

	constexpr inline auto minus_op = [](auto lhs, auto rhs) { return lhs - rhs; };

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator -=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, dimensional_scalar OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator -=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator -(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, minus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator -(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, minus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator -(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], minus_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator -(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, minus_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator -(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, minus_op);
	}

	// binary operators *=, *

	constexpr inline auto times_op = [](auto lhs, auto rhs) { return lhs * rhs; };

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator *=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, dimensional_scalar OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator *=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator *(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, times_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator *(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, times_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator *(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], times_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator *(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, times_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator *(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, times_op);
	}

	// binary operators /=, /

	constexpr inline auto div_op = [](auto lhs, auto rhs) { return lhs / rhs; };

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator /=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, dimensional_scalar OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator /=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator /(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, div_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator /(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, div_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator /(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], div_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator /(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, div_op);
	}

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived, typename OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator /(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, div_op);
	}

	// binary operators %=, %

	constexpr inline auto mod_op = [](auto lhs, auto rhs) { return lhs % rhs; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator %=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, mod_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator %=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, mod_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator %(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, mod_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator %(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, mod_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator %(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], mod_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator %(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, mod_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator %(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, mod_op);
	}

	// unary operator ~

	constexpr inline auto bit_not_op = [](auto arg) { return ~arg; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	constexpr auto operator ~(const vector_base<Writable, ScalarType, Count, Derived> &arg) noexcept
	{
		return detail::unary_op_execute(std::make_index_sequence<Count>{}, arg, bit_not_op);
	}

	// binary operators <<=, <<

	constexpr inline auto lshift_op = [](auto lhs, auto rhs) { return lhs << rhs; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator <<=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								 const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator <<=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator <<(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							   const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, lshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator <<(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							   const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, lshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator <<(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							   const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], lshift_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator <<(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							   OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, lshift_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator <<(OtherScalarType lhs,
							   const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, lshift_op);
	}

	// binary operators >>=, >>

	constexpr inline auto rshift_op = [](auto lhs, auto rhs) { return lhs >> rhs; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator >>=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								 const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator >>=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator >>(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							   const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, rshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator >>(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							   const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, rshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator >>(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							   const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], rshift_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator >>(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							   OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, rshift_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator >>(OtherScalarType lhs,
							   const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, rshift_op);
	}

	// binary operators &=, &

	constexpr inline auto and_op = [](auto lhs, auto rhs) { return lhs & rhs; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator &=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator &=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator &(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, and_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator &(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, and_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator &(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], and_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator &(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, and_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator &(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, and_op);
	}

	// binary operators |=, |

	constexpr inline auto or_op = [](auto lhs, auto rhs) { return lhs | rhs; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator |=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator |=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator |(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, or_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator |(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, or_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator |(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], or_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator |(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, or_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator |(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, or_op);
	}

	// binary operators ^=, ^

	constexpr inline auto xor_op = [](auto lhs, auto rhs) { return lhs ^ rhs; };

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator ^=(vector_base<Writable, ScalarType, Count, Derived> &lhs,
								const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires Writable && implicitly_convertible_to<OtherScalarType, ScalarType>
	constexpr auto &operator ^=(vector_base<Writable, ScalarType, Count, Derived> &lhs, OtherScalarType rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<Count>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator ^(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, xor_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, std::size_t Count, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator ^(const vector_base<Writable, ScalarType, 1u, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, Count, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs[0u], rhs, xor_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived,
		bool OtherWritable, integral_dimensional_scalar OtherScalarType, typename OtherDerived>
	requires (implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>) && (Count > 1u)
	constexpr auto operator ^(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  const vector_base<OtherWritable, OtherScalarType, 1u, OtherDerived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs[0u], xor_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator ^(const vector_base<Writable, ScalarType, Count, Derived> &lhs,
							  OtherScalarType rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, xor_op);
	}

	template <bool Writable, integral_dimensional_scalar ScalarType, std::size_t Count, typename Derived, std::integral OtherScalarType>
	requires implicitly_convertible_to<OtherScalarType, ScalarType> || implicitly_convertible_to<ScalarType, OtherScalarType>
	constexpr auto operator ^(OtherScalarType lhs,
							  const vector_base<Writable, ScalarType, Count, Derived> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<Count>{}, lhs, rhs, xor_op);
	}

	// unary operator +

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires non_bool_arithmetic<ScalarType>
	constexpr basic_vector<ScalarType, Count> operator +(const vector_base<Writable, ScalarType, Count, Derived> &arg) noexcept
	{
		return basic_vector(arg);						// no-op copy
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived>
	requires non_bool_arithmetic<ScalarType>
	constexpr ScalarType operator +(const vector_base<Writable, ScalarType, 1u, Derived> &arg) noexcept
	{
		return arg[0u];									// no-op scalar copy
	}

	// unary operator -

	constexpr inline auto neg_op = [](auto arg) { return -arg; };

	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires non_bool_arithmetic<ScalarType>
	constexpr auto operator -(const vector_base<Writable, ScalarType, Count, Derived> &arg) noexcept
	{
		return detail::unary_op_execute(std::make_index_sequence<Count>{}, arg, neg_op);
	}

	// unary operators ++

	// pre-increment
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires Writable && non_bool_arithmetic<ScalarType>
	constexpr auto &operator ++(const vector_base<Writable, ScalarType, Count, Derived> &arg) noexcept
	{
		arg += ScalarType(1);
		return arg.as_derived();
	}

	// post-increment
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires Writable && non_bool_arithmetic<ScalarType>
	constexpr basic_vector<ScalarType, Count> operator ++(vector_base<Writable, ScalarType, Count, Derived> &arg, int) noexcept
	{
		basic_vector<ScalarType, Count> value(arg);
		arg += ScalarType(1);
		return value;
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived>
	requires Writable && non_bool_arithmetic<ScalarType>
	constexpr ScalarType operator ++(vector_base<Writable, ScalarType, 1u, Derived> &arg, int) noexcept
	{
		ScalarType value = arg[0u];
		arg += ScalarType(1);
		return value;
	}

	// unary operators --

	// pre-decrement
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires Writable && non_bool_arithmetic<ScalarType>
	constexpr auto &operator --(const vector_base<Writable, ScalarType, Count, Derived> &arg) noexcept
	{
		arg -= ScalarType(1);
		return arg.as_derived();
	}

	// post-decrement
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires Writable && non_bool_arithmetic<ScalarType>
	constexpr basic_vector<ScalarType, Count> operator --(vector_base<Writable, ScalarType, Count, Derived> &arg, int) noexcept
	{
		basic_vector<ScalarType, Count> value(arg);
		arg -= ScalarType(1);
		return value;
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived>
	requires Writable && non_bool_arithmetic<ScalarType>
	constexpr ScalarType operator --(vector_base<Writable, ScalarType, 1u, Derived> &arg, int) noexcept
	{
		ScalarType value = arg[0u];
		arg -= ScalarType(1);
		return value;
	}

	//
	// equality comparisons
	//

	template <bool FirstWritable, dimensional_scalar FirstScalarType, std::size_t Count, typename FirstDerived,
		bool SecondWritable, dimensional_scalar SecondScalarType, typename SecondDerived>
	requires implicitly_convertible_to<SecondScalarType, FirstScalarType>
	constexpr bool operator ==(const vector_base<FirstWritable, FirstScalarType, Count, FirstDerived> &first,
							   const vector_base<SecondWritable, SecondScalarType, Count, SecondDerived> &second) noexcept
	{
		for (std::size_t i = 0; i < Count; ++i)
			if (first[i] != static_cast<FirstScalarType>(second[i]))
				return false;

		return true;
	}

	template <bool FirstWritable, dimensional_scalar ScalarType, std::size_t Count, typename FirstDerived, bool SecondWritable, typename SecondDerived>
	constexpr bool operator ==(const vector_base<FirstWritable, ScalarType, Count, FirstDerived> &first,
							   const vector_base<SecondWritable, ScalarType, Count, SecondDerived> &second) noexcept
	{
		for (std::size_t i = 0; i < Count; ++i)
			if (first[i] != second[i])
				return false;

		return true;
	}

	// when Count == 1, treat it like a scalar value
	template <bool Writable, dimensional_scalar ScalarType, typename Derived, dimensional_scalar OtherScalarType>
	requires (std::same_as<ScalarType, bool> == std::same_as<OtherScalarType, bool>)
	constexpr bool operator ==(const vector_base<Writable, ScalarType, 1u, Derived> &first,
							   OtherScalarType second) noexcept
	{
		using CommonType = std::common_type_t<ScalarType, OtherScalarType>;
		return static_cast<CommonType>(first[0u]) == static_cast<CommonType>(second);
	}

	//
	// get<> part of tuple interface -- needed for structured bindings
	//

	template <int N, bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires (N >= 0) && (N < Count)
	constexpr auto && get(dsga::vector_base<Writable, ScalarType, Count, Derived> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires (N >= 0) && (N < Count)
	constexpr auto && get(const dsga::vector_base<Writable, ScalarType, Count, Derived> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires (N >= 0) && (N < Count)
	constexpr auto && get(dsga::vector_base<Writable, ScalarType, Count, Derived> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires (N >= 0) && (N < Count)
	constexpr auto && get(const dsga::vector_base<Writable, ScalarType, Count, Derived> && arg) noexcept
	{
		return std::move(arg[N]);
	}

}	// namespace dsga

//
// tuple inteface for basic_vector and indexed_vector and vec_base -- supports structured bindings
//

template<dsga::dimensional_scalar ScalarType, std::size_t Size>
struct std::tuple_size<dsga::basic_vector<ScalarType, Size>> : std::integral_constant<std::size_t, Size>
{
};

template <std::size_t element_index, dsga::dimensional_scalar ScalarType, std::size_t Size>
struct std::tuple_element<element_index, dsga::basic_vector<ScalarType, Size>>
{
	using type = ScalarType;
};

template <dsga::dimensional_scalar ScalarType, std::size_t Size, std::size_t IndexCount, std::size_t ...Indexes>
struct std::tuple_size<dsga::indexed_vector<ScalarType, Size, IndexCount, Indexes...>> : std::integral_constant<std::size_t, IndexCount>
{
};

template <std::size_t element_index, dsga::dimensional_scalar ScalarType, std::size_t Size, std::size_t IndexCount, std::size_t ...Indexes>
struct std::tuple_element<element_index, dsga::indexed_vector<ScalarType, Size, IndexCount, Indexes...>>
{
	using type = ScalarType;
};

template <bool Writable, dsga::dimensional_scalar ScalarType, std::size_t Count, typename Derived>
struct std::tuple_size<dsga::vector_base<Writable, ScalarType, Count, Derived>> : std::integral_constant<std::size_t, Count>
{
};

template <std::size_t element_index, bool Writable, dsga::dimensional_scalar ScalarType, std::size_t Count, typename Derived>
struct std::tuple_element<element_index, dsga::vector_base<Writable, ScalarType, Count, Derived>>
{
	using type = ScalarType;
};

// converting from external vector type or data to internal vector type

template <dsga::dimensional_scalar ScalarType, std::size_t Size>
requires dsga::dimensional_storage<ScalarType, Size>
constexpr auto to_vec(const std::array<ScalarType, Size> &arg) noexcept
{
	return dsga::detail::passthru_execute(std::make_index_sequence<Size>{}, arg);
}

template <dsga::dimensional_scalar ScalarType, std::size_t Size>
requires dsga::dimensional_storage<ScalarType, Size>
constexpr auto to_vec(const ScalarType (&arg)[Size]) noexcept
{
	return dsga::detail::passthru_execute(std::make_index_sequence<Size>{}, arg);
}

// converting from internal vector type to std::array

template <dsga::dimensional_scalar ScalarType, std::size_t Size>
constexpr std::array<ScalarType, Size> from_vec(const dsga::basic_vector<ScalarType, Size> &arg)
{
	return arg.store.value;
}

// fill vectors from spans

template <dsga::dimensional_scalar ScalarType, std::size_t Size, typename OtherScalarType, std::size_t Extent>
requires ((Extent != 0) && (Extent != std::dynamic_extent)) && dsga::non_bool_arithmetic<OtherScalarType> && std::convertible_to<OtherScalarType, ScalarType>
constexpr void copy_to_vec(dsga::basic_vector<ScalarType, Size> &lhs, std::span<OtherScalarType, Extent> rhs)
{
	constexpr std::size_t count = std::min(Size, Extent);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<ScalarType>(rhs[i]);
}

template <dsga::dimensional_scalar ScalarType, std::size_t Size, typename OtherScalarType, std::size_t Extent>
requires ((Extent == 0) || (Extent == std::dynamic_extent)) && dsga::non_bool_arithmetic<OtherScalarType> && std::convertible_to<OtherScalarType, ScalarType>
constexpr void copy_to_vec(dsga::basic_vector<ScalarType, Size> &lhs, std::span<OtherScalarType, Extent> rhs)
{
	std::size_t count = std::min(Size, rhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<ScalarType>(rhs[i]);
}

// fill spans from vectors

template <dsga::dimensional_scalar ScalarType, std::size_t Size, typename OtherScalarType, std::size_t Extent>
requires ((Extent != 0) && (Extent != std::dynamic_extent)) && dsga::non_bool_arithmetic<OtherScalarType> && std::convertible_to<ScalarType, OtherScalarType>
constexpr void copy_from_vec(std::span<OtherScalarType, Extent> lhs, const dsga::basic_vector<ScalarType, Size> &rhs)
{
	constexpr std::size_t count = std::min(Size, Extent);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<OtherScalarType>(rhs[i]);
}

template <dsga::dimensional_scalar ScalarType, std::size_t Size, typename OtherScalarType, std::size_t Extent>
requires ((Extent == 0) || (Extent == std::dynamic_extent)) && dsga::non_bool_arithmetic<OtherScalarType> && std::convertible_to<ScalarType, OtherScalarType>
constexpr void copy_from_vec(std::span<OtherScalarType, Extent> lhs, const dsga::basic_vector<ScalarType, Size> &rhs)
{
	std::size_t count = std::min(Size, lhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<OtherScalarType>(rhs[i]);
}

//
// specialized using types
//

// this 1D vector is a swizzlable scalar -- rough glsl analog to this with primitive types
template <dsga::dimensional_scalar ScalarType>
using vectype1 = dsga::basic_vector<ScalarType, 1u>;

// 2D vector
template <dsga::dimensional_scalar ScalarType>
using vectype2 = dsga::basic_vector<ScalarType, 2u>;

// 3D vector
template <dsga::dimensional_scalar ScalarType>
using vectype3 = dsga::basic_vector<ScalarType, 3u>;

// 4D vector
template <dsga::dimensional_scalar ScalarType>
using vectype4 = dsga::basic_vector<ScalarType, 4u>;

// boolean vectors
using bscal = vectype1<bool>;
using bvec2 = vectype2<bool>;
using bvec3 = vectype3<bool>;
using bvec4 = vectype4<bool>;

// int vectors
using iscal = vectype1<int>;
using ivec2 = vectype2<int>;
using ivec3 = vectype3<int>;
using ivec4 = vectype4<int>;

// unsigned int vectors
using uscal = vectype1<unsigned>;
using uvec2 = vectype2<unsigned>;
using uvec3 = vectype3<unsigned>;
using uvec4 = vectype4<unsigned>;

// long long vectors (not in glsl)
using llscal = vectype1<long long>;
using llvec2 = vectype2<long long>;
using llvec3 = vectype3<long long>;
using llvec4 = vectype4<long long>;

// unsigned long long vectors (not in glsl)
using ullscal = vectype1<unsigned long long>;
using ullvec2 = vectype2<unsigned long long>;
using ullvec3 = vectype3<unsigned long long>;
using ullvec4 = vectype4<unsigned long long>;

// float vectors with out an 'f' prefix -- this is from glsl
using scal = vectype1<float>;
using vec2 = vectype2<float>;
using vec3 = vectype3<float>;
using vec4 = vectype4<float>;

// also float vectors, but using the same naming convention as the other vectors do (not in glsl)
using fscal = vectype1<float>;
using fvec2 = vectype2<float>;
using fvec3 = vectype3<float>;
using fvec4 = vectype4<float>;

// double vectors
using dscal = vectype1<double>;
using dvec2 = vectype2<double>;
using dvec3 = vectype3<double>;
using dvec4 = vectype4<double>;

// closing include guard
#endif
