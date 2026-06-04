# dsga API

The API is primarily dictated by the OpenGL Shading Language 4.6 specification ([pdf](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) | [html](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html)) for vectors and matrices. It is also dictated by the need to interact with the ```c++20``` ecosystem.

## Contents
* [Library ```cxcm```](#library-cxcm)
* [Namespace ```dsga```](#namespace-dsga)
* [Concepts](#concepts)
* [Variable Templates](#variable-templates)
* [Using Directives](#using-directives)
* [Utility Functions](#utility-functions)
* [Class Templates](#class-templates)
* [Class Template Instantiations](#class-template-instantiations)
* Vector
  * [Vector Operators](#vector-operators)
  * [Vector Free Functions](#vector-free-functions)
  * [Scalar Functions](#scalar-functions)
* Matrix
  * [Matrix Operators](#matrix-operators)
  * [Matrix Free Functions](#matrix-free-functions)
* [Simple Conversion Functions](#simple-conversion-functions)
* [Tuple Protocol](#tuple-protocol)

## Library cxcm
[cxcm](https://github.com/davidbrowne/cxcm) is its own stand-alone project for extending ```<cmath>``` to have more functions be constexpr, targeted for ```c++20```. ```c++23``` and hopefully ```c++26``` extend the amount of functions in ```<cmath>``` to be constexpr, but this project aims to support ```c++20```. ```cxcm``` has been brought in under ```namespace dsga``` as a nested ```namespace cxcm```, with all the functionality of the stand-alone ```cxcm``` release. This allows us to have a single header library for ```dsga```.

```dsga``` needs ```cxcm``` to implement some of the vector free functions as constexpr functions. Some of these functions are designed so that if they are invoked at runtime instead of compile time, then they invoke the corresponding functions in the standard library, giving us the fastest implementation. See the [```cxcm``` API](https://github.com/davidbrowne/cxcm/blob/main/README.md#cxcm-free-functions) for how to use. Remember, ```cxcm``` is a nested namespace under ```namespace dsga``` for this project.

## namespace dsga
Unless otherwise stated, all of the API is defined in ```namespace dsga```.

### Concepts

* [```bool_scalar```](#bool_scalar)
* [```signed_scalar```](#signed_scalar)
* [```unsigned_scalar```](#unsigned_scalar)
* [```numeric_integral_scalar```](#numeric_integral_scalar)
* [```floating_point_scalar```](#floating_point_scalar)
* [```non_bool_scalar```](#non_bool_scalar)
* [```dimensional_scalar```](#dimensional_scalar)
* [```dimensional_size```](#dimensional_size)
* [```dimensional_storage```](#dimensional_storage)
* [```promotes_to```](#promotes_to)
* [```implicitly_convertible_to```](#implicitly_convertible_to)
* [```indexable```](#indexable)

#### ```bool_scalar```
```c++
template <typename T>
concept bool_scalar = std::same_as<bool, T>;
```
Plain undecorated boolean type.

#### ```signed_scalar```
```c++
template <typename T>
concept signed_scalar = (std::same_as<int, T> || std::same_as<long long, T>);
```
Plain undecorated signed types.

#### ```unsigned_scalar```
```c++
template <typename T>
concept unsigned_scalar = (std::same_as<unsigned int, T> || std::same_as<unsigned long long, T> || std::same_as<std::size_t, T>);
```
Plain undecorated unsigned types.

#### ```numeric_integral_scalar```
```c++
template <typename T>
concept numeric_integral_scalar = (signed_scalar<T> || unsigned_scalar<T>);
```
Plain undecorated integral types.

#### ```floating_point_scalar```
```c++
template <typename T>
concept floating_point_scalar = (std::same_as<float, T> || std::same_as<double, T>);
```
Plain undecorated floating point types.

#### ```non_bool_scalar```
```c++
template <typename T>
concept non_bool_scalar = (numeric_integral_scalar<T> || floating_point_scalar<T>);
```
Plain undecorated integral and floating point types.

#### ```dimensional_scalar```
```c++
template <typename T>
concept dimensional_scalar = (non_bool_scalar<T> || bool_scalar<T>);
```
Plain undecorated arithmetic types.

#### ```dimensional_size```
```c++
template <std::size_t Size>
concept dimensional_size = ((Size >= 1) && (Size <= 4));
```
We want the size to be between 1 and 4, inclusive.

#### ```dimensional_storage```
```c++
template <typename T, std::size_t Size>
concept dimensional_storage = dimensional_scalar<T> && dimensional_size<Size>;
```
Vector type and size requirements.

#### ```promotes_to```
```c++
template <typename T, typename U>
concept promotes_to =
requires
{
    typename std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
    requires std::same_as<std::common_type_t<std::remove_cvref_t<T>, std::remove_cvref_t<U>>, std::remove_cvref_t<U>>;
};
```
Is the second type U also the common type of the two types T and U?

#### ```implicitly_convertible_to```
```c++
template <typename T, typename U>
concept implicitly_convertible_to = non_bool_scalar<T> && non_bool_scalar<U> && promotes_to<T, U>;
```
Are implicit conversions allowed for non-boolean arithmetic purposes?

#### ```indexable```
```c++
template <std::size_t Size, std::size_t Count, std::size_t ...Is>
concept indexable = detail::valid_index_count<Count, Is...>() && detail::valid_range_indexes<Size, Is...>();
```
Do the argument indexes and count/size make for valid indirect indexing, such as for ```swizzle_vec```.

Size and Count are two different things, with Size being the physical number of elements, and Count being the logical number of elements. Count must be the ```sizeof...Is```, and for every one of the Is, it must be smaller than Size.

### Variable Templates

* [```writable_swizzle```](#writable_swizzle)
* [```degrees_per_radian_v```](#degrees_per_radian_v)
* [```radians_per_degree_v```](#radians_per_degree_v)

#### ```writable_swizzle```
```c++
template <std::size_t Size, std::size_t Count, std::size_t ...Is>
requires indexable<Size, Count, Is...>
constexpr inline bool writable_swizzle = detail::unique_indexes(std::index_sequence<Is...>{});
```
Used for the Writable template parameter of a ```swizzle_vec```. Can a particular swizzle be used as an lvalue reference. All of the swizzle indexes must have been used at most once, e.g., for ```xyz```, Writable is true, and for ```xyx```, Writable is false (assuming that all other requirements are met).

#### ```degrees_per_radian_v```
```c++
template <floating_point_scalar T>
inline constexpr T degrees_per_radian_v = std::numbers::inv_pi_v<T> * T(180);
```

#### ```radians_per_degree_v```
```c++
template <floating_point_scalar T>
inline constexpr T radians_per_degree_v = std::numbers::pi_v<T> / T(180);
```

### Using Directives

* [```dimensional_storage_t```](#dimensional_storage_t)
* [```make_index_range```](#make_index_range)
* [```make_closed_index_range```](#make_closed_index_range)
* [```make_array_sequence```](#make_array_sequence)
* Using Directives for Creating ```swizzle_vec```s
  * [```dexvec1```](#dexvec1)
  * [```dexvec2```](#dexvec2)
  * [```dexvec3```](#dexvec3)
  * [```dexvec4```](#dexvec4)

#### ```dimensional_storage_t```
```c++
template <dimensional_scalar T, std::size_t Size>
requires dimensional_storage<T, Size>
using dimensional_storage_t = std::array<T, Size>;
```
The underlying storage type for ```storage_wrappper``` and ```swizzle_vec```. It is contiguous, and it has contiguous iterators.

#### ```make_index_range```
```c++
template<std::size_t Start, std::size_t End>
using make_index_range = decltype(detail::index_range<Start, End>());
```
This gives a half-open/half-closed interval in a ```std::index_sequence``` -> [Start, End). If End is less than Start, the sequence is in descending order.

#### ```make_closed_index_range```
```c++
template<std::size_t Start, std::size_t End>
using make_closed_index_range = decltype(detail::closed_index_range<Start, End>());
```
This gives a closed interval in a ```std::index_sequence``` -> [Start, End]. If End is less than Start, the sequence is in descending order.

#### ```make_array_sequence```
```c++
template <detail::sequence_indexable auto vals>
using make_array_sequence = decltype(detail::indexable_to_sequence<vals>(std::make_index_sequence<vals.size()>{}));
```
This gives a ```std::index_sequence``` that contains the elements of a constexpr ```std::array<T, N> vals```, where ```T``` is convertible to a ```std::size_t``` and none of the elements are negative. Constexpr non-type template parameter```vals``` doesn't have to be of type ```std::array<T, N>```, but it must have constexpr member functions ```size``` and ```operator []```, both of whose return values are convertible to ```std::size_t```, e.g., ```dsga::vec<T, N>```, as long as all values in the vector are non-negative and ```std::convertible_to<T, std::size_t>``` is true.

#### Using Directives for Creating ```swizzle_vec```s

#### ```dexvec1```
```c++
template <typename T, std::size_t Size, std::size_t I>
using dexvec1 = swizzle_vec<std::remove_cvref_t<T>, Size, 1, I>;
```
Convenience using directive for creating a ```swizzle_vec``` for ```Count == 1```.

#### ```dexvec2```
```c++
template <typename T, std::size_t Size, std::size_t ...Is>
requires (sizeof...(Is) == 2)
using dexvec2 = swizzle_vec<std::remove_cvref_t<T>, Size, 2, Is...>;
```
Convenience using directive for creating a ```swizzle_vec``` for ```Count == 2```.

#### ```dexvec3```
```c++
template <typename T, std::size_t Size, std::size_t ...Is>
requires (sizeof...(Is) == 3)
using dexvec3 = swizzle_vec<std::remove_cvref_t<T>, Size, 3, Is...>;
```
Convenience using directive for creating a ```swizzle_vec``` for ```Count == 3```.

#### ```dexvec4```
```c++
template <typename T, std::size_t Size, std::size_t ...Is>
requires (sizeof...(Is) == 4)
using dexvec4 = swizzle_vec<std::remove_cvref_t<T>, Size, 4, Is...>;
```
Convenience using directive for creating a ```swizzle_vec``` for ```Count == 4```.

### Utility Functions

* [```make_sequence_array```](#make_sequence_array)
* [```make_reverse_sequence```](#make_reverse_sequence)
* [```is_constexpr```](#is_constexpr)
* [```to_underlying```](#to_underlying)

#### ```make_sequence_array```
```c++
template <std::size_t... Is>
constexpr std::array<std::size_t, sizeof...(Is)> make_sequence_array(std::index_sequence<Is...>) noexcept;
```
Build an array from the indexes of a ```std::index_sequence```.

#### ```make_reverse_sequence```
```c++
template<std::size_t ...Is>
constexpr auto make_reverse_sequence(std::index_sequence<Is...> seq) noexcept;
```
Convert a ```std::index_sequence<Is...>``` to another ```std::index_sequence``` with the ```Is...``` in reverse order from input.

#### ```is_constexpr```
```c++
template <typename C, auto val = std::bool_constant<(C{}(), true)>{}>
consteval auto is_constexpr(C) noexcept;
```
Is a default-constructible callable constexpr? If not, then compile error due to trying to evaluate something that is not constexpr at compile time.

##### ```to_underlying```
```c++
template <typename E>
requires std::is_enum_v<E>
[[nodiscard]] constexpr std::underlying_type_t<E> to_underlying(E e) noexcept;
```
Not a vector or matrix function. Not in GLSL. This functionality was added to ```c++23```, but since this is a ```c++20``` library, we have to provide the underlying implementation ourselves. It is not really a good fit for dsga, but it is handy to have around anyway.

### Class Templates

* [```vec```](#vec)
* [```mat```](#mat)
* [```swizzle_vec```](#swizzle_vec)
  * [```swizzle_vec_iterator```](#swizzle_vec_iterator)
  * [```swizzle_vec_const_iterator```](#swizzle_vec_const_iterator)
* [```vec_interface```](#vec_interface)
* [```vec_storage```](#vec_storage)

This diagram explores the relationships between the various ```dsga``` template classes, which are actually all structs. The main two template structs are ```vec``` and ```mat```. The other structs are in support of those two primary components.

![dsga class diagram](./dsga.svg)

#### ```vec_storage```
```c++
template <dimensional_scalar T, std::size_t Size>
requires dimensional_storage<T, Size>
struct vec_storage;
```
This struct is structurally equivalent to [```swizzle_vec```](#swizzle_vec). They both wrap a data member of type ```dimensional_storage_t```. When comparing an [```swizzle_vec```](#swizzle_vec) and ```dimensional_storage_t``` of the same type and size, as members in a union, they satisfy the type trait ```std::is_corresponding_member```. For a union of many members of [```swizzle_vec```](#swizzle_vec) and a member of ```dimensional_storage_t```, these all share a [common initial sequence](https://en.cppreference.com/w/cpp/language/data_members). This allows us to read any of the union members without having to make a member active first (via writing). An anonymous union of a ```vec_storage``` and multiple swizzles of [```swizzle_vec```](#swizzle_vec) instantiations constitutes the data of a [```vec```](#vec).

```vec_storage``` is not part of the GLSL specification, but it is used by [```vec```](#vec) for writing data to storage. It has been designed to have a similar API to [```vec```](#vec).

* Template Parameters
  * ```T``` - type of the ```vec_storage``` elements. Must satisfy [```dimensional_scalar```](#dimensional_scalar).
  * ```Size``` - number of ```vec_storage``` elements. Must satisfy [```dimensional_size```](#dimensional_size).
* Non-Static Data Members
  * [```store```](#vec_storagestore)
* Static Data Members
  * [```Count```](#vec_storagecount)
  * [```Writable```](#vec_storagewritable)
  * [```offsets```](#vec_storageoffsets)
  * [```size``` (```std::integral_constant```)](#vec_storagesize-stdintegral_constant)
* Static Member Functions
  * [```size``` (theoretical function)](#vec_storagesize-theoretical-function)
  * [```sequence```](#vec_storagesequence)
* Non-Static Member Functions
  * [```length```](#vec_storagelength)
  * [```operator []```](#vec_storageoperator-)
  * [```set```](#vec_storageset)
  * [```swap```](#vec_storageswap)
  * [Iterators](#vec_storage-iterators)
* Using Directives
  * [```sequence_pack```](#vec_storagesequence_pack)
  * ```value_type```
  * ```iterator```
  * ```const_iterator```
  * ```reverse_iterator```
  * ```const_reverse_iterator```

* [Free Functions](#vec_storage-free-functions)
* [Class Template Argument Deduction (CTAD)](#vec_storage-ctad)

##### ```vec_storage::store```
```c++
dimensional_storage_t<T, Size> store;
```
The data member for the storage.

##### ```vec_storage::Count```
```c++
static constexpr std::size_t Count = Size;
```
The static variable that holds the number of items that we are handling. It is always the same value as the template parameter ```Size```. It is unnecessary for this class, but it is used here to be in solidarity with ```swizzle_vec```, where ```Size``` and ```Count``` are often not the same value.

##### ```vec_storage::Writable```
```c++
static constexpr bool Writable = true;
```
This static variable is always true for all ```vec_storage```s. It is unnecessary for this class, but it is used in here to be in solidarity with ```swizzle_vec```, where Writable is not always true (mostly false actually).

##### ```vec_storage::offsets```
```c++
static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});
```
The static ```std::array``` for how the physical representation is mapped to the logical representation. For ```vec_storage```, the physical and logical representations are the same, i.e., a contiguous representation, and the sequence ascends from 0.

##### ```vec_storage::size``` (```std::integral_constant```)
```c++
static constexpr std::integral_constant<std::size_t, Count> size = {};
```

##### ```vec_storage::size``` (theoretical function)
```c++
[[nodiscard]] static constexpr std::size_t size() const noexcept;
```
Return the number of components in the struct. The declaration for ```size()``` is a fiction due to the fact that this function does not exist; however, the static ```std::integral_constant``` ```size``` has an ```operator()()``` that operates exactly as the above declaration. This approach of using a ```std::integral_constant``` for ```size``` is supposed to be an up and coming idiom in the C++ standard for all new standard library components with constant sizes.

##### ```vec_storage::length```
```c++
[[nodiscard]] constexpr int length() const noexcept;
```
The GLSL specification requires that the ```length method``` behave as a member function, returning the number of components in the struct.

##### ```vec_storage::operator []```
```c++
template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable;

template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept;
```
Data access through the indexing operator. The indexing operator already takes the physical to logical data mapping into account. For ```vec_storage```, the elements are contiguous, so the physical and logical mapping is the same.

##### ```vec_storage::sequence```
```c++
[[nodiscard]] static constexpr auto sequence() noexcept;
```
The ```std::index_sequence``` for how the physical representation is mapped to the logical representation. For ```vec_storage```, the physical and logical representation are the same, i.e., a contiguous representation, and the sequence ascends from 0.

##### ```vec_storage::set```
```c++
template <typename ...Args>
requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
constexpr void set(Args ...args) noexcept;
```
Set all the values of a ```vec_storage``` via parameter copies. This prevents aliasing issues with references.

##### ```vec_storage::swap```
```c++
constexpr void swap(vec_storage &sw) noexcept requires Writable;
```
Swap the data using the underlying ```dimensional_storage_t```'s ```swap``` function.

##### ```vec_storage``` Iterators
```c++
[[nodiscard]] constexpr auto begin() noexcept requires Writable;
[[nodiscard]] constexpr auto begin() const noexcept;
[[nodiscard]] constexpr auto cbegin() const noexcept;
[[nodiscard]] constexpr auto end() noexcept requires Writable;
[[nodiscard]] constexpr auto end() const noexcept;
[[nodiscard]] constexpr auto cend() const noexcept;

[[nodiscard]] constexpr auto rbegin() noexcept requires Writable;
[[nodiscard]] constexpr auto rbegin() const noexcept;
[[nodiscard]] constexpr auto crbegin() const noexcept;
[[nodiscard]] constexpr auto rend() noexcept requires Writable;
[[nodiscard]] constexpr auto rend() const noexcept;
[[nodiscard]] constexpr auto crend() const noexcept;
```
These contiguous iterators are supplied by the underlying ```dimensional_storage_t```.

##### ```vec_storage::sequence_pack```
```c++
using sequence_pack = std::make_index_sequence<Count>;
```
The instantiation of ```std::index_sequence``` that represents the physical to logical mapping. ```sequence``` returns an instance of this type.

##### ```vec_storage``` Free Functions

```c++
template <dimensional_scalar T, std::size_t Size>
constexpr void swap(vec_storage<T, Size> &lhs, vec_storage<T, Size> &rhs) noexcept;
```
Free function ```swap``` wraps the member function ```vec_storage::swap```.

```c++
template <dimensional_scalar T1, std::size_t C, dimensional_scalar T2>
requires implicitly_convertible_to<T2, T1>
constexpr bool operator ==(const vec_storage<T1, C> &first,
                           const vec_storage<T2, C> &second) noexcept;
```
Returns whether all the components are exactly equal.

```c++
template <dimensional_scalar T1, std::size_t C, dimensional_scalar T2>
requires implicitly_convertible_to<T2, T1>
constexpr bool operator !=(const vec_storage<T1, C> &first,
                           const vec_storage<T2, C> &second) noexcept;
```
Function automatically generated from ```operator ==```. Returns whether any of the components are not exactly equal.

##### ```vec_storage``` CTAD
```c++
template <dimensional_scalar T, dimensional_scalar ...U>
vec_storage(T, U...) -> vec_storage<T, 1 + sizeof...(U)>;
```
[Class template argument deduction (CTAD)](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction) for ```vec_storage```.

#### ```vec_interface```
```c++
template <bool Writable, dimensional_scalar T, std::size_t Count, typename Derived>
requires dimensional_storage<T, Count>
struct vec_interface;
```
This [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) base class is inherited by derived classes [```vec```](#vec) and [```swizzle_vec```](#swizzle_vec). It has no data members of its own. Most all of the vector operators and free functions operate on ```vec_interface``` instead of the derived vectors, as we want to treat the two derived vector types as similarly as possible.

* Template Parameters
  * ```Writable``` - certain derived classes are not writable, namely most template instantiations of ```swizzle_vec```. If ```Writable``` is false, then ```vec_interface``` is effectively const.
  * ```T``` - type of the vector elements. Must satisfy [```dimensional_scalar```](#dimensional_scalar).
  * ```Count``` - number of logical vector elements. Must satisfy [```dimensional_size```](#dimensional_size).
  * ```Derived``` - the type of this derived class for which ```vec_interface``` is a CRTP base class.
* Non-Static Data Members
  * None.
* Static Data Members
  * [```size``` (```std::integral_constant```)](#vec_interfacesize-stdintegral_constant)
* Static Member Functions
  * [```size``` (theoretical function)](#vec_interfacesize-theoretical-function)
  * [```sequence```](#vec_interfacesequence)
* Non-Static Member Functions
  * [```length```](#vec_interfacelength)
  * [```as_derived```](#vec_interfaceas_derived)
  * [```operator []```](#vec_interfaceoperator-)
  * [```set```](#vec_interfaceset)
  * [Iterators](#vec_interface-iterators)
  * [```std::valarray```](https://en.cppreference.com/w/cpp/numeric/valarray) API
    * [```apply```](#vec_interfaceapply)
    * [```query```](#vec_interfacequery) - not in ```std::valarray```
    * [```shift```](#vec_interfaceshift)
    * [```cshift```](#vec_interfacecshift)
    * [```min```](#vec_interfacemin)
    * [```max```](#vec_interfacemax)
    * [```sum```](#vec_interfacesum)
* [Vector Operators](#vector-operators)
* [Vector Free Functions](#vector-free-functions)

##### ```vec_interface::size``` (```std::integral_constant```)
```c++
static constexpr std::integral_constant<std::size_t, Count> size = {};
```

##### ```vec_interface::size``` (theoretical function)
```c++
[[nodiscard]] static constexpr std::size_t size() const noexcept;
```
Return the number of components in the derived struct. The declaration for ```size()``` is a fiction due to the fact that this function does not exist; however, the static ```std::integral_constant``` ```size``` has an ```operator()()``` that operates exactly as the above declaration. This approach of using a ```std::integral_constant``` for ```size``` is supposed to be an up and coming idiom in the C++ standard for all new standard library components with constant sizes.

##### ```vec_interface::length```
```c++
[[nodiscard]] constexpr int length() const noexcept;
```
The GLSL specification requires that the ```length method``` behave as a member function, returning the number of components in the struct.

##### ```vec_interface::as_derived```
```c++
[[nodiscard]] constexpr Derived &as_derived() noexcept requires Writable;

[[nodiscard]] constexpr const Derived &as_derived() const noexcept;
```
Since this is a CRTP base class, one of the template type parameters is for the class/struct that derived from ```vec_interface```. This function returns a reference to the derived class/struct version of the ```vec_interface```.

##### ```vec_interface::operator []```
```c++
template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable;

template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept;
```
Data access through the indexing operator. This CRTP function calls the derived class/struct version of the function.

##### ```vec_interface::sequence```
```c++
[[nodiscard]] static constexpr auto sequence() noexcept;
```
The ```std::index_sequence``` for how the physical representation is mapped to the logical representation. This CRTP function calls the derived class/struct version of the function.

##### ```vec_interface::set```
```c++
template <typename ...Args>
requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> &&...)
constexpr void set(Args ...args) noexcept;
```
Set all the values of the derived struct/class via parameter copies. This prevents aliasing issues with references. This CRTP function calls the derived class/struct version of the function.

##### ```vec_interface``` Iterators
```c++
[[nodiscard]] constexpr auto begin() noexcept requires Writable;
[[nodiscard]] constexpr auto begin() const noexcept;
[[nodiscard]] constexpr auto cbegin() const noexcept;
[[nodiscard]] constexpr auto end() noexcept requires Writable;
[[nodiscard]] constexpr auto end() const noexcept;
[[nodiscard]] constexpr auto cend() const noexcept;

[[nodiscard]] constexpr auto rbegin() noexcept requires Writable;
[[nodiscard]] constexpr auto rbegin() const noexcept;
[[nodiscard]] constexpr auto crbegin() const noexcept;
[[nodiscard]] constexpr auto rend() noexcept requires Writable;
[[nodiscard]] constexpr auto rend() const noexcept;
[[nodiscard]] constexpr auto crend() const noexcept;
```
These CRTP functions call the derived class/struct versions of the functions.

##### ```vec_interface::apply```
```c++
template <typename UnOp>
requires (std::same_as<T, std::invoke_result_t<UnOp, T>> || std::same_as<T, std::invoke_result_t<UnOp, const T &>>)
[[nodiscard]] constexpr vec<T, Count> apply(UnOp op) const noexcept;
```
Applies a lambda/function/function object/callable to every element of a vector, in element order (order only matters if callable is side-effecting and/or has state). The callable must take either a ```T``` or ```const T &```, and it must return a ```T```. Returns a vector of the results.

##### ```vec_interface::query```
```c++
template <typename UnOp>
requires (std::same_as<bool, std::invoke_result_t<UnOp, T>> || std::same_as<bool, std::invoke_result_t<UnOp, const T &>>)
[[nodiscard]] constexpr vec<bool, Count> query(UnOp op) const noexcept;
```
Applies a predicate lambda/function/function object/callable to every element of a vector, in element order (order only matters if callable is side-effecting and/or has state). The callable must take either a ```T``` or ```const T &```, and it must return a ```bool```. Returns a vector of the results. Not in GLSL nor ```std::valarray```.

##### ```vec_interface::shift```
```c++
[[nodiscard]] constexpr vec<T, Count> shift(int by) const noexcept;
```
Zero-filling shift the elements of the vector. Returns a vector of the results.

##### ```vec_interface::cshift```
```c++
[[nodiscard]] constexpr vec<T, Count> cshift(int by) const noexcept;
```
Circular shift of the elements of the vector. Returns a vector of the results.

##### ```vec_interface::min```
```c++
[[nodiscard]] constexpr T min() const noexcept requires non_bool_scalar<T>;
```
Returns the smallest element.

##### ```vec_interface::max```
```c++
[[nodiscard]] constexpr T max() const noexcept requires non_bool_scalar<T>;
```
Returns the largest element.

##### ```vec_interface::sum```
```c++
[[nodiscard]] constexpr T sum() const noexcept requires non_bool_scalar<T>;
```
Returns the sum of all elements.

#### ```swizzle_vec```
```c++
template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ...Is>
requires indexable<T, Size, Count, Is...>
struct swizzle_vec<T, Size, Count, Is...>
    : vec_interface<writable_swizzle<Size, Count, Is...>, T, Count, swizzle_vec<T, Size, Count, Is...>>;
```
One of the two vector types that inherit from [```vec_interface```](#vec_interface). This type represents a [swizzle](https://en.wikipedia.org/wiki/Swizzling_(computer_graphics)) of a [```vec```](#vec). It is a physically non-contiguous, logically contiguous vector. It is not expected to be used except as the swizzle members of the anonymous union in a [```vec```](#vec). A [```vec```](#vec) is easily constructed from or assigned to from an ```swizzle_vec```.

Like [```vec_storage```](#vec_storage), this struct wraps a data member of type ```dimensional_storage_t```.

Since this vector type is physically non-contiguous, the iterator structs are not contiguous. The two iterator structs [```swizzle_vec_iterator```](#swizzle_vec_iterator) and [```swizzle_vec_const_iterator```](#swizzle_vec_const_iterator) are [random-access iterators](https://en.cppreference.com/w/cpp/iterator/random_access_iterator).

Size and Count are two different things, with Size being the physical number of elements, and Count being the logical number of elements. Count must be the ```sizeof...(Is)```, and for every one of the ```Is```, the value must be smaller than Size.

* Template Parameters
  * ```T``` - type of the ```swizzle_vec``` elements. Must satisfy [```dimensional_scalar```](#dimensional_scalar).
  * ```Size``` - physical number of ```swizzle_vec``` elements. Must satisfy [```dimensional_size```](#dimensional_size).
  * ```Count``` - logical number of ```swizzle_vec``` elements. Must satisfy [```dimensional_size```](#dimensional_size).
  * ```Is...``` - a parameter pack of the logical indexes for an ```swizzle_vec```. Count must be the ```sizeof...(Is)```, and for every one of the ```Is```, the value must be smaller than Size.
* Non-Static Data Members
  * [```base```](#swizzle_vecbase)
* Static Data Members
  * [```Writable```](#swizzle_vecwritable)
  * [```offsets```](#swizzle_vecoffsets)
* Static Member Functions
  * [```sequence```](#swizzle_vecsequence)
* Non-Static Member Functions
  * [```operator []```](#swizzle_vecoperator-)
  * [```operator =```](#swizzle_vecoperator--assignment)
  * [```set```](#swizzle_vecset)
  * [Iterators](#swizzle_vec-iterators)
* Using Directives
  * [```sequence_pack```](#swizzle_vecsequence_pack)
  * ```value_type```
  * ```iterator```
  * ```const_iterator```
  * ```reverse_iterator```
  * ```const_reverse_iterator```
* [Vector Operators](#vector-operators)
* [Vector Free Functions](#vector-free-functions)

##### ```swizzle_vec::base```
```c++
dimensional_storage_t<T, Size> base;
```
The data member for the storage.

##### ```swizzle_vec::Writable```
```c++
static constexpr bool Writable = writable_swizzle<Size, Count, Is...>;
```
Is this a writable vector. All of the ```Is...``` must be unique for this vector to be writable. If not writable, then it is effectively const.

##### ```swizzle_vec::offsets```
```c++
static constexpr std::array<std::size_t, Count> offsets;
```
The static ```std::array``` for how the physical representation is mapped to the logical representation.

##### ```swizzle_vec::operator []```
```c++
template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable;

template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept;
```
Data access through the indexing operator. The indexing operator already takes the physical to logical data mapping into account, so no need to use [```sequence```](#swizzle_vecsequence) or [```offsets```](#swizzle_vecoffsets) with this operator.

##### ```swizzle_vec::operator =``` (Assignment)
```c++
template <bool W, dimensional_scalar U, typename D>
requires Writable && implicitly_convertible_to<U, T>
constexpr swizzle_vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept;
```
The assignment operator. It can be assigned from objects that inherit from ```vec_interface```. Can assign only to lvalues.

##### ```swizzle_vec::sequence```
```c++
[[nodiscard]] static constexpr auto sequence() noexcept;
```
The ```std::index_sequence``` for how the physical representation is mapped to the logical representation.

##### ```swizzle_vec::set```
```c++
template <typename ... Args>
requires Writable && (std::convertible_to<Args, T> && ...) && (sizeof...(Args) == Count)
constexpr void set(Args ...args) noexcept;
```
Set all the vector element values via parameter copies. This prevents aliasing issues with references.

##### ```swizzle_vec``` Iterators
```c++
[[nodiscard]] constexpr auto begin() noexcept requires Writable;
[[nodiscard]] constexpr auto begin() const noexcept;
[[nodiscard]] constexpr auto cbegin() const noexcept;
[[nodiscard]] constexpr auto end() noexcept requires Writable;
[[nodiscard]] constexpr auto end() const noexcept;
[[nodiscard]] constexpr auto cend() const noexcept;

[[nodiscard]] constexpr auto rbegin() noexcept requires Writable;
[[nodiscard]] constexpr auto rbegin() const noexcept;
[[nodiscard]] constexpr auto crbegin() const noexcept;
[[nodiscard]] constexpr auto rend() noexcept requires Writable;
[[nodiscard]] constexpr auto rend() const noexcept;
[[nodiscard]] constexpr auto crend() const noexcept;
```
[Random-access iterators](https://en.cppreference.com/w/cpp/iterator/random_access_iterator) of types [```swizzle_vec_iterator```](#swizzle_vec_iterator) and [```swizzle_vec_const_iterator```](#swizzle_vec_const_iterator).

##### ```swizzle_vec::sequence_pack```
```c++
using sequence_pack = std::index_sequence<Is...>;
```
The sequence pack is formed by the variadic ```Is...``` from the template parameter pack.

#### ```swizzle_vec_iterator```
```c++
template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
requires indexable<T, Size, Count, Is...>
struct swizzle_vec_iterator : swizzle_vec_const_iterator<T, Size, Count, Is...>;
```
[Random-access iterator](https://en.cppreference.com/w/cpp/iterator/random_access_iterator) for [```swizzle_vec```](#swizzle_vec)s. Inspired by ```std::array```'s iterator, even though it is a contiguous iterator. It implements the normal API for a [random-access iterator](https://en.cppreference.com/w/cpp/iterator/random_access_iterator).

#### ```swizzle_vec_const_iterator```
```c++
template <dimensional_scalar T, std::size_t Size, std::size_t Count, std::size_t ... Is>
requires indexable<T, Size, Count, Is...>
struct swizzle_vec_const_iterator;
```
Const [random-access iterator](https://en.cppreference.com/w/cpp/iterator/random_access_iterator) for [```swizzle_vec```](#swizzle_vec)s. Inspired by ```std::array```'s iterator, even though it is a contiguous iterator. It implements the normal API for a const [random-access iterator](https://en.cppreference.com/w/cpp/iterator/random_access_iterator).

#### ```vec```
```c++
template <dimensional_scalar T, std::size_t Size>
requires dimensional_storage<T, Size>
struct vec : vec_interface<true, T, Size, vec<T, Size>>;
```
This is the primary struct for vectors. One of the two vector types that inherit from [```vec_interface```](#vec_interface). ```vec```s can be [swizzled](https://en.wikipedia.org/wiki/Swizzling_(computer_graphics)), where a swizzle is one of its anonymous union members of type [```swizzle_vec```](#swizzle_vec). The anonymous union also has a [```vec_storage```](#vec_storage) member, and the ```vec``` member functions operate on its instances through this data member. All the anonymous union members share a [common initial sequence](https://en.cppreference.com/w/cpp/language/data_members), which means they can be read by any member, regardless of whether it is the active member.

The different sized versions of ```vec``` are individually partially specialized, with sizes from 1 to 4. Each partial specialization has different data members as part of its anonymous union, due to the fact that the swizzles are different depending on ```Size```.

* Template Parameters
  * ```T``` - type of the ```vec``` elements. Must satisfy [```dimensional_scalar```](#dimensional_scalar).
  * ```Size``` - physical number of ```vec``` elements. Must satisfy [```dimensional_size```](#dimensional_size).
* [```vec``` Constructors](#vec-constructors)
* Non-Static Data Members
  * Anonymous Union
    * [```base```](#vecbase)
    * [```vec``` Swizzles](#vec-swizzles)
* Static Data Members
  * [```Size```](#vecsize)
  * [```Count```](#veccount)
  * [```Writable```](#vecwritable)
  * [```offsets```](#vecoffsets)
* Static Member Functions
  * [```sequence```](#vecsequence)
* Non-Static Member Functions
  * [```operator []```](#vecoperator-)
  * [```operator =```](#vecoperator--assignment)
  * [```set```](#vecset)
  * [```swap```](#vecswap)
  * [Iterators](#vec-iterators)
* Using Directives
  * [```sequence_pack```](#vecsequence_pack)
  * ```value_type```
  * ```iterator```
  * ```const_iterator```
  * ```reverse_iterator```
  * ```const_reverse_iterator```
* [Vector Operators](#vector-operators)
* [Vector Free Functions](#vector-free-functions)
* [```vec``` Free Functions](#vec-free-functions)
* [Class Template Argument Deduction (CTAD)](#vec-ctad)

##### ```vec``` Constructors
```c++
constexpr vec() noexcept = default;
constexpr vec(const vec &) noexcept = default;
constexpr vec(vec &&) noexcept = default;
```
Default constructors.

```c++
template <typename U>
requires std::convertible_to<U, T>
explicit constexpr vec(U value) noexcept;
```
All elements of the vector initialized to the same value.

```c++
template <typename U1, typename U2, typename U3, typename U4>
requires
    std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
    std::convertible_to<U3, T> && std::convertible_to<U4, T>
explicit constexpr vec(U1 xvalue,
                                U2 yvalue,
                                U3 zvalue,
                                U4 wvalue) noexcept;
```
Each of the elements have a scalar value passed in to initialize them. The above declaration is for a ```vec``` of ```Size == 4```. ```vec```s where ```Size == 3``` and ```Size == 2``` have similar constructors based on the number of their elements.

```c++
template <bool W, dimensional_scalar U, typename D>
requires implicitly_convertible_to<U, T>
explicit(false) constexpr vec(const vec_interface<W, U, Count, D> &other) noexcept;
```
Initialize a ```vec``` from any of the vector types that derived from ```vec_interface```.

```c++
template <typename U, typename ... Args>
requires (detail::valid_vector_component<U, T>::value) && (detail::valid_vector_component<Args, T>::value && ...) && detail::met_component_count<Count, U, Args...>
explicit constexpr vec(const U &u, const Args & ...args) noexcept;
```
Variadic constructor. Can take a combination of vectors, scalars, and matrixes as arguments to initialize the ```vec```.

```c++
constexpr vec(const std::initializer_list<T> &init_list) noexcept;
```
An intializer list of values. If too few values for the vector, the rest of the elements will be set to 0. If too many values for the vector, the rest of the initialization list will be ignored.

##### ```vec::base```
```c++
vec_storage<T, Size> base;
```
The anonymous union data member through which ```vec``` accesses the vector elements.

##### ```vec``` Swizzles
```c++
dexvec1<T, Size, 0> x;
...
...
dexvec2<T, Size, 0, 0> xx;
...
...
dexvec3<T, Size, 0, 0, 0> xxx;
...
...
dexvec4<T, Size, 0, 0, 0, 0> xxxx;
...
...
```
The other anonymous union data members are all swizzles of the ```vec```. They are of type ```swizzle_vec```.

A swizzle data member is named with 1 to 4 element reference characters, e.g., ```xy```, ```wxwz```. Depending on the value of ```Count```, there are restrictions on the swizzle element access. ```x``` maps to the first element, ```y``` maps to the second element, ```z``` maps to the third element, and ```w``` maps to the fourth element.

* ```Count == 1```

Can use {````x````} for swizzle names.

Examples: ```x```, ```xx```, ```xxx```, ```xxxx```.
* ```Count == 2```

Can use {````x````, ````y````} for swizzle names.

Examples: ```y```, ```yx```, ```xyx```, ```yxxy```.
* ```Count == 3```

Can use {````x````, ````y````, ````z````} for swizzle names.

Examples: ```z```, ```zx```, ```zyx```, ```yzxz```.
* ```Count == 4```

Can use {````x````, ````y````, ````z````, ````w````} for swizzle names.

Examples: ```w```, ```yw```, ```zwx```, ```wyxz```.

##### ```vec::Size```
```c++
static constexpr std::size_t Size;
```
Since we instantiate partial specializations of ```vec```, we manually set this value for each of the partial specializations. The value must satisfy [```dimensional_size```](#dimensional_size). For the partial specializations, there is no template parameter Size available to use within the class/struct, and this static data member simulates such a template parameter.

##### ```vec::Count```
```c++
static constexpr std::size_t Count = Size;
```
The static variable that holds the number of items that we are handling. It is always the same value as the template parameter ```Size```. It is unnecessary for this class, but it is used here to be in solidarity with ```swizzle_vec```, where ```Size``` and ```Count``` are often not the same value.

##### ```vec::Writable```
```c++
static constexpr bool Writable = true;
```
This static variable is always true for all ```vec```s. It is unnecessary for this class, but it is used in here to be in solidarity with ```swizzle_vec```, where Writable is not always true (mostly false actually).

##### ```vec::offsets```
```c++
static constexpr std::array<std::size_t, Count> offsets = make_sequence_array(sequence_pack{});
```
The static ```std::array``` for how the physical representation is mapped to the logical representation. For ```vec```, the physical and logical representations are the same, i.e., a contiguous representation, and the sequence ascends from 0.

##### ```vec::operator []```
```c++
template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr T &operator [](const U &index) noexcept requires Writable;

template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr const T &operator [](const U &index) const noexcept;
```
Data access through the indexing operator. The indexing operator already takes the physical to logical data mapping into account. For ```vec```, the elements are contiguous, so the physical and logical mapping is the same.

##### ```vec::operator =``` (Assignment)
```c++
template <bool W, dimensional_scalar U, typename D>
requires Writable && implicitly_convertible_to<U, T>
constexpr vec &operator =(const vec_interface<W, U, Count, D> &other) & noexcept;
```
The assignment operator. It can be assigned from objects that inherit from ```vec_interface```. Can assign only to lvalues.

##### ```vec::sequence```
```c++
[[nodiscard]] static constexpr auto sequence() noexcept;
```
The ```std::index_sequence``` for how the physical representation is mapped to the logical representation. For ```vec```, the physical and logical representation are the same, i.e., a contiguous representation, and the sequence ascends from 0.

##### ```vec::set```
```c++
template <typename ...Args>
requires Writable && (sizeof...(Args) == Count) && (std::convertible_to<Args, T> && ...)
constexpr void set(Args ...args) noexcept;
```
Set all the vector element values via parameter copies. This prevents aliasing issues with references.

##### ```vec::swap```
```c++
constexpr void swap(vec &bv) noexcept requires Writable;
```
Swap the data using the underlying ```dimensional_storage_t```'s ```swap``` function.

##### ```vec``` Iterators
```c++
[[nodiscard]] constexpr auto begin() noexcept requires Writable;
[[nodiscard]] constexpr auto begin() const noexcept;
[[nodiscard]] constexpr auto cbegin() const noexcept;
[[nodiscard]] constexpr auto end() noexcept requires Writable;
[[nodiscard]] constexpr auto end() const noexcept;
[[nodiscard]] constexpr auto cend() const noexcept;

[[nodiscard]] constexpr auto rbegin() noexcept requires Writable;
[[nodiscard]] constexpr auto rbegin() const noexcept;
[[nodiscard]] constexpr auto crbegin() const noexcept;
[[nodiscard]] constexpr auto rend() noexcept requires Writable;
[[nodiscard]] constexpr auto rend() const noexcept;
[[nodiscard]] constexpr auto crend() const noexcept;
```
These contiguous iterators are supplied indirectly by the underlying ```dimensional_storage_t```. Each data member of the anonymous union, whether a ```vec_storage``` or an ```swizzle_vec```, has a single data member of type ```dimensional_storage_t```. The iterators are accessed through ```base```.

##### ```vec::sequence_pack```
```c++
using sequence_pack = std::make_index_sequence<Count>;
```
The instantiation of ```std::index_sequence``` that represents the physical to logical mapping. ```sequence``` returns an instance of this type.

##### ```vec``` Free Functions
```c++
template <dimensional_scalar T, std::size_t Size>
constexpr void swap(vec<T, Size> &lhs, vec<T, Size> &rhs) noexcept;
```
Free function ```swap``` wraps the member function ```vec::swap```.

##### ```vec``` CTAD
```c++
template <dimensional_scalar T, dimensional_scalar ...U>
vec(T, U...) -> vec<T, 1 + sizeof...(U)>;

template <bool W, dimensional_scalar T, std::size_t C, typename D>
vec(const vec_interface<W, T, C, D> &) -> vec<T, C>;
```
[Class template argument deduction (CTAD)](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction) for ```vec```.

#### ```mat```
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4)))
struct mat;
```
The struct that represents a matrix. The matrix elements are stored column order, as an array of column [```vec```](#vec)s. The terminology looks backwards, with number of columns coming before the number of rows, but it makes sense from a data storage perspective. It is also how GLSL does it.

* Template Parameters
  * ```T``` - type of the ```mat``` elements. Must satisfy [```floating_point_scalar```](#floating_point_scalar).
  * ```C``` - number of columns, ```where 2 <= C <= 4```.
  * ```R``` - number of rows, ```where 2 <= R <= 4```.
* [```mat``` Constructors](#mat-constructors)
* Non-Static Data Members
  * [```columns```](#matcolumns)
* Static Data Members
  * [```ComponentCount```](#matcomponentcount)
  * [```size``` (```std::integral_constant```)](#matsize-stdintegral_constant)
  * [```column_size``` (```std::integral_constant```)](#matcolumn_size-stdintegral_constant)
* Static Member Functions
  * [```size``` (theoretical function)](#matsize-theoretical-function)
  * [```column_size``` (theoretical function)](#matcolumn_size-theoretical-function)
* Non-Static Member Functions
  * [```length```](#matlength)
  * [```column_length```](#matcolumn_length)
  * [```operator []```](#matoperator-)
  * [```operator =```](#matoperator--assignment)
  * [```row```](#matrow)
  * [```swap```](#matswap)
  * [Iterators](#mat-iterators)
* [Matrix Operators](#matrix-operators)
* [Matrix Free Functions](#matrix-free-functions)
* [```mat``` Free Functions](#mat-free-functions)

##### ```mat``` Constructors
```c++
constexpr mat() noexcept = default;
constexpr mat(const mat &) noexcept = default;
constexpr mat(mat &&) noexcept = default;
```
Default constructors.

```c++
template <typename U, typename ... Args>
requires (detail::valid_matrix_component<U, T>::value) && (detail::valid_matrix_component<Args, T>::value && ...) && detail::met_component_count<ComponentCount, U, Args...>
explicit constexpr mat(const U &u, const Args & ...args) noexcept;
```
Variadic constructor. Can take a combination of vectors and scalars as arguments to initialize the ```mat```.

```c++
template <typename U>
requires std::convertible_to<U, T> && (C == R)
explicit constexpr mat(U arg) noexcept;
```
Diagonal constructor for square matrices.

```c++
template <floating_point_scalar U>
requires implicitly_convertible_to<U, T>
explicit(false) constexpr mat(const mat<U, C, R> &arg) noexcept;

template <floating_point_scalar U, std::size_t Cols, std::size_t Rows>
requires implicitly_convertible_to<U, T> && (Cols != C || Rows != R)
explicit(false) constexpr mat(const mat<U, Cols, Rows> &arg) noexcept;

template <floating_point_scalar U, std::size_t Cols, std::size_t Rows>
requires (!implicitly_convertible_to<U, T> && std::convertible_to<U, T>)
explicit constexpr mat(const mat<U, Cols, Rows> &arg) noexcept;
```
Constructors that take a matrix as an argument, but not a normal copy constructor (see default constructors).

```c++
constexpr mat(const std::initializer_list<T> &init_list) noexcept;
```
An intializer list of values. If too few values for the matrix, the rest of the elements will be set to 0. If too many values for the matrix, the rest of the initialization list will be ignored.

##### ```mat::columns```
```c++
std::array<vec<T, R>, C> columns;
```
The matrix elements' storage.

##### ```mat::ComponentCount```
```c++
static constexpr std::size_t ComponentCount = C * R;
```
The number of matrix elements.

##### ```mat::size``` (```std::integral_constant```)
```c++
static constexpr std::integral_constant<std::size_t, C> size = {};
```
Holds the number of columns, which is the row size.

##### ```mat::column_size``` (```std::integral_constant```)
```c++
static constexpr std::integral_constant<std::size_t, R> column_size = {};
```
Holds the number of rows, which is the column size.

##### ```mat::size``` (theoretical function)
```c++
[[nodiscard]] static constexpr std::size_t size() const noexcept;
```
Return the number of columns, which is the row size. The declaration for ```size()``` is a fiction due to the fact that this function does not exist; however, the static ```std::integral_constant``` ```size``` has an ```operator()()``` that operates exactly as the above declaration. This approach of using a ```std::integral_constant``` for ```size``` is supposed to be an up and coming idiom in the C++ standard for all new standard library components with constant sizes.

##### ```mat::column_size``` (theoretical function)
```c++
[[nodiscard]] constexpr std::size_t column_size() const noexcept;
```
Return the number of rows, which is the column size. The declaration for ```column_size()``` is a fiction due to the fact that this function does not exist; however, the static ```std::integral_constant``` ```column_size``` has an ```operator()()``` that operates exactly as the above declaration. This approach of using a ```std::integral_constant``` for ```size``` (and here also ```column_size```) is supposed to be an up and coming idiom in the C++ standard for all new standard library components with constant sizes.

##### ```mat::length```
```c++
[[nodiscard]] constexpr int length() const noexcept;
```
Return the number of columns. This is also the row length.

The GLSL specification requires that the ```length method``` behave as a member function.

##### ```mat::column_length```
```c++
[[nodiscard]] constexpr int column_length() const noexcept;
```
Return the number of rows.

##### ```mat::operator []```
```c++
template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr vec<T, R> &operator [](const U &index) noexcept;

template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr const vec<T, R> &operator [](const U &index) const noexcept;
```
Data access through the indexing operator.

##### ```mat::operator =``` (Assignment)
```c++
template <floating_point_scalar U>
requires implicitly_convertible_to<U, T>
constexpr mat &operator =(const mat<U, C, R> &other) & noexcept;
```
The assignment operator. Can assign only to lvalues.

##### ```mat::row```
```c++
template <typename U>
requires std::convertible_to<U, std::size_t>
[[nodiscard]] constexpr vec<T, C> row(const U &row_index) const noexcept;
```
Return a row from the column-order matrix.

##### ```mat::swap```
```c++
constexpr void swap(mat &bm) noexcept;
```
Swap the data using the underlying ```std::array```'s ```swap``` function.

##### ```mat``` Iterators
```c++
[[nodiscard]] constexpr auto begin() noexcept;
[[nodiscard]] constexpr auto begin() const noexcept;
[[nodiscard]] constexpr auto cbegin() const noexcept;
[[nodiscard]] constexpr auto end() noexcept;
[[nodiscard]] constexpr auto end() const noexcept;
[[nodiscard]] constexpr auto cend() const noexcept;

[[nodiscard]] constexpr auto rbegin() noexcept;
[[nodiscard]] constexpr auto rbegin() const noexcept;
[[nodiscard]] constexpr auto crbegin() const noexcept;
[[nodiscard]] constexpr auto rend() noexcept;
[[nodiscard]] constexpr auto rend() const noexcept;
[[nodiscard]] constexpr auto crend() const noexcept;
```
These contiguous ```std::array``` iterators are accessed through ```columns```.

##### ```mat``` Free Functions
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
constexpr void swap(mat<T, C, R> &lhs, mat<T, C, R> &rhs) noexcept;
```
Free function ```swap``` wraps the member function ```mat::swap```.

### Class Template Instantiations
These instantiations represent the classes (structs) that are provided by GLSL, with a few others not in GLSL.

```c++
// boolean vectors
using bscal = vec<bool, 1>;
using bvec2 = vec<bool, 2>;
using bvec3 = vec<bool, 3>;
using bvec4 = vec<bool, 4>;

// int vectors
using iscal = vec<int, 1>;
using ivec2 = vec<int, 2>;
using ivec3 = vec<int, 3>;
using ivec4 = vec<int, 4>;

// unsigned int vectors
using uscal = vec<unsigned, 1>;
using uvec2 = vec<unsigned, 2>;
using uvec3 = vec<unsigned, 3>;
using uvec4 = vec<unsigned, 4>;

// long long vectors (not in GLSL)
using llscal = vec<long long, 1>;
using llvec2 = vec<long long, 2>;
using llvec3 = vec<long long, 3>;
using llvec4 = vec<long long, 4>;

// unsigned long long vectors (not in GLSL)
using ullscal = vec<unsigned long long, 1>;
using ullvec2 = vec<unsigned long long, 2>;
using ullvec3 = vec<unsigned long long, 3>;
using ullvec4 = vec<unsigned long long, 4>;

// float vectors with out an 'f' prefix -- this is from GLSL
using scal = vec<float, 1>;
using vec2 = vec<float, 2>;
using vec3 = vec<float, 3>;
using vec4 = vec<float, 4>;

// also float vectors, but using the common naming convention (not in GLSL)
using fscal = vec<float, 1>;
using fvec2 = vec<float, 2>;
using fvec3 = vec<float, 3>;
using fvec4 = vec<float, 4>;

// double vectors
using dscal = vec<double, 1>;
using dvec2 = vec<double, 2>;
using dvec3 = vec<double, 3>;
using dvec4 = vec<double, 4>;

// float matrices
using mat2x2 = mat<float, 2, 2>;
using mat2x3 = mat<float, 2, 3>;
using mat2x4 = mat<float, 2, 4>;
using mat3x2 = mat<float, 3, 2>;
using mat3x3 = mat<float, 3, 3>;
using mat3x4 = mat<float, 3, 4>;
using mat4x2 = mat<float, 4, 2>;
using mat4x3 = mat<float, 4, 3>;
using mat4x4 = mat<float, 4, 4>;

using mat2 = mat<float, 2, 2>;
using mat3 = mat<float, 3, 3>;
using mat4 = mat<float, 4, 4>;

// double matrices
using dmat2x2 = mat<double, 2, 2>;
using dmat2x3 = mat<double, 2, 3>;
using dmat2x4 = mat<double, 2, 4>;
using dmat3x2 = mat<double, 3, 2>;
using dmat3x3 = mat<double, 3, 3>;
using dmat3x4 = mat<double, 3, 4>;
using dmat4x2 = mat<double, 4, 2>;
using dmat4x3 = mat<double, 4, 3>;
using dmat4x4 = mat<double, 4, 4>;

using dmat2 = mat<double, 2, 2>;
using dmat3 = mat<double, 3, 3>;
using dmat4 = mat<double, 4, 4>;
```

### Vector Operators

* Vector Unary Operators
  * [```operator +```](#vector-unary-plus)
  * [```operator -```](#vector-unary-minus)
  * [```operator ++```](#vector-unary-increment)
  * [```operator --```](#vector-unary-decrement)
  * [```operator ~```](#vector-unary-bit-wise-ones-complement)
* Vector Binary Operators
  * [```operator +```](#vector-binary-plus)
  * [```operator -```](#vector-binary-minus)
  * [```operator *```](#vector-binary-times)
  * [```operator /```](#vector-binary-division)
  * [```operator %```](#vector-binary-modulus)
  * [```operator >>```](#vector-binary-right-shift)
  * [```operator <<```](#vector-binary-left-shift)
  * [```operator &```](#vector-binary-bitwise-and)
  * [```operator |```](#vector-binary-bitwise-or)
  * [```operator ^```](#vector-binary-bitwise-xor)
* Vector Compound Assignment Operators
  * [```operator +=```](#vector-plus-assignment)
  * [```operator -=```](#vector-minus-assignment)
  * [```operator *=```](#vector-times-assignment)
  * [```operator /=```](#vector-division-assignment)
  * [```operator %=```](#vector-modulus-assignment)
  * [```operator >>=```](#vector-right-shift-assignment)
  * [```operator <<=```](#vector-left-shift-assignment)
  * [```operator &=```](#vector-bitwise-and-assignment)
  * [```operator |=```](#vector-bitwise-or-assignment)
  * [```operator ^=```](#vector-bitwise-xor-assignment)
* Vector Comparison Operators
  * [```operator ==```](#vector-equals)
  * [```operator !=```](#vector-not-equals)

#### Vector Unary Operators

##### Vector Unary Plus
```c++
template <bool W, non_bool_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto operator +(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### Vector Unary Minus
```c++
template <bool W, non_bool_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto operator -(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### Vector Unary Increment
```c++
template <bool W, non_bool_scalar T, std::size_t C, typename D>
requires W
constexpr auto &operator ++(vec_interface<W, T, C, D> &arg) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D>
requires W
constexpr auto operator ++(vec_interface<W, T, C, D> &arg, int) noexcept;
```

##### Vector Unary Decrement
```c++
template <bool W, non_bool_scalar T, std::size_t C, typename D>
requires W
constexpr auto &operator --(vec_interface<W, T, C, D> &arg) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D>
requires W
constexpr auto operator --(vec_interface<W, T, C, D> &arg, int) noexcept;
```

##### Vector Unary Bit-wise One's Complement
```c++
template <bool W, numeric_integral_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto operator ~(const vec_interface<W, T, C, D> &arg) noexcept;
```

#### Vector Binary Operators

##### Vector Binary Plus
```c++
template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator +(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator +(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator +(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Minus
```c++
template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator -(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator -(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator -(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Times
```c++
template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator *(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator *(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator *(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Division
```c++
template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator /(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator /(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator /(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Modulus
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator %(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator %(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator %(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Right Shift
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator >>(const vec_interface<W1, T1, C1, D1> &lhs,
                                         const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator >>(const vec_interface<W, T, C, D> &lhs,
                                         U rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator >>(U lhs,
                                         const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Left Shift
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1)
[[nodiscard]] constexpr auto operator <<(const vec_interface<W1, T1, C1, D1> &lhs,
                                         const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator <<(const vec_interface<W, T, C, D> &lhs,
                                         U rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>
[[nodiscard]] constexpr auto operator <<(U lhs,
                                         const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Bitwise And
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1) && detail::same_sizeof<T1, T2>
[[nodiscard]] constexpr auto operator &(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
[[nodiscard]] constexpr auto operator &(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
[[nodiscard]] constexpr auto operator &(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Bitwise Or
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1) && detail::same_sizeof<T1, T2>
[[nodiscard]] constexpr auto operator |(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
[[nodiscard]] constexpr auto operator |(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
[[nodiscard]] constexpr auto operator |(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

##### Vector Binary Bitwise Xor
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C1, typename D1, bool W2, numeric_integral_scalar T2, std::size_t C2, typename D2>
requires (implicitly_convertible_to<T2, T1> || implicitly_convertible_to<T1, T2>) && (C1 == C2 || C1 == 1 || C2 == 1) && detail::same_sizeof<T1, T2>
[[nodiscard]] constexpr auto operator ^(const vec_interface<W1, T1, C1, D1> &lhs,
                                        const vec_interface<W2, T2, C2, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
[[nodiscard]] constexpr auto operator ^(const vec_interface<W, T, C, D> &lhs,
                                        U rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires (implicitly_convertible_to<U, T> || implicitly_convertible_to<T, U>) && detail::same_sizeof<T, U>
[[nodiscard]] constexpr auto operator ^(U lhs,
                                        const vec_interface<W, T, C, D> &rhs) noexcept;
```

#### Vector Compound Assignment Operators

##### Vector Plus Assignment
```c++
template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator +=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator +=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator +=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Minus Assignment
```c++
template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator -=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator -=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator -=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Times Assignment
```c++
template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator *=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator *=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator *=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Division Assignment
```c++
template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator /=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator /=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator /=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Modulus Assignment
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator %=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator %=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator %=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Right Shift Assignment
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator >>=(vec_interface<W1, T1, C, D1> &lhs,
                             const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator >>=(vec_interface<W1, T1, C, D1> &lhs,
                             const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator >>=(vec_interface<W, T, C, D> &lhs,
                             U rhs) noexcept;
```

##### Vector Left Shift Assignment
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1>
constexpr auto &operator <<=(vec_interface<W1, T1, C, D1> &lhs,
                             const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1)
constexpr auto &operator <<=(vec_interface<W1, T1, C, D1> &lhs,
                             const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires W && implicitly_convertible_to<U, T>
constexpr auto &operator <<=(vec_interface<W, T, C, D> &lhs,
                             U rhs) noexcept;
```

##### Vector Bitwise And Assignment
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && detail::same_sizeof<T1, T2>
constexpr auto &operator &=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1) && detail::same_sizeof<T1, T2>
constexpr auto &operator &=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires W && implicitly_convertible_to<U, T> && detail::same_sizeof<T, U>
constexpr auto &operator &=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Bitwise Or Assignment
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && detail::same_sizeof<T1, T2>
constexpr auto &operator |=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1) && detail::same_sizeof<T1, T2>
constexpr auto &operator |=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires W && implicitly_convertible_to<U, T> && detail::same_sizeof<T, U>
constexpr auto &operator |=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

##### Vector Bitwise Xor Assignment
```c++
template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && detail::same_sizeof<T1, T2>
constexpr auto &operator ^=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, C, D2> &rhs) noexcept;

template <bool W1, numeric_integral_scalar T1, std::size_t C, typename D1, bool W2, numeric_integral_scalar T2, typename D2>
requires W1 && implicitly_convertible_to<T2, T1> && (C > 1) && detail::same_sizeof<T1, T2>
constexpr auto &operator ^=(vec_interface<W1, T1, C, D1> &lhs,
                            const vec_interface<W2, T2, 1, D2> &rhs) noexcept;

template <bool W, numeric_integral_scalar T, std::size_t C, typename D, numeric_integral_scalar U>
requires W && implicitly_convertible_to<U, T> && detail::same_sizeof<T, U>
constexpr auto &operator ^=(vec_interface<W, T, C, D> &lhs,
                            U rhs) noexcept;
```

#### Vector Comparison Operators

##### Vector Equals
```c++
template <bool W1, dimensional_scalar T1, std::size_t C, typename D1, bool W2, dimensional_scalar T2, typename D2>
requires implicitly_convertible_to<T2, T1>
constexpr bool operator ==(const vec_interface<W1, T1, C, D1> &first,
                           const vec_interface<W2, T2, C, D2> &second) noexcept;

template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
constexpr bool operator ==(const vec_interface<W1, T, C, D1> &first,
                           const vec_interface<W2, T, C, D2> &second) noexcept;
```
If a signature of ```operator ==``` is not matched, ```c++20``` will swap the argument order to see if there is a match.

##### Vector Not Equals
```c++20``` automatically creates ```operator !=``` from [```operator ==```](#vector-equals).

### Vector Free Functions
We have two different vector types, ```vec``` and ```swizzle_vec```. We want the vector functions take work for both types, so most of the vector functions take instances of the vector types' common base class, ```vec_interface```.

Most of the functions perform their operation component-wise. There are some functions that treat the vector geometrically and treat the components as part of a whole.

* Angle and Trigonometry Functions
  * [```radians```](#radians)
  * [```degrees```](#degrees)
  * [```sin```](#sin)
  * [```cos```](#cos)
  * [```tan```](#tan)
  * [```asin```](#asin)
  * [```acos```](#acos)
  * [```atan```](#atan)
  * [```sinh```](#sinh)
  * [```cosh```](#cosh)
  * [```tanh```](#tanh)
  * [```asinh```](#asinh)
  * [```acosh```](#acosh)
  * [```atanh```](#atanh)
* Exponential Functions
  * [```pow```](#pow)
  * [```exp```](#exp)
  * [```log```](#log)
  * [```exp2```](#exp2)
  * [```log2```](#log2)
  * [```sqrt```](#sqrt)
  * [```inversesqrt```](#inversesqrt)
  * [```fast_inversesqrt```](#fast_inversesqrt)
* Common Functions
  * [```abs```](#abs)
  * [```sign```](#sign)
  * [```floor```](#floor)
  * [```trunc```](#trunc)
  * [```round```](#round)
  * [```roundEven```](#roundeven)
  * [```ceil```](#ceil)
  * [```fract```](#fract)
  * [```mod```](#mod)
  * [```modf```](#modf)
  * [```min```](#min)
  * [```max```](#max)
  * [```clamp```](#clamp)
  * [```mix```](#mix)
  * [```step```](#step)
  * [```smoothstep```](#smoothstep)
  * [```isnan```](#isnan)
  * [```isinf```](#isinf)
  * [```floatBitsToInt```](#floatbitstoint)
  * [```floatBitsToUint```](#floatbitstouint)
  * [```doubleBitsToLongLong```](#doublebitstolonglong)
  * [```doubleBitsToUlongLong```](#doublebitstoulonglong)
  * [```intBitsToFloat```](#intbitstofloat)
  * [```uintBitsToFloat```](#uintbitstofloat)
  * [```longLongBitsToDouble```](#longlongbitstodouble)
  * [```ulongLongBitsToDouble```](#ulonglongbitstodouble)
  * [```fma```](#fma)
  * [```frexp```](#frexp)
  * [```ldexp```](#ldexp)
  * [```byteswap```](#byteswap)
* Geometric Functions
  * [```length```](#length)
  * [```distance```](#distance)
  * [```innerProduct```](#innerproduct)
  * [```dot```](#dot)
  * [```cross```](#cross)
  * [```normalize```](#normalize)
  * [```faceforward```](#faceforward)
  * [```reflect```](#reflect)
  * [```refract```](#refract)
* Vector Relational Functions
  * [```lessThan```](#lessthan)
  * [```lessThanEqual```](#lessthanequal)
  * [```greaterThan```](#greaterthan)
  * [```greaterThanEqual```](#greaterthanequal)
  * [```equal```](#equal)
  * [```notEqual```](#notequal)
  * [```any```](#any)
  * [```all```](#all)
  * [```none```](#none)
  * [```compNot```](#compnot)
  * [```compAnd```](#compand)
  * [```compOr```](#compor)
* Other Vector Functions
  * [```swizzle```](#swizzle)

#### Angle and Trigonometry Functions

##### ```radians```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto radians(const vec_interface<W, T, C, D> &deg) noexcept;
```

##### ```degrees```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto degrees(const vec_interface<W, T, C, D> &rad) noexcept;
```

##### ```sin```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto sin(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```cos```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto cos(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```tan```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto tan(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```asin```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto asin(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```acos```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto acos(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```atan```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto atan(const vec_interface<W, T, C, D> &arg) noexcept;

template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] inline auto atan(const vec_interface<W1, T, C, D1> &y,
                               const vec_interface<W2, T, C, D2> &x) noexcept;
```

##### ```sinh```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto sinh(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```cosh```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto cosh(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```tanh```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto tanh(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```asinh```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto asinh(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```acosh```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto acosh(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```atanh```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto atanh(const vec_interface<W, T, C, D> &arg) noexcept;
```

#### Exponential Functions

##### ```pow```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] inline auto pow(const vec_interface<W1, T, C, D1> &base,
                              const vec_interface<W2, T, C, D2> &exp) noexcept;
```

##### ```exp```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto exp(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```log```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto log(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```exp2```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto exp2(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```log2```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] inline auto log2(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```sqrt```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto sqrt(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```inversesqrt```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto inversesqrt(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```fast_inversesqrt```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto fast_inversesqrt(const vec_interface<W, T, C, D> &arg) noexcept;
```
Not in GLSL. May or may not actually be faster that ```rsqrt()```, for which this function is an approximation for. For ```float```, this function is 100% in agreement with ```rsqrt()```. For ```double```, the relationship to ```rsqrt()``` is:
  * 0 ulps: ~68.58%
  * 1 ulps: ~31.00%
  * 2 ulps:  ~0.42%

#### Common Functions

##### ```abs```
```c++
template <bool W, non_bool_scalar T, std::size_t C, typename D>
requires (!unsigned_scalar<T>)
[[nodiscard]] constexpr auto abs(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```sign```
```c++
template <bool W, non_bool_scalar T, std::size_t C, typename D>
requires (!unsigned_scalar<T>)
[[nodiscard]] constexpr auto sign(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```floor```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto floor(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```trunc```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto trunc(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```round```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto round(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```roundEven```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto roundEven(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```ceil```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto ceil(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```fract```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto fract(const vec_interface<W, T, C, D> &arg) noexcept;
```

##### ```mod```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto mod(const vec_interface<W1, T, C, D1> &x,
                                 const vec_interface<W2, T, C, D2> &y) noexcept;

template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto mod(const vec_interface<W, T, C, D> &x,
                                 T y) noexcept;
```

##### ```modf```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
requires W2
[[nodiscard]] constexpr auto modf(const vec_interface<W1, T, C, D1> &arg,
                                  vec_interface<W2, T, C, D2> &i) noexcept;
```

##### ```min```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto min(const vec_interface<W1, T, C, D1> &x,
                                 const vec_interface<W2, T, C, D2> &y) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto min(const vec_interface<W, T, C, D> &x,
                                 T y) noexcept;
```

##### ```max```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto max(const vec_interface<W1, T, C, D1> &x,
                                 const vec_interface<W2, T, C, D2> &y) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto max(const vec_interface<W, T, C, D> &x,
                                 T y) noexcept;
```

##### ```clamp```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
[[nodiscard]] constexpr auto clamp(const vec_interface<W1, T, C, D1> &x,
                                   const vec_interface<W2, T, C, D2> &min_val,
                                   const vec_interface<W3, T, C, D3> &max_val) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto clamp(const vec_interface<W, T, C, D> &x,
                                   T min_val,
                                   T max_val) noexcept;
```

##### ```mix```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
[[nodiscard]] constexpr auto mix(const vec_interface<W1, T, C, D1> &x,
                                 const vec_interface<W2, T, C, D2> &y,
                                 const vec_interface<W3, T, C, D3> &a) noexcept;

template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto mix(const vec_interface<W1, T, C, D1> &x,
                                 const vec_interface<W2, T, C, D2> &y,
                                 T a) noexcept;

template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, bool_scalar B, typename D3>
[[nodiscard]] constexpr auto mix(const vec_interface<W1, T, C, D1> &x,
                                 const vec_interface<W2, T, C, D2> &y,
                                 const vec_interface<W3, B, C, D3> &a) noexcept;
```

##### ```step```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto step(const vec_interface<W1, T, C, D1> &edge,
                                  const vec_interface<W2, T, C, D2> &x) noexcept;

template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto step(T edge,
                                  const vec_interface<W, T, C, D> &x) noexcept;
```

##### ```smoothstep```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
[[nodiscard]] constexpr auto smoothstep(const vec_interface<W1, T, C, D1> &edge0,
                                        const vec_interface<W2, T, C, D2> &edge1,
                                        const vec_interface<W3, T, C, D3> &x) noexcept;

template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto smoothstep(T edge0,
                                        T edge1,
                                        const vec_interface<W, T, C, D> &x) noexcept;
```

##### ```isnan```
```c++
template <floating_point_scalar T, std::size_t C>
[[nodiscard]] constexpr auto isnan(const vec<T, C> &arg) noexcept;

template <floating_point_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
[[nodiscard]] constexpr auto isnan(const swizzle_vec<T, S, C, Is...> &arg) noexcept;
```
These functions can not use ```vec_interface``` as the parameter type. They must be specialized for the actual types, not the base type. This is due to the C++ Standard Library implementation, at least for MSVC.

##### ```isinf```
```c++
template <floating_point_scalar T, std::size_t C>
[[nodiscard]] constexpr auto isinf(const vec<T, C> &arg) noexcept;

template <floating_point_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
[[nodiscard]] constexpr auto isinf(const swizzle_vec<T, S, C, Is...> &arg) noexcept;
```
These functions can not use ```vec_interface``` as the parameter type. They must be specialized for the actual types, not the base type. This is due to the C++ Standard Library implementation, at least for MSVC.

##### ```floatBitsToInt```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto floatBitsToInt(const vec_interface<W, float, C, D> &arg) noexcept;
```

##### ```floatBitsToUint```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto floatBitsToUint(const vec_interface<W, float, C, D> &arg) noexcept;
```

##### ```doubleBitsToLongLong```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto doubleBitsToLongLong(const vec_interface<W, double, C, D> &arg) noexcept;
```
Not in GLSL.

##### ```doubleBitsToUlongLong```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto doubleBitsToUlongLong(const vec_interface<W, double, C, D> &arg) noexcept;
```
Not in GLSL.

##### ```intBitsToFloat```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto intBitsToFloat(const vec_interface<W, int, C, D> &arg) noexcept;
```

##### ```uintBitsToFloat```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto uintBitsToFloat(const vec_interface<W, unsigned int, C, D> &arg) noexcept;
```

##### ```longLongBitsToDouble```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto longLongBitsToDouble(const vec_interface<W, long long, C, D> &arg) noexcept;
```
Not in GLSL.

##### ```ulongLongBitsToDouble```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto ulongLongBitsToDouble(const vec_interface<W, unsigned long long, C, D> &arg) noexcept;
```
Not in GLSL.

##### ```fma```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
[[nodiscard]] inline auto fma(const vec_interface<W1, T, C, D1> &a,
                              const vec_interface<W2, T, C, D2> &b,
                              const vec_interface<W3, T, C, D3> &c) noexcept;
```

##### ```frexp```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
requires W2
[[nodiscard]] inline auto frexp(const vec_interface<W1, T, C, D1> &x,
                                vec_interface<W2, int, C, D2> &exp) noexcept;
```

##### ```ldexp```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] inline auto ldexp(const vec_interface<W1, T, C, D1> &x,
                                const vec_interface<W2, int, C, D2> &exp) noexcept;
```

##### ```byteswap```
```c++
template <bool W, numeric_integral_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto byteswap(const vec_interface<W, T, C, D> &arg) noexcept;
```
Not in GLSL. This functionality was added to ```c++23```, but since this is a ```c++20``` library, we have to provide the underlying implementation ourselves.

#### Geometric Functions

##### ```length```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr auto length(const vec_interface<W, T, C, D> &x) noexcept;
```

##### ```distance```
```c++
template <bool W1, floating_point_scalar T1, std::size_t C, typename D1, bool W2, floating_point_scalar T2, typename D2>
[[nodiscard]] constexpr auto distance(const vec_interface<W1, T1, C, D1> &p0,
                                      const vec_interface<W2, T2, C, D2> &p1) noexcept;
```

##### ```innerProduct```
```c++
template <bool W1, non_bool_scalar T1, std::size_t C, typename D1, bool W2, non_bool_scalar T2, typename D2>
[[nodiscard]] constexpr auto innerProduct(const vec_interface<W1, T1, C, D1> &x,
                                          const vec_interface<W2, T2, C, D2> &y) noexcept;
```
Not in GLSL. This is just like ```dot```, except without the floating-point restriction.

##### ```dot```
```c++
template <bool W1, floating_point_scalar T1, std::size_t C, typename D1, bool W2, floating_point_scalar T2, typename D2>
[[nodiscard]] constexpr auto dot(const vec_interface<W1, T1, C, D1> &x,
                                 const vec_interface<W2, T2, C, D2> &y) noexcept;
```

##### ```cross```
```c++
template <bool W1, floating_point_scalar T1, typename D1, bool W2, floating_point_scalar T2, typename D2>
[[nodiscard]] constexpr auto cross(const vec_interface<W1, T1, 3, D1> &a,
                                   const vec_interface<W2, T2, 3, D2> &b) noexcept;

template <floating_point_scalar T1, floating_point_scalar T2>
[[nodiscard]] constexpr auto cross(const vec<T1, 3> &a,
                                   const vec<T2, 3> &b) noexcept;
```

##### ```normalize```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
requires (C > 1)
[[nodiscard]] constexpr auto normalize(const vec_interface<W, T, C, D> &x) noexcept;
```

##### ```faceforward```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, typename D3>
requires (C > 1)
[[nodiscard]] constexpr auto faceforward(const vec_interface<W1, T, C, D1> &n,
                                         const vec_interface<W2, T, C, D2> &i,
                                         const vec_interface<W3, T, C, D3> &nref) noexcept;
```

##### ```reflect```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
requires (C > 1)
[[nodiscard]] constexpr auto reflect(const vec_interface<W1, T, C, D1> &i,
                                     const vec_interface<W2, T, C, D2> &n) noexcept;
```

##### ```refract```
```c++
template <bool W1, floating_point_scalar T, std::size_t C, typename D1, bool W2, typename D2>
requires (C > 1)
[[nodiscard]] constexpr auto refract(const vec_interface<W1, T, C, D1> &i,
                                     const vec_interface<W2, T, C, D2> &n,
                                     T eta) noexcept;
```

#### Vector Relational Functions

##### ```lessThan```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto lessThan(const vec_interface<W1, T, C, D1> &x,
                                      const vec_interface<W2, T, C, D2> &y) noexcept;
```

##### ```lessThanEqual```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto lessThanEqual(const vec_interface<W1, T, C, D1> &x,
                                           const vec_interface<W2, T, C, D2> &y) noexcept;
```

##### ```greaterThan```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto greaterThan(const vec_interface<W1, T, C, D1> &x,
                                         const vec_interface<W2, T, C, D2> &y) noexcept;
```

##### ```greaterThanEqual```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto greaterThanEqual(const vec_interface<W1, T, C, D1> &x,
                                              const vec_interface<W2, T, C, D2> &y) noexcept;
```

##### ```equal```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto equal(const vec_interface<W1, T, C, D1> &x,
                                   const vec_interface<W2, T, C, D2> &y) noexcept;

template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto equal(const vec_interface<W1, bool, C, D1> &x,
                                   const vec_interface<W2, bool, C, D2> &y) noexcept;
```

##### ```notEqual```
```c++
template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto notEqual(const vec_interface<W1, T, C, D1> &x,
                                      const vec_interface<W2, T, C, D2> &y) noexcept;

template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto notEqual(const vec_interface<W1, bool, C, D1> &x,
                                      const vec_interface<W2, bool, C, D2> &y) noexcept;

```

##### ```any```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr bool any(const vec_interface<W, bool, C, D> &x) noexcept;
```
Not a component-wise operation. Relies on all values.

##### ```all```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr bool all(const vec_interface<W, bool, C, D> &x) noexcept;
```
Not a component-wise operation. Relies on all values.

##### ```none```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr bool none(const vec_interface<W, bool, C, D> &x) noexcept;
```
Not in GLSL. Same effect as ```!any(vec)```.

Not a component-wise operation. Relies on all values.

##### ```compNot```
```c++
template <bool W, std::size_t C, typename D>
[[nodiscard]] constexpr auto compNot(const vec_interface<W, bool, C, D> &x) noexcept;
```
This function takes the place of GLSL function ```not```. We can't define a function named ```not``` in C++ because it is a reserved keyword. This performs a component-wise ```not`` operation on the boolean inputs.

##### ```compAnd```
```c++
template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto compAnd(const vec_interface<W1, bool, C, D1> &x,
                                     const vec_interface<W2, bool, C, D2> &y) noexcept;
```
Not in GLSL. The function returns a vector from performing component-wise ```and``` operations of the boolean inputs.

##### ```compOr```
```c++
template <bool W1, std::size_t C, typename D1, bool W2, typename D2>
[[nodiscard]] constexpr auto compOr(const vec_interface<W1, bool, C, D1> &x,
                                    const vec_interface<W2, bool, C, D2> &y) noexcept;
```
Not in GLSL. The function returns a vector from performing component-wise ```or``` operations of the boolean inputs.

##### ```swizzle```
```c++
template <bool W, dimensional_scalar T, std::size_t C, typename D, typename Arg>
requires std::convertible_to<Arg, std::size_t>
inline auto swizzle(const vec_interface<W, T, C, D> &v, const Arg &index);

template <bool W, dimensional_scalar T, std::size_t C, typename D, typename ...Args>
requires (std::convertible_to<Args, std::size_t> && ...) && (sizeof...(Args) > 0) && (sizeof...(Args) <= 4)
inline vec<T, sizeof...(Args)> swizzle(const vec_interface<W, T, C, D> &v, const Args &...Is);
```
Not in GLSL. Runtime function for swizzling. Returns a stand-alone ```dsga::vec``` version of a swizzle, instead of a ```dsga::swizzle_vec``` data member. Will return a scalar value if only one index argument. If the index arguments are invalid (out of bounds), this function will throw a ```std::out_of_range()``` exception. Inspired by the [Odin Programming Language](https://odin-lang.org/docs/overview/#swizzle-operations).

### Scalar Functions
Scalar versions of most of the vector free functions exist. It is not recommended to use them if there is a function in the C++ Standard Library that does the same thing.

### Matrix Operators
The matrix unary and binary operators (though not the linear algebraic operators) operate on the matrices component-wise.

The linear algebraic binary operators use linear algebraic concepts to multiply matrices and vectors.

* Matrix Unary Operators
  * [```operator +```](#matrix-unary-plus)
  * [```operator -```](#matrix-unary-minus)
  * [```operator ++```](#matrix-unary-increment)
  * [```operator --```](#matrix-unary-decrement)
* Matrix Binary Operators
  * [```operator +```](#matrix-binary-plus)
  * [```operator -```](#matrix-binary-minus)
  * [```operator *```](#matrix-binary-times)
  * [```operator /```](#matrix-binary-division)
    * Matrix Linear Algebraic Operations
      * [Vector * Matrix](#matrix-linear-algebraic-vector-times-matrix)
      * [Matrix * Vector](#matrix-linear-algebraic-matrix-times-vector)
      * [Matrix * Matrix](#matrix-linear-algebraic-matrix-times-matrix)
* Matrix Comparison Operators
  * [```operator ==```](#matrix-equals)
  * [```operator !=```](#matrix-not-equals)

#### Matrix Unary Operators

##### Matrix Unary Plus
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
[[nodiscard]] constexpr auto operator +(const mat<T, C, R> &arg) noexcept;
```

##### Matrix Unary Minus
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
[[nodiscard]] constexpr auto operator -(const mat<T, C, R> &arg) noexcept;
```

##### Matrix Unary Increment
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
constexpr auto &operator ++(mat<T, C, R> &arg) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R>
constexpr auto operator ++(mat<T, C, R> &arg, int) noexcept;
```

##### Matrix Unary Decrement
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
constexpr auto &operator --(mat<T, C, R> &arg) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R>
constexpr auto operator --(mat<T, C, R> &arg, int) noexcept;
```

#### Matrix Binary Operators

##### Matrix Binary Plus
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator +(const mat<T, C, R> &lhs,
                                        U rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator +(U lhs,
                                        const mat<T, C, R> &rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
[[nodiscard]] constexpr auto operator +(const mat<T, C, R> &lhs,
                                        const mat<U, C, R> &rhs) noexcept
```

##### Matrix Binary Minus
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator -(const mat<T, C, R> &lhs,
                                        U rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator -(U lhs,
                                        const mat<T, C, R> &rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
[[nodiscard]] constexpr auto operator -(const mat<T, C, R> &lhs,
                                        const mat<U, C, R> &rhs) noexcept;
```

##### Matrix Binary Times
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator *(const mat<T, C, R> &lhs,
                                        U rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator *(U lhs,
                                        const mat<T, C, R> &rhs) noexcept;
```

##### Matrix Binary Division
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator /(const mat<T, C, R> &lhs,
                                        U rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, non_bool_scalar U>
[[nodiscard]] constexpr auto operator /(U lhs,
                                        const mat<T, C, R> &rhs) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
[[nodiscard]] constexpr auto operator /(const mat<T, C, R> &lhs,
                                        const mat<U, C, R> &rhs) noexcept;
```

#### Matrix Linear Algebraic Operations

##### Matrix Linear Algebraic Vector Times Matrix
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, bool W, non_bool_scalar U, typename D>
[[nodiscard]] constexpr auto operator *(const vec_interface<W, U, R, D> &lhs,
                                        const mat<T, C, R> &rhs) noexcept;
```
For performing a ```vector * matrix``` operation, the vector is treated as if it were transposed, i.e., a row vector, as is the result.

##### Matrix Linear Algebraic Matrix Times Vector
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, bool W, non_bool_scalar U, typename D>
[[nodiscard]] constexpr auto operator *(const mat<T, C, R> &lhs,
                                        const vec_interface<W, U, C, D> &rhs) noexcept;
```
For performing a ```matrix * vector``` operation, the vector is treated as if it were a column vector, as is the result.

##### Matrix Linear Algebraic Matrix Times Matrix
```c++
template <floating_point_scalar T, std::size_t C1, std::size_t R1, floating_point_scalar U, std::size_t C2, std::size_t R2>
requires (C1 == R2)
[[nodiscard]] constexpr auto operator *(const mat<T, C1, R1> &lhs,
                                        const mat<U, C2, R2> &rhs) noexcept;
```

#### Matrix Comparison Operators

##### Matrix Equals
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
requires implicitly_convertible_to<U, T>
constexpr bool operator ==(const mat<T, C, R> &lhs,
                           const mat<U, C, R> &rhs) noexcept;
```
If a signature of ```operator ==``` is not matched, ```c++20``` will swap the argument order to see if there is a match.

##### Matrix Not Equals
```c++20``` automatically creates ```operator !=``` from [```operator ==```](#matrix-equals).

### Matrix Free Functions
The matrix functions treat a matrix as an entity instead of as a collection of components, except for ```matrixCompMult```, which works component-wise.
* [```matrixCompMult```](#matrixcompmult)
* [```outerProduct```](#outerproduct)
* [```transpose```](#transpose)
* [```determinant```](#determinant)
* [```inverse```](#inverse)
* [```cross_matrix```](#cross_matrix)
* [```diagonal_matrix```](#diagonal_matrix)

##### ```matrixCompMult```
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R, floating_point_scalar U>
[[nodiscard]] constexpr auto matrixCompMult(const mat<T, C, R> &lhs,
                                            const mat<U, C, R> &rhs) noexcept;
```
This function exists because ```operator *``` is used for linear algebraic purposes instead of component-wise multiplication.

##### ```outerProduct```
```c++
template <bool W1, non_bool_scalar T1, std::size_t C1, typename D1, bool W2, non_bool_scalar T2, std::size_t C2, typename D2>
requires (floating_point_scalar<T1> || floating_point_scalar<T2>) && ((C1 >= 2) && (C1 <= 4)) && ((C2 >= 2) && (C2 <= 4))
[[nodiscard]] constexpr auto outerProduct(const vec_interface<W1, T1, C1, D1> &lhs,
                                          const vec_interface<W2, T2, C2, D2> &rhs) noexcept;
```

##### ```transpose```
```c++
template <floating_point_scalar T, std::size_t C, std::size_t R>
[[nodiscard]] constexpr mat<T, R, C> transpose(const mat<T, C, R> &arg) noexcept;
```

##### ```determinant```
```c++
template <floating_point_scalar T, std::size_t C>
requires ((2 <= C) && (C <= 4))
[[nodiscard]] constexpr auto determinant(const mat<T, C, C> &arg) noexcept;
```

##### ```inverse```
```c++
template <floating_point_scalar T>
requires ((2 <= C) && (C <= 4))
[[nodiscard]] constexpr auto inverse(const mat<T, C, C> &arg) noexcept;
```

##### ```cross_matrix```
```c++
template <bool W, floating_point_scalar T, typename D>
[[nodiscard]] constexpr mat<T, 3, 3> cross_matrix(const vec_interface<W, T, 3, D> &vec) noexcept;
```
```cross(u, v) == cross_matrix(u) * v == u * cross_matrix(v)```. This creates a matrix that can be used to compute the cross product when multiplied by a vector. This is not in GLSL.

##### ```diagonal_matrix```
```c++
template <bool W, floating_point_scalar T, std::size_t C, typename D>
requires (C > 1)
[[nodiscard]] constexpr mat<T, C, C> diagonal_matrix(const vec_interface<W, T, C, D> &vec) noexcept;
```
This creates a symmetric diagonal matrix (square matrix) using the vector parameter for the diagonal values, with all other matrix elements having value 0. This is not in GLSL.

### Simple Conversion Functions
These functions allow the vector and matrix classes in ```namespace dsga``` to interoperate with C++ array types, for both producing and consuming.

* [```to_vector```](#convert-to-vector)
* [```to_array```](#convert-to-array)
* [```to_matrix```](#convert-to-matrix)

##### Convert To Vector
```c++
template <dimensional_scalar T, std::size_t S>
requires dimensional_storage<T, S>
[[nodiscard]] constexpr vec<T, S> to_vector(const std::array<T, S> &arg) noexcept;

template <dimensional_scalar T, std::size_t S>
requires dimensional_storage<T, S>
[[nodiscard]] constexpr vec<T, S> to_vector(const T(&arg)[S]) noexcept;
```
Convenience functions for converting a ```std::array```  or ```C-style array``` to a ```vec```. The array types must satisfy the ```dimensional_storage``` concept.

##### Convert To Array
```c++
template <bool W, dimensional_scalar T, std::size_t C, typename D>
[[nodiscard]] constexpr std::array<T, C> to_array(const vec_interface<W, T, C, D> &arg) noexcept;

template <floating_point_scalar T, std::size_t C, std::size_t R>
requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4)))
[[nodiscard]] constexpr std::array<T, C * R> to_array(const mat<T, C, R> &arg) noexcept;
```
Convenience functions for converting a vector or matrix to a ```std::array```. For the vector classes, the elements are written to the array in logical order (as opposed to physical order). For ```mat```, the elements are written to the array in column-order.

##### Convert To Matrix
```c++
template <std::size_t C, std::size_t R, floating_point_scalar T, std::size_t S>
requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4))) && (C * R <= S)
[[nodiscard]] constexpr dsga::mat<T, C, R> to_matrix(const std::array<T, S> &arg) noexcept;

template <std::size_t C, std::size_t R, floating_point_scalar T, std::size_t S>
requires (((C >= 2) && (C <= 4)) && ((R >= 2) && (R <= 4))) && (C * R <= S)
[[nodiscard]] constexpr dsga::mat<T, C, R> to_matrix(const T(&arg)[S]) noexcept;
```
Convenience functions for converting a ```std::array```  or ```C-style array``` to a ```mat```. The array types must store the data in column-order.

### Tuple Protocol
The non-iterator classes in ```namespace dsga``` support the tuple protocol. The most important use case is for [structured bindings](https://en.cppreference.com/w/cpp/language/structured_binding).

* [```get```](#get)
* [```tuple_size```](#tuple-size)
* [```tuple_element```](#tuple-element)

##### Get
```c+++
// vec_storage

template <int N, dimensional_scalar T, std::size_t S>
requires (N >= 0) && (N < S)
[[nodiscard]] constexpr auto & get(vec_storage<T, S> & arg) noexcept;

template <int N, dimensional_scalar T, std::size_t S>
requires (N >= 0) && (N < S)
[[nodiscard]] constexpr const auto & get(const vec_storage<T, S> & arg) noexcept;

template <int N, dimensional_scalar T, std::size_t S>
requires (N >= 0) && (N < S)
[[nodiscard]] constexpr auto && get(vec_storage<T, S> && arg) noexcept;

template <int N, dimensional_scalar T, std::size_t S>
requires (N >= 0) && (N < S)
[[nodiscard]] constexpr const auto && get(const vec_storage<T, S> && arg) noexcept;

// vec_interface -- covers use for vec and swizzle_vec

template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
requires W && (N >= 0) && (N < C)
[[nodiscard]] constexpr auto & get(vec_interface<W, T, C, D> & arg) noexcept;

template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr const auto & get(const vec_interface<W, T, C, D> & arg) noexcept;

template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr auto && get(vec_interface<W, T, C, D> && arg) noexcept;

template <int N, bool W, dimensional_scalar T, std::size_t C, typename D>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr const auto && get(const vec_interface<W, T, C, D> && arg) noexcept;

// mat

template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr auto & get(dsga::mat<T, C, R> & arg) noexcept;

template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr const auto & get(const dsga::mat<T, C, R> & arg) noexcept;

template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr auto && get(dsga::mat<T, C, R> && arg) noexcept;

template <int N, dimensional_scalar T, std::size_t C, std::size_t R>
requires (N >= 0) && (N < C)
[[nodiscard]] constexpr const auto && get(const dsga::mat<T, C, R> && arg) noexcept;
```
The ```get``` free functions for all ref-qualifier versions of arguments, defined in ```namespace dsga```. This allows ADL for finding the correct ```get``` function.

##### Tuple Size
```c+++
template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::vec_storage<T, S>> : std::integral_constant<std::size_t, S>;

template<dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_size<dsga::vec<T, S>> : std::integral_constant<std::size_t, S>;

template <dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_size<dsga::swizzle_vec<T, S, C, Is...>> : std::integral_constant<std::size_t, C>;

template <bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_size<dsga::vec_interface<W, T, C, D>> : std::integral_constant<std::size_t, C>;

template <dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::tuple_size<dsga::mat<T, C, R>> : std::integral_constant<std::size_t, C>;
```
Specialized versions of ```std::tuple_size``` for ```namespace dsga``` classes/structs.

##### Tuple Element
```c+++
template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::vec_storage<T, S>>;

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S>
struct std::tuple_element<I, dsga::vec<T, S>>;

template <std::size_t I, dsga::dimensional_scalar T, std::size_t S, std::size_t C, std::size_t ...Is>
struct std::tuple_element<I, dsga::swizzle_vec<T, S, C, Is...>>;

template <std::size_t I, bool W, dsga::dimensional_scalar T, std::size_t C, typename D>
struct std::tuple_element<I, dsga::vec_interface<W, T, C, D>>;

template <std::size_t I, dsga::floating_point_scalar T, std::size_t C, std::size_t R>
struct std::tuple_element<I, dsga::mat<T, C, R>>;
```
Specialized versions of ```std::tuple_element``` for ```namespace dsga``` classes/structs.
