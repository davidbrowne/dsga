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

	// common initial sequence wrapper with basic storage access -- forwards function calls to wrapped storage
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

		template <typename ...Args>
		requires (sizeof...(Args) == Size) && (std::convertible_to<Args, T> &&...)
		constexpr		void		set(Args ...args)						noexcept	{ set_impl(sequence_pack{}, args...); }

		// support for range-for loop
		constexpr		auto		begin()									noexcept	{ return value.begin(); }
		constexpr		auto		begin()							const	noexcept	{ return value.cbegin(); }
		constexpr		auto		cbegin()						const	noexcept	{ return value.cbegin(); }
		constexpr		auto		end()									noexcept	{ return value.end(); }
		constexpr		auto		end()							const	noexcept	{ return value.cend(); }
		constexpr		auto		cend()							const	noexcept	{ return value.cend(); }

		// details
		private:
			template <typename ...Args, std::size_t ...Is>
			requires (sizeof...(Args) == Size) && (sizeof...(Is) == Size) && (std::convertible_to<Args, T> &&...)
			constexpr void set_impl(std::index_sequence<Is...> /* dummy */,
									Args ...args) noexcept { ((value[Is] = static_cast<T>(args)),...); }

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

	// this is a CRTP struct that will help us with operators, compound assignment operators, and functions.
	//
	// It provides:
	// 
	// 		set() - relies on init() in Derived
	// 		operator[] - relies on at() in Derived
	// 		size() - relies on Count template parameter
	//
	template <bool Writable, dimensional_scalar T, std::size_t Count, typename Derived>
	requires dimensional_storage<T, Count>
	struct vector_base
	{
		// CRTP access to Derived class
		constexpr		Derived		&as_derived()							noexcept	requires Writable	{ return static_cast<Derived &>(*this); }
		constexpr const Derived		&as_derived()					const	noexcept						{ return static_cast<const Derived &>(*this); }

		// logically contiguous write access to all data that allows for self-assignment that works properly
		template <typename ...Args>
		requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
		constexpr		void		set(Args ...args)						noexcept						{ this->as_derived().init(args...); }

		// logically contiguous access to piecewise data as index goes from 0 to (Count - 1)
		constexpr		T			&operator [](std::size_t index)			noexcept	requires Writable	{ return this->as_derived().at(index); }
		constexpr const	T			&operator [](std::size_t index) const	noexcept						{ return this->as_derived().at(index); }

		// number of accessible T elements
		constexpr		std::size_t	size()							const	noexcept						{ return Count; }
	};

	// basic_vector will act as the primary vector class in this library.
	//
	// T is the type of the elements stored in the vector/storage
	// Size is number of elements referencable in vector/storage

	template <dimensional_scalar T, std::size_t Size>
	requires dimensional_storage<T, Size>
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

	template <typename T, std::size_t Size, std::size_t Count, std::size_t ...Is>
	requires indexable<T, Size, Count, Is...>
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

	// for swizzling 1D parts of basic_vector - like a scalar accessor
	template <dimensional_scalar T, std::size_t Size, std::size_t I>
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

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename U>
		requires Writable && std::convertible_to<U, T>
		constexpr void init(U value0) noexcept
		{
			value[I] = static_cast<T>(value0);
		}

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u]);

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			init(other[0u]);

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
		constexpr T &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[I];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const T &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[I];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
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
		constexpr auto begin()				noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto begin()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto cbegin()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, 0u); }
		constexpr auto end()				noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I>(*this, Count); }
		constexpr auto end()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, Count); }
		constexpr auto cend()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I>(*this, Count); }
	};

	// for swizzling 2D parts of basic_vector
	template <dimensional_scalar T, std::size_t Size, std::size_t I0, std::size_t I1>
	struct indexed_vector<T, Size, 2u, I0, I1>
		: vector_base<writable_swizzle<Size, 2u, I0, I1>, T, 2u, indexed_vector<T, Size, 2u, I0, I1>>
	{
		// we have partial specialization, so can't use template parameter for Count number of logical storage elements
		static constexpr std::size_t Count = 2u;

		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, I0, I1>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<I0, I1>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename U1, typename U2>
		requires Writable && std::convertible_to<U1, T> && std::convertible_to<U2, T>
		constexpr void init(U1 value0, U2 value1) noexcept
		{
			value[I0] = static_cast<T>(value0);
			value[I1] = static_cast<T>(value1);
		}

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u]);

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			init(other[0u], other[1u]);

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr T &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[I0];
				case 1u:
					return value[I1];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const T &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[I0];
				case 1u:
					return value[I1];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<T, Count>() const noexcept
		{
			return basic_vector<T, Count>(value[I0], value[I1]);
		}

		template <dimensional_scalar U>
		requires (!std::same_as<T, U> && std::convertible_to<T, U>)
		explicit constexpr operator basic_vector<U, Count>() const noexcept
		{
			return basic_vector<U, Count>(static_cast<U>(value[I0]),
										  static_cast<U>(value[I1]));
		}

		// support for range-for loop
		constexpr auto begin()				noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I0, I1>(*this, 0u); }
		constexpr auto begin()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1>(*this, 0u); }
		constexpr auto cbegin()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1>(*this, 0u); }
		constexpr auto end()				noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I0, I1>(*this, Count); }
		constexpr auto end()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1>(*this, Count); }
		constexpr auto cend()		const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1>(*this, Count); }
	};

	// for swizzling 3D parts of basic_vector
	template <dimensional_scalar T, std::size_t Size, std::size_t I0, std::size_t I1, std::size_t I2>
	struct indexed_vector<T, Size, 3u, I0, I1, I2>
		: vector_base<writable_swizzle<Size, 3u, I0, I1, I2>, T, 3u, indexed_vector<T, Size, 3u, I0, I1, I2>>
	{
		// we have partial specialization, so can't use template parameter for Count number of logical storage elements
		static constexpr std::size_t Count = 3u;

		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, I0, I1, I2>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<I0, I1, I2>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename U1, typename U2, typename U3>
		requires Writable && std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		constexpr void init(U1 value0, U2 value1, U3 value2) noexcept
		{
			value[I0] = static_cast<T>(value0);
			value[I1] = static_cast<T>(value1);
			value[I2] = static_cast<T>(value2);
		}

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u]);

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr T &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[I0];
				case 1u:
					return value[I1];
				case 2u:
					return value[I2];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const T &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[I0];
				case 1u:
					return value[I1];
				case 2u:
					return value[I2];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<T, Count>() const noexcept
		{
			return basic_vector<T, Count>(value[I0], value[I1], value[I2]);
		}

		template <dimensional_scalar U>
		requires (!std::same_as<T, U> && std::convertible_to<T, U>)
		explicit constexpr operator basic_vector<U, Count>() const noexcept
		{
			return basic_vector<U, Count>(static_cast<U>(value[I0]),
										  static_cast<U>(value[I1]),
										  static_cast<U>(value[I2]));
		}

		// support for range-for loop
		constexpr auto begin()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I0, I1, I2>(*this, 0u); }
		constexpr auto begin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2>(*this, 0u); }
		constexpr auto cbegin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2>(*this, 0u); }
		constexpr auto end()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I0, I1, I2>(*this, Count); }
		constexpr auto end()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2>(*this, Count); }
		constexpr auto cend()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2>(*this, Count); }
	};

	// for swizzling 4D parts of basic_vector
	template <dimensional_scalar T, std::size_t Size, std::size_t I0, std::size_t I1, std::size_t I2, std::size_t I3>
	struct indexed_vector<T, Size, 4u, I0, I1, I2, I3>
		: vector_base<writable_swizzle<Size, 4u, I0, I1, I2, I3>, T, 4u, indexed_vector<T, Size, 4u, I0, I1, I2, I3>>
	{
		// we have partial specialization, so can't use template parameter for Count number of logical storage elements
		static constexpr std::size_t Count = 4u;

		// we have partial specialization, so can't use template parameter for Writable if this swizzle can be an lvalue
		static constexpr bool Writable = writable_swizzle<Size, Count, I0, I1, I2, I3>;

		//
		// the underlying ordered storage sequence for this logical vector - possibly helpful for indirection.
		// currently unused because at() does this logically for us.
		//

		// as a parameter pack
		using sequence_pack = std::index_sequence<I0, I1, I2, I3>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		// common initial sequence data - the storage is Size in length, not Count which is number of indexes
		dimensional_storage_t<T, Size> value;

		// default/deleted boilerplate
		indexed_vector() noexcept = default;
		~indexed_vector() noexcept = default;

		// logically contiguous - used by set() for write access to data
		// allows for self-assignment that works properly
		template <typename U1, typename U2, typename U3, typename U4>
		requires Writable &&
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		constexpr void init(U1 value0, U2 value1, U3 value2, U4 value3) noexcept
		{
			value[I0] = static_cast<T>(value0);
			value[I1] = static_cast<T>(value1);
			value[I2] = static_cast<T>(value2);
			value[I3] = static_cast<T>(value3);
		}

		// copy assignment
		template <bool W, dimensional_scalar U, typename D>
		requires Writable && implicitly_convertible_to<U, T>
		constexpr indexed_vector &operator =(const vector_base<W, U, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);

			return *this;
		}

		template <bool W, typename D>
		requires Writable
		constexpr indexed_vector &operator =(const vector_base<W, T, Count, D> &other) noexcept
		{
			init(other[0u], other[1u], other[2u], other[3u]);

			return *this;
		}

		// logically contiguous - used by operator [] for read/write access to data
		constexpr T &at(std::size_t index) requires Writable
		{
			switch (index)
			{
				case 0u:
					return value[I0];
				case 1u:
					return value[I1];
				case 2u:
					return value[I2];
				case 3u:
					return value[I3];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// logically contiguous - used by operator [] for read access to data
		constexpr const T &at(std::size_t index) const
		{
			switch (index)
			{
				case 0u:
					return value[I0];
				case 1u:
					return value[I1];
				case 2u:
					return value[I2];
				case 3u:
					return value[I3];
			}

			throw std::out_of_range("invalid indexed_vector subscript");
		}

		// basic_vector conversion operator
		// this is extremely important.
		constexpr operator basic_vector<T, Count>() const noexcept
		{
			return basic_vector<T, Count>(value[I0], value[I1], value[I2], value[I3]);
		}

		template <dimensional_scalar U>
		requires (!std::same_as<T, U> && std::convertible_to<T, U>)
		explicit constexpr operator basic_vector<U, Count>() const noexcept
		{
			return basic_vector<U, Count>(static_cast<U>(value[I0]),
										  static_cast<U>(value[I1]),
										  static_cast<U>(value[I2]),
										  static_cast<U>(value[I3]));
		}

		// support for range-for loop
		constexpr auto begin()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I0, I1, I2, I3>(*this, 0u); }
		constexpr auto begin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2, I3>(*this, 0u); }
		constexpr auto cbegin()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2, I3>(*this, 0u); }
		constexpr auto end()			noexcept	requires Writable	{ return indexed_vector_iterator<T, Size, Count, I0, I1, I2, I3>(*this, Count); }
		constexpr auto end()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2, I3>(*this, Count); }
		constexpr auto cend()	const	noexcept						{ return indexed_vector_const_iterator<T, Size, Count, I0, I1, I2, I3>(*this, Count); }
	};


	// convenience using types for indexed_vector as members of basic_vector

	template <typename T, std::size_t Size, std::size_t I>
	using index1_vector = indexed_vector<std::remove_cvref_t<T>, Size, 1u, I>;

	template <typename T, std::size_t Size, std::size_t I0, std::size_t I1>
	using index2_vector = indexed_vector<std::remove_cvref_t<T>, Size, 2u, I0, I1>;

	template <typename T, std::size_t Size, std::size_t I0, std::size_t I1, std::size_t I2>
	using index3_vector = indexed_vector<std::remove_cvref_t<T>, Size, 3u, I0, I1, I2>;

	template <typename T, std::size_t Size, std::size_t I0, std::size_t I1, std::size_t I2, std::size_t I3>
	using index4_vector = indexed_vector<std::remove_cvref_t<T>, Size, 4u, I0, I1, I2, I3>;


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
			storage_wrapper<T, Size>				store;

			index1_vector<T, Size, 0>				x;				// Writable

			index2_vector<T, Size, 0, 0>			xx;

			index3_vector<T, Size, 0, 0, 0>			xxx;

			index4_vector<T, Size, 0, 0, 0, 0>		xxxx;
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
			: store()
		{
			init(other[0u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store()
		{
			init(other[0u]);
		}

		template <typename U>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(U value) noexcept
			: store()
		{
			init(value);
		}

		template <typename U>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(U value) noexcept
			: store()
		{
			init(value);
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
		using sequence_pack = std::make_index_sequence<Count>;

		// as an array
		static constexpr std::array<std::size_t, Count> sequence_array = make_sequence_array(sequence_pack{});

		union
		{
			storage_wrapper<T, Size>				store;

			index1_vector<T, Size, 0>				x;				// Writable
			index1_vector<T, Size, 1>				y;				// Writable

			index2_vector<T, Size, 0, 0>			xx;
			index2_vector<T, Size, 0, 1>			xy;				// Writable
			index2_vector<T, Size, 1, 0>			yx;				// Writable
			index2_vector<T, Size, 1, 1>			yy;

			index3_vector<T, Size, 0, 0, 0>			xxx;
			index3_vector<T, Size, 0, 0, 1>			xxy;
			index3_vector<T, Size, 0, 1, 0>			xyx;
			index3_vector<T, Size, 0, 1, 1>			xyy;
			index3_vector<T, Size, 1, 0, 0>			yxx;
			index3_vector<T, Size, 1, 0, 1>			yxy;
			index3_vector<T, Size, 1, 1, 0>			yyx;
			index3_vector<T, Size, 1, 1, 1>			yyy;

			index4_vector<T, Size, 0, 0, 0, 0>		xxxx;
			index4_vector<T, Size, 0, 0, 0, 1>		xxxy;
			index4_vector<T, Size, 0, 0, 1, 0>		xxyx;
			index4_vector<T, Size, 0, 0, 1, 1>		xxyy;
			index4_vector<T, Size, 0, 1, 0, 0>		xyxx;
			index4_vector<T, Size, 0, 1, 0, 1>		xyxy;
			index4_vector<T, Size, 0, 1, 1, 0>		xyyx;
			index4_vector<T, Size, 0, 1, 1, 1>		xyyy;
			index4_vector<T, Size, 1, 0, 0, 0>		yxxx;
			index4_vector<T, Size, 1, 0, 0, 1>		yxxy;
			index4_vector<T, Size, 1, 0, 1, 0>		yxyx;
			index4_vector<T, Size, 1, 0, 1, 1>		yxyy;
			index4_vector<T, Size, 1, 1, 0, 0>		yyxx;
			index4_vector<T, Size, 1, 1, 0, 1>		yyxy;
			index4_vector<T, Size, 1, 1, 1, 0>		yyyx;
			index4_vector<T, Size, 1, 1, 1, 1>		yyyy;
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
			: store()
		{
			init(value, value);
		}

		template <typename U1, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to <U2, T>
		explicit constexpr basic_vector(U1 xvalue, U2 yvalue) noexcept
			: store()
		{
			init(xvalue, yvalue);
		}

		template <typename U1, bool W, dimensional_scalar U2, std::size_t C, typename D>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(U1 xvalue, const vector_base<W, U2, C, D> &yvalue_source) noexcept
			: store()
		{
			init(xvalue, yvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store()
		{
			init(other[0u], other[1u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T> && (C >= Count))
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store()
		{
			init(other[0u], other[1u]);
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
		template <typename U1, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		constexpr void init(U1 value0, U2 value1) noexcept
		{
			store.value[0u] = static_cast<T>(value0);
			store.value[1u] = static_cast<T>(value1);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

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
			storage_wrapper<T, Size>				store;

			index1_vector<T, Size, 0>				x;				// Writable
			index1_vector<T, Size, 1>				y;				// Writable
			index1_vector<T, Size, 2>				z;				// Writable

			index2_vector<T, Size, 0, 0>			xx;
			index2_vector<T, Size, 0, 1>			xy;				// Writable
			index2_vector<T, Size, 0, 2>			xz;				// Writable
			index2_vector<T, Size, 1, 0>			yx;				// Writable
			index2_vector<T, Size, 1, 1>			yy;
			index2_vector<T, Size, 1, 2>			yz;				// Writable
			index2_vector<T, Size, 2, 0>			zx;				// Writable
			index2_vector<T, Size, 2, 1>			zy;				// Writable
			index2_vector<T, Size, 2, 2>			zz;

			index3_vector<T, Size, 0, 0, 0>		xxx;
			index3_vector<T, Size, 0, 0, 1>		xxy;
			index3_vector<T, Size, 0, 0, 2>		xxz;
			index3_vector<T, Size, 0, 1, 0>		xyx;
			index3_vector<T, Size, 0, 1, 1>		xyy;
			index3_vector<T, Size, 0, 1, 2>		xyz;			// Writable
			index3_vector<T, Size, 0, 2, 0>		xzx;
			index3_vector<T, Size, 0, 2, 1>		xzy;			// Writable
			index3_vector<T, Size, 0, 2, 2>		xzz;
			index3_vector<T, Size, 1, 0, 0>		yxx;
			index3_vector<T, Size, 1, 0, 1>		yxy;
			index3_vector<T, Size, 1, 0, 2>		yxz;			// Writable
			index3_vector<T, Size, 1, 1, 0>		yyx;
			index3_vector<T, Size, 1, 1, 1>		yyy;
			index3_vector<T, Size, 1, 1, 2>		yyz;
			index3_vector<T, Size, 1, 2, 0>		yzx;			// Writable
			index3_vector<T, Size, 1, 2, 1>		yzy;
			index3_vector<T, Size, 1, 2, 2>		yzz;
			index3_vector<T, Size, 2, 0, 0>		zxx;
			index3_vector<T, Size, 2, 0, 1>		zxy;			// Writable
			index3_vector<T, Size, 2, 0, 2>		zxz;
			index3_vector<T, Size, 2, 1, 0>		zyx;			// Writable
			index3_vector<T, Size, 2, 1, 1>		zyy;
			index3_vector<T, Size, 2, 1, 2>		zyz;
			index3_vector<T, Size, 2, 2, 0>		zzx;
			index3_vector<T, Size, 2, 2, 1>		zzy;
			index3_vector<T, Size, 2, 2, 2>		zzz;

			index4_vector<T, Size, 0, 0, 0, 0>		xxxx;
			index4_vector<T, Size, 0, 0, 0, 1>		xxxy;
			index4_vector<T, Size, 0, 0, 0, 2>		xxxz;
			index4_vector<T, Size, 0, 0, 1, 0>		xxyx;
			index4_vector<T, Size, 0, 0, 1, 1>		xxyy;
			index4_vector<T, Size, 0, 0, 1, 2>		xxyz;
			index4_vector<T, Size, 0, 0, 2, 0>		xxzx;
			index4_vector<T, Size, 0, 0, 2, 1>		xxzy;
			index4_vector<T, Size, 0, 0, 2, 2>		xxzz;
			index4_vector<T, Size, 0, 1, 0, 0>		xyxx;
			index4_vector<T, Size, 0, 1, 0, 1>		xyxy;
			index4_vector<T, Size, 0, 1, 0, 2>		xyxz;
			index4_vector<T, Size, 0, 1, 1, 0>		xyyx;
			index4_vector<T, Size, 0, 1, 1, 1>		xyyy;
			index4_vector<T, Size, 0, 1, 1, 2>		xyyz;
			index4_vector<T, Size, 0, 1, 2, 0>		xyzx;
			index4_vector<T, Size, 0, 1, 2, 1>		xyzy;
			index4_vector<T, Size, 0, 1, 2, 2>		xyzz;
			index4_vector<T, Size, 0, 2, 0, 0>		xzxx;
			index4_vector<T, Size, 0, 2, 0, 1>		xzxy;
			index4_vector<T, Size, 0, 2, 0, 2>		xzxz;
			index4_vector<T, Size, 0, 2, 1, 0>		xzyx;
			index4_vector<T, Size, 0, 2, 1, 1>		xzyy;
			index4_vector<T, Size, 0, 2, 1, 2>		xzyz;
			index4_vector<T, Size, 0, 2, 2, 0>		xzzx;
			index4_vector<T, Size, 0, 2, 2, 1>		xzzy;
			index4_vector<T, Size, 0, 2, 2, 2>		xzzz;
			index4_vector<T, Size, 1, 0, 0, 0>		yxxx;
			index4_vector<T, Size, 1, 0, 0, 1>		yxxy;
			index4_vector<T, Size, 1, 0, 0, 2>		yxxz;
			index4_vector<T, Size, 1, 0, 1, 0>		yxyx;
			index4_vector<T, Size, 1, 0, 1, 1>		yxyy;
			index4_vector<T, Size, 1, 0, 1, 2>		yxyz;
			index4_vector<T, Size, 1, 0, 2, 0>		yxzx;
			index4_vector<T, Size, 1, 0, 2, 1>		yxzy;
			index4_vector<T, Size, 1, 0, 2, 2>		yxzz;
			index4_vector<T, Size, 1, 1, 0, 0>		yyxx;
			index4_vector<T, Size, 1, 1, 0, 1>		yyxy;
			index4_vector<T, Size, 1, 1, 0, 2>		yyxz;
			index4_vector<T, Size, 1, 1, 1, 0>		yyyx;
			index4_vector<T, Size, 1, 1, 1, 1>		yyyy;
			index4_vector<T, Size, 1, 1, 1, 2>		yyyz;
			index4_vector<T, Size, 1, 1, 2, 0>		yyzx;
			index4_vector<T, Size, 1, 1, 2, 1>		yyzy;
			index4_vector<T, Size, 1, 1, 2, 2>		yyzz;
			index4_vector<T, Size, 1, 2, 0, 0>		yzxx;
			index4_vector<T, Size, 1, 2, 0, 1>		yzxy;
			index4_vector<T, Size, 1, 2, 0, 2>		yzxz;
			index4_vector<T, Size, 1, 2, 1, 0>		yzyx;
			index4_vector<T, Size, 1, 2, 1, 1>		yzyy;
			index4_vector<T, Size, 1, 2, 1, 2>		yzyz;
			index4_vector<T, Size, 1, 2, 2, 0>		yzzx;
			index4_vector<T, Size, 1, 2, 2, 1>		yzzy;
			index4_vector<T, Size, 1, 2, 2, 2>		yzzz;
			index4_vector<T, Size, 2, 0, 0, 0>		zxxx;
			index4_vector<T, Size, 2, 0, 0, 1>		zxxy;
			index4_vector<T, Size, 2, 0, 0, 2>		zxxz;
			index4_vector<T, Size, 2, 0, 1, 0>		zxyx;
			index4_vector<T, Size, 2, 0, 1, 1>		zxyy;
			index4_vector<T, Size, 2, 0, 1, 2>		zxyz;
			index4_vector<T, Size, 2, 0, 2, 0>		zxzx;
			index4_vector<T, Size, 2, 0, 2, 1>		zxzy;
			index4_vector<T, Size, 2, 0, 2, 2>		zxzz;
			index4_vector<T, Size, 2, 1, 0, 0>		zyxx;
			index4_vector<T, Size, 2, 1, 0, 1>		zyxy;
			index4_vector<T, Size, 2, 1, 0, 2>		zyxz;
			index4_vector<T, Size, 2, 1, 1, 0>		zyyx;
			index4_vector<T, Size, 2, 1, 1, 1>		zyyy;
			index4_vector<T, Size, 2, 1, 1, 2>		zyyz;
			index4_vector<T, Size, 2, 1, 2, 0>		zyzx;
			index4_vector<T, Size, 2, 1, 2, 1>		zyzy;
			index4_vector<T, Size, 2, 1, 2, 2>		zyzz;
			index4_vector<T, Size, 2, 2, 0, 0>		zzxx;
			index4_vector<T, Size, 2, 2, 0, 1>		zzxy;
			index4_vector<T, Size, 2, 2, 0, 2>		zzxz;
			index4_vector<T, Size, 2, 2, 1, 0>		zzyx;
			index4_vector<T, Size, 2, 2, 1, 1>		zzyy;
			index4_vector<T, Size, 2, 2, 1, 2>		zzyz;
			index4_vector<T, Size, 2, 2, 2, 0>		zzzx;
			index4_vector<T, Size, 2, 2, 2, 1>		zzzy;
			index4_vector<T, Size, 2, 2, 2, 2>		zzzz;
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
			: store()
		{
			init(value, value, value);
		}

		template <typename U1, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue);
		}

		template <typename U1, typename U2, bool W, dimensional_scalar U3, std::size_t C, typename D>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										const vector_base<W, U3, C, D> &zvalue_source) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires implicitly_convertible_to<U, T> && (C >= Count)
		constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u]);
		}

		template <bool W, dimensional_scalar U, std::size_t C, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T> && (C >= Count))
		explicit constexpr basic_vector(const vector_base<W, U, C, D> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u]);
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 2u, D> &other,
										U2 yet_another) noexcept
			: store()
		{
			init(other[0u], other[1u], yet_another);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, typename U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &other,
										const vector_base<W2, U2, C, D2> &yet_another) noexcept
			: store()
		{
			init(other[0u], other[1u], yet_another[0u]);
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 2)
		explicit constexpr basic_vector(U2 yet_another,
										const vector_base<W, U1, C, D> &other) noexcept
			: store()
		{
			init(yet_another, other[0u], other[1u]);
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
		template <typename U1, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		constexpr void init(U1 value0, U2 value1, U3 value2) noexcept
		{
			store.value[0u] = static_cast<T>(value0);
			store.value[1u] = static_cast<T>(value1);
			store.value[2u] = static_cast<T>(value2);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

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
			storage_wrapper<T, Size>				store;

			index1_vector<T, Size, 0>				x;				// Writable
			index1_vector<T, Size, 1>				y;				// Writable
			index1_vector<T, Size, 2>				z;				// Writable
			index1_vector<T, Size, 3>				w;				// Writable

			index2_vector<T, Size, 0, 0>			xx;
			index2_vector<T, Size, 0, 1>			xy;				// Writable
			index2_vector<T, Size, 0, 2>			xz;				// Writable
			index2_vector<T, Size, 0, 3>			xw;				// Writable
			index2_vector<T, Size, 1, 0>			yx;				// Writable
			index2_vector<T, Size, 1, 1>			yy;
			index2_vector<T, Size, 1, 2>			yz;				// Writable
			index2_vector<T, Size, 1, 3>			yw;				// Writable
			index2_vector<T, Size, 2, 0>			zx;				// Writable
			index2_vector<T, Size, 2, 1>			zy;				// Writable
			index2_vector<T, Size, 2, 2>			zz;
			index2_vector<T, Size, 2, 3>			zw;				// Writable
			index2_vector<T, Size, 3, 0>			wx;				// Writable
			index2_vector<T, Size, 3, 1>			wy;				// Writable
			index2_vector<T, Size, 3, 2>			wz;				// Writable
			index2_vector<T, Size, 3, 3>			ww;

			index3_vector<T, Size, 0, 0, 0>			xxx;
			index3_vector<T, Size, 0, 0, 1>			xxy;
			index3_vector<T, Size, 0, 0, 2>			xxz;
			index3_vector<T, Size, 0, 0, 3>			xxw;
			index3_vector<T, Size, 0, 1, 0>			xyx;
			index3_vector<T, Size, 0, 1, 1>			xyy;
			index3_vector<T, Size, 0, 1, 2>			xyz;			// Writable
			index3_vector<T, Size, 0, 1, 3>			xyw;			// Writable
			index3_vector<T, Size, 0, 2, 0>			xzx;
			index3_vector<T, Size, 0, 2, 1>			xzy;			// Writable
			index3_vector<T, Size, 0, 2, 2>			xzz;
			index3_vector<T, Size, 0, 2, 3>			xzw;			// Writable
			index3_vector<T, Size, 0, 3, 0>			xwx;
			index3_vector<T, Size, 0, 3, 1>			xwy;			// Writable
			index3_vector<T, Size, 0, 3, 2>			xwz;			// Writable
			index3_vector<T, Size, 0, 3, 3>			xww;
			index3_vector<T, Size, 1, 0, 0>			yxx;
			index3_vector<T, Size, 1, 0, 1>			yxy;
			index3_vector<T, Size, 1, 0, 2>			yxz;			// Writable
			index3_vector<T, Size, 1, 0, 3>			yxw;			// Writable
			index3_vector<T, Size, 1, 1, 0>			yyx;
			index3_vector<T, Size, 1, 1, 1>			yyy;
			index3_vector<T, Size, 1, 1, 2>			yyz;
			index3_vector<T, Size, 1, 1, 3>			yyw;
			index3_vector<T, Size, 1, 2, 0>			yzx;			// Writable
			index3_vector<T, Size, 1, 2, 1>			yzy;
			index3_vector<T, Size, 1, 2, 2>			yzz;
			index3_vector<T, Size, 1, 2, 3>			yzw;			// Writable
			index3_vector<T, Size, 1, 3, 0>			ywx;			// Writable
			index3_vector<T, Size, 1, 3, 1>			ywy;
			index3_vector<T, Size, 1, 3, 2>			ywz;			// Writable
			index3_vector<T, Size, 1, 3, 3>			yww;
			index3_vector<T, Size, 2, 0, 0>			zxx;
			index3_vector<T, Size, 2, 0, 1>			zxy;			// Writable
			index3_vector<T, Size, 2, 0, 2>			zxz;
			index3_vector<T, Size, 2, 0, 3>			zxw;			// Writable
			index3_vector<T, Size, 2, 1, 0>			zyx;			// Writable
			index3_vector<T, Size, 2, 1, 1>			zyy;
			index3_vector<T, Size, 2, 1, 2>			zyz;
			index3_vector<T, Size, 2, 1, 3>			zyw;			// Writable
			index3_vector<T, Size, 2, 2, 0>			zzx;
			index3_vector<T, Size, 2, 2, 1>			zzy;
			index3_vector<T, Size, 2, 2, 2>			zzz;
			index3_vector<T, Size, 2, 2, 3>			zzw;
			index3_vector<T, Size, 2, 3, 0>			zwx;			// Writable
			index3_vector<T, Size, 2, 3, 1>			zwy;			// Writable
			index3_vector<T, Size, 2, 3, 2>			zwz;
			index3_vector<T, Size, 2, 3, 3>			zww;
			index3_vector<T, Size, 3, 0, 0>			wxx;
			index3_vector<T, Size, 3, 0, 1>			wxy;			// Writable
			index3_vector<T, Size, 3, 0, 2>			wxz;			// Writable
			index3_vector<T, Size, 3, 0, 3>			wxw;
			index3_vector<T, Size, 3, 1, 0>			wyx;			// Writable
			index3_vector<T, Size, 3, 1, 1>			wyy;
			index3_vector<T, Size, 3, 1, 2>			wyz;			// Writable
			index3_vector<T, Size, 3, 1, 3>			wyw;
			index3_vector<T, Size, 3, 2, 0>			wzx;			// Writable
			index3_vector<T, Size, 3, 2, 1>			wzy;
			index3_vector<T, Size, 3, 2, 2>			wzz;			// Writable
			index3_vector<T, Size, 3, 2, 3>			wzw;
			index3_vector<T, Size, 3, 3, 0>			wwx;
			index3_vector<T, Size, 3, 3, 1>			wwy;
			index3_vector<T, Size, 3, 3, 2>			wwz;
			index3_vector<T, Size, 3, 3, 3>			www;

			index4_vector<T, Size, 0, 0, 0, 0>		xxxx;
			index4_vector<T, Size, 0, 0, 0, 1>		xxxy;
			index4_vector<T, Size, 0, 0, 0, 2>		xxxz;
			index4_vector<T, Size, 0, 0, 0, 3>		xxxw;
			index4_vector<T, Size, 0, 0, 1, 0>		xxyx;
			index4_vector<T, Size, 0, 0, 1, 1>		xxyy;
			index4_vector<T, Size, 0, 0, 1, 2>		xxyz;
			index4_vector<T, Size, 0, 0, 1, 3>		xxyw;
			index4_vector<T, Size, 0, 0, 2, 0>		xxzx;
			index4_vector<T, Size, 0, 0, 2, 1>		xxzy;
			index4_vector<T, Size, 0, 0, 2, 2>		xxzz;
			index4_vector<T, Size, 0, 0, 2, 3>		xxzw;
			index4_vector<T, Size, 0, 0, 3, 0>		xxwx;
			index4_vector<T, Size, 0, 0, 3, 1>		xxwy;
			index4_vector<T, Size, 0, 0, 3, 2>		xxwz;
			index4_vector<T, Size, 0, 0, 3, 3>		xxww;
			index4_vector<T, Size, 0, 1, 0, 0>		xyxx;
			index4_vector<T, Size, 0, 1, 0, 1>		xyxy;
			index4_vector<T, Size, 0, 1, 0, 2>		xyxz;
			index4_vector<T, Size, 0, 1, 0, 3>		xyxw;
			index4_vector<T, Size, 0, 1, 1, 0>		xyyx;
			index4_vector<T, Size, 0, 1, 1, 1>		xyyy;
			index4_vector<T, Size, 0, 1, 1, 2>		xyyz;
			index4_vector<T, Size, 0, 1, 1, 3>		xyyw;
			index4_vector<T, Size, 0, 1, 2, 0>		xyzx;
			index4_vector<T, Size, 0, 1, 2, 1>		xyzy;
			index4_vector<T, Size, 0, 1, 2, 2>		xyzz;
			index4_vector<T, Size, 0, 1, 2, 3>		xyzw;			// Writable
			index4_vector<T, Size, 0, 1, 3, 0>		xywx;
			index4_vector<T, Size, 0, 1, 3, 1>		xywy;
			index4_vector<T, Size, 0, 1, 3, 2>		xywz;			// Writable
			index4_vector<T, Size, 0, 1, 3, 3>		xyww;
			index4_vector<T, Size, 0, 2, 0, 0>		xzxx;
			index4_vector<T, Size, 0, 2, 0, 1>		xzxy;
			index4_vector<T, Size, 0, 2, 0, 2>		xzxz;
			index4_vector<T, Size, 0, 2, 0, 3>		xzxw;
			index4_vector<T, Size, 0, 2, 1, 0>		xzyx;
			index4_vector<T, Size, 0, 2, 1, 1>		xzyy;
			index4_vector<T, Size, 0, 2, 1, 2>		xzyz;
			index4_vector<T, Size, 0, 2, 1, 3>		xzyw;			// Writable
			index4_vector<T, Size, 0, 2, 2, 0>		xzzx;
			index4_vector<T, Size, 0, 2, 2, 1>		xzzy;
			index4_vector<T, Size, 0, 2, 2, 2>		xzzz;
			index4_vector<T, Size, 0, 2, 2, 3>		xzzw;
			index4_vector<T, Size, 0, 2, 3, 0>		xzwx;
			index4_vector<T, Size, 0, 2, 3, 1>		xzwy;			// Writable
			index4_vector<T, Size, 0, 2, 3, 2>		xzwz;
			index4_vector<T, Size, 0, 2, 3, 3>		xzww;
			index4_vector<T, Size, 0, 3, 0, 0>		xwxx;
			index4_vector<T, Size, 0, 3, 0, 1>		xwxy;
			index4_vector<T, Size, 0, 3, 0, 2>		xwxz;
			index4_vector<T, Size, 0, 3, 0, 3>		xwxw;
			index4_vector<T, Size, 0, 3, 1, 0>		xwyx;
			index4_vector<T, Size, 0, 3, 1, 1>		xwyy;
			index4_vector<T, Size, 0, 3, 1, 2>		xwyz;			// Writable
			index4_vector<T, Size, 0, 3, 1, 3>		xwyw;
			index4_vector<T, Size, 0, 3, 2, 0>		xwzx;
			index4_vector<T, Size, 0, 3, 2, 1>		xwzy;			// Writable
			index4_vector<T, Size, 0, 3, 2, 2>		xwzz;
			index4_vector<T, Size, 0, 3, 2, 3>		xwzw;
			index4_vector<T, Size, 0, 3, 3, 0>		xwwx;
			index4_vector<T, Size, 0, 3, 3, 1>		xwwy;
			index4_vector<T, Size, 0, 3, 3, 2>		xwwz;
			index4_vector<T, Size, 0, 3, 3, 3>		xwww;
			index4_vector<T, Size, 1, 0, 0, 0>		yxxx;
			index4_vector<T, Size, 1, 0, 0, 1>		yxxy;
			index4_vector<T, Size, 1, 0, 0, 2>		yxxz;
			index4_vector<T, Size, 1, 0, 0, 3>		yxxw;
			index4_vector<T, Size, 1, 0, 1, 0>		yxyx;
			index4_vector<T, Size, 1, 0, 1, 1>		yxyy;
			index4_vector<T, Size, 1, 0, 1, 2>		yxyz;
			index4_vector<T, Size, 1, 0, 1, 3>		yxyw;
			index4_vector<T, Size, 1, 0, 2, 0>		yxzx;
			index4_vector<T, Size, 1, 0, 2, 1>		yxzy;
			index4_vector<T, Size, 1, 0, 2, 2>		yxzz;
			index4_vector<T, Size, 1, 0, 2, 3>		yxzw;			// Writable
			index4_vector<T, Size, 1, 0, 3, 0>		yxwx;
			index4_vector<T, Size, 1, 0, 3, 1>		yxwy;
			index4_vector<T, Size, 1, 0, 3, 2>		yxwz;			// Writable
			index4_vector<T, Size, 1, 0, 3, 3>		yxww;
			index4_vector<T, Size, 1, 1, 0, 0>		yyxx;
			index4_vector<T, Size, 1, 1, 0, 1>		yyxy;
			index4_vector<T, Size, 1, 1, 0, 2>		yyxz;
			index4_vector<T, Size, 1, 1, 0, 3>		yyxw;
			index4_vector<T, Size, 1, 1, 1, 0>		yyyx;
			index4_vector<T, Size, 1, 1, 1, 1>		yyyy;
			index4_vector<T, Size, 1, 1, 1, 2>		yyyz;
			index4_vector<T, Size, 1, 1, 1, 3>		yyyw;
			index4_vector<T, Size, 1, 1, 2, 0>		yyzx;
			index4_vector<T, Size, 1, 1, 2, 1>		yyzy;
			index4_vector<T, Size, 1, 1, 2, 2>		yyzz;
			index4_vector<T, Size, 1, 1, 2, 3>		yyzw;
			index4_vector<T, Size, 1, 1, 3, 0>		yywx;
			index4_vector<T, Size, 1, 1, 3, 1>		yywy;
			index4_vector<T, Size, 1, 1, 3, 2>		yywz;
			index4_vector<T, Size, 1, 1, 3, 3>		yyww;
			index4_vector<T, Size, 1, 2, 0, 0>		yzxx;
			index4_vector<T, Size, 1, 2, 0, 1>		yzxy;
			index4_vector<T, Size, 1, 2, 0, 2>		yzxz;
			index4_vector<T, Size, 1, 2, 0, 3>		yzxw;			// Writable
			index4_vector<T, Size, 1, 2, 1, 0>		yzyx;
			index4_vector<T, Size, 1, 2, 1, 1>		yzyy;
			index4_vector<T, Size, 1, 2, 1, 2>		yzyz;
			index4_vector<T, Size, 1, 2, 1, 3>		yzyw;
			index4_vector<T, Size, 1, 2, 2, 0>		yzzx;
			index4_vector<T, Size, 1, 2, 2, 1>		yzzy;
			index4_vector<T, Size, 1, 2, 2, 2>		yzzz;
			index4_vector<T, Size, 1, 2, 2, 3>		yzzw;
			index4_vector<T, Size, 1, 2, 3, 0>		yzwx;			// Writable
			index4_vector<T, Size, 1, 2, 3, 1>		yzwy;
			index4_vector<T, Size, 1, 2, 3, 2>		yzwz;
			index4_vector<T, Size, 1, 2, 3, 3>		yzww;
			index4_vector<T, Size, 1, 3, 0, 0>		ywxx;
			index4_vector<T, Size, 1, 3, 0, 1>		ywxy;
			index4_vector<T, Size, 1, 3, 0, 2>		ywxz;			// Writable
			index4_vector<T, Size, 1, 3, 0, 3>		ywxw;
			index4_vector<T, Size, 1, 3, 1, 0>		ywyx;
			index4_vector<T, Size, 1, 3, 1, 1>		ywyy;
			index4_vector<T, Size, 1, 3, 1, 2>		ywyz;
			index4_vector<T, Size, 1, 3, 1, 3>		ywyw;
			index4_vector<T, Size, 1, 3, 2, 0>		ywzx;			// Writable
			index4_vector<T, Size, 1, 3, 2, 1>		ywzy;
			index4_vector<T, Size, 1, 3, 2, 2>		ywzz;
			index4_vector<T, Size, 1, 3, 2, 3>		ywzw;
			index4_vector<T, Size, 1, 3, 3, 0>		ywwx;
			index4_vector<T, Size, 1, 3, 3, 1>		ywwy;
			index4_vector<T, Size, 1, 3, 3, 2>		ywwz;
			index4_vector<T, Size, 1, 3, 3, 3>		ywww;
			index4_vector<T, Size, 2, 0, 0, 0>		zxxx;
			index4_vector<T, Size, 2, 0, 0, 1>		zxxy;
			index4_vector<T, Size, 2, 0, 0, 2>		zxxz;
			index4_vector<T, Size, 2, 0, 0, 3>		zxxw;
			index4_vector<T, Size, 2, 0, 1, 0>		zxyx;
			index4_vector<T, Size, 2, 0, 1, 1>		zxyy;
			index4_vector<T, Size, 2, 0, 1, 2>		zxyz;
			index4_vector<T, Size, 2, 0, 1, 3>		zxyw;			// Writable
			index4_vector<T, Size, 2, 0, 2, 0>		zxzx;
			index4_vector<T, Size, 2, 0, 2, 1>		zxzy;
			index4_vector<T, Size, 2, 0, 2, 2>		zxzz;
			index4_vector<T, Size, 2, 0, 2, 3>		zxzw;
			index4_vector<T, Size, 2, 0, 3, 0>		zxwx;
			index4_vector<T, Size, 2, 0, 3, 1>		zxwy;			// Writable
			index4_vector<T, Size, 2, 0, 3, 2>		zxwz;
			index4_vector<T, Size, 2, 0, 3, 3>		zxww;
			index4_vector<T, Size, 2, 1, 0, 0>		zyxx;
			index4_vector<T, Size, 2, 1, 0, 1>		zyxy;
			index4_vector<T, Size, 2, 1, 0, 2>		zyxz;
			index4_vector<T, Size, 2, 1, 0, 3>		zyxw;			// Writable
			index4_vector<T, Size, 2, 1, 1, 0>		zyyx;
			index4_vector<T, Size, 2, 1, 1, 1>		zyyy;
			index4_vector<T, Size, 2, 1, 1, 2>		zyyz;
			index4_vector<T, Size, 2, 1, 1, 3>		zyyw;
			index4_vector<T, Size, 2, 1, 2, 0>		zyzx;
			index4_vector<T, Size, 2, 1, 2, 1>		zyzy;
			index4_vector<T, Size, 2, 1, 2, 2>		zyzz;
			index4_vector<T, Size, 2, 1, 2, 3>		zyzw;
			index4_vector<T, Size, 2, 1, 3, 0>		zywx;			// Writable
			index4_vector<T, Size, 2, 1, 3, 1>		zywy;
			index4_vector<T, Size, 2, 1, 3, 2>		zywz;
			index4_vector<T, Size, 2, 1, 3, 3>		zyww;
			index4_vector<T, Size, 2, 2, 0, 0>		zzxx;
			index4_vector<T, Size, 2, 2, 0, 1>		zzxy;
			index4_vector<T, Size, 2, 2, 0, 2>		zzxz;
			index4_vector<T, Size, 2, 2, 0, 3>		zzxw;
			index4_vector<T, Size, 2, 2, 1, 0>		zzyx;
			index4_vector<T, Size, 2, 2, 1, 1>		zzyy;
			index4_vector<T, Size, 2, 2, 1, 2>		zzyz;
			index4_vector<T, Size, 2, 2, 1, 3>		zzyw;
			index4_vector<T, Size, 2, 2, 2, 0>		zzzx;
			index4_vector<T, Size, 2, 2, 2, 1>		zzzy;
			index4_vector<T, Size, 2, 2, 2, 2>		zzzz;
			index4_vector<T, Size, 2, 2, 2, 3>		zzzw;
			index4_vector<T, Size, 2, 2, 3, 0>		zzwx;
			index4_vector<T, Size, 2, 2, 3, 1>		zzwy;
			index4_vector<T, Size, 2, 2, 3, 2>		zzwz;
			index4_vector<T, Size, 2, 2, 3, 3>		zzww;
			index4_vector<T, Size, 2, 3, 0, 0>		zwxx;
			index4_vector<T, Size, 2, 3, 0, 1>		zwxy;			// Writable
			index4_vector<T, Size, 2, 3, 0, 2>		zwxz;
			index4_vector<T, Size, 2, 3, 0, 3>		zwxw;
			index4_vector<T, Size, 2, 3, 1, 0>		zwyx;			// Writable
			index4_vector<T, Size, 2, 3, 1, 1>		zwyy;
			index4_vector<T, Size, 2, 3, 1, 2>		zwyz;
			index4_vector<T, Size, 2, 3, 1, 3>		zwyw;
			index4_vector<T, Size, 2, 3, 2, 0>		zwzx;
			index4_vector<T, Size, 2, 3, 2, 1>		zwzy;
			index4_vector<T, Size, 2, 3, 2, 2>		zwzz;
			index4_vector<T, Size, 2, 3, 2, 3>		zwzw;
			index4_vector<T, Size, 2, 3, 3, 0>		zwwx;
			index4_vector<T, Size, 2, 3, 3, 1>		zwwy;
			index4_vector<T, Size, 2, 3, 3, 2>		zwwz;
			index4_vector<T, Size, 2, 3, 3, 3>		zwww;
			index4_vector<T, Size, 3, 0, 0, 0>		wxxx;
			index4_vector<T, Size, 3, 0, 0, 1>		wxxy;
			index4_vector<T, Size, 3, 0, 0, 2>		wxxz;
			index4_vector<T, Size, 3, 0, 0, 3>		wxxw;
			index4_vector<T, Size, 3, 0, 1, 0>		wxyx;
			index4_vector<T, Size, 3, 0, 1, 1>		wxyy;
			index4_vector<T, Size, 3, 0, 1, 2>		wxyz;			// Writable
			index4_vector<T, Size, 3, 0, 1, 3>		wxyw;
			index4_vector<T, Size, 3, 0, 2, 0>		wxzx;
			index4_vector<T, Size, 3, 0, 2, 1>		wxzy;			// Writable
			index4_vector<T, Size, 3, 0, 2, 2>		wxzz;
			index4_vector<T, Size, 3, 0, 2, 3>		wxzw;
			index4_vector<T, Size, 3, 0, 3, 0>		wxwx;
			index4_vector<T, Size, 3, 0, 3, 1>		wxwy;
			index4_vector<T, Size, 3, 0, 3, 2>		wxwz;
			index4_vector<T, Size, 3, 0, 3, 3>		wxww;
			index4_vector<T, Size, 3, 1, 0, 0>		wyxx;
			index4_vector<T, Size, 3, 1, 0, 1>		wyxy;
			index4_vector<T, Size, 3, 1, 0, 2>		wyxz;			// Writable
			index4_vector<T, Size, 3, 1, 0, 3>		wyxw;
			index4_vector<T, Size, 3, 1, 1, 0>		wyyx;
			index4_vector<T, Size, 3, 1, 1, 1>		wyyy;
			index4_vector<T, Size, 3, 1, 1, 2>		wyyz;
			index4_vector<T, Size, 3, 1, 1, 3>		wyyw;
			index4_vector<T, Size, 3, 1, 2, 0>		wyzx;			// Writable
			index4_vector<T, Size, 3, 1, 2, 1>		wyzy;
			index4_vector<T, Size, 3, 1, 2, 2>		wyzz;
			index4_vector<T, Size, 3, 1, 2, 3>		wyzw;
			index4_vector<T, Size, 3, 1, 3, 0>		wywx;
			index4_vector<T, Size, 3, 1, 3, 1>		wywy;
			index4_vector<T, Size, 3, 1, 3, 2>		wywz;
			index4_vector<T, Size, 3, 1, 3, 3>		wyww;
			index4_vector<T, Size, 3, 2, 0, 0>		wzxx;
			index4_vector<T, Size, 3, 2, 0, 1>		wzxy;			// Writable
			index4_vector<T, Size, 3, 2, 0, 2>		wzxz;
			index4_vector<T, Size, 3, 2, 0, 3>		wzxw;
			index4_vector<T, Size, 3, 2, 1, 0>		wzyx;			// Writable
			index4_vector<T, Size, 3, 2, 1, 1>		wzyy;
			index4_vector<T, Size, 3, 2, 1, 2>		wzyz;
			index4_vector<T, Size, 3, 2, 1, 3>		wzyw;
			index4_vector<T, Size, 3, 2, 2, 0>		wzzx;
			index4_vector<T, Size, 3, 2, 2, 1>		wzzy;
			index4_vector<T, Size, 3, 2, 2, 2>		wzzz;
			index4_vector<T, Size, 3, 2, 2, 3>		wzzw;
			index4_vector<T, Size, 3, 2, 3, 0>		wzwx;
			index4_vector<T, Size, 3, 2, 3, 1>		wzwy;
			index4_vector<T, Size, 3, 2, 3, 2>		wzwz;
			index4_vector<T, Size, 3, 2, 3, 3>		wzww;
			index4_vector<T, Size, 3, 3, 0, 0>		wwxx;
			index4_vector<T, Size, 3, 3, 0, 1>		wwxy;
			index4_vector<T, Size, 3, 3, 0, 2>		wwxz;
			index4_vector<T, Size, 3, 3, 0, 3>		wwxw;
			index4_vector<T, Size, 3, 3, 1, 0>		wwyx;
			index4_vector<T, Size, 3, 3, 1, 1>		wwyy;
			index4_vector<T, Size, 3, 3, 1, 2>		wwyz;
			index4_vector<T, Size, 3, 3, 1, 3>		wwyw;
			index4_vector<T, Size, 3, 3, 2, 0>		wwzx;
			index4_vector<T, Size, 3, 3, 2, 1>		wwzy;
			index4_vector<T, Size, 3, 3, 2, 2>		wwzz;
			index4_vector<T, Size, 3, 3, 2, 3>		wwzw;
			index4_vector<T, Size, 3, 3, 3, 0>		wwwx;
			index4_vector<T, Size, 3, 3, 3, 1>		wwwy;
			index4_vector<T, Size, 3, 3, 3, 2>		wwwz;
			index4_vector<T, Size, 3, 3, 3, 3>		wwww;
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
			: store()
		{
			init(value, value, value, value);
		}

		template <typename U1, typename U2, typename U3, typename U4>
		requires
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		explicit constexpr basic_vector(U1 xvalue,
										U2 yvalue,
										U3 zvalue,
										U4 wvalue) noexcept
			: store()
		{
			init(xvalue, yvalue, zvalue, wvalue);
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
			: store()
		{
			init(xvalue, yvalue, zvalue, wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U, typename D>
		requires implicitly_convertible_to<U, T>
		constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], other[3u]);
		}

		template <bool W, dimensional_scalar U, typename D>
		requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
		explicit constexpr basic_vector(const vector_base<W, U, Count, D> &other) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], other[3u]);
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 3u, D> &other,
										U2 yet_another) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], yet_another);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, dimensional_scalar U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 3u, D1> &other,
										const vector_base<W2, U2, C, D2> &wvalue_source) noexcept
			: store()
		{
			init(other[0u], other[1u], other[2u], wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 3)
		explicit constexpr basic_vector(U2 yet_another,
										const vector_base<W, U1, C, D> &other) noexcept
			: store()
		{
			init(yet_another, other[0u], other[1u], other[2u]);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			bool W2, dimensional_scalar U2, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && (C >= 2)
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &first,
										const vector_base<W2, U2, C, D2> &second) noexcept
			: store()
		{
			init(first[0u], first[1u], second[0u], second[1u]);
		}

		template <bool W, dimensional_scalar U1, typename D,  typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(const vector_base<W, U1, 2u, D> &other,
										U2 first,
										U3 second) noexcept
			: store()
		{
			init(other[0u], other[1u], first, second);
		}

		template <bool W1, dimensional_scalar U1, typename D1, typename U2,
			bool W2, dimensional_scalar U3, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(const vector_base<W1, U1, 2u, D1> &other,
										U2 first,
										const vector_base<W2, U3, C, D2> &wvalue_source) noexcept
			: store()
		{
			init(other[0u], other[1u], first, wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U1, typename D, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U2 first,
										const vector_base<W, U1, 2u, D> &other,
										U3 second) noexcept
			: store()
		{
			init(first, other[0u], other[1u], second);
		}

		template <bool W1, dimensional_scalar U1, typename D1,
			typename U2, bool W2, dimensional_scalar U3, std::size_t C, typename D2>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T>
		explicit constexpr basic_vector(U2 first,
										const vector_base<W1, U1, 2u, D1> &other,
										const vector_base<W2, U3, C, D2> &wvalue_source) noexcept
			: store()
		{
			init(first, other[0u], other[1u], wvalue_source[0u]);
		}

		template <bool W, dimensional_scalar U1, std::size_t C, typename D, typename U2, typename U3>
		requires std::convertible_to<U1, T> && std::convertible_to<U2, T> && std::convertible_to<U3, T> && (C >= 2)
		explicit constexpr basic_vector(U2 first,
										U3 second,
										const vector_base<W, U1, C, D> &other) noexcept
			: store()
		{
			init(first, second, other[0u], other[1u]);
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
		template <typename U1, typename U2, typename U3, typename U4>
		requires
			std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
			std::convertible_to<U3, T> && std::convertible_to<U4, T>
		constexpr void init(U1 value0, U2 value1, U3 value2, U4 value3) noexcept
		{
			store.value[0u] = static_cast<T>(value0);
			store.value[1u] = static_cast<T>(value1);
			store.value[2u] = static_cast<T>(value2);
			store.value[3u] = static_cast<T>(value3);
		}

		// logically and physically contiguous - used by operator [] for access to data
		constexpr		T	&at(std::size_t index)					noexcept	{ return store.value[index]; }
		constexpr const	T	&at(std::size_t index)			const	noexcept	{ return store.value[index]; }

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

		// perform the lambda action, setting the lhs vector_base to new values

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
