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
	//		data() - relies on raw_data() in Derived - access in physical order
	// 		size() - relies on Count template parameter
	// 		sequence() - relies on make_sequence_pack() in Derived - shows physical to logical conversion info
	//
	// we also need to rely on the fact that Derived::sequence_pack is a valid typename.
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

#if 1
		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename ... Args>
		requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
		constexpr void init(Args ...args) noexcept
		{
			((value[Is] = static_cast<T>(args)),...);
		}
#else
		template <typename ... Args>
		requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
		constexpr void init(Args ...args) noexcept
		{
			[&] (T *ptr)
			{
				((ptr[Is] = static_cast<T>(args)),...);
			}(raw_data());
		}
#endif

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

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<T, Count>() const noexcept
		{
			return basic_vector<T, Count>(value[Is]...);
		}

		template <dimensional_scalar U>
		requires (!std::same_as<T, U> && std::convertible_to<T, U>)
		explicit constexpr operator basic_vector<U, Count>() const noexcept
		{
			return basic_vector<U, Count>(static_cast<U>(value[Is])...);
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

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<T, Count>() const noexcept
		{
			return basic_vector<T, Count>(value[I]);
		}

		template <dimensional_scalar U>
		requires (!std::same_as<T, U> && std::convertible_to<T, U>)
		explicit constexpr operator basic_vector<U, Count>() const noexcept
		{
			return basic_vector<U, Count>(static_cast<U>(value[I]));
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
//			init(other[0u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]) }
		{
//			init(other[0u]);
		}

		template <typename U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value) }
		{
//			init(value);
		}

		template <typename U>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(U value) noexcept
			: store{ static_cast<T>(value) }
		{
//			init(value);
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

		// logically and physically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		//template <typename ...Args>
		//requires (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		//constexpr void init(Args ...args) noexcept
		//{
		//	[&] <std::size_t ...Js, typename ...As>(std::index_sequence<Js ...> /* dummy */, As ...same_args)
		//	{
		//		((store.value[Js] = static_cast<T>(same_args)),...);
		//	}(make_sequence_pack(), args...);
		//}

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
//			init(value, value);
		}

		template <typename U1, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to <U2, T>
		explicit constexpr basic_vector(U1 xvalue, U2 yvalue) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue) }
		{
//			init(xvalue, yvalue);
		}

		template <typename U1, bool W, dimensional_scalar U2, std::size_t C, typename D>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(U1 xvalue, const vector_base<W, U2, C, D> &yvalue_source) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue_source[0u]) }
		{
//			init(xvalue, yvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
//			init(other[0u], other[1u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T> && (C >= Count))
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
//			init(other[0u], other[1u]);
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
//			init(value, value, value);
		}

		template <typename U1, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue) }
		{
//			init(xvalue, yvalue, zvalue);
		}

		template <typename U1, typename U2, bool W, dimensional_scalar U3, std::size_t C, typename D>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										const vector_base<W, U3, C, D> &zvalue_source) noexcept
			: store{ static_cast<T>(xvalue), static_cast<T>(yvalue), static_cast<T>(zvalue_source[0u]) }
		{
//			init(xvalue, yvalue, zvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
//			init(other[0u], other[1u], other[2u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T> && (C >= Count))
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
//			init(other[0u], other[1u], other[2u]);
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 2u, D> &other,
										U2 yet_another) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(yet_another) }
		{
//			init(other[0u], other[1u], yet_another);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, typename U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &other,
										const vector_base<W2, U2, C, D2> &yet_another) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(yet_another[0u]) }
		{
//			init(other[0u], other[1u], yet_another[0u]);
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 2)
		explicit constexpr basic_vector(U2 yet_another,
										const vector_base<W, U1, C, D> &other) noexcept
			: store{ static_cast<T>(yet_another), static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
//			init(yet_another, other[0u], other[1u]);
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
//			init(value, value, value, value);
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
//			init(xvalue, yvalue, zvalue, wvalue);
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
//			init(xvalue, yvalue, zvalue, wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(other[3u]) }
		{
//			init(other[0u], other[1u], other[2u], other[3u]);
		}

		template <bool W, dimensional_scalar U, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(other[3u]) }
		{
//			init(other[0u], other[1u], other[2u], other[3u]);
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 3u, D> &other,
										U2 yet_another) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(yet_another) }
		{
//			init(other[0u], other[1u], other[2u], yet_another);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, dimensional_scalar U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 3u, D1> &other,
										const vector_base<W2, U2, C, D2> &wvalue_source) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]), static_cast<T>(wvalue_source[0u]) }
		{
//			init(other[0u], other[1u], other[2u], wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 3)
		explicit constexpr basic_vector(U2 yet_another,
										const vector_base<W, U1, C, D> &other) noexcept
			: store{ static_cast<T>(yet_another), static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(other[2u]) }
		{
//			init(yet_another, other[0u], other[1u], other[2u]);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, dimensional_scalar U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 2)
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &first,
										const vector_base<W2, U2, C, D2> &second) noexcept
			: store{ static_cast<T>(first[0u]), static_cast<T>(first[1u]), static_cast<T>(second[0u]), static_cast<T>(second[1u]) }
		{
//			init(first[0u], first[1u], second[0u], second[1u]);
		}

		template <bool W, dimensional_scalar U1, typename D,  typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 2u, D> &other,
										U2 first,
										U3 second) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(first), static_cast<T>(second) }
		{
//			init(other[0u], other[1u], first, second);
		}

		template <bool W1, dimensional_scalar U1, typename D1, typename U2,
			bool W2, dimensional_scalar U3, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &other,
										U2 first,
										const vector_base<W2, U3, C, D2> &wvalue_source) noexcept
			: store{ static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(first), static_cast<T>(wvalue_source[0u]) }
		{
//			init(other[0u], other[1u], first, wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U2 first,
										const vector_base<W, U1, 2u, D> &other,
										U3 second) noexcept
			: store{ static_cast<T>(first), static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(second) }
		{
//			init(first, other[0u], other[1u], second);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			typename U2, bool W2, dimensional_scalar U3, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U2 first,
										const vector_base<W1, U1, 2u, D1> &other,
										const vector_base<W2, U3, C, D2> &wvalue_source) noexcept
			: store{ static_cast<T>(first), static_cast<T>(other[0u]), static_cast<T>(other[1u]), static_cast<T>(wvalue_source[0u]) }
		{
//			init(first, other[0u], other[1u], wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T> && (C >= 2)
		explicit constexpr basic_vector(U2 first,
										U3 second,
										const vector_base<W, U1, C, D> &other) noexcept
			: store{ static_cast<T>(first), static_cast<T>(second), static_cast<T>(other[0u]), static_cast<T>(other[1u]) }
		{
//			init(first, second, other[0u], other[1u]);
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

		//
		// these rely on vector_base::operator[] to have dealt with indirection, if any, of derived vector types.
		//

		// perform the lambda action on components of vector_base arguments, returning a new basic_vector

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D, typename UnOp, std::size_t ...Is>
		constexpr auto unary_op_execute(std::index_sequence<Is...> /* dummy */,
										const vector_base<W, T, C, D> &arg,
										UnOp &lambda) noexcept
		{
			return basic_vector<unary_op_return_t<UnOp, T>, C>(lambda(arg[Is])...);
		}

		// when Count == 1, treat it like a scalar value
		template <bool W, dsga::dimensional_scalar T, typename D, typename UnOp, std::size_t ...Is>
		constexpr auto unary_op_execute(std::index_sequence<Is...> /* dummy */,
										const vector_base<W, T, 1u, D> &arg,
										UnOp &lambda) noexcept
		{
			return static_cast<unary_op_return_t<UnOp, T>>(lambda(arg[0u]));
		}

		template <bool W1, dsga::dimensional_scalar T1, std::size_t C, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, C, D1> &lhs,
										 const vector_base<W2, T2, C, D2> &rhs,
										 BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, T1, T2>, C>(lambda(lhs[Is], rhs[Is])...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W, T, C, D> &lhs,
										 U rhs,
										 BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, T, U>, C>(lambda(lhs[Is], rhs)...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 U lhs,
										 const vector_base<W, T, C, D> &rhs,
										 BinOp &lambda) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, U, T>, C>(lambda(lhs, rhs[Is])...);
		}

		// when Count == 1, treat it like a scalar value
		template <bool W1, dsga::dimensional_scalar T1, typename D1,
			bool W2, dsga::dimensional_scalar T2, typename D2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W1, T1, 1u, D1> &lhs,
										 const vector_base<W2, T2, 1u, D2> &rhs,
										 BinOp &lambda) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, T1, T2>>(lambda(lhs[0u], rhs[0u]));
		}

		// when Count == 1, treat it like a scalar value
		template <bool W, dsga::dimensional_scalar T, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 const vector_base<W, T, 1u, D> &lhs,
										 U rhs,
										 BinOp &lambda) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, T, U>>(lambda(lhs[0u], rhs));
		}

		// when Count == 1, treat it like a scalar value
		template <bool W, dsga::dimensional_scalar T1, typename D,
			dsga::dimensional_scalar T2, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(std::index_sequence<Is...> /* dummy */,
										 T2 lhs,
										 const vector_base<W, T1, 1u, D> &rhs,
										 BinOp &lambda) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, T2, T1>>(lambda(lhs, rhs[0u]));
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
			lhs.set(lambda(lhs[Is], rhs[Is])...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires W
		constexpr void binary_op_set(std::index_sequence<Is...> /* dummy */,
									 vector_base<W, T, C, D> &lhs,
									 U rhs,
									 BinOp &lambda) noexcept
		{
			lhs.set(lambda(lhs[Is], rhs)...);
		}

		//
		// these rely on vector_base::data() and vector_base::sequence() to deal directly with indexing of
		// derived vector types at a low level, where indirection is implicitly accounted for.
		//

		// perform the lambda action on components of vector_base arguments, returning a new basic_vector

		// when Count == 1, treat it like a scalar value
		template <dsga::dimensional_scalar T, typename UnOp, std::size_t ...Is>
		requires (sizeof...(Is) == 1u)
		constexpr auto unary_op_execute(UnOp &lambda,
										const T *arg,
										std::index_sequence<Is...> /* dummy */) noexcept
		{
			return static_cast<unary_op_return_t<UnOp, T>>(lambda(arg[Is]...));
		}

		template <dsga::dimensional_scalar T, typename UnOp, std::size_t ...Is>
		constexpr auto unary_op_execute(UnOp &lambda,
										const T *arg,
										std::index_sequence<Is...> /* dummy */) noexcept
		{
			return basic_vector<unary_op_return_t<UnOp, T>, sizeof ...(Is)>(lambda(arg[Is])...);
		}

		template <dsga::dimensional_scalar T1, dsga::dimensional_scalar T2, typename BinOp, std::size_t ...Is, std::size_t ...Js>
		requires (sizeof...(Is) == sizeof ...(Js))
		constexpr auto binary_op_execute(BinOp &lambda,
										 const T1 *lhs,
										 std::index_sequence<Is...> /* dummy */,
										 const T2 *rhs,
										 std::index_sequence<Js...> /* dummy */) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, T1, T2>, sizeof...(Is)>(lambda(lhs[Is], rhs[Js])...);
		}

		template <dsga::dimensional_scalar T, dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(BinOp &lambda,
										 const T *lhs,
										 std::index_sequence<Is...> /* dummy */,
										 U rhs) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, T, U>, sizeof ...(Is)>(lambda(lhs[Is], rhs)...);
		}

		template <dsga::dimensional_scalar T, dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		constexpr auto binary_op_execute(BinOp &lambda,
										 U lhs,
										 const T *rhs,
										 std::index_sequence<Is...> /* dummy */) noexcept
		{
			return basic_vector<binary_op_return_t<BinOp, U, T>, sizeof ...(Is)>(lambda(lhs, rhs[Is])...);
		}

		// when Count == 1, treat it like a scalar value
		template <dsga::dimensional_scalar T1, dsga::dimensional_scalar T2, typename BinOp, std::size_t ...Is, std::size_t ...Js>
		requires (sizeof ...(Is) == 1u) && (sizeof ...(Js) == 1u)
		constexpr auto binary_op_execute(BinOp &lambda,
										 const T1 *lhs,
										 std::index_sequence<Is...> /* dummy */,
										 const T2 *rhs,
										 std::index_sequence<Js...> /* dummy */) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, T1, T2>>(lambda(lhs[Is]..., rhs[Js]...));
		}

		// when Count == 1, treat it like a scalar value
		template <dsga::dimensional_scalar T, dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires (sizeof ...(Is) == 1u)
		constexpr auto binary_op_execute(BinOp &lambda,
										 const T *lhs,
										 std::index_sequence<Is...> /* dummy */,
										 U rhs) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, T, U>>(lambda(lhs[Is]..., rhs));
		}

		// when Count == 1, treat it like a scalar value
		template <dsga::dimensional_scalar T, dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires (sizeof ...(Is) == 1u)
		constexpr auto binary_op_execute(BinOp &lambda,
										 U lhs,
										 const T *rhs,
										 std::index_sequence<Is...> /* dummy */) noexcept
		{
			return static_cast<binary_op_return_t<BinOp, U, T>>(lambda(lhs, rhs[Is]...));
		}

		// perform the lambda action, setting new values on the vector_base struct that
		// corresponds to the lhs pointer.
		// we need all the new values upfront before we set them. it is possible that
		// we are indexing into the same vector, which could give unexpected results if
		// we set values as we iterate. we need to set them all only after the new values
		// have all been gathered.

		template <bool W, dsga::dimensional_scalar T1, std::size_t C, typename D,
			dsga::dimensional_scalar T2, typename BinOp, std::size_t ...Is, std::size_t ...Js>
		requires W && (sizeof ...(Is) == sizeof ...(Js))
		constexpr void binary_op_set(BinOp &lambda,
									 vector_base<W, T1, C, D> &val,
									 T1 *lhs,
									 std::index_sequence<Is...> /* dummy */,
									 const T2 *rhs,
									 std::index_sequence<Js...> /* dummy */) noexcept
		{
			val.set(lambda(lhs[Is], rhs[Js])...);
		}

		template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D,
			dsga::dimensional_scalar U, typename BinOp, std::size_t ...Is>
		requires W
		constexpr void binary_op_set(BinOp &lambda,
									 vector_base<W, T, C, D> &val,
									 T *lhs,
									 std::index_sequence<Is...> /* dummy */,
									 U rhs) noexcept
		{
			val.set(lambda(lhs[Is], rhs)...);
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

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator +=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
		return lhs.as_derived();
	}

#if 1

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator +(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, plus_op);
	}

#else

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator +(const dsga::vector_base<W1, T1, 1u, D1> &lhs,
							  const dsga::vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return plus_op(lhs[0u], rhs[0u]);
	}

#if 1

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator +(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return
		[&] <std::size_t ...Is, std::size_t ...Js, typename BinOp>(std::index_sequence<Is ...> /* dummy */, const T1 *lhs_ptr,
																   std::index_sequence<Js ...> /* dummy */, const T2 *rhs_ptr, BinOp lambda)
		{
			return basic_vector<detail::binary_op_return_t<BinOp, T1, T2>, C>(lambda(lhs_ptr[Is], rhs_ptr[Js])...);
		}(lhs.sequence(), lhs.data(), rhs.sequence(), rhs.data(), plus_op);
	}

#else

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator +(const dsga::vector_base<W1, T1, 1u, D1> &lhs,
							  const dsga::vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return plus_op(lhs[0u], rhs[0u]);
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator +(const dsga::vector_base<W1, T1, C, D1> &lhs,
							  const dsga::vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return
		[&]<typename BinOp>(BinOp lambda)
		{
			basic_vector<detail::binary_op_return_t<BinOp, T1, T2>, C> v(0);

			for (int i = 0; i < C; ++i)
				v[i] = lambda(lhs[i], rhs[i]);

			return v;
		}(plus_op);
}
#endif

#endif

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator +(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, plus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator +(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], plus_op);
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

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator -=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator -(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, minus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator -(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, minus_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator -(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], minus_op);
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

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator *=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, times_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator *(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, times_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator *(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, times_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator *(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], times_op);
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

	template <bool W, dimensional_scalar T, std::size_t C, typename D, dimensional_scalar U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator /=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, div_op);
		return lhs.as_derived();
	}

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator /(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, div_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, typename D1,
		bool W2, dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator /(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, div_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator /(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], div_op);
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

	constexpr inline auto mod_op = [](auto lhs, auto rhs) { return lhs % rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator %=(vector_base<W1, T1, C, D1> &lhs,
								const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, mod_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator %=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, mod_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator %(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, mod_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator %(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, mod_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator %(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], mod_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator %(const vector_base<W, T, C, D> &lhs,
							  U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, mod_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator %(U lhs,
							  const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, mod_op);
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
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator <<=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator <<(const vector_base<W1, T1, C, D1> &lhs,
							   const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator <<(const vector_base<W1, T1, 1u, D1> &lhs,
							   const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, lshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator <<(const vector_base<W1, T1, C, D1> &lhs,
							   const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], lshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator <<(const vector_base<W, T, C, D> &lhs,
							   U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator <<(U lhs,
							   const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, lshift_op);
	}

	// binary operators >>=, >>

	constexpr inline auto rshift_op = [](auto lhs, auto rhs) { return lhs >> rhs; };

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires W1 && implicitly_convertible_to<T2, T1>
	constexpr auto &operator >>=(vector_base<W1, T1, C, D1> &lhs,
								 const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator >>=(vector_base<W, T, C, D> &lhs,
								 U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator >>(const vector_base<W1, T1, C, D1> &lhs,
							   const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator >>(const vector_base<W1, T1, 1u, D1> &lhs,
							   const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, rshift_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator >>(const vector_base<W1, T1, C, D1> &lhs,
							   const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], rshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator >>(const vector_base<W, T, C, D> &lhs,
							   U rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
	}

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
	constexpr auto operator >>(U lhs,
							   const vector_base<W, T, C, D> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, rshift_op);
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

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator &=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, and_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator &(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, and_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator &(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, and_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator &(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], and_op);
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

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator |=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, or_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator |(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, or_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator |(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, or_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator |(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], or_op);
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

	template <bool W, integral_dimensional_scalar T, std::size_t C, typename D, std::integral U>
	requires W && implicitly_convertible_to<U, T>
	constexpr auto &operator ^=(vector_base<W, T, C, D> &lhs,
								U rhs) noexcept
	{
		detail::binary_op_set(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
		return lhs.as_derived();
	}

	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>
	constexpr auto operator ^(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs, xor_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, typename D1,
		bool W2, integral_dimensional_scalar T2, std::size_t C, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator ^(const vector_base<W1, T1, 1u, D1> &lhs,
							  const vector_base<W2, T2, C, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs[0u], rhs, xor_op);
	}

	// when Count == 1, treat it like a scalar value
	template <bool W1, integral_dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, integral_dimensional_scalar T2, typename D2>
	requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C > 1u)
	constexpr auto operator ^(const vector_base<W1, T1, C, D1> &lhs,
							  const vector_base<W2, T2, 1u, D2> &rhs) noexcept
	{
		return detail::binary_op_execute(std::make_index_sequence<C>{}, lhs, rhs[0u], xor_op);
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
	constexpr basic_vector<T, C> operator +(const vector_base<W, T, C, D> &arg) noexcept
	{
		return basic_vector(arg);						// no-op copy
	}

	// when Count == 1, treat it like a scalar value
	template <bool W, dimensional_scalar T, typename D>
	requires non_bool_arithmetic<T>
	constexpr T operator +(const vector_base<W, T, 1u, D> &arg) noexcept
	{
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
	constexpr auto &operator ++(const vector_base<W, T, C, D> &arg) noexcept
	{
		arg += T(1);
		return arg.as_derived();
	}

	// post-increment
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr basic_vector<T, C> operator ++(vector_base<W, T, C, D> &arg, int) noexcept
	{
		basic_vector<T, C> value(arg);
		arg += T(1);
		return value;
	}

	// post-increment
	// when Count == 1, treat it like a scalar value
	template <bool W, dimensional_scalar T, typename C>
	requires W && non_bool_arithmetic<T>
	constexpr T operator ++(vector_base<W, T, 1u, C> &arg, int) noexcept
	{
		T value = arg[0u];
		arg += T(1);
		return value;
	}

	// unary operators --

	// pre-decrement
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr auto &operator --(const vector_base<W, T, C, D> &arg) noexcept
	{
		arg -= T(1);
		return arg.as_derived();
	}

	// post-decrement
	template <bool W, dimensional_scalar T, std::size_t C, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr basic_vector<T, C> operator --(vector_base<W, T, C, D> &arg, int) noexcept
	{
		basic_vector<T, C> value(arg);
		arg -= T(1);
		return value;
	}

	// post-decrement
	// when Count == 1, treat it like a scalar value
	template <bool W, dimensional_scalar T, typename D>
	requires W && non_bool_arithmetic<T>
	constexpr T operator --(vector_base<W, T, 1u, D> &arg, int) noexcept
	{
		T value = arg[0u];
		arg -= T(1);
		return value;
	}

	//
	// equality comparisons
	//

	template <bool W1, dimensional_scalar T1, std::size_t C, typename D1,
		bool W2, dimensional_scalar T2, typename D2>
	requires implicitly_convertible_to<T2, T1>
	constexpr bool operator ==(const vector_base<W1, T1, C, D1> &first,
							   const vector_base<W2, T2, C, D2> &second) noexcept
	{
		for (std::size_t i = 0; i < C; ++i)
			if (first[i] != static_cast<T1>(second[i]))
				return false;

		return true;
	}

	template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
	constexpr bool operator ==(const vector_base<W1, T, C, D1> &first,
							   const vector_base<W2, T, C, D2> &second) noexcept
	{
		for (std::size_t i = 0; i < C; ++i)
			if (first[i] != second[i])
				return false;

		return true;
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
using vectype1 = dsga::basic_vector<T, 1u>;

// 2D vector
template <dsga::dimensional_scalar T>
using vectype2 = dsga::basic_vector<T, 2u>;

// 3D vector
template <dsga::dimensional_scalar T>
using vectype3 = dsga::basic_vector<T, 3u>;

// 4D vector
template <dsga::dimensional_scalar T>
using vectype4 = dsga::basic_vector<T, 4u>;

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
