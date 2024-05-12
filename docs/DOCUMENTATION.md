# Documentation

Go to [detailed API documentation](API.md)

Jump to [API concept descriptions](#api).

This single header library aims to provide the basic functionality of the types and functions in the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). We are only interested in the vector and matrix types, along with the corresponding functions that operate on these types. We don't support the language as a whole, just the data types, and that is so we can develop geometric algebraic algorithms. The types are very flexible, and they can be used for a natural rapid prototyping environment for ```c++```.

Here we provide the documentation on what is in the specification, and also some information on how we differ from the specification. To see more about how this works in the context of ```c++20```, have a look at the [details](DETAILS.md) for more information.

## Types and Functions

The vectors structs, matrix struct, and all the corresponding functions are defined in the ```dsga``` namespace. These types are all different based on their sizes and the types of data they hold. They can be dealt with more generically, but for that information see the [details](DETAILS.md) page.

``` c++
// the underlying c++20 template classes (concepts have been mostly omitted)
namespace dsga
{
    template <dimensional_scalar T, std::size_t Size>
    struct basic_vector;
    // ...

    template <floating_point_scalar T, std::size_t Columns, std::size_t Rows>
    struct basic_matrix;
    // ...

    // specialized using types

    // boolean vectors
    using bscal = dsga::basic_vector<bool, 1>;        // not in glsl
    using bvec2 = dsga::basic_vector<bool, 2>;
    using bvec3 = dsga::basic_vector<bool, 3>;
    using bvec4 = dsga::basic_vector<bool, 4>;

    // int vectors
    using iscal = dsga::basic_vector<int, 1>;         // not in glsl
    using ivec2 = dsga::basic_vector<int, 2>;
    using ivec3 = dsga::basic_vector<int, 3>;
    using ivec4 = dsga::basic_vector<int, 4>;

    // unsigned int vectors
    using uscal = dsga::basic_vector<unsigned, 1>;    // not in glsl
    using uvec2 = dsga::basic_vector<unsigned, 2>;
    using uvec3 = dsga::basic_vector<unsigned, 3>;
    using uvec4 = dsga::basic_vector<unsigned, 4>;

    // long long vectors (not in glsl)
    using llscal = dsga::basic_vector<long long, 1>;
    using llvec2 = dsga::basic_vector<long long, 2>;
    using llvec3 = dsga::basic_vector<long long, 3>;
    using llvec4 = dsga::basic_vector<long long, 4>;

    // unsigned long long vectors (not in glsl)
    using ullscal = dsga::basic_vector<unsigned long long, 1>;
    using ullvec2 = dsga::basic_vector<unsigned long long, 2>;
    using ullvec3 = dsga::basic_vector<unsigned long long, 3>;
    using ullvec4 = dsga::basic_vector<unsigned long long, 4>;

    // float vectors with out an 'f' prefix -- this is from glsl
    using scal = dsga::basic_vector<float, 1>;        // not in glsl
    using vec2 = dsga::basic_vector<float, 2>;
    using vec3 = dsga::basic_vector<float, 3>;
    using vec4 = dsga::basic_vector<float, 4>;

    // also float vectors, but using the common naming convention (not in glsl)
    using fscal = dsga::basic_vector<float, 1>;
    using fvec2 = dsga::basic_vector<float, 2>;
    using fvec3 = dsga::basic_vector<float, 3>;
    using fvec4 = dsga::basic_vector<float, 4>;

    // double vectors
    using dscal = dsga::basic_vector<double, 1>;      // not in glsl
    using dvec2 = dsga::basic_vector<double, 2>;
    using dvec3 = dsga::basic_vector<double, 3>;
    using dvec4 = dsga::basic_vector<double, 4>;

    // float matrices
    using mat2x2 = dsga::basic_matrix<float, 2, 2>;
    using mat2x3 = dsga::basic_matrix<float, 2, 3>;
    using mat2x4 = dsga::basic_matrix<float, 2, 4>;
    using mat3x2 = dsga::basic_matrix<float, 3, 2>;
    using mat3x3 = dsga::basic_matrix<float, 3, 3>;
    using mat3x4 = dsga::basic_matrix<float, 3, 4>;
    using mat4x2 = dsga::basic_matrix<float, 4, 2>;
    using mat4x3 = dsga::basic_matrix<float, 4, 3>;
    using mat4x4 = dsga::basic_matrix<float, 4, 4>;

    using mat2 = dsga::basic_matrix<float, 2, 2>;
    using mat3 = dsga::basic_matrix<float, 3, 3>;
    using mat4 = dsga::basic_matrix<float, 4, 4>;

    // double matrices
    using dmat2x2 = dsga::basic_matrix<double, 2, 2>;
    using dmat2x3 = dsga::basic_matrix<double, 2, 3>;
    using dmat2x4 = dsga::basic_matrix<double, 2, 4>;
    using dmat3x2 = dsga::basic_matrix<double, 3, 2>;
    using dmat3x3 = dsga::basic_matrix<double, 3, 3>;
    using dmat3x4 = dsga::basic_matrix<double, 3, 4>;
    using dmat4x2 = dsga::basic_matrix<double, 4, 2>;
    using dmat4x3 = dsga::basic_matrix<double, 4, 3>;
    using dmat4x4 = dsga::basic_matrix<double, 4, 4>;

    using dmat2 = dsga::basic_matrix<double, 2, 2>;
    using dmat3 = dsga::basic_matrix<double, 3, 3>;
    using dmat4 = dsga::basic_matrix<double, 4, 4>;

    //
    // bring the vector and matrix free functions into the dsga namespace
    //

    using namespace dsga::functions;
}

```

## Vector Types

See [details](DETAILS.md) for more info.

In [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), the vectors can only be of dimension 2 through 4, and same for the matrix rows and columns. For ```dsga```, this is true for the matrices, but for the vectors we also can have dimension of 1. They are not very useful, but play a couple roles in the library. Just like the vector types have a suffix of "vec2", "vec3", or "vec4", e.g., ivec2, vec3, dvec4, etc., for the dimension 1 vectors we have types suffixed with "scal" for scalar, e.g., iscal, fscal, bscal, dscal, etc.

We have gone against the specification for a few reasons. Having dimension 1 vectors is a good way of dealing with how GLSL has modifed the basic types, e.g., float, double, int. In GLSL, these basic types are not the same as they are in ```c++```. They behave as if they are vectors of dimension 1, including [swizzling](#swizzling). While GLSL 4.6 does this, note that [OpenGL ES Shading Language](https://www.khronos.org/files/opengles_shading_language.pdf) doesn't do this, so then neither does WebGL. Since we can't change the basic types, we provide the "scalar" vector type that mimics what happens, e.g., bscal, iscal, fscal, dscal, etc.

```c++
// glsl
int some_int = 34;
ivec3 some_vector = some_int.xxx;

// c++20
dsga::iscal some_scalar = 34;
dsga::ivec3 some_vector = some_scalar.xxx;
```

The usual vectors that we create in ```dsga``` are of type ```basic_vector```. The "scalar" types are really ```basic_vector``` of dimension 1. The vectors used for swizzling, of type ```indexed_vector```, can be of dimension 1 through 4, so having the ability to have a ```basic_vector``` of dimension 1 is analogous to a 1 dimensional swizzle.

There doesn't appear to be a a lot of utility for dimension 1 ```basic_vector```s, so in the future they may be removed. However, swizzling is very important for dimension 1 ```indexed_vector```s, so they will never be removed.

### Swizzling

[Swizzling](https://en.wikipedia.org/wiki/Swizzling_(computer_graphics)) is the act of taking a vector and creating a new vector from it that is a specialized "view" on the original vector. The "swizzle" of a vector is itself a vector, although of a different sort, and can be mostly used like a non-swizzle. If there is ever a problem, just wrap up the swizzle in a vector constructor:

```c++
vec4 big_vec;
...
// if you have a problem using the swizzle directly, construct a new vector with it
vec3 smaller_vec(big_vec.zyx);
```

Swizzling uses dot notation, e.g., ```foo.xy, bar.zw, baz.xxyy```. This gives you a type of vector that is a view on the data of the original vector. The swizzles are part of the original vector, and they have the same lifetime. The "x" index means the first value in the vector, "y" means the second, "z" means the third, and "w" means the fourth, so "xyzw" are the possible values in a swizzle, depending on the size of the original vector. In [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), there are 3 different domains for swizzling: ```xyzw```, ```rgba```, and ```stpq```. We use "xyzw" when talking about spatial coordinates, "rgba" when talking about color coordinates, and "stpq" when talking about texture coordinates. Since dsga is intended for geometry and algebra, we only felt the need to support the "xyzw" set for swizzling.

Dot notation swizzle vectors (```indexed_vector```) _can not_ also be swizzled, only ```basic_vector``` types, e.g., dvec2, uvec3. You can create a ```basic_vector``` from a dot notation swizzle vector, and then that can be swizzled.

```c++
auto some_vec = uvec4(0, 1, 2, 3);      // 4 dimensional basic_vector
auto some_swiz = some_vec.xyz;          // 3 dimensional indexed_vector
auto swiz_twice1 = some_vec.xyz.yx;     // compile error, can't swizzle indexed_vector
auto swiz_twice2 = some_swiz.yx;        // compile error, can't swizzle indexed_vector
auto swiz_again = uvec2(some_swiz).yx;  // ok, converted to basic_vector first
```

A 1 dimensional vector can only refer to "x", but it can do so up to 4 times in a swizzle:
```c++
fscal length_one_vec;
...
auto length_four_vec = vec4(length_one_vec.xxxx);
```

Similarly, dimension 2 vectors can refer to combinations of "xy", dimension 3 vectors can refer to combinations of "xyz", and dimension 4 vectors can refer to combinations of "xyzw". Since the maximum size of a vector is 4, that is the maximum number of swizzle characters you can use.

Swizzling to a size of 1 may be another reason to allow ```basic_vector```s of dimension 1:
```c++
vec4 big_vec;
...
fscal z_val = big_vec.z;
```

Dimension 1 vectors, whether ```basic_vector``` (e.g., iscal) or ```indexed_vector``` (from a dot-notation swizzle), are treated as scalar values in most all operations and functions. The values returned from the operations and functions, where only vectors of dimension 1 and fundamental types are arguments, return scalar values of fundamental types, e.g., float, double, bool, int, etc.
```c++
auto value1 = dsga::ivec2(34) + 8;             // value1 is of type dsga::basic_vector<int, 2>
auto value2 = dsga::iscal(34) + 8;             // value2 is of type int

auto some_vec = dsga::ivec4(12, 54, 88, 99);   // some_vec is of type dsga::basic_vector<int,4>
auto value3 = dsga::iscal(34) + some_vec.y;    // value3 is of type int
auto value4 = some_vec.z + some_vec.x;         // value4 is of type int
auto value5 = 100 + some_vec.w;                // value5 is of type int
```

We can also assign to the swizzles if they meet certain criteria. If a swizzle uses an ordinate more than once, then that can't be assigned to, since we could potentially try to give that ordinate different values:
```c++
vec4 big_vec;
...
big_vec.xyx = vec3(1, 2, 3);    // compile error, trying to assign to position "x" more than once
big_vec.zyx = big_vec.xzz;      // ok, data destinations are all unique even if sources are not
```
We also have ways to swizzle at runtime. The ```swizzle()``` function takes a vector and a variable number of indexes into the vector, and returns either a ```basic_vector``` if number of indexes > 1, or a scalar value for a single index applied to the vector. This is not in GLSL. Will return a scalar value if only one index argument. If the index arguments are invalid (out of bounds), this function will throw a ```std::out_of_range()``` exception. Inspired by the [Odin Programming Language](https://odin-lang.org/docs/overview/#swizzle-operations).
```c++
template <bool W, dimensional_scalar T, std::size_t C, typename D, typename Arg>
requires std::convertible_to<Arg, std::size_t>
inline auto swizzle(const vector_base<W, T, C, D> &v, const Arg &index);

template <bool W, dimensional_scalar T, std::size_t C, typename D, typename ...Args>
requires (std::convertible_to<Args, std::size_t> && ...) && (sizeof...(Args) > 0) && (sizeof...(Args) <= 4)
inline basic_vector<T, sizeof...(Args)> swizzle(const vector_base<W, T, C, D> &v, const Args &...Is);
```

For example:
```c++
auto some_vec = uvec3(0, 1, 2);                      // returns a basic_vector<unsigned int, 3>
auto swiz_1d = swizzle(some_vec, 1);                 // returns an unsigned int
auto swiz_2d = swizzle(some_vec, 2, 0);              // returns a basic_vector<unsigned int, 2>
auto swiz_3d = swizzle(some_vec, 2, 0, 3);           // runtime error, all indexes must be less than Count (Count == 3 in this case) for some_vec
auto swiz_4d = swizzle(some_vec, 2, 0, 0, 1);        // returns a basic_vector<unsigned int, 4>
auto swiz_5d = swizzle(some_vec, 2, 0, 0, 1, 2);     // compile error, can't have dimension 5 vectors
auto some_swiz = swizzle(some_vec, 1, 1, 1, 1).zxwy; // returns a copy of an indexed_vector - does not dangle

// Equivalent, with runtime vs. compile time:
// swizzle(some_vec, 2, 0, 0, 1) <==> uvec4(some_vec.zxxy)
// swizzle(some_vec, 2, 0, 0, 1).xyzw <==> some_vec.zxxy
```

### Tolerance Checking

GLSL doesn't provide functions for testing how close vectors are to each other within a tolerance. In our ```dsga``` implementation, we provide a few types of tolerance checking:
* General Tolerance Checking - this checks if one or more values is close to 0 within a scalar or vector of tolerances. Returns true if all the values are within tolerance. The tolerance checking is a less-than-or-equal comparison. Tolerances need to be non-negative, or the function will assert. We use ```vector_base``` as the vector argument type so as to cover both ```basic_vector``` and ```indexed_vector```.
```c++
template <non_bool_scalar T, non_bool_scalar U>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_tolerance(T x,
                                              U tolerance) noexcept;

template <bool W, non_bool_scalar T, std::size_t C, typename D, non_bool_scalar U>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_tolerance(const vector_base<W, T, C, D> &x,
                                              U tolerance) noexcept;

template <bool W1, non_bool_scalar T, std::size_t C1, typename D1, bool W2, non_bool_scalar U, std::size_t C2, typename D2>
requires ((C1 == C2) || (C2 == 1)) && implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_tolerance(const vector_base<W1, T, C1, D1> &x,
                                              const vector_base<W2, U, C2, D2> &tolerance) noexcept;
```
* Euclidean Distance - this treats the vectors as mathematical vectors or points, and it compares the Euclidean distance between them to see if the distance is 0 within the tolerance. The tolerance checking is a less-than-or-equal comparison. Tolerances need to be non-negative, or the function will assert. The use of type ```vector_base``` is an advanced feature, and it is used to make sure it covers vectors and their swizzles. More can be learned about ```vector_base``` in the [details](DETAILS.md).
```c++
template <non_bool_scalar T, non_bool_scalar U>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_distance(T x,
                                             T y,
                                             U tolerance) noexcept;

template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, non_bool_scalar U>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
                                             const vector_base<W2, T, C, D2> &y,
                                             U tolerance) noexcept;

template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, bool W3, non_bool_scalar U, typename D3>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
                                             const vector_base<W2, T, C, D2> &y,
                                             const vector_base<W3, U, 1, D3> &tolerance) noexcept;
```
* Bounding Box - this function performs a component-wise tolerance check, where the end result depends on all the checks. There is a version of this function that takes a vector of tolerances as well. This function checks if the difference of the value(s) are 0 within the tolerance(s). All the vector elements must be within tolerance or the whole answer is false. The tolerance checking is a less-than-or-equal comparison. All tolerances need to be non-negative, or the function will assert. This function also uses ```vector_base``` like the other tolerance functions, and for the same reasons.
```c++
template <non_bool_scalar T, non_bool_scalar U>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_box(T x,
                                        T y,
                                        U tolerance) noexcept;

template <bool W1, non_bool_scalar T, std::size_t C, typename D1, bool W2, typename D2, non_bool_scalar U>
requires implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_box(const vector_base<W1, T, C, D1> &x,
                                        const vector_base<W2, T, C, D2> &y,
                                        U tolerance) noexcept;

template <bool W1, non_bool_scalar T, std::size_t C1, typename D1, bool W2, typename D2, bool W3, non_bool_scalar U, std::size_t C2, typename D3>
requires ((C1 == C2) || (C2 == 1)) && implicitly_convertible_to<U, T>
[[nodiscard]] constexpr bool within_box(const vector_base<W1, T, C1, D1> &x,
                                        const vector_base<W2, T, C1, D2> &y,
                                        const vector_base<W3, U, C2, D3> &tolerance) noexcept;
```

## Matrix Types

As noted above, matrices each have between 2-4 rows and 2-4 columns, giving 9 possible matrix sizes. The components of the matrices must be floating-point types. The matrices store things in column major order, and the type naming reflects that. It can be confusing to read since that is the opposite of the mathematical notation for matrices. The set of columns is represented as an array of floating point vectors. The columns of the matrix are accessible via array notation, i.e., ```operator []```. The rows of the matrix are accessible via the ```row()``` function. Any component of a matrix ```A``` can be accessed by two adjacent ```operator []``` calls, such as ```A[col_num][row_num]```.

For an example of GLSL type names vs. math notation, ```mat4x2``` is a matrix with 4 columns with 2 rows, but math notation specifies the number of rows first, followed by number of columns. So the GLSL type ```mat4x2``` is a 2x4 matrix using math notation ("m by n" which is "rows by columns").

The matrix types are very generic. One can pre-mulitply (matrix on left, vector on right), post-multiply (vector on left, matrix on right), treat square matrices that are meant to represent transformations as left-handed or right-handed, etc. There is no default preferred interpretation in ```dsga```, although users may have a preferred approach to using matrices.

# API
[Detailed API documentation](API.md) for ```dsga```.

* Matrices and Vectors
   * [Index Interface](#index-interface)
   * [Iterators](#iterators)
   * [Tuple Protocol](#tuple-protocol)
   * [Low Level Pointer Access](#low-level-pointer-access)
* [Vector](#vector)
   * [Rule Of Six For Vectors](#rule-of-six-for-vectors)
   * [Vector Constructors](#vector-constructors)
   * [Vector Members](#vector-member-functions)
   * [Vector Operators](#vector-operators)
   * [Vector Functions](#vector-free-functions)
* [Matrix](#matrix)
   * [Rule Of Six For Matrices](#rule-of-six-for-matrices)
   * [Matrix Constructors](#matrix-constructors)
   * [Matrix Members](#matrix-member-functions)
   * [Matrix Operators](#matrix-operators)
   * [Matrix Functions](#matrix-free-functions)

It is difficult to give a straightforward list of all the functions in the vector and matrix structs. First, there are many different classes for different sized vectors, although each has roughly the same API. Second, we specialize the vectors and matrices based on size and type. Third, the function signatures are pretty difficult to read, as they:

* are generic, which means they usually use base classes
* use templates
* use concepts
* have many different versions

We have [enumerated all the specific classes](#types-and-functions) we support in the above section on types and functions, and there are a lot of them. GLSL has a bias towards the type ```float```, but we implemented the functions without that bias. If there is a ```float``` version of a function, then there is likely a ```double``` version. GLSL also does not provide support for the 64-bit integer types ```long long``` and ```unsigned long long```. So for the most part (but not in all cases), if there is function for ```int``` types, there should be a version for ```long long``` types. The same is true for ```unsigned int``` and ```unsigned long long```.

Please look at what is in the [GLSL spec](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), especially Section 5 and Section 8, for a thorough look at the API. We will summarize what was implemented and how we supplemented matrix and vector.

### Index Interface

The vector structs closely model short versions of ```std::array```, including using ```operator []``` for access to scalar components. Matrix also has "operator []", but it is for accessing column vectors.

For vector swizzles, only *writable* swizzles can use ```operator []``` for write access to the vector.

### Iterators

Both the vector and matrix structs support ```begin()/cbegin()/rbegin()/crbegin()``` and ```end()/cend()/rend()/crend()``` iterator creation functions in order to provide non-const and const iterators. The ```indexed_vector``` iterators pass the ```std::random_access_iterator``` concept. This gives us access to:

* Standard Library Algorithms
* [Range-based for loop](https://en.cppreference.com/w/cpp/language/range-for)

### Tuple Protocol

Both the vector and matrix structs support ```std::tuple_element<>```, ```std::tuple_size<>``` and ```get<>``` in order to provide basic ```std::tuple``` support. This gives us access to:

* Data structures in same manner as ```tuple```
* [Structured Binding](https://en.cppreference.com/w/cpp/language/structured_binding)

### Low-level Pointer Access

Both the vector and matrix structs support ```data()``` and ```size()``` in order to provide pointer access to the underlying data. The parameter pack returned by ```sequence()``` can also be helpful here to map the physical order from ```data()``` to the logical order (only useful for generic vector situations or when using ```indexed_vector```). *Hopefully*, no one wants to use pointer data to manipulate or access the data structures, but this approach exists if it is deemed appropriate:

* ```T *data()``` gives a pointer to the underlying vector elements or matrix columns, in physical order.
* ```std::size_t size()``` gives the number of elements in the vector or number of columns in the matrix.
* ```std::index_sequence<Is...> sequence()``` (only for vectors) gives a parameter pack that maps the physical order to the logical order. For a ```basic_vector``` those are the same, but for an ```indexed_vector``` they are mostly not the same. Pack expansion and folding are tools that might help with the low-level pointer access for vectors.
* ```std::array<std::size_t, Count> offsets``` is an array with same data that is in ```sequence()```, just in another data structure. Each vector class has this static member variable.

The value returned by ```size()``` is not the length of the underlying ```data()```, but it is the size of the ```sequence()```/```offsets``` view of the data. ```dsga::basic_vector<>``` is contiguous, and the ```data()``` is in order, but for ```dsga::indexed_vector<>``` it definitely mostly is not in order, which is the whole point of the struct.

## Vector

```c++
// length 1 vector/scalar hybrids
//
// bscal - bool
// iscal - int
// uscal - unsigned
// llscal - long long
// ullscal - unsigned long long
// scal - float (following GLSL's preference for float)
// fscal - float
// dscal - double
```

These length 1 vectors are not officially in GLSL, but GLSL did change the fundamental types (e.g., int, float, double, etc.) to behave as if they are. We try to automatically convert from these to the underlying scalar type when we can. If there is a problem, just cast it to the underlying type.

```c++
// length 2-4 vectors
//
// bvec2, bvec3, bvec4 - bool
// ivec2, ivec3, ivec4 - int
// uvec2, uvec3, uvec4 - unsigned
// llvec2, llvec3, llvec4 - long long                 not in GLSL
// ullvec2, ullvec3, ullvec4 - unsigned long long     not in GLSL
// vec2, vec3, vec4 - float
// fvec2, fvec3, fvec4 - float                        not in GLSL
// dvec2, dvec3, dvec4 - double
```

GLSL does not support the types ```long long``` or ```unsigned long long```, but we extended the vectors to support those types. ```fvec2, fvec3, fvec4``` are redundant with ```vec2, vec3, vec4```, but they exist because they follow the type prefix style naming if one prefers to be more explicit.

### Rule of Six For Vectors

The six special functions are all defaulted for ```basic_vector```.
```c++
constexpr basic_vector() noexcept = default;
constexpr ~basic_vector() noexcept = default;

constexpr basic_vector(const basic_vector &) noexcept = default;
constexpr basic_vector(basic_vector &&) noexcept = default;
constexpr basic_vector &operator =(const basic_vector &) noexcept = default;
constexpr basic_vector &operator =(basic_vector &&) noexcept = default;
```

### Vector Constructors
* **Single Scalar Argument** - the scalar parameter must be convertible to the underlying type, and it is used to initialize every element of the vector.

```c++
template <typename U>
explicit constexpr basic_vector(U value) noexcept;
```

* **Multiple Scalar Arguments** - the scalar parameters must be convertible to the underlying type, and there is one for each element to initialize the vector. For example, where C == 4:

```c++
template <typename U1, typename U2, typename U3, typename U4>
requires
    std::convertible_to<U1, T> && std::convertible_to<U2, T> &&
    std::convertible_to<U3, T> && std::convertible_to<U4, T>
explicit constexpr basic_vector(U1 xvalue,
                                U2 yvalue,
                                U3 zvalue,
                                U4 wvalue) noexcept;
```

* **Variable Arguments** - any combination of scalar values, vectors, and matrices can be arguments to the constructor, as long as there is enough data to initialize all the vector elements, and as long as the types are convertible. It is fine if an argument has more data than necessary to complete the vector initialization, as long as some of the argument data is used. It is an error to pass unused arguments:

```c++
// variadic constructor of scalar and vector arguments
template <typename ... Args>
constexpr basic_vector(const Args & ...args) noexcept;
```

* **Intializer List of Values** - ```basic_vector``` has user-declared constructors, so it is not an aggregate struct/class, so we can't do aggregate initialization with an initializer list. We can use brace initialization to call the constructors. This is different from GLSL.

### Vector Member Functions

These are the members that are not part of the [index interface](#index-interface), [iterator interface](#iterators), the [tuple protocol](#tuple-protocol), or the [pointer interface](#low-level-pointer-access).

* ```operator =``` - assignment operator. The vector needs to be the same length and underlying types must be convertible. All ```basic_vector```s are writable, but not all ```indexed_vector```s.
* ```set()``` - like assignment, but copies all the individual argument values into a vector. All ```basic_vector```s are writable, but not all ```indexed_vector```s.
* ```int length()``` - returns the number of elements in a vector. This is part of the spec, and is the same as ```size()``` except it has a different return type.

#### Valarray Functions

There is conceptual overlap between the vector structs and ```std::valarray```. Much of this conceptual overlap is implemented in ```dsga``` in terms of operators and vector free functions. The following functions have been implemented as vector member functions. These functions are part of the [```std::valarray``` API](https://en.cppreference.com/w/cpp/numeric/valarray).

* ```apply()```
* ```shift()```
* ```cshift()```
* ```min()```
* ```max()```
* ```sum()```
* ```query()``` - not in GLSL nor ```std::valarray``` - like ```apply()``` but for boolean predicates

### Vector Operators
The vector operators all work component-wise.

* unary ```operator +``` - not in GLSL
* unary ```operator -```
* unary ```operator ++``` (pre-increment)
* unary ```operator ++``` (post-increment)
* unary ```operator --``` (pre-decrement)
* unary ```operator --``` (post-decrement)
* binary ```operator +```
* binary ```operator -```
* binary ```operator *```
* binary ```operator /```
* binary ```operator %```
* binary ```operator >>```
* binary ```operator <<```
* binary ```operator &```
* binary ```operator |```
* binary ```operator ^```
* unary ```operator ~```
* ```operator +=```
* ```operator -=```
* ```operator *=```
* ```operator /=```
* ```operator %=```
* ```operator <<=```
* ```operator >>=```
* ```operator &=```
* ```operator |=```
* ```operator ^=```
* ```operator ==```
* ```operator !=``` - created automatically from ```operator ==``` in ```c++20```

### Vector Free Functions
The vector functions all work component-wise, except for the geometric functions and the two vector relational functions ```any()``` and ```all()```.
There are scalar versions of these vector functions where it makes sense, i.e., angle and trignometry, exponential, and common functions.


#### Angle and Trigonometry Functions
* ```radians()```
* ```degrees()```
* ```sin()```
* ```cos()```
* ```tan()```
* ```asin()```
* ```acos()```
* ```atan()```
* ```sinh()```
* ```cosh()```
* ```tanh()```
* ```asinh()```
* ```acosh()```
* ```atanh()```

#### Exponential Functions
* ```pow()```
* ```exp()```
* ```log()```
* ```exp2()```
* ```log2()```
* ```sqrt()```
* ```inversesqrt()```
* ```fast_inversesqrt()``` - not in GLSL

#### Common Functions
* ```abs()```
* ```sign()```
* ```floor()```
* ```trunc()```
* ```round()```
* ```roundEven()```
* ```ceil()```
* ```fract()```
* ```mod()```
* ```modf()```
* ```min()```
* ```max()```
* ```clamp()```
* ```mix()```
* ```step()```
* ```smoothstep()```
* ```isnan()```
* ```isinf()```
* ```floatBitsToInt()```
* ```floatBitsToUint()```
* ```doubleBitsToLongLong()``` - not in GLSL
* ```doubleBitsToUlongLong()``` - not in GLSL
* ```intBitsToFloat()```
* ```uintBitsToFloat()```
* ```longLongBitsToDouble()``` - not in GLSL
* ```ulongLongBitsToDouble()``` - not in GLSL
* ```fma()```
* ```frexp()```
* ```ldexp()```
* ```byteswap()``` - not in GLSL - doesn't use ```std::byteswap()``` since that is in c++23
* ```swizzle()``` - not in GLSL - runtime swizzle() function
* ```to_underlying``` - not in GLSL - not for vectors or matrices, but is a c++23 function that might come in handy

#### Geometric Functions
The geometric functions treat a vector as an entity instead of as a collection of components.

The vector component type may be any of the vector types except bool.

* ```innerProduct()``` - not in GLSL -  similar to ```dot()```

The vector component type must be floating-point.

* ```length()```
* ```distance()```
* ```dot()```
* ```cross()``` - only works for vectors of size 3
* ```normalize()```
* ```faceforward()```
* ```reflect()```
* ```refract()```

#### Vector Relational Functions

* ```lessThan()```
* ```lessThanEqual()```
* ```greaterThan()```
* ```greaterThanEqual()```
* ```equal()```
* ```notEqual()```
* ```any()``` - result relies on the components' relationship with each other
* ```all()``` - result relies on the components' relationship with each other
* ```none()``` - result relies on the components' relationship with each other - not in GLSL
* ```logicalNot()``` - can't use keyword ```not``` as a function name in ```c++```, so using ```logicalNot()```

#### The Rest of the Specification

Functions from the [GLSL spec](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) not mentioned here were not implemented. The unimplemented functions likely had to do with rendering or textures or something that is not related to the vector and matrix geometric uses that this library aims for.

## Matrix

Matrix elements are of a floating-point type. All of these types are in GLSL.

```c++
mat2x2/mat2, mat2x3, mat2x4 - 2 column float matrices
mat3x2, mat3x3/mat3, mat3x4 - 3 column float matrices
mat4x2, mat4x3, mat4x4/mat4 - 4 column float matrices

dmat2x2/dmat2, dmat2x3, dmat2x4 - 2 column double matrices
dmat3x2, dmat3x3/dmat3, dmat3x4 - 3 column double matrices
dmat4x2, dmat4x3, dmat4x4/dmat4 - 4 column double matrices
```

### Rule of Six For Matrices

The six special functions are all defaulted for ```basic_matrix```.
```c++
constexpr basic_matrix() noexcept = default;
constexpr ~basic_matrix() noexcept = default;

constexpr basic_matrix(const basic_matrix &) noexcept = default;
constexpr basic_matrix(basic_matrix &&) noexcept = default;
constexpr basic_matrix &operator =(const basic_matrix &) noexcept = default;
constexpr basic_matrix &operator =(basic_matrix &&) noexcept = default;
```

### Matrix Constructors
* **Single Scalar Argument** - this constructor only works on square matrices, where the number of rows and columns is the same. The parameter is used to initialize every diagonal element of the matrix, with all the other elements set to 0. The parameter must be convertible to the underlying floating-point type stored in the matrix.

```c++
// diagonal constructor for square matrices
template <typename U>
constexpr basic_matrix(U arg) noexcept;
```

* **Single Matrix Argument, same type** - any matrix can be used to create another matrix, regardless of any size differences. If they are the same size, then the defaulted copy/move constructor will be called. If they are different sizes, then this constructor intializes what it can of the matrix as if the rows and columns were intersected with the argument's rows and columns. If there are matrix elements that are not initialized by the matrix argument, they will be set to 0. If it is a square matrix, and a diagonal element has not been initialized, it will be set to 1.

```c++
template <floating_point_scalar U, std::size_t Cols, std::size_t Rows>
constexpr basic_matrix(const basic_matrix<U, Cols, Rows> &arg) noexcept;
```

* **Single Matrix Argument, different type** - for matrices of the same dimensions, where the matrix type is implicitly convertible to the type of matrix you are trying to construct.

```c++
template <floating_point_scalar U>
requires implicitly_convertible_to<U, T>
constexpr basic_matrix(const basic_matrix<U, C, R> &arg) noexcept;
```

* **Variable Arguments** - any combination of scalar values and vectors can be arguments to the constructor, as long as there is enough data to initialize all the matrix elements, and as long as the types are convertible. It is fine if an argument has more data than necessary to complete the matrix initialization, as long as some of the argument data is used. It is an error to pass unused arguments:

```c++
// variadic constructor of scalar and vector arguments
template <typename ... Args>
constexpr basic_matrix(const Args & ...args) noexcept;
``` 

* **Intializer List of Values** - ```basic_matrix``` has user-declared constructors, so it is not an aggregate struct/class, so we can't do aggregate initialization with an initializer list. We can use brace initialization to call the constructors. This is different from GLSL.

```c++
constexpr basic_matrix(const std::initializer_list<T> &init_list) noexcept;
```

### Matrix Member Functions

These are the members that are not part of the [iterator interface](#iterators), the [tuple protocol](#tuple-protocol), or the [low-level interface](#low-level-pointer-access).

* **operator =** - assignment operator. The matrix needs to be the same size and underlying types must be convertible.
* **int length()** - returns the number of columns as an ```int```. This is part of the spec, and is the same as ```size()``` except it has a different return type.
* **int column_length()** - like ```length()``` but for rows. Returns the number of rows as an ```int```.
* **std::size_t size()** - returns the number of columns as as a ```std::size_t```.
* **std::size_t column_size()** - like ```size()``` but for rows. Returns the number of rows as a ```std::size_t```.
* **operator []** - a generic way to access matrix data. The values returned by this operator are the columns of the matrix, which are of type ```basic_vector```. Can be used for both reading and writing, assuming it isn't const or otherwise not allowed for writing. Along with ```operator []``` in ```basic_vector```, we can access individual matrix elements with notation such as **auto val = my_matrix[col][row];**.
* **row()** - this returns a ```basic_vector``` that represents a row of the matrix.

### Matrix Operators
The matrix operators all work component-wise, except for ```operator *```, which performs linear algebraic functions with vectors and other matrices.

* unary ```operator +``` - not in GLSL
* unary ```operator -```
* unary ```operator ++``` (pre-increment)
* unary ```operator ++``` (post-increment)
* unary ```operator --``` (pre-decrement)
* unary ```operator --``` (post-decrement)
* binary ```operator +```
* binary ```operator -```
* binary ```operator *``` - linear algebraic operations
   * vector * matrix
   * matrix * vector
   * matrix * matrix
* binary ```operator /```
* ```operator ==```
* ```operator !=``` - created automatically from ```operator ==``` in ```c++20```

### Matrix Free Functions
The matrix functions treat a matrix as an entity instead of as a collection of components, except for ```matrixCompMult()```, which works component-wise.

* ```matrixCompMult()``` - this function exists because ```operator *``` is used for linear algebraic purposes instead of component-wise multiplication
* ```outerProduct()```
* ```transpose()```
* ```determinant()```
* ```inverse()```
* ```cross_matrix()``` - ```dsga::cross(u, v) == dsga::cross_matrix(u) * v == u * dsga::cross_matrix(v)```  - not in GLSL - create matrix to compute cross product - only works for vectors of size 3 
* ```diagonal_matrix()``` - not in GLSL - create matrix whose diagonal values come from vector parameter, with all other values being 0
