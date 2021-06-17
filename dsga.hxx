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
#include <numbers>
#include <bit>						// bit_cast
#include "cxcm.hxx"

//
// Data Structures for Geometric Algebra (dsga)
//

// version info

constexpr inline int DSGA_MAJOR_VERSION = 0;
constexpr inline int DSGA_MINOR_VERSION = 2;
constexpr inline int DSGA_PATCH_VERSION = 0;

namespace dsga
{
	//
	// helper template function to convert a std::index_sequence to a std::array.
	// can be used for indirect indexing.
	//

	template <std::size_t... Is>
	static constexpr std::array<std::size_t, sizeof...(Is)> make_sequence_array(std::index_sequence<Is...> /* dummy */)
	{
		return { Is... };
	}

	// plain undecorated arithmetic types
	template <typename T>
	concept dimensional_scalar = std::is_arithmetic_v<T> && std::same_as<T, std::remove_cvref_t<T>>;

	// plain undecorated integral types
	template <typename T>
	concept integral_dimensional_scalar = std::integral<T> && dimensional_scalar<T>;

	// plain undecorated floating point types
	template <typename T>
	concept floating_point_dimensional_scalar = std::floating_point<T> && dimensional_scalar<T>;

	// plain undecorated boolean type
	template <typename T>
	concept boolean_dimensional_scalar = std::same_as<bool, T> && dimensional_scalar<T>;

	// want the size to be between 1 and 4, inclusive
	template <std::size_t Size>
	concept dimensional_size = ((Size >= 1u) && (Size <= 4u));

	// dimensional storage needs the arithmetic type and size restrictions
	template <typename T, std::size_t Size>
	concept dimensional_storage = dimensional_scalar<T> && dimensional_size<Size>;

	// we want dimensional_storage_t to have length from 1 to 4 (1 gives just a sneaky kind of T that can swizzle),
	// and the storage has to have room for all the data. We also need dimensional_storage_t to support operator[] to access
	// the data. It needs to also support iterators so we can use it in ranged-for loops, algorithms, etc.

	// the underlying storage for vector and indexed_vector types. originally this was to be a template parameter and
	// fairly generic, but that is a detail that can happen in a future version of this library. it makes things
	// much simpler to not have to pass this stuff around in template parameters.

	// as alluded to above, dimensional_storage_t has two roles: 1) the storage in a vector for each dimension of
	// the vector, and 2) the backing storage used by a swizzle of a vector (storage is in that vector), that
	// is used to index into as required by the swizzle.

	// this implementation uses std::array as the backing storage type. It satisfies the requirements described above.

	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
	using dimensional_storage_t = std::array<T, Size>;


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

	// common initial sequence wrapper with basic storage access -- forwards function calls to wrapped storage.
	// this struct is an aggregate
	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
	struct storage_wrapper
	{
		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical storage - indirection is same as physical contiguous order.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		dimensional_storage_t<T, Size> value;

		constexpr		std::size_t	size()							const	noexcept	{ return Size; }

		// physically contiguous access to data
		constexpr		T			&operator [](std::size_t index)			noexcept	{ return value[index]; }
		constexpr const T			&operator [](std::size_t index)	const	noexcept	{ return value[index]; }

		constexpr			T *		data()									noexcept	{ return value.data(); }
		constexpr	const	T * 	data()							const	noexcept	{ return value.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr		auto		sequence()								noexcept	{ return sequence_pack{}; }

		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr		void		set(Args ...args)						noexcept
		{
			[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args)
			{
				((value[Js] = static_cast<T>(same_args)),...);
			}(std::make_index_sequence<Count>{}, args...);
		}

		// support for range-for loop
		constexpr		auto		begin()									noexcept	{ return value.begin(); }
		constexpr		auto		begin()							const	noexcept	{ return value.cbegin(); }
		constexpr		auto		cbegin()						const	noexcept	{ return value.cbegin(); }
		constexpr		auto		end()									noexcept	{ return value.end(); }
		constexpr		auto		end()							const	noexcept	{ return value.cend(); }
		constexpr		auto		cend()							const	noexcept	{ return value.cend(); }

	};


	namespace detail
	{
		// the concepts will help indexed_vector determine if it can be assigned to, like an lvalue reference,
		// i.e., if all indexes are unique then it can be used as an lvalue reference, i.e., is writable to.

		// see if all the std::size_t index values are unique

		template <std::size_t ...Is>
		struct unique_indexes_impl;

		template <>
		struct unique_indexes_impl<> : std::true_type
		{
		};

		template <std::size_t Index>
		struct unique_indexes_impl<Index> : std::true_type
		{
		};

		template <std::size_t First, std::size_t ...Rest>
		struct unique_indexes_impl<First, Rest...>
		{
			static constexpr bool value = ((First != Rest) && ...) && unique_indexes_impl<Rest...>::value;
		};

		// all Index values must be in ranged [0, Size) -- not checking here about Size and number of Is

		template <std::size_t Size, std::size_t ...Is>
		struct valid_indexes_impl
		{
			static constexpr bool value = ((Is < Size) && ...);
		};

	}

	// concepts required to build concept for testing for writable swizzle indexes

	template <std::size_t ...Is>
	concept unique_indexes = (sizeof...(Is) > 0) && detail::unique_indexes_impl<Is...>::value;

	template <std::size_t Count, std::size_t ...Is>
	concept valid_index_count = (sizeof...(Is) == Count) && dimensional_size<Count>;

	template <std::size_t Size, std::size_t ...Is>
	concept valid_range_indexes = detail::valid_indexes_impl<Size, Is...>::value;

	// writable_swizzle can determine whether a particular indexed_vector can be used as an lvalue reference

	template <std::size_t Size, std::size_t Count, std::size_t ...Is>
	constexpr inline bool writable_swizzle = valid_index_count<Count, Is...> && unique_indexes<Is...> && valid_range_indexes<Size, Is...>;

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

	// This is a CRTP base struct for the vector structs, primarily for data access.
	// It will help with arithmetic operators, compound assignment operators, and functions.
	// 
	// template parameters:
	// 
	//		Writable - bool value about whether struct can be modified (e.g., "foo.set(3, 4, 5);", "foo[0] = 3;")
	//		ScalarType - the type stored
	//		Count - the number of indexes available to access ScalarType data
	//		Derived - the CRTP struct/class that is derived from this struct
	//
	// It provides:
	// 
	// 		set() - relies on init() in Derived - access in logical order
	// 		operator[] - relies on at() in Derived - access in logical order
	// 		size() - relies on Count template parameter
	//		data() - relies on raw_data() in Derived - access in physical order
	// 		sequence() - relies on make_sequence_pack() in Derived - the physical order to logical order mapping
	//
	template <bool Writable, dimensional_scalar ScalarType, std::size_t Count, typename Derived>
	requires dimensional_storage<ScalarType, Count>
	struct vector_base
	{
		// CRTP access to Derived class
		constexpr		Derived		&as_derived()							noexcept	requires Writable	{ return static_cast<Derived &>(*this); }
		constexpr const Derived		&as_derived()					const	noexcept						{ return static_cast<const Derived &>(*this); }

		// logically contiguous write access to all data that allows for self-assignment that works properly
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, ScalarType> &&...)
		constexpr		void		set(Args ...args)						noexcept						{ this->as_derived().init(args...); }

		// logically contiguous access to piecewise data as index goes from 0 to (Count - 1)
		constexpr		ScalarType	&operator [](std::size_t index)			noexcept	requires Writable	{ return this->as_derived().at(index); }
		constexpr const	ScalarType	&operator [](std::size_t index)	const	noexcept						{ return this->as_derived().at(index); }

		// physically contiguous access via pointer
		constexpr			ScalarType *		data()						noexcept	requires Writable	{ return this->as_derived().raw_data(); }
		constexpr	const	ScalarType *		data()				const	noexcept						{ return this->as_derived().raw_data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr		auto					sequence()			const	noexcept						{ return this->as_derived().make_sequence_pack(); }

		// number of accessible T elements
		constexpr		std::size_t	size()							const	noexcept						{ return Count; }
	};

	// basic_vector will act as the primary vector class in this library.
	//
	// T is the type of the elements stored in the vector/storage
	// Size is number of elements referencable in vector/storage

	// the foundational vector type for dsga
	// 
	// template parameters:
	//
	//		ScalarType - the type stored
	//		Size - the number of actual elements in storage
	//
	template <dimensional_scalar ScalarType, std::size_t Size>
	requires dimensional_storage<ScalarType, Size>
	struct basic_vector;

	// indexed_vector will act as a swizzle of a basic_vector. basic_vector relies on the anonymous union of indexed_vector data members.
	//
	// T is the type of the elements stored in the underlying storage
	// Size relates to the number of elements in the underlying storage, which informs the values the Is can hold
	// Count is the number of elements accessible in swizzle -- often works alongside with basic_vector's Size
	// Is... are the number of swizzlable values available -- there are Count of them, and their values are in the range:  0 <= Indexes < Size

	// we want indexed_vector (vector swizzles) to have length from 1 to 4 (1 is just a sneaky type of T swizzle) in order
	// to work with the basic_vector which also has these lengths. The number of indexes is the same as the Count, between 1 and 4.
	// The indexes are valid for indexing into the values in the storage which is Size big.

	template <typename T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	concept indexable = dimensional_storage<T, Size> && valid_index_count<Count, Is...> && valid_range_indexes<Size, Is...>;

	// the type of a basic_vector swizzle
	// 
	// template parameters:
	//
	//		ScalarType - the type stored
	//		Size - the number of actual elements in storage
	//		Count - the number of indexes available to access ScalarType data
	//		Is - an ordered variable set of indexes into the storage -- there will be Count of them
	//
	template <typename ScalarType, std::size_t Size, std::size_t Count, std::size_t ...Is>
	struct indexed_vector;

	//
	// iterators for indexed_vector so it can participate in range-for loop
	//

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable<T, Size, Count, Is...>
	struct indexed_vector_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = T *;
		using reference = T &;

		constexpr static std::size_t begin_index = 0;
		constexpr static std::size_t end_index = Count;

		// the data
		indexed_vector<T, Size, Count, Is ...> *mapper_ptr;
		std::size_t mapper_index;

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr indexed_vector_iterator(indexed_vector<T, Size, Count, Is ...> &mapper, std::size_t index) noexcept
		{
			mapper_ptr = std::addressof(mapper);
			mapper_index = (index > Count) ? Count : index;			// std::size_t, so don't need lower bound check
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
			if (mapper_index < Count)
				mapper_index++;
			return *this;
		}

		constexpr indexed_vector_iterator operator++(int) noexcept
		{
			indexed_vector_iterator temp = *this;
			if (mapper_index < Count)
				mapper_index++;
			return temp;
		}

		constexpr bool operator ==(const indexed_vector_iterator &other) const noexcept
		{
			return (this->mapper_ptr == other.mapper_ptr) && (this->mapper_index == other.mapper_index);
		}
	};

	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
	requires indexable <T, Size, Count, Is...>
	struct indexed_vector_const_iterator
	{
		// publicly need these type using declarations or typedefs in iterator class,
		// since c++17 deprecated std::iterator
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = const T *;
		using reference = const T &;

		constexpr static std::size_t begin_index = 0;
		constexpr static std::size_t end_index = Count;

		// the data
		const indexed_vector<T, Size, Count, Is ...> *mapper_ptr;
		std::size_t mapper_index;

		// index == 0 is begin iterator
		// index == Count is end iterator -- clamp index in [0, Count] range
		constexpr indexed_vector_const_iterator(const indexed_vector<T, Size, Count, Is ...> &mapper, std::size_t index) noexcept
		{
			mapper_ptr = std::addressof(mapper);
			mapper_index = (index > Count) ? Count : index;			// std::size_t, so don't need lower bound check
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
			if (mapper_index < Count)
				mapper_index++;
			return *this;
		}

		constexpr indexed_vector_const_iterator operator++(int) noexcept
		{
			indexed_vector_const_iterator temp = *this;
			if (mapper_index < Count)
				mapper_index++;
			return temp;
		}

		constexpr bool operator ==(const indexed_vector_const_iterator &other) const noexcept
		{
			return (this->mapper_ptr == other.mapper_ptr) && (this->mapper_index == other.mapper_index);
		}
	};

	// for swizzling 2D-4D parts of basic_vector
	template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	requires indexable<T, Size, Count, Is...>
	struct indexed_vector<T, Size, Count, Is...>
		: vector_base<writable_swizzle<Size, Count, Is...>, T, Count, indexed_vector<T, Size, Count, Is...>>
	{
		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, Is...>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<Is...>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> value;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename ... Args>
		requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
		constexpr void init(Args ...args) noexcept
		{
			((value[Is] = static_cast<T>(args)),...);
		}

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			[&] <std::size_t ...Js>(std::index_sequence<Js ...> /* dummy */)
			{
				init( other[Js]... );
			}(std::make_index_sequence<Count>{});

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			[&] <std::size_t ...Js>(std::index_sequence<Js ...> /* dummy */)
			{
				init( other[Js]... );
			}(std::make_index_sequence<Count>{});

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr T &at(std::size_t index) noexcept requires Writable
		{
			return value[sequence_array[index]];
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const T &at(std::size_t index) const noexcept
		{
			return value[sequence_array[index]];
		}

		// physically contiguous -- used by data() for read/write access to data
		constexpr T *raw_data() noexcept
		{
			return value.data();
		}

		// physically contiguous -- used by data() for read access to data
		constexpr const T * raw_data() const noexcept
		{
			return value.data();
		}

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr auto make_sequence_pack() const noexcept
		{
			return sequence_pack{};
		}

		// support for range-for loop
		constexpr auto begin()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, Is...>(*this, 0u); }
		constexpr auto begin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, 0u); }
		constexpr auto cbegin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, 0u); }
		constexpr auto end()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, Is...>(*this, Count); }
		constexpr auto end()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, Count); }
		constexpr auto cend()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, Is...>(*this, Count); }
	};

	// for swizzling 1D parts of basic_vector - like a scalar accessor
	template <dimensional_scalar T, std::size_t Size, std::size_t I>
	requires indexable<T, Size, 1u, I>
	struct indexed_vector<T, Size, 1u, I>
		: vector_base<writable_swizzle<Size, 1u, I>, T, 1u, indexed_vector<T, Size, 1u, I>>
	{
		// we have partial specialization, so can't use template parameter for Count number of logical storage elements
		static constexpr std::size_t Count = 1u;

		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, I>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<I>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> value;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename U>
		requires Writable && std::convertible_to<U, T>
		constexpr void init(U other) noexcept
		{
			value[I] = static_cast<T>(other);
		}

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init( other[0u] );

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			init( other[0u] );

			return *this;
		}

		// scalar assignment
		// assignment for some scalar type that converts to T and is only for indexed_vector of [Size == 1]
		template <dimensional_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(U other) noexcept
		{
			init(other);

			return *this;
		}

		constexpr indexed_vector &operator =(T other) noexcept
		{
			init(other);

			return *this;
		}

		// scalar conversion operator
		// this is extremely important and is only for indexed_vector of [Count == 1]
		constexpr operator T() const noexcept
		{
			return value[I];
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr T &at(std::size_t index) noexcept requires Writable
		{
			return value[sequence_array[index]];
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const T &at(std::size_t index) const noexcept
		{
			return value[sequence_array[index]];
		}

		// physically contiguous -- used by data() for read/write access to data
		constexpr T *raw_data() noexcept
		{
			return value.data();
		}

		// physically contiguous -- used by data() for read access to data
		constexpr const T * raw_data() const noexcept
		{
			return value.data();
		}

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr auto make_sequence_pack() const noexcept
		{
			return sequence_pack{};
		}

		// support for range-for loop
		constexpr auto begin()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto begin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto cbegin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto end()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I>(*this, Count); }
		constexpr auto end()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, Count); }
		constexpr auto cend()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, Count); }
	};


	// convenience using types for indexed_vector as members of basic_vector

	template <typename T, std::size_t Size, std::size_t I>
	using dexvec1 = indexed_vector<std::remove_cvref_t<T>, Size, 1u, I>;

	template <typename T, std::size_t Size, std::size_t ...Is>
	using dexvec2 = indexed_vector<std::remove_cvref_t<T>, Size, 2u, Is...>;

	template <typename T, std::size_t Size, std::size_t ...Is>
	using dexvec3 = indexed_vector<std::remove_cvref_t<T>, Size, 3u, Is...>;

	template <typename T, std::size_t Size, std::size_t ...Is>
	using dexvec4 = indexed_vector<std::remove_cvref_t<T>, Size, 4u, Is...>;


	template <dimensional_scalar T>
	struct basic_vector<T, 1u> : vector_base<true, T, 1u, basic_vector<T, 1u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 1u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			store;

			dexvec1<T, Size, 0>					x;				// Writable

			dexvec2<T, Size, 0, 0>				xx;

			dexvec3<T, Size, 0, 0, 0>			xxx;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
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

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]) }
		{
		}

		template <typename U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value) }
		{
		}

		template <typename U>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value) }
		{
		}

		//
		// implicit assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u]);
			return *this;
		}

		template <dimensional_scalar U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(U value) noexcept
		{
			init(value);
			return *this;
		}

		//
		// conversion operators
		//

		// this is extremely important and is only for basic_vector of [Size == 1]

		template <typename U>
		requires implicitly_convertible_to<T, U>
		constexpr operator U() const noexcept
		{
			return static_cast<U>(store.value[0u]);
		}

		template <typename U>
		requires (!implicitly_convertible_to<T, U>) && std::convertible_to<T, U>
		explicit constexpr operator U() const noexcept
		{
			return static_cast<U>(store.value[0u]);
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename U>
		requires std::convertible_to<U, T>
		constexpr void init(U value) noexcept
		{
			store.value[0u] = static_cast<T>(value);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// physically contiguous -- used by data()
		constexpr			T *		raw_data()						noexcept	{ return store.value.data(); }
		constexpr	const	T *		raw_data()				const	noexcept	{ return store.value.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr		auto		make_sequence_pack()	const	noexcept	{ return sequence_pack{}; }

		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 2u> : vector_base<true, T, 2u, basic_vector<T, 2u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 2u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Size>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			store;

			dexvec1<T, Size, 0>					x;				// Writable
			dexvec1<T, Size, 1>					y;				// Writable

			dexvec2<T, Size, 0, 0>				xx;
			dexvec2<T, Size, 0, 1>				xy;				// Writable
			dexvec2<T, Size, 1, 0>				yx;				// Writable
			dexvec2<T, Size, 1, 1>				yy;

			dexvec3<T, Size, 0, 0, 0>			xxx;
			dexvec3<T, Size, 0, 0, 1>			xxy;
			dexvec3<T, Size, 0, 1, 0>			xyx;
			dexvec3<T, Size, 0, 1, 1>			xyy;
			dexvec3<T, Size, 1, 0, 0>			yxx;
			dexvec3<T, Size, 1, 0, 1>			yxy;
			dexvec3<T, Size, 1, 1, 0>			yyx;
			dexvec3<T, Size, 1, 1, 1>			yyy;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
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

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to <U2, T>
		explicit constexpr basic_vector(U1 xvalue, U2 yvalue) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue) }
		{
		}

		template <typename U1, bool W, dimensional_scalar U2, std::size_t C, typename D>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(U1 xvalue, const vector_base<W, U2, C, D> &yvalue_source) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue_source[0u]) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T> && (C >= Count))
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
		}

		//
		// assignment operator
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u]);
			return *this;
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void init(Args ...args) noexcept
		{
			[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args)
			{
				((store.value[Js] = static_cast<T>(same_args)),...);
			}(make_sequence_pack(), args...);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// physically contiguous -- used by data()
		constexpr			T *		raw_data()						noexcept	{ return store.value.data(); }
		constexpr	const	T *		raw_data()				const	noexcept	{ return store.value.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr		auto		make_sequence_pack()	const	noexcept	{ return sequence_pack{}; }

		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 3u> : vector_base<true, T, 3u, basic_vector<T, 3u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 3u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			store;

			dexvec1<T, Size, 0>					x;				// Writable
			dexvec1<T, Size, 1>					y;				// Writable
			dexvec1<T, Size, 2>					z;				// Writable

			dexvec2<T, Size, 0, 0>				xx;
			dexvec2<T, Size, 0, 1>				xy;				// Writable
			dexvec2<T, Size, 0, 2>				xz;				// Writable
			dexvec2<T, Size, 1, 0>				yx;				// Writable
			dexvec2<T, Size, 1, 1>				yy;
			dexvec2<T, Size, 1, 2>				yz;				// Writable
			dexvec2<T, Size, 2, 0>				zx;				// Writable
			dexvec2<T, Size, 2, 1>				zy;				// Writable
			dexvec2<T, Size, 2, 2>				zz;

			dexvec3<T, Size, 0, 0, 0>			xxx;
			dexvec3<T, Size, 0, 0, 1>			xxy;
			dexvec3<T, Size, 0, 0, 2>			xxz;
			dexvec3<T, Size, 0, 1, 0>			xyx;
			dexvec3<T, Size, 0, 1, 1>			xyy;
			dexvec3<T, Size, 0, 1, 2>			xyz;			// Writable
			dexvec3<T, Size, 0, 2, 0>			xzx;
			dexvec3<T, Size, 0, 2, 1>			xzy;			// Writable
			dexvec3<T, Size, 0, 2, 2>			xzz;
			dexvec3<T, Size, 1, 0, 0>			yxx;
			dexvec3<T, Size, 1, 0, 1>			yxy;
			dexvec3<T, Size, 1, 0, 2>			yxz;			// Writable
			dexvec3<T, Size, 1, 1, 0>			yyx;
			dexvec3<T, Size, 1, 1, 1>			yyy;
			dexvec3<T, Size, 1, 1, 2>			yyz;
			dexvec3<T, Size, 1, 2, 0>			yzx;			// Writable
			dexvec3<T, Size, 1, 2, 1>			yzy;
			dexvec3<T, Size, 1, 2, 2>			yzz;
			dexvec3<T, Size, 2, 0, 0>			zxx;
			dexvec3<T, Size, 2, 0, 1>			zxy;			// Writable
			dexvec3<T, Size, 2, 0, 2>			zxz;
			dexvec3<T, Size, 2, 1, 0>			zyx;			// Writable
			dexvec3<T, Size, 2, 1, 1>			zyy;
			dexvec3<T, Size, 2, 1, 2>			zyz;
			dexvec3<T, Size, 2, 2, 0>			zzx;
			dexvec3<T, Size, 2, 2, 1>			zzy;
			dexvec3<T, Size, 2, 2, 2>			zzz;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dexvec4<T, Size, 0, 0, 0, 2>		xxxz;
			dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dexvec4<T, Size, 0, 0, 1, 2>		xxyz;
			dexvec4<T, Size, 0, 0, 2, 0>		xxzx;
			dexvec4<T, Size, 0, 0, 2, 1>		xxzy;
			dexvec4<T, Size, 0, 0, 2, 2>		xxzz;
			dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dexvec4<T, Size, 0, 1, 0, 2>		xyxz;
			dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dexvec4<T, Size, 0, 1, 1, 2>		xyyz;
			dexvec4<T, Size, 0, 1, 2, 0>		xyzx;
			dexvec4<T, Size, 0, 1, 2, 1>		xyzy;
			dexvec4<T, Size, 0, 1, 2, 2>		xyzz;
			dexvec4<T, Size, 0, 2, 0, 0>		xzxx;
			dexvec4<T, Size, 0, 2, 0, 1>		xzxy;
			dexvec4<T, Size, 0, 2, 0, 2>		xzxz;
			dexvec4<T, Size, 0, 2, 1, 0>		xzyx;
			dexvec4<T, Size, 0, 2, 1, 1>		xzyy;
			dexvec4<T, Size, 0, 2, 1, 2>		xzyz;
			dexvec4<T, Size, 0, 2, 2, 0>		xzzx;
			dexvec4<T, Size, 0, 2, 2, 1>		xzzy;
			dexvec4<T, Size, 0, 2, 2, 2>		xzzz;
			dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dexvec4<T, Size, 1, 0, 0, 2>		yxxz;
			dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dexvec4<T, Size, 1, 0, 1, 2>		yxyz;
			dexvec4<T, Size, 1, 0, 2, 0>		yxzx;
			dexvec4<T, Size, 1, 0, 2, 1>		yxzy;
			dexvec4<T, Size, 1, 0, 2, 2>		yxzz;
			dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dexvec4<T, Size, 1, 1, 0, 2>		yyxz;
			dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
			dexvec4<T, Size, 1, 1, 1, 2>		yyyz;
			dexvec4<T, Size, 1, 1, 2, 0>		yyzx;
			dexvec4<T, Size, 1, 1, 2, 1>		yyzy;
			dexvec4<T, Size, 1, 1, 2, 2>		yyzz;
			dexvec4<T, Size, 1, 2, 0, 0>		yzxx;
			dexvec4<T, Size, 1, 2, 0, 1>		yzxy;
			dexvec4<T, Size, 1, 2, 0, 2>		yzxz;
			dexvec4<T, Size, 1, 2, 1, 0>		yzyx;
			dexvec4<T, Size, 1, 2, 1, 1>		yzyy;
			dexvec4<T, Size, 1, 2, 1, 2>		yzyz;
			dexvec4<T, Size, 1, 2, 2, 0>		yzzx;
			dexvec4<T, Size, 1, 2, 2, 1>		yzzy;
			dexvec4<T, Size, 1, 2, 2, 2>		yzzz;
			dexvec4<T, Size, 2, 0, 0, 0>		zxxx;
			dexvec4<T, Size, 2, 0, 0, 1>		zxxy;
			dexvec4<T, Size, 2, 0, 0, 2>		zxxz;
			dexvec4<T, Size, 2, 0, 1, 0>		zxyx;
			dexvec4<T, Size, 2, 0, 1, 1>		zxyy;
			dexvec4<T, Size, 2, 0, 1, 2>		zxyz;
			dexvec4<T, Size, 2, 0, 2, 0>		zxzx;
			dexvec4<T, Size, 2, 0, 2, 1>		zxzy;
			dexvec4<T, Size, 2, 0, 2, 2>		zxzz;
			dexvec4<T, Size, 2, 1, 0, 0>		zyxx;
			dexvec4<T, Size, 2, 1, 0, 1>		zyxy;
			dexvec4<T, Size, 2, 1, 0, 2>		zyxz;
			dexvec4<T, Size, 2, 1, 1, 0>		zyyx;
			dexvec4<T, Size, 2, 1, 1, 1>		zyyy;
			dexvec4<T, Size, 2, 1, 1, 2>		zyyz;
			dexvec4<T, Size, 2, 1, 2, 0>		zyzx;
			dexvec4<T, Size, 2, 1, 2, 1>		zyzy;
			dexvec4<T, Size, 2, 1, 2, 2>		zyzz;
			dexvec4<T, Size, 2, 2, 0, 0>		zzxx;
			dexvec4<T, Size, 2, 2, 0, 1>		zzxy;
			dexvec4<T, Size, 2, 2, 0, 2>		zzxz;
			dexvec4<T, Size, 2, 2, 1, 0>		zzyx;
			dexvec4<T, Size, 2, 2, 1, 1>		zzyy;
			dexvec4<T, Size, 2, 2, 1, 2>		zzyz;
			dexvec4<T, Size, 2, 2, 2, 0>		zzzx;
			dexvec4<T, Size, 2, 2, 2, 1>		zzzy;
			dexvec4<T, Size, 2, 2, 2, 2>		zzzz;
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

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value), static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue) }
		{
		}

		template <typename U1, typename U2, bool W, dimensional_scalar U3, std::size_t C, typename D>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										const vector_base<W, U3, C, D> &zvalue_source) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue_source[0u]) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T> && (C >= Count))
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 2u, D> &other,
										U2 yet_another) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(yet_another) }
		{
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, typename U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &other,
										const vector_base<W2, U2, C, D2> &yet_another) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(yet_another[0u]) }
		{
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 2)
		explicit constexpr basic_vector(U2 yet_another,
										const vector_base<W, U1, C, D> &other) noexcept
			: store{ static_cast<T>(yet_another), static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
		}

		//
		// assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);
			return *this;
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void init(Args ...args) noexcept
		{
			[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args)
			{
				((store.value[Js] = static_cast<T>(same_args)),...);
			}(std::make_index_sequence<Count>{}, args...);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// physically contiguous -- used by data()
		constexpr			T *		raw_data()						noexcept	{ return store.value.data(); }
		constexpr	const	T *		raw_data()				const	noexcept	{ return store.value.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr		auto		make_sequence_pack()	const	noexcept	{ return sequence_pack{}; }


		// support for range-for loop
		constexpr auto begin()			noexcept	{ return store.value.begin(); }
		constexpr auto begin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto cbegin()	const	noexcept	{ return store.value.cbegin(); }
		constexpr auto end()			noexcept	{ return store.value.end(); }
		constexpr auto end()	const	noexcept	{ return store.value.cend(); }
		constexpr auto cend()	const	noexcept	{ return store.value.cend(); }
	};

	template <dimensional_scalar T>
	struct basic_vector<T, 4u> : vector_base<true, T, 4u, basic_vector<T, 4u>>
	{
		// number of physical storage elements
		static constexpr std::size_t Size = 4u;

		// number of indexable elements
		static constexpr std::size_t Count = Size;

		// this can be used as an lvalue
		static constexpr bool Writable = true;

		//
		// the underlying ordered storage sequence for this physical vector - indirection is same as physical contiguous order.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>			store;

			dexvec1<T, Size, 0>					x;				// Writable
			dexvec1<T, Size, 1>					y;				// Writable
			dexvec1<T, Size, 2>					z;				// Writable
			dexvec1<T, Size, 3>					w;				// Writable

			dexvec2<T, Size, 0, 0>				xx;
			dexvec2<T, Size, 0, 1>				xy;				// Writable
			dexvec2<T, Size, 0, 2>				xz;				// Writable
			dexvec2<T, Size, 0, 3>				xw;				// Writable
			dexvec2<T, Size, 1, 0>				yx;				// Writable
			dexvec2<T, Size, 1, 1>				yy;
			dexvec2<T, Size, 1, 2>				yz;				// Writable
			dexvec2<T, Size, 1, 3>				yw;				// Writable
			dexvec2<T, Size, 2, 0>				zx;				// Writable
			dexvec2<T, Size, 2, 1>				zy;				// Writable
			dexvec2<T, Size, 2, 2>				zz;
			dexvec2<T, Size, 2, 3>				zw;				// Writable
			dexvec2<T, Size, 3, 0>				wx;				// Writable
			dexvec2<T, Size, 3, 1>				wy;				// Writable
			dexvec2<T, Size, 3, 2>				wz;				// Writable
			dexvec2<T, Size, 3, 3>				ww;

			dexvec3<T, Size, 0, 0, 0>			xxx;
			dexvec3<T, Size, 0, 0, 1>			xxy;
			dexvec3<T, Size, 0, 0, 2>			xxz;
			dexvec3<T, Size, 0, 0, 3>			xxw;
			dexvec3<T, Size, 0, 1, 0>			xyx;
			dexvec3<T, Size, 0, 1, 1>			xyy;
			dexvec3<T, Size, 0, 1, 2>			xyz;			// Writable
			dexvec3<T, Size, 0, 1, 3>			xyw;			// Writable
			dexvec3<T, Size, 0, 2, 0>			xzx;
			dexvec3<T, Size, 0, 2, 1>			xzy;			// Writable
			dexvec3<T, Size, 0, 2, 2>			xzz;
			dexvec3<T, Size, 0, 2, 3>			xzw;			// Writable
			dexvec3<T, Size, 0, 3, 0>			xwx;
			dexvec3<T, Size, 0, 3, 1>			xwy;			// Writable
			dexvec3<T, Size, 0, 3, 2>			xwz;			// Writable
			dexvec3<T, Size, 0, 3, 3>			xww;
			dexvec3<T, Size, 1, 0, 0>			yxx;
			dexvec3<T, Size, 1, 0, 1>			yxy;
			dexvec3<T, Size, 1, 0, 2>			yxz;			// Writable
			dexvec3<T, Size, 1, 0, 3>			yxw;			// Writable
			dexvec3<T, Size, 1, 1, 0>			yyx;
			dexvec3<T, Size, 1, 1, 1>			yyy;
			dexvec3<T, Size, 1, 1, 2>			yyz;
			dexvec3<T, Size, 1, 1, 3>			yyw;
			dexvec3<T, Size, 1, 2, 0>			yzx;			// Writable
			dexvec3<T, Size, 1, 2, 1>			yzy;
			dexvec3<T, Size, 1, 2, 2>			yzz;
			dexvec3<T, Size, 1, 2, 3>			yzw;			// Writable
			dexvec3<T, Size, 1, 3, 0>			ywx;			// Writable
			dexvec3<T, Size, 1, 3, 1>			ywy;
			dexvec3<T, Size, 1, 3, 2>			ywz;			// Writable
			dexvec3<T, Size, 1, 3, 3>			yww;
			dexvec3<T, Size, 2, 0, 0>			zxx;
			dexvec3<T, Size, 2, 0, 1>			zxy;			// Writable
			dexvec3<T, Size, 2, 0, 2>			zxz;
			dexvec3<T, Size, 2, 0, 3>			zxw;			// Writable
			dexvec3<T, Size, 2, 1, 0>			zyx;			// Writable
			dexvec3<T, Size, 2, 1, 1>			zyy;
			dexvec3<T, Size, 2, 1, 2>			zyz;
			dexvec3<T, Size, 2, 1, 3>			zyw;			// Writable
			dexvec3<T, Size, 2, 2, 0>			zzx;
			dexvec3<T, Size, 2, 2, 1>			zzy;
			dexvec3<T, Size, 2, 2, 2>			zzz;
			dexvec3<T, Size, 2, 2, 3>			zzw;
			dexvec3<T, Size, 2, 3, 0>			zwx;			// Writable
			dexvec3<T, Size, 2, 3, 1>			zwy;			// Writable
			dexvec3<T, Size, 2, 3, 2>			zwz;
			dexvec3<T, Size, 2, 3, 3>			zww;
			dexvec3<T, Size, 3, 0, 0>			wxx;
			dexvec3<T, Size, 3, 0, 1>			wxy;			// Writable
			dexvec3<T, Size, 3, 0, 2>			wxz;			// Writable
			dexvec3<T, Size, 3, 0, 3>			wxw;
			dexvec3<T, Size, 3, 1, 0>			wyx;			// Writable
			dexvec3<T, Size, 3, 1, 1>			wyy;
			dexvec3<T, Size, 3, 1, 2>			wyz;			// Writable
			dexvec3<T, Size, 3, 1, 3>			wyw;
			dexvec3<T, Size, 3, 2, 0>			wzx;			// Writable
			dexvec3<T, Size, 3, 2, 1>			wzy;
			dexvec3<T, Size, 3, 2, 2>			wzz;			// Writable
			dexvec3<T, Size, 3, 2, 3>			wzw;
			dexvec3<T, Size, 3, 3, 0>			wwx;
			dexvec3<T, Size, 3, 3, 1>			wwy;
			dexvec3<T, Size, 3, 3, 2>			wwz;
			dexvec3<T, Size, 3, 3, 3>			www;

			dexvec4<T, Size, 0, 0, 0, 0>		xxxx;
			dexvec4<T, Size, 0, 0, 0, 1>		xxxy;
			dexvec4<T, Size, 0, 0, 0, 2>		xxxz;
			dexvec4<T, Size, 0, 0, 0, 3>		xxxw;
			dexvec4<T, Size, 0, 0, 1, 0>		xxyx;
			dexvec4<T, Size, 0, 0, 1, 1>		xxyy;
			dexvec4<T, Size, 0, 0, 1, 2>		xxyz;
			dexvec4<T, Size, 0, 0, 1, 3>		xxyw;
			dexvec4<T, Size, 0, 0, 2, 0>		xxzx;
			dexvec4<T, Size, 0, 0, 2, 1>		xxzy;
			dexvec4<T, Size, 0, 0, 2, 2>		xxzz;
			dexvec4<T, Size, 0, 0, 2, 3>		xxzw;
			dexvec4<T, Size, 0, 0, 3, 0>		xxwx;
			dexvec4<T, Size, 0, 0, 3, 1>		xxwy;
			dexvec4<T, Size, 0, 0, 3, 2>		xxwz;
			dexvec4<T, Size, 0, 0, 3, 3>		xxww;
			dexvec4<T, Size, 0, 1, 0, 0>		xyxx;
			dexvec4<T, Size, 0, 1, 0, 1>		xyxy;
			dexvec4<T, Size, 0, 1, 0, 2>		xyxz;
			dexvec4<T, Size, 0, 1, 0, 3>		xyxw;
			dexvec4<T, Size, 0, 1, 1, 0>		xyyx;
			dexvec4<T, Size, 0, 1, 1, 1>		xyyy;
			dexvec4<T, Size, 0, 1, 1, 2>		xyyz;
			dexvec4<T, Size, 0, 1, 1, 3>		xyyw;
			dexvec4<T, Size, 0, 1, 2, 0>		xyzx;
			dexvec4<T, Size, 0, 1, 2, 1>		xyzy;
			dexvec4<T, Size, 0, 1, 2, 2>		xyzz;
			dexvec4<T, Size, 0, 1, 2, 3>		xyzw;			// Writable
			dexvec4<T, Size, 0, 1, 3, 0>		xywx;
			dexvec4<T, Size, 0, 1, 3, 1>		xywy;
			dexvec4<T, Size, 0, 1, 3, 2>		xywz;			// Writable
			dexvec4<T, Size, 0, 1, 3, 3>		xyww;
			dexvec4<T, Size, 0, 2, 0, 0>		xzxx;
			dexvec4<T, Size, 0, 2, 0, 1>		xzxy;
			dexvec4<T, Size, 0, 2, 0, 2>		xzxz;
			dexvec4<T, Size, 0, 2, 0, 3>		xzxw;
			dexvec4<T, Size, 0, 2, 1, 0>		xzyx;
			dexvec4<T, Size, 0, 2, 1, 1>		xzyy;
			dexvec4<T, Size, 0, 2, 1, 2>		xzyz;
			dexvec4<T, Size, 0, 2, 1, 3>		xzyw;			// Writable
			dexvec4<T, Size, 0, 2, 2, 0>		xzzx;
			dexvec4<T, Size, 0, 2, 2, 1>		xzzy;
			dexvec4<T, Size, 0, 2, 2, 2>		xzzz;
			dexvec4<T, Size, 0, 2, 2, 3>		xzzw;
			dexvec4<T, Size, 0, 2, 3, 0>		xzwx;
			dexvec4<T, Size, 0, 2, 3, 1>		xzwy;			// Writable
			dexvec4<T, Size, 0, 2, 3, 2>		xzwz;
			dexvec4<T, Size, 0, 2, 3, 3>		xzww;
			dexvec4<T, Size, 0, 3, 0, 0>		xwxx;
			dexvec4<T, Size, 0, 3, 0, 1>		xwxy;
			dexvec4<T, Size, 0, 3, 0, 2>		xwxz;
			dexvec4<T, Size, 0, 3, 0, 3>		xwxw;
			dexvec4<T, Size, 0, 3, 1, 0>		xwyx;
			dexvec4<T, Size, 0, 3, 1, 1>		xwyy;
			dexvec4<T, Size, 0, 3, 1, 2>		xwyz;			// Writable
			dexvec4<T, Size, 0, 3, 1, 3>		xwyw;
			dexvec4<T, Size, 0, 3, 2, 0>		xwzx;
			dexvec4<T, Size, 0, 3, 2, 1>		xwzy;			// Writable
			dexvec4<T, Size, 0, 3, 2, 2>		xwzz;
			dexvec4<T, Size, 0, 3, 2, 3>		xwzw;
			dexvec4<T, Size, 0, 3, 3, 0>		xwwx;
			dexvec4<T, Size, 0, 3, 3, 1>		xwwy;
			dexvec4<T, Size, 0, 3, 3, 2>		xwwz;
			dexvec4<T, Size, 0, 3, 3, 3>		xwww;
			dexvec4<T, Size, 1, 0, 0, 0>		yxxx;
			dexvec4<T, Size, 1, 0, 0, 1>		yxxy;
			dexvec4<T, Size, 1, 0, 0, 2>		yxxz;
			dexvec4<T, Size, 1, 0, 0, 3>		yxxw;
			dexvec4<T, Size, 1, 0, 1, 0>		yxyx;
			dexvec4<T, Size, 1, 0, 1, 1>		yxyy;
			dexvec4<T, Size, 1, 0, 1, 2>		yxyz;
			dexvec4<T, Size, 1, 0, 1, 3>		yxyw;
			dexvec4<T, Size, 1, 0, 2, 0>		yxzx;
			dexvec4<T, Size, 1, 0, 2, 1>		yxzy;
			dexvec4<T, Size, 1, 0, 2, 2>		yxzz;
			dexvec4<T, Size, 1, 0, 2, 3>		yxzw;			// Writable
			dexvec4<T, Size, 1, 0, 3, 0>		yxwx;
			dexvec4<T, Size, 1, 0, 3, 1>		yxwy;
			dexvec4<T, Size, 1, 0, 3, 2>		yxwz;			// Writable
			dexvec4<T, Size, 1, 0, 3, 3>		yxww;
			dexvec4<T, Size, 1, 1, 0, 0>		yyxx;
			dexvec4<T, Size, 1, 1, 0, 1>		yyxy;
			dexvec4<T, Size, 1, 1, 0, 2>		yyxz;
			dexvec4<T, Size, 1, 1, 0, 3>		yyxw;
			dexvec4<T, Size, 1, 1, 1, 0>		yyyx;
			dexvec4<T, Size, 1, 1, 1, 1>		yyyy;
			dexvec4<T, Size, 1, 1, 1, 2>		yyyz;
			dexvec4<T, Size, 1, 1, 1, 3>		yyyw;
			dexvec4<T, Size, 1, 1, 2, 0>		yyzx;
			dexvec4<T, Size, 1, 1, 2, 1>		yyzy;
			dexvec4<T, Size, 1, 1, 2, 2>		yyzz;
			dexvec4<T, Size, 1, 1, 2, 3>		yyzw;
			dexvec4<T, Size, 1, 1, 3, 0>		yywx;
			dexvec4<T, Size, 1, 1, 3, 1>		yywy;
			dexvec4<T, Size, 1, 1, 3, 2>		yywz;
			dexvec4<T, Size, 1, 1, 3, 3>		yyww;
			dexvec4<T, Size, 1, 2, 0, 0>		yzxx;
			dexvec4<T, Size, 1, 2, 0, 1>		yzxy;
			dexvec4<T, Size, 1, 2, 0, 2>		yzxz;
			dexvec4<T, Size, 1, 2, 0, 3>		yzxw;			// Writable
			dexvec4<T, Size, 1, 2, 1, 0>		yzyx;
			dexvec4<T, Size, 1, 2, 1, 1>		yzyy;
			dexvec4<T, Size, 1, 2, 1, 2>		yzyz;
			dexvec4<T, Size, 1, 2, 1, 3>		yzyw;
			dexvec4<T, Size, 1, 2, 2, 0>		yzzx;
			dexvec4<T, Size, 1, 2, 2, 1>		yzzy;
			dexvec4<T, Size, 1, 2, 2, 2>		yzzz;
			dexvec4<T, Size, 1, 2, 2, 3>		yzzw;
			dexvec4<T, Size, 1, 2, 3, 0>		yzwx;			// Writable
			dexvec4<T, Size, 1, 2, 3, 1>		yzwy;
			dexvec4<T, Size, 1, 2, 3, 2>		yzwz;
			dexvec4<T, Size, 1, 2, 3, 3>		yzww;
			dexvec4<T, Size, 1, 3, 0, 0>		ywxx;
			dexvec4<T, Size, 1, 3, 0, 1>		ywxy;
			dexvec4<T, Size, 1, 3, 0, 2>		ywxz;			// Writable
			dexvec4<T, Size, 1, 3, 0, 3>		ywxw;
			dexvec4<T, Size, 1, 3, 1, 0>		ywyx;
			dexvec4<T, Size, 1, 3, 1, 1>		ywyy;
			dexvec4<T, Size, 1, 3, 1, 2>		ywyz;
			dexvec4<T, Size, 1, 3, 1, 3>		ywyw;
			dexvec4<T, Size, 1, 3, 2, 0>		ywzx;			// Writable
			dexvec4<T, Size, 1, 3, 2, 1>		ywzy;
			dexvec4<T, Size, 1, 3, 2, 2>		ywzz;
			dexvec4<T, Size, 1, 3, 2, 3>		ywzw;
			dexvec4<T, Size, 1, 3, 3, 0>		ywwx;
			dexvec4<T, Size, 1, 3, 3, 1>		ywwy;
			dexvec4<T, Size, 1, 3, 3, 2>		ywwz;
			dexvec4<T, Size, 1, 3, 3, 3>		ywww;
			dexvec4<T, Size, 2, 0, 0, 0>		zxxx;
			dexvec4<T, Size, 2, 0, 0, 1>		zxxy;
			dexvec4<T, Size, 2, 0, 0, 2>		zxxz;
			dexvec4<T, Size, 2, 0, 0, 3>		zxxw;
			dexvec4<T, Size, 2, 0, 1, 0>		zxyx;
			dexvec4<T, Size, 2, 0, 1, 1>		zxyy;
			dexvec4<T, Size, 2, 0, 1, 2>		zxyz;
			dexvec4<T, Size, 2, 0, 1, 3>		zxyw;			// Writable
			dexvec4<T, Size, 2, 0, 2, 0>		zxzx;
			dexvec4<T, Size, 2, 0, 2, 1>		zxzy;
			dexvec4<T, Size, 2, 0, 2, 2>		zxzz;
			dexvec4<T, Size, 2, 0, 2, 3>		zxzw;
			dexvec4<T, Size, 2, 0, 3, 0>		zxwx;
			dexvec4<T, Size, 2, 0, 3, 1>		zxwy;			// Writable
			dexvec4<T, Size, 2, 0, 3, 2>		zxwz;
			dexvec4<T, Size, 2, 0, 3, 3>		zxww;
			dexvec4<T, Size, 2, 1, 0, 0>		zyxx;
			dexvec4<T, Size, 2, 1, 0, 1>		zyxy;
			dexvec4<T, Size, 2, 1, 0, 2>		zyxz;
			dexvec4<T, Size, 2, 1, 0, 3>		zyxw;			// Writable
			dexvec4<T, Size, 2, 1, 1, 0>		zyyx;
			dexvec4<T, Size, 2, 1, 1, 1>		zyyy;
			dexvec4<T, Size, 2, 1, 1, 2>		zyyz;
			dexvec4<T, Size, 2, 1, 1, 3>		zyyw;
			dexvec4<T, Size, 2, 1, 2, 0>		zyzx;
			dexvec4<T, Size, 2, 1, 2, 1>		zyzy;
			dexvec4<T, Size, 2, 1, 2, 2>		zyzz;
			dexvec4<T, Size, 2, 1, 2, 3>		zyzw;
			dexvec4<T, Size, 2, 1, 3, 0>		zywx;			// Writable
			dexvec4<T, Size, 2, 1, 3, 1>		zywy;
			dexvec4<T, Size, 2, 1, 3, 2>		zywz;
			dexvec4<T, Size, 2, 1, 3, 3>		zyww;
			dexvec4<T, Size, 2, 2, 0, 0>		zzxx;
			dexvec4<T, Size, 2, 2, 0, 1>		zzxy;
			dexvec4<T, Size, 2, 2, 0, 2>		zzxz;
			dexvec4<T, Size, 2, 2, 0, 3>		zzxw;
			dexvec4<T, Size, 2, 2, 1, 0>		zzyx;
			dexvec4<T, Size, 2, 2, 1, 1>		zzyy;
			dexvec4<T, Size, 2, 2, 1, 2>		zzyz;
			dexvec4<T, Size, 2, 2, 1, 3>		zzyw;
			dexvec4<T, Size, 2, 2, 2, 0>		zzzx;
			dexvec4<T, Size, 2, 2, 2, 1>		zzzy;
			dexvec4<T, Size, 2, 2, 2, 2>		zzzz;
			dexvec4<T, Size, 2, 2, 2, 3>		zzzw;
			dexvec4<T, Size, 2, 2, 3, 0>		zzwx;
			dexvec4<T, Size, 2, 2, 3, 1>		zzwy;
			dexvec4<T, Size, 2, 2, 3, 2>		zzwz;
			dexvec4<T, Size, 2, 2, 3, 3>		zzww;
			dexvec4<T, Size, 2, 3, 0, 0>		zwxx;
			dexvec4<T, Size, 2, 3, 0, 1>		zwxy;			// Writable
			dexvec4<T, Size, 2, 3, 0, 2>		zwxz;
			dexvec4<T, Size, 2, 3, 0, 3>		zwxw;
			dexvec4<T, Size, 2, 3, 1, 0>		zwyx;			// Writable
			dexvec4<T, Size, 2, 3, 1, 1>		zwyy;
			dexvec4<T, Size, 2, 3, 1, 2>		zwyz;
			dexvec4<T, Size, 2, 3, 1, 3>		zwyw;
			dexvec4<T, Size, 2, 3, 2, 0>		zwzx;
			dexvec4<T, Size, 2, 3, 2, 1>		zwzy;
			dexvec4<T, Size, 2, 3, 2, 2>		zwzz;
			dexvec4<T, Size, 2, 3, 2, 3>		zwzw;
			dexvec4<T, Size, 2, 3, 3, 0>		zwwx;
			dexvec4<T, Size, 2, 3, 3, 1>		zwwy;
			dexvec4<T, Size, 2, 3, 3, 2>		zwwz;
			dexvec4<T, Size, 2, 3, 3, 3>		zwww;
			dexvec4<T, Size, 3, 0, 0, 0>		wxxx;
			dexvec4<T, Size, 3, 0, 0, 1>		wxxy;
			dexvec4<T, Size, 3, 0, 0, 2>		wxxz;
			dexvec4<T, Size, 3, 0, 0, 3>		wxxw;
			dexvec4<T, Size, 3, 0, 1, 0>		wxyx;
			dexvec4<T, Size, 3, 0, 1, 1>		wxyy;
			dexvec4<T, Size, 3, 0, 1, 2>		wxyz;			// Writable
			dexvec4<T, Size, 3, 0, 1, 3>		wxyw;
			dexvec4<T, Size, 3, 0, 2, 0>		wxzx;
			dexvec4<T, Size, 3, 0, 2, 1>		wxzy;			// Writable
			dexvec4<T, Size, 3, 0, 2, 2>		wxzz;
			dexvec4<T, Size, 3, 0, 2, 3>		wxzw;
			dexvec4<T, Size, 3, 0, 3, 0>		wxwx;
			dexvec4<T, Size, 3, 0, 3, 1>		wxwy;
			dexvec4<T, Size, 3, 0, 3, 2>		wxwz;
			dexvec4<T, Size, 3, 0, 3, 3>		wxww;
			dexvec4<T, Size, 3, 1, 0, 0>		wyxx;
			dexvec4<T, Size, 3, 1, 0, 1>		wyxy;
			dexvec4<T, Size, 3, 1, 0, 2>		wyxz;			// Writable
			dexvec4<T, Size, 3, 1, 0, 3>		wyxw;
			dexvec4<T, Size, 3, 1, 1, 0>		wyyx;
			dexvec4<T, Size, 3, 1, 1, 1>		wyyy;
			dexvec4<T, Size, 3, 1, 1, 2>		wyyz;
			dexvec4<T, Size, 3, 1, 1, 3>		wyyw;
			dexvec4<T, Size, 3, 1, 2, 0>		wyzx;			// Writable
			dexvec4<T, Size, 3, 1, 2, 1>		wyzy;
			dexvec4<T, Size, 3, 1, 2, 2>		wyzz;
			dexvec4<T, Size, 3, 1, 2, 3>		wyzw;
			dexvec4<T, Size, 3, 1, 3, 0>		wywx;
			dexvec4<T, Size, 3, 1, 3, 1>		wywy;
			dexvec4<T, Size, 3, 1, 3, 2>		wywz;
			dexvec4<T, Size, 3, 1, 3, 3>		wyww;
			dexvec4<T, Size, 3, 2, 0, 0>		wzxx;
			dexvec4<T, Size, 3, 2, 0, 1>		wzxy;			// Writable
			dexvec4<T, Size, 3, 2, 0, 2>		wzxz;
			dexvec4<T, Size, 3, 2, 0, 3>		wzxw;
			dexvec4<T, Size, 3, 2, 1, 0>		wzyx;			// Writable
			dexvec4<T, Size, 3, 2, 1, 1>		wzyy;
			dexvec4<T, Size, 3, 2, 1, 2>		wzyz;
			dexvec4<T, Size, 3, 2, 1, 3>		wzyw;
			dexvec4<T, Size, 3, 2, 2, 0>		wzzx;
			dexvec4<T, Size, 3, 2, 2, 1>		wzzy;
			dexvec4<T, Size, 3, 2, 2, 2>		wzzz;
			dexvec4<T, Size, 3, 2, 2, 3>		wzzw;
			dexvec4<T, Size, 3, 2, 3, 0>		wzwx;
			dexvec4<T, Size, 3, 2, 3, 1>		wzwy;
			dexvec4<T, Size, 3, 2, 3, 2>		wzwz;
			dexvec4<T, Size, 3, 2, 3, 3>		wzww;
			dexvec4<T, Size, 3, 3, 0, 0>		wwxx;
			dexvec4<T, Size, 3, 3, 0, 1>		wwxy;
			dexvec4<T, Size, 3, 3, 0, 2>		wwxz;
			dexvec4<T, Size, 3, 3, 0, 3>		wwxw;
			dexvec4<T, Size, 3, 3, 1, 0>		wwyx;
			dexvec4<T, Size, 3, 3, 1, 1>		wwyy;
			dexvec4<T, Size, 3, 3, 1, 2>		wwyz;
			dexvec4<T, Size, 3, 3, 1, 3>		wwyw;
			dexvec4<T, Size, 3, 3, 2, 0>		wwzx;
			dexvec4<T, Size, 3, 3, 2, 1>		wwzy;
			dexvec4<T, Size, 3, 3, 2, 2>		wwzz;
			dexvec4<T, Size, 3, 3, 2, 3>		wwzw;
			dexvec4<T, Size, 3, 3, 3, 0>		wwwx;
			dexvec4<T, Size, 3, 3, 3, 1>		wwwy;
			dexvec4<T, Size, 3, 3, 3, 2>		wwwz;
			dexvec4<T, Size, 3, 3, 3, 3>		wwww;
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

		template <typename U>
		requires std::convertible_to<U, T>
		explicit constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value), static_cast<T>(value), static_cast<T>(value), static_cast<T>(value) }
		{
		}

		template <typename U1, typename U2, typename U3, typename U4>
		requires
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue,
										U4 wvalue) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue), static_cast<T>(wvalue) }
		{
		}

		template <typename U1, typename U2, typename U3,
			bool W, dimensional_scalar U4, std::size_t C, typename D>
		requires
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue,
										const vector_base<W, U4, C, D> &wvalue_source) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue), static_cast<T>(wvalue_source[0u]) }
		{
		}

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(other[3u]) }
		{
		}

		template <bool W, dimensional_scalar U, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(other[3u]) }
		{
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 3u, D> &other,
										U2 yet_another) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(yet_another) }
		{
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, dimensional_scalar U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 3u, D1> &other,
										const vector_base<W2, U2, C, D2> &wvalue_source) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(wvalue_source[0u]) }
		{
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 3)
		explicit constexpr basic_vector(U2 yet_another,
										const vector_base<W, U1, C, D> &other) noexcept
			: store{ static_cast<T>(yet_another), static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, dimensional_scalar U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 2)
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &first,
										const vector_base<W2, U2, C, D2> &second) noexcept
			: store{ static_cast<T>(first[0u]), static_cast<T>(first[1u]), static_cast<T>(second[0u]), static_cast<T>(second[1u]) }
		{
		}

		template <bool W, dimensional_scalar U1, typename D,  typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 2u, D> &other,
										U2 first,
										U3 second) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(first), static_cast<T>(second) }
		{
		}

		template <bool W1, dimensional_scalar U1, typename D1, typename U2,
			bool W2, dimensional_scalar U3, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &other,
										U2 first,
										const vector_base<W2, U3, C, D2> &wvalue_source) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(first), static_cast<T>(wvalue_source[0u]) }
		{
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U2 first,
										const vector_base<W, U1, 2u, D> &other,
										U3 second) noexcept
			: store{ static_cast<T>(first), static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(second) }
		{
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			typename U2, bool W2, dimensional_scalar U3, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U2 first,
										const vector_base<W1, U1, 2u, D1> &other,
										const vector_base<W2, U3, C, D2> &wvalue_source) noexcept
			: store{ static_cast<T>(first), static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(wvalue_source[0u]) }
		{
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T> && (C >= 2)
		explicit constexpr basic_vector(U2 first,
										U3 second,
										const vector_base<W, U1, C, D> &other) noexcept
			: store{ static_cast<T>(first), static_cast<T>(second), static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
		}

		//
		// assignment operators
		//

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);
			return *this;
		}

		//
		// data access
		//

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename ...Args>
		requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr void init(Args ...args) noexcept
		{
			[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args)
			{
				((store.value[Js] = static_cast<T>(same_args)),...);
			}(std::make_index_sequence<Count>{}, args...);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

		// physically contiguous -- used by data()
		constexpr			T *		raw_data()						noexcept	{ return store.value.data(); }
		constexpr	const	T * 	raw_data()				const	noexcept	{ return store.value.data(); }

		// get an instance of the index sequence that converts the physically contiguous to the logically contiguous
		constexpr		auto		make_sequence_pack()	const	noexcept	{ return sequence_pack{}; }

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
		// convert a parameter pack into a basic_vector

		template <typename ...Ts>
		requires dimensional_storage<std::common_type_t<Ts...>, sizeof...(Ts)>
			constexpr auto parameter_pack_to_vec(Ts ...args) noexcept
		{
			using ArgType = std::common_type_t<Ts...>;
			constexpr std::size_t Size = sizeof...(Ts);

			return basic_vector<ArgType, Size>{(static_cast<ArgType>(args))...};
		}

		// convert basic array types to a basic_vector

		template <typename T, std::size_t S, std::size_t ...Is>
		constexpr auto passthru_execute(std::index_sequence<Is...> /* dummy */,
										const std::array<T, S> &arg) noexcept
		{
			return basic_vector<T, S>(arg[Is]...);
		}

		template <typename T, std::size_t S, std::size_t ...Is>
		constexpr auto passthru_execute(std::index_sequence<Is...> /* dummy */,
										const T(&arg)[S]) noexcept
		{
			return basic_vector<T, S>(arg[Is]...);
		}

		// return types from executing lambdas on arguments of various types

		template <typename UnOp, dsga::dimensional_scalar T>
		using unary_op_return_t = decltype(UnOp()(std::declval<T>()));

		template <typename BinOp, dsga::dimensional_scalar T, dsga::dimensional_scalar U>
		using binary_op_return_t = decltype(BinOp()(std::declval<T>(), std::declval<U>()));

		template <typename TernOp, dsga::dimensional_scalar T, dsga::dimensional_scalar U, dsga::dimensional_scalar V>
		using ternary_op_return_t = decltype(TernOp()(std::declval<T>(), std::declval<U>(), std::declval<V>()));

		//
		// these rely on vector_base::operator[] to have dealt with indirection, if any, of derived vector types.
		//

		// perform the lambda action on components of vector_base arguments, returning a new basic_vector.
		// when Count == 1, treat it like a scalar value and return a scalar value.

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D, typename UnOp, std::size_t ...Is>
		constexpr auto unary_op_execute(std::index_sequence<Is...> /* dummy */,
										const vector_base<W, T, C, D> &arg,
										UnOp &lambda) noexcept
		{
			using ReturnType = unary_op_return_t<UnOp, T>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(arg[Is])...);
			else
				return static_cast<ReturnType>(lambda(arg[0u]));
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &lhs,
										 const vector_base<W2, T2, C, D2> &rhs,
										 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2>;
			using ReturnType = binary_op_return_t<BinOp, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs[Is]))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(lhs[0u]), static_cast<ArgType>(rhs[0u])));
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W, T, C, D> &lhs,
										 U rhs,
										 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U>;
			using ReturnType = binary_op_return_t<BinOp, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(lhs[0u]), static_cast<ArgType>(rhs)));
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 U lhs,
										 const vector_base<W, T, C, D> &rhs,
										 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U>;
			using ReturnType = binary_op_return_t<BinOp, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(lhs), static_cast<ArgType>(rhs[Is]))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(lhs), static_cast<ArgType>(rhs[0u])));
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													const vector_base<W1, T1, C, D1> &lhs,
													const vector_base<W2, T2, C, D2> &rhs,
													BinOp &lambda) noexcept
		{
			if constexpr (C > 1u)
				return basic_vector<binary_op_return_t<BinOp, T1, T2>, C>(lambda(lhs[Is], rhs[Is])...);
			else
				return static_cast<binary_op_return_t<BinOp, T1, T2>>(lambda(lhs[0u], rhs[0u]));
		}

		// second argument is non-const
		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													const vector_base<W1, T1, C, D1> &lhs,
													vector_base<W2, T2, C, D2> &rhs,
													BinOp &lambda) noexcept
		{
			if constexpr (C > 1u)
				return basic_vector<binary_op_return_t<BinOp, T1, T2>, C>(lambda(lhs[Is], rhs[Is])...);
			else
				return static_cast<binary_op_return_t<BinOp, T1, T2>>(lambda(lhs[0u], rhs[0u]));
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													const vector_base<W, T, C, D> &lhs,
													U rhs,
													BinOp &lambda) noexcept
		{
			if constexpr (C > 1u)
				return basic_vector<binary_op_return_t<BinOp, T, U>, C>(lambda(lhs[Is], rhs)...);
			else
				return static_cast<binary_op_return_t<BinOp, T, U>>(lambda(lhs[0u], rhs));
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													U lhs,
													const vector_base<W, T, C, D> &rhs,
													BinOp &lambda) noexcept
		{
			if constexpr (C > 1u)
				return basic_vector<binary_op_return_t<BinOp, U, T>, C>(lambda(lhs, rhs[Is])...);
			else
				return static_cast<binary_op_return_t<BinOp, U, T>>(lambda(lhs, rhs[0u]));
		}

		// perform the lambda action, setting the lhs vector_base to new values.
		// we need all the new values upfront before we set them. it is possible that
		// we are indexing into the same vector, which could give unexpected results if
		// we set values as we iterate. we need to set them all only after the new values
		// have all been gathered.

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		requires W1
		constexpr void binary_op_set(std::index_sequence<Is...> /* dummy */,
									 vector_base<W1, T1, C, D1> &lhs,
									 const vector_base<W2, T2, C, D2> &rhs,
									 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2>;
			lhs.set(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs[Is]))...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires W
		constexpr void binary_op_set(std::index_sequence<Is...> /* dummy */,
									 vector_base<W, T, C, D> &lhs,
									 U rhs,
									 BinOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U>;
			lhs.set(lambda(static_cast<ArgType>(lhs[Is]), static_cast<ArgType>(rhs))...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		requires W1
		constexpr void binary_op_set_no_convert(std::index_sequence<Is...> /* dummy */,
												vector_base<W1, T1, C, D1> &lhs,
												const vector_base<W2, T2, C, D2> &rhs,
												BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Is], rhs[Is])...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires W
		constexpr void binary_op_set_no_convert(std::index_sequence<Is...> /* dummy */,
												vector_base<W, T, C, D> &lhs,
												U rhs,
												BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Is], rhs)...);
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, bool W3, dsga::dimensional_scalar T3, typename D3,
			typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &x,
										 const vector_base<W2, T2, C, D2> &y,
										 const vector_base<W3, T3, C, D3> &z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2, T3>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x[Is]), static_cast<ArgType>(y[Is]), static_cast<ArgType>(z[Is]))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(x[0u]), static_cast<ArgType>(y[0u]), static_cast<ArgType>(z[0u])));
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, dsga::dimensional_scalar U, typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &x,
										 const vector_base<W2, T2, C, D2> &y,
										 U z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T1, T2, U>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x[Is]), static_cast<ArgType>(y[Is]), static_cast<ArgType>(z))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(x[0u]), static_cast<ArgType>(y[0u]), static_cast<ArgType>(z)));
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, dsga::dimensional_scalar V, typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W, T, C, D> &x,
										 U y,
										 V z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U, V>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x[Is]), static_cast<ArgType>(y), static_cast<ArgType>(z))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(x[0u]), static_cast<ArgType>(y), static_cast<ArgType>(z)));
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, dsga::dimensional_scalar V, typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute(std::index_sequence<Is...> /* dummy */,
										 U x,
										 V y,
										 const vector_base<W, T, C, D> &z,
										 TernOp &lambda) noexcept
		{
			using ArgType = std::common_type_t<T, U, V>;
			using ReturnType = ternary_op_return_t<TernOp, ArgType, ArgType, ArgType>;
			if constexpr (C > 1u)
				return basic_vector<ReturnType, C>(lambda(static_cast<ArgType>(x), static_cast<ArgType>(y), static_cast<ArgType>(z[Is]))...);
			else
				return static_cast<ReturnType>(lambda(static_cast<ArgType>(x), static_cast<ArgType>(y), static_cast<ArgType>(z[0u])));
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2,
			bool W3, dsga::dimensional_scalar T3, typename D3,
			typename TernOp, std::size_t ...Is>
		constexpr auto ternary_op_execute_no_convert(std::index_sequence<Is...> /* dummy */,
													 const vector_base<W1, T1, C, D1> &x,
													 const vector_base<W2, T2, C, D2> &y,
													 const vector_base<W3, T3, C, D3> &z,
													 TernOp &lambda) noexcept
		{
			if constexpr (C > 1u)
				return basic_vector<ternary_op_return_t<TernOp, T1, T2, T3>, C>(lambda(x[Is], y[Is], z[Is])...);
			else
				return static_cast<ternary_op_return_t<TernOp, T1, T2, T3>>(lambda(x[0u], y[0u], z[0u]));
		}

	}

	//
	// operators
	//

	// binary operators +=, +

	constexpr inline auto plus_op = [](auto lhs, auto rhs) { return lhs + rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator +=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator +=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], plus_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator +=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator +(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, plus_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, plus_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], plus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator +(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator +(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
	}

	// binary operators -=, -

	constexpr inline auto minus_op = [](auto lhs, auto rhs) { return lhs - rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator -=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator -=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], minus_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator -=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator -(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, minus_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, minus_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], minus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator -(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator -(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
	}

	// binary operators *=, *

	constexpr inline auto times_op = [](auto lhs, auto rhs) { return lhs * rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator *=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator *=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], times_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator *=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator *(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, times_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, times_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], times_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator *(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, times_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator *(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, times_op);
	}

	// binary operators /=, /

	constexpr inline auto div_op = [](auto lhs, auto rhs) { return lhs / rhs; };

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator /=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator /=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], div_op);
		return lhs.as_derived();
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator /=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator /(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, div_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, div_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], div_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator /(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, div_op);
	}

	template <bool W, dimensional_scalar T, std::size_t C, typename D, typename U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator /(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, div_op);
	}

	// binary operators %=, %

	constexpr inline auto modulus_op = [](auto lhs, auto rhs) { return lhs % rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], modulus_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator %=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator %(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, modulus_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, modulus_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], modulus_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator %(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator %(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, modulus_op);
	}

	// unary operator ~

	constexpr inline auto bit_not_op = [](auto arg) { return ~arg; };

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D>
	constexpr auto operator ~(const vector_base<W, T, C, D> &arg) noexcept
	{
		return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, bit_not_op);
	}

	// binary operators <<=, <<

	constexpr inline auto lshift_op = [](auto lhs, auto rhs) { return lhs << rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator <<=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator <<=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs[0u], lshift_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator <<=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator <<(const vector_base<W1, T1, C1, D1> &lhs,
							   const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs, lshift_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C2>{}, lhs[0u], rhs, lshift_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs[0u], lshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator <<(const vector_base<W, T, C, D> &lhs,
							   U rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator <<(U lhs,
							   const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	// binary operators >>=, >>

	constexpr inline auto rshift_op = [](auto lhs, auto rhs) { return lhs >> rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs[0u], rshift_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator >>=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		detail::binary_op_set_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator >>(const vector_base<W1, T1, C1, D1> &lhs,
							   const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs, rshift_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C2>{}, lhs[0u], rhs, rshift_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C1>{}, lhs, rhs[0u], rshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator >>(const vector_base<W, T, C, D> &lhs,
							   U rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator >>(U lhs,
							   const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
	}

	// binary operators &=, &

	constexpr inline auto and_op = [](auto lhs, auto rhs) { return lhs & rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator &=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator &=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], and_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator &=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator &(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, and_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, and_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], and_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator &(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, and_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator &(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, and_op);
	}

	// binary operators |=, |

	constexpr inline auto or_op = [](auto lhs, auto rhs) { return lhs | rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator |=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator |=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], or_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator |=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator |(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, or_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, or_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], or_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator |(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, or_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator |(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, or_op);
	}

	// binary operators ^=, ^

	constexpr inline auto xor_op = [](auto lhs, auto rhs) { return lhs ^ rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator ^=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
	constexpr auto &operator ^=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs[0u], xor_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator ^=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1u || C2 == 1u)
	constexpr auto operator ^(const vector_base<W1, T1, C1, D1> &lhs,
							  const vector_base<W2, T2, C2, D2> &rhs) noexcept
	{
		if constexpr (C1 == C2)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs, xor_op);
		else if constexpr (C1 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C2>{}, lhs[0u], rhs, xor_op);
		else if constexpr (C2 == 1u)
			return detail::binary_op_execute(std::make_index_sequence<C1>{}, lhs, rhs[0u], xor_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator ^(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator ^(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
	}

	// unary operator +

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires non_bool_arithmetic<T>
	constexpr auto operator +(const vector_base<W, T, C, D> &arg) noexcept
	{
		if constexpr (C > 1u)
			return basic_vector<T, C>(arg);					// no-op copy
		else
			return arg[0u];									// no-op scalar copy
	}

	// unary operator -

	constexpr inline auto neg_op = [](auto arg) { return -arg; };

	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires non_bool_arithmetic<T>
	constexpr auto operator -(const vector_base<W, T, C, D> &arg) noexcept
	{
		return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, neg_op);
	}

	// unary operators ++

	// pre-increment
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto &operator ++(vector_base<W, T, C, D> &arg) noexcept
	{
		arg += T(1);
		return arg.as_derived();
	}

	// post-increment
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto operator ++(vector_base<W, T, C, D> &arg, int) noexcept
	{
		if constexpr (C > 1u)
		{
			basic_vector<T, C> value(arg);
			arg += T(1);
			return value;
		}
		else
		{
			T value = arg[0u];
			arg += T(1);
			return value;
		}
	}

	// unary operators --

	// pre-decrement
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto &operator --(vector_base<W, T, C, D> &arg) noexcept
	{
		arg -= T(1);
		return arg.as_derived();
	}

	// post-decrement
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto operator --(vector_base<W, T, C, D> &arg, int) noexcept
	{
		if constexpr (C > 1u)
		{
			basic_vector<T, C> value(arg);
			arg -= T(1);
			return value;
		}
		else
		{
			T value = arg[0u];
			arg -= T(1);
			return value;
		}
	}

	//
	// equality comparisons - these make unit tests easier, otherwise use vector relational functions (section 8.7)
	//

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1>
	constexpr bool operator ==(const vector_base<W1, T1, C, D1> &first,
							   const vector_base<W2, T2, C, D2> &second) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			return ((first[Is] == static_cast<T1>(second[Is])) && ...);
		}(std::make_index_sequence<C>{});
	}

	template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
	constexpr bool operator ==(const vector_base<W1, T, C, D1> &first,
							   const vector_base<W2, T, C, D2> &second) noexcept
	{
		return [&]<std::size_t ...Is>(std::index_sequence<Is...>)
		{
			return ((first[Is] == second[Is]) && ...);
		}(std::make_index_sequence<C>{});
	}

	// when Count == 1, treat it like a scalar value
	template <bool W, dimensional_scalar T, typename D, dimensional_scalar U>
	requires (std::same_as<T, bool> == std::same_as<U, bool>)
	constexpr bool operator ==(const vector_base<W, T, 1u, D> &first,
							   U second) noexcept
	{
		using CommonType = std::common_type_t<T, U>;
		return static_cast<CommonType>(first[0u]) == static_cast<CommonType>(second);
	}

	//
	// get<> part of tuple interface -- needed for structured bindings
	//

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(dsga::vector_base<W, T, C, D> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(const dsga::vector_base<W, T, C, D> & arg) noexcept
	{
		return arg[N];
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(dsga::vector_base<W, T, C, D> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
	requires (N >= 0) && (N < C)
	constexpr auto && get(const dsga::vector_base<W, T, C, D> && arg) noexcept
	{
		return std::move(arg[N]);
	}

	//
	//
	// vector functions
	//
	//

	namespace functions
	{
		//
		// 8.1 - angle and trigonometry
		//

		template <std::floating_point T>
		inline constexpr T degrees_per_radian_v = std::numbers::inv_pi_v<T> * T(180);

		template <std::floating_point T>
		inline constexpr T radians_per_degree_v = std::numbers::pi_v<T> / T(180);

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto radians(const vector_base<W, T, C, D> &deg) noexcept
		{
			return deg * radians_per_degree_v<T>;
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto degrees(const vector_base<W, T, C, D> &rad) noexcept
		{
			return rad * degrees_per_radian_v<T>;
		}

		constexpr inline auto sin_op = [](floating_point_dimensional_scalar auto arg) { return std::sin(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto sin(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sin_op);
		}

		constexpr inline auto cos_op = [](floating_point_dimensional_scalar auto arg) { return std::cos(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto cos(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, cos_op);
		}

		constexpr inline auto tan_op = [](floating_point_dimensional_scalar auto arg) { return std::tan(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto tan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, tan_op);
		}

		constexpr inline auto asin_op = [](floating_point_dimensional_scalar auto arg) { return std::asin(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto asin(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, asin_op);
		}

		constexpr inline auto acos_op = [](floating_point_dimensional_scalar auto arg) { return std::acos(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto acos(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, acos_op);
		}

		constexpr inline auto atan_op = [](floating_point_dimensional_scalar auto arg) { return std::atan(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto atan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, atan_op);
		}

		constexpr inline auto atan2_op = []<floating_point_dimensional_scalar U>(U arg_y, U arg_x) { return std::atan2(arg_y, arg_x); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1,
		bool W2, typename D2>
		auto atan(const vector_base<W1, T, C, D1> &y,
				  const vector_base<W2, T, C, D2> &x) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, y, x, atan2_op);
		}

		constexpr inline auto sinh_op = [](floating_point_dimensional_scalar auto arg) { return std::sinh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto sinh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sinh_op);
		}

		constexpr inline auto cosh_op = [](floating_point_dimensional_scalar auto arg) { return std::cosh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto cosh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, cosh_op);
		}

		constexpr inline auto tanh_op = [](floating_point_dimensional_scalar auto arg) { return std::tanh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto tanh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, tanh_op);
		}

		constexpr inline auto asinh_op = [](floating_point_dimensional_scalar auto arg) { return std::asinh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto asinh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, asinh_op);
		}

		constexpr inline auto acosh_op = [](floating_point_dimensional_scalar auto arg) { return std::acosh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto acosh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, acosh_op);
		}

		constexpr inline auto atanh_op = [](floating_point_dimensional_scalar auto arg) { return std::atanh(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto atanh(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, atanh_op);
		}

		//
		// 8.2 - exponential
		//

		constexpr inline auto pow_op = []<floating_point_dimensional_scalar U>(U base, U exp) { return std::pow(base, exp); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		auto pow(const vector_base<W1, T, C, D1> &base,
				 const vector_base<W2, T, C, D2> &exp) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, base, exp, pow_op);
		}

		constexpr inline auto exp_op = [](floating_point_dimensional_scalar auto arg) { return std::exp(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto exp(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, exp_op);
		}

		constexpr inline auto log_op = [](floating_point_dimensional_scalar auto arg) { return std::log(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto log(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, log_op);
		}

		constexpr inline auto exp2_op = [](floating_point_dimensional_scalar auto arg) { return std::exp2(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto exp2(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, exp2_op);
		}

		constexpr inline auto log2_op = [](floating_point_dimensional_scalar auto arg) { return std::log2(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		auto log2(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, log2_op);
		}

		constexpr inline auto sqrt_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::sqrt(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto sqrt(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sqrt_op);
		}

		constexpr inline auto rsqrt_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::rsqrt(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto inversesqrt(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, rsqrt_op);
		}

		//
		// 8.3 - common
		//

		constexpr inline auto abs_op = [](dimensional_scalar auto arg) { return cxcm::abs(arg); };

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		constexpr auto abs(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, abs_op);
		}

		constexpr inline auto sign_op = []<dimensional_scalar T>(T arg) { return (arg > T(0)) ? T(1) : ((arg < T(0)) ? T(-1) : T(0)); };

		template <bool W, dimensional_scalar T, std::size_t C, typename D>
		constexpr auto sign(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, sign_op);
		}

		constexpr inline auto floor_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::floor(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto floor(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, floor_op);
		}

		constexpr inline auto trunc_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::trunc(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto trunc(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, trunc_op);
		}

		constexpr inline auto round_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::round(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto round(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, round_op);
		}

		constexpr inline auto round_even_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::round_even(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto roundEven(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, round_even_op);
		}

		constexpr inline auto ceil_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::ceil(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto ceil(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, ceil_op);
		}

		constexpr inline auto fract_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::fract(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto fract(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, fract_op);
		}

		constexpr inline auto mod_op = []<floating_point_dimensional_scalar T>(T x, T y) { return x - y * cxcm::floor(x / y); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto mod(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, mod_op);
		}

		constexpr inline auto fmod_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::fmod(arg); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires W2
		constexpr auto modf(const vector_base<W1, T, C, D1> &arg,
							vector_base<W2, T, C, D2> &i) noexcept
		{
			i = trunc(arg);
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, fmod_op);
		}

		constexpr inline auto min_op = []<floating_point_dimensional_scalar T>(T x, T y) { return x <= y ? x : y; };

		template <bool W1, non_bool_arithmetic T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto min(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, min_op);
		}

		constexpr inline auto max_op = []<floating_point_dimensional_scalar T>(T x, T y) { return y <= x ? x : y; };

		template <bool W1, non_bool_arithmetic T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto max(const vector_base<W1, T, C, D1> &x, const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, max_op);
		}

		constexpr inline auto clamp_op = []<dimensional_scalar T>(T x, T min_val, T max_val) { return min(max(x, min_val), max_val); };

		template <bool W1, non_bool_arithmetic T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		constexpr auto clamp(const vector_base<W1, T, C, D1> &x,
							 const vector_base<W2, T, C, D2> &min_val,
							 const vector_base<W3, T, C, D3> &max_val) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, min_val, max_val, clamp_op);
		}

		template <bool W, non_bool_arithmetic T, std::size_t C, typename D>
		constexpr auto clamp(const vector_base<W, T, C, D> &x,
							 T min_val,
							 T max_val) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, min_val, max_val, clamp_op);
		}

		constexpr inline auto mix1_op = []<floating_point_dimensional_scalar T>(T x, T y, T a) { return (x * (T(1.) - a)) + (y * a); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		constexpr auto mix(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y,
						   const vector_base<W3, T, C, D3> &a) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, y, a, mix1_op);
		}

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto mix(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y,
						   T a) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, x, y, a, mix1_op);
		}

		constexpr inline auto mix2_op = []<dimensional_scalar T, boolean_dimensional_scalar B>(T x, T y, B a) -> T { return a ? y : x; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1,
			bool W2, typename D2, bool W3, boolean_dimensional_scalar B, typename D3>
		constexpr auto mix(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y,
						   const vector_base<W3, B, C, D3> &a) noexcept
		{
			return detail::ternary_op_execute_no_convert(std::make_index_sequence<C>{}, x, y, a, mix2_op);
		}

		constexpr inline auto step_op = []<floating_point_dimensional_scalar T>(T edge, T x) { return ((x < edge) ? T(0) : T(1)); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto step(const vector_base<W1, T, C, D1> &edge,
							const vector_base<W2, T, C, D2> &x) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, edge, x, step_op);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto step(T edge,
							const vector_base<W, T, C, D> &x) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, edge, x, step_op);
		}

		constexpr inline auto smoothstep_op = []<floating_point_dimensional_scalar T>(T edge0, T edge1, T x)
		{
			T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
			return t * t * (T(3) - T(2) * t);
		};

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		constexpr auto smoothstep(const vector_base<W1, T, C, D1> &edge0,
								  const vector_base<W2, T, C, D2> &edge1,
								  const vector_base<W3, T, C, D3> &x) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, edge0, edge1, x, smoothstep_op);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto smoothstep(T edge0,
								  T edge1,
								  const vector_base<W, T, C, D> &x) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, edge0, edge1, x, smoothstep_op);
		}

		constexpr inline auto isnan_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::isnan(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto isnan(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, isnan_op);
		}

		constexpr inline auto isinf_op = [](floating_point_dimensional_scalar auto arg) { return cxcm::isinf(arg); };

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto isinf(const vector_base<W, T, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, isinf_op);
		}

		constexpr inline auto float_bits_to_int_op = [](float arg) { return std::bit_cast<int>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto floatBitsToInt(const vector_base<W, float, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, float_bits_to_int_op);
		}

		constexpr inline auto float_bits_to_uint_op = [](float arg) { return std::bit_cast<unsigned int>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto floatBitsToUint(const vector_base<W, float, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, float_bits_to_uint_op);
		}

		constexpr inline auto double_bits_to_long_long_op = [](double arg) { return std::bit_cast<long long>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto doubleBitsToLongLong(const vector_base<W, double, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, double_bits_to_long_long_op);
		}

		constexpr inline auto double_bits_to_ulong_long_op = [](double arg) { return std::bit_cast<unsigned long long>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto doubleBitsToUlongLong(const vector_base<W, double, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, double_bits_to_ulong_long_op);
		}

		constexpr inline auto int_bits_to_float_op = [](int arg) { return std::bit_cast<float>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto intBitsToFloat(const vector_base<W, int, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, int_bits_to_float_op);
		}

		constexpr inline auto uint_bits_to_float_op = [](unsigned int arg) { return std::bit_cast<float>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto uintBitsToFloat(const vector_base<W, unsigned int, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, uint_bits_to_float_op);
		}

		constexpr inline auto long_long_bits_to_double_op = [](long long arg) { return std::bit_cast<double>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto longLongBitsToDouble(const vector_base<W, long long, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, long_long_bits_to_double_op);
		}

		constexpr inline auto ulong_long_bits_to_double_op = [](unsigned long long arg) { return std::bit_cast<double>(arg); };

		template <bool W, std::size_t C, typename D>
		constexpr auto ulongLongBitsToDouble(const vector_base<W, unsigned long long, C, D> &arg) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, arg, ulong_long_bits_to_double_op);
		}

		constexpr inline auto fma_op = []<floating_point_dimensional_scalar T>(T a, T b, T c) { return std::fma(a, b, c); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
		auto fma(const vector_base<W1, T, C, D1> &a,
				 const vector_base<W2, T, C, D2> &b,
				 const vector_base<W3, T, C, D3> &c) noexcept
		{
			return detail::ternary_op_execute(std::make_index_sequence<C>{}, a, b, c, fma_op);
		}

		constexpr inline auto frexp_op = []<floating_point_dimensional_scalar T>(T x, int &exp) { return std::frexp(x, &exp); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires W2
		auto frexp(const vector_base<W1, T, C, D1> &x,
				   vector_base<W2, int, C, D2> &exp) noexcept
		{
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, x, exp, frexp_op);
		}

		constexpr inline auto ldexp_op = []<floating_point_dimensional_scalar T>(T x, int exp) { return std::ldexp(x, exp); };

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		auto ldexp(const vector_base<W1, T, C, D1> &x,
				   const vector_base<W2, int, C, D2> &exp) noexcept
		{
			return detail::binary_op_execute_no_convert(std::make_index_sequence<C>{}, x, exp, ldexp_op);
		}

		//
		// 8.4 is omitted
		//

		//
		// 8.5 - geometric
		//

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto dot(const vector_base<W1, T, C, D1> &x,
						   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return[&]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				return ((x[Is] * y[Is]) + ...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W1, floating_point_dimensional_scalar T, typename D1, bool W2, typename D2>
		constexpr auto cross(const vector_base<W1, T, 3u, D1> &x,
							 const vector_base<W2, T, 3u, D2> &y) noexcept
		{
			return basic_vector<T, 3u>((x[1] * y[2]) - (y[1] * x[2]),
									   (x[2] * y[0]) - (y[2] * x[0]),
									   (x[0] * y[1]) - (y[0] * x[1]));
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto length(const vector_base<W, T, C, D> &x) noexcept
		{
			return cxcm::sqrt(dot(x, x));
		}

		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto distance(const vector_base<W1, T, C, D1> &p0,
								const vector_base<W2, T, C, D2> &p1) noexcept
		{
			return length(p1 - p0);
		}

		template <bool W, floating_point_dimensional_scalar T, std::size_t C, typename D>
		constexpr auto normalize(const vector_base<W, T, C, D> &x) noexcept
		{
			return x / length(x);
		}

		//
		// vec4 ftransform() omitted
		//
		
		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1,
			bool W2, typename D2, bool W3, typename D3>
		constexpr auto faceforward(const vector_base<W1, T, C, D1> &n,
								   const vector_base<W2, T, C, D2> &i,
								   const vector_base<W3, T, C, D3> &nref) noexcept
		{
			return (dot(nref, i) < T(0)) ? +n : -n;
		}

		// n must be normalized in order to achieve desired results
		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto reflect(const vector_base<W1, T, C, D1> &i,
							   const vector_base<W2, T, C, D2> &n) noexcept
		{
			return i - T(2) * dot(n, i) * n;
		}

		// i and n must be normalized in order to achieve desired results
		template <bool W1, floating_point_dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto reflect(const vector_base<W1, T, C, D1> &i,
							   const vector_base<W2, T, C, D2> &n,
							   T eta) noexcept
		{
			T k = T(1) - eta * eta * (T(1) - dot(n, i) * dot(n, i));

			if (k < T(0))
				return basic_vector<T, C>(T(0));

			return eta * i - (eta * dot(n, i) + sqrt(k)) * n;
		}

		//
		// 8.6 is matrix functions which will happen when we have matrices
		//

		//
		// 8.7 - vector relational
		//

		constexpr inline auto less_op = []<dimensional_scalar T>(T x, T y) -> bool { return x < y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto lessThan(const vector_base<W1, T, C, D1> &x,
								const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, less_op);
		}

		constexpr inline auto less_equal_op = []<dimensional_scalar T>(T x, T y) -> bool { return x <= y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto lessThanEqual(const vector_base<W1, T, C, D1> &x,
									 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, less_equal_op);
		}

		constexpr inline auto greater_op = []<dimensional_scalar T>(T x, T y) -> bool { return x > y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto greaterThan(const vector_base<W1, T, C, D1> &x,
								   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, greater_op);
		}

		constexpr inline auto greater_equal_op = []<dimensional_scalar T>(T x, T y) -> bool { return x >= y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		requires non_bool_arithmetic<T>
		constexpr auto greaterThanEqual(const vector_base<W1, T, C, D1> &x,
								   const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, greater_equal_op);
		}

		constexpr inline auto equal_op = []<non_bool_arithmetic T>(T x, T y) -> bool { return x == y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto equal(const vector_base<W1, T, C, D1> &x,
							 const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, equal_op);
		}

		constexpr inline auto not_equal_op = []<non_bool_arithmetic T>(T x, T y) -> bool { return x != y; };

		template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
		constexpr auto notEqual(const vector_base<W1, T, C, D1> &x,
								const vector_base<W2, T, C, D2> &y) noexcept
		{
			return detail::binary_op_execute(std::make_index_sequence<C>{}, x, y, not_equal_op);
		}

		template <bool W, std::size_t C, typename D>
		constexpr bool any(const vector_base<W, bool, C, D> &x) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				return (x[Is] || ...);
			}(std::make_index_sequence<C>{});
		}

		template <bool W, std::size_t C, typename D>
		constexpr bool all(const vector_base<W, bool, C, D> &x) noexcept
		{
			return [&]<std::size_t ...Is>(std::index_sequence<Is...>)
			{
				return (x[Is] && ...);
			}(std::make_index_sequence<C>{});
		}

		constexpr inline auto not_op = [](bool x) -> bool { return !x; };

		// c++ is not allowing a function named not()
		template <bool W, std::size_t C, typename D>
		constexpr auto Not(const vector_base<W, bool, C, D> &x) noexcept
		{
			return detail::unary_op_execute(std::make_index_sequence<C>{}, x, not_op);
		}

		//
		// 8.8 - 8.19 are omitted
		//

	}

}	// namespace dsga

//
// tuple inteface for basic_vector and indexed_vector and vec_base -- supports structured bindings
//

template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::basic_vector<T, S>> : std::integral_constant<std::size_t, S>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::basic_vector<T, S>>
{
	using type = T;
};

template <dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_size<dsga::indexed_vector<T, S, C, Is...>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_element<I, dsga::indexed_vector<T, S, C, Is...>>
{
	using type = T;
};

template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_size<dsga::vector_base<W, T, C, D>> : std::integral_constant<std::size_t, C>
{
};

template <std::size_t I, bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_element<I, dsga::vector_base<W, T, C, D>>
{
	using type = T;
};

// converting from external vector type or data to internal vector type

template <dsga::dimensional_scalar T, std::size_t S>
requires dsga::dimensional_storage<T, S>
constexpr auto to_vec(const std::array<T, S> &arg) noexcept
{
	return dsga::detail::passthru_execute(std::make_index_sequence<S>{}, arg);
}

template <dsga::dimensional_scalar T, std::size_t S>
requires dsga::dimensional_storage<T, S>
constexpr auto to_vec(const T (&arg)[S]) noexcept
{
	return dsga::detail::passthru_execute(std::make_index_sequence<S>{}, arg);
}

// converting from internal vector type to std::array

template <dsga::dimensional_scalar T, std::size_t S>
constexpr std::array<T, S> from_vec(const dsga::basic_vector<T, S> &arg)
{
	return arg.store.value;
}

// not constexpr --  we can't use indexed_vector in constexpr expressions because it is from a swizzle which isn't the active union member at compile time
template <dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
std::array<T, C> from_vec(const dsga::indexed_vector<T, S, C, Is...> &arg)
{
	return [&] <std::size_t ...Js>(std::index_sequence<Js ...> /* dummy */) -> std::array<T, C>
	{
		return { arg[Js]... };
	}(std::make_index_sequence<C>{});
}

// fill vectors from spans

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<U, T>
constexpr void copy_to_vec(dsga::basic_vector<T, S> &lhs, std::span<U, E> rhs)
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<U, T>
constexpr void copy_to_vec(dsga::basic_vector<T, S> &lhs, std::span<U, E> rhs)
{
	std::size_t count = std::min(S, rhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<T>(rhs[i]);
}

// fill spans from vectors

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E != 0) && (E != std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<T, U>
constexpr void copy_from_vec(std::span<U, E> lhs, const dsga::basic_vector<T, S> &rhs)
{
	constexpr std::size_t count = std::min(S, E);
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

template <dsga::dimensional_scalar T, std::size_t S, typename U, std::size_t E>
requires ((E == 0) || (E == std::dynamic_extent)) && dsga::non_bool_arithmetic<U> && std::convertible_to<T, U>
constexpr void copy_from_vec(std::span<U, E> lhs, const dsga::basic_vector<T, S> &rhs)
{
	std::size_t count = std::min(S, lhs.size());
	for (std::size_t i = 0; i < count; ++i)
		lhs[i] = static_cast<U>(rhs[i]);
}

//
// specialized using types
//

// this 1D vector is a swizzlable scalar -- rough glsl analog to this with primitive types
template <dsga::dimensional_scalar T>
using regvec1 = dsga::basic_vector<T, 1u>;

// 2D vector
template <dsga::dimensional_scalar T>
using regvec2 = dsga::basic_vector<T, 2u>;

// 3D vector
template <dsga::dimensional_scalar T>
using regvec3 = dsga::basic_vector<T, 3u>;

// 4D vector
template <dsga::dimensional_scalar T>
using regvec4 = dsga::basic_vector<T, 4u>;

// boolean vectors
using bscal = regvec1<bool>;
using bvec2 = regvec2<bool>;
using bvec3 = regvec3<bool>;
using bvec4 = regvec4<bool>;

// int vectors
using iscal = regvec1<int>;
using ivec2 = regvec2<int>;
using ivec3 = regvec3<int>;
using ivec4 = regvec4<int>;

// unsigned int vectors
using uscal = regvec1<unsigned>;
using uvec2 = regvec2<unsigned>;
using uvec3 = regvec3<unsigned>;
using uvec4 = regvec4<unsigned>;

// long long vectors (not in glsl)
using llscal = regvec1<long long>;
using llvec2 = regvec2<long long>;
using llvec3 = regvec3<long long>;
using llvec4 = regvec4<long long>;

// unsigned long long vectors (not in glsl)
using ullscal = regvec1<unsigned long long>;
using ullvec2 = regvec2<unsigned long long>;
using ullvec3 = regvec3<unsigned long long>;
using ullvec4 = regvec4<unsigned long long>;

// float vectors with out an 'f' prefix -- this is from glsl
using scal = regvec1<float>;
using vec2 = regvec2<float>;
using vec3 = regvec3<float>;
using vec4 = regvec4<float>;

// also float vectors, but using the same naming convention as the other vectors do (not in glsl)
using fscal = regvec1<float>;
using fvec2 = regvec2<float>;
using fvec3 = regvec3<float>;
using fvec4 = regvec4<float>;

// double vectors
using dscal = regvec1<double>;
using dvec2 = regvec2<double>;
using dvec3 = regvec3<double>;
using dvec4 = regvec4<double>;

//
// bring the vector functions into the global namespace
//
using namespace dsga::functions;

// closing include guard
#endif
