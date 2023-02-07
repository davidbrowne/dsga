# Documentation

Jump to [API](#api).

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

    template <floating_point_dimensional_scalar T, std::size_t Columns, std::size_t Rows>
    struct basic_matrix;
    // ...

    // specialized using types

    // boolean vectors
    using bscal = dsga::basic_vector<bool, 1u>;
    using bvec2 = dsga::basic_vector<bool, 2u>;
    using bvec3 = dsga::basic_vector<bool, 3u>;
    using bvec4 = dsga::basic_vector<bool, 4u>;

    // int vectors
    using iscal = dsga::basic_vector<int, 1u>;
    using ivec2 = dsga::basic_vector<int, 2u>;
    using ivec3 = dsga::basic_vector<int, 3u>;
    using ivec4 = dsga::basic_vector<int, 4u>;

    // unsigned int vectors
    using uscal = dsga::basic_vector<unsigned, 1u>;
    using uvec2 = dsga::basic_vector<unsigned, 2u>;
    using uvec3 = dsga::basic_vector<unsigned, 3u>;
    using uvec4 = dsga::basic_vector<unsigned, 4u>;

    // long long vectors (not in glsl)
    using llscal = dsga::basic_vector<long long, 1u>;
    using llvec2 = dsga::basic_vector<long long, 2u>;
    using llvec3 = dsga::basic_vector<long long, 3u>;
    using llvec4 = dsga::basic_vector<long long, 4u>;

    // unsigned long long vectors (not in glsl)
    using ullscal = dsga::basic_vector<unsigned long long, 1u>;
    using ullvec2 = dsga::basic_vector<unsigned long long, 2u>;
    using ullvec3 = dsga::basic_vector<unsigned long long, 3u>;
    using ullvec4 = dsga::basic_vector<unsigned long long, 4u>;

    // float vectors with out an 'f' prefix -- this is from glsl
    using scal = dsga::basic_vector<float, 1u>;
    using vec2 = dsga::basic_vector<float, 2u>;
    using vec3 = dsga::basic_vector<float, 3u>;
    using vec4 = dsga::basic_vector<float, 4u>;

    // also float vectors, but using the common naming convention (not in glsl)
    using fscal = dsga::basic_vector<float, 1u>;
    using fvec2 = dsga::basic_vector<float, 2u>;
    using fvec3 = dsga::basic_vector<float, 3u>;
    using fvec4 = dsga::basic_vector<float, 4u>;

    // double vectors
    using dscal = dsga::basic_vector<double, 1u>;
    using dvec2 = dsga::basic_vector<double, 2u>;
    using dvec3 = dsga::basic_vector<double, 3u>;
    using dvec4 = dsga::basic_vector<double, 4u>;

    // float matrices
    using mat2x2 = dsga::basic_matrix<float, 2u, 2u>;
    using mat2x3 = dsga::basic_matrix<float, 2u, 3u>;
    using mat2x4 = dsga::basic_matrix<float, 2u, 4u>;
    using mat3x2 = dsga::basic_matrix<float, 3u, 2u>;
    using mat3x3 = dsga::basic_matrix<float, 3u, 3u>;
    using mat3x4 = dsga::basic_matrix<float, 3u, 4u>;
    using mat4x2 = dsga::basic_matrix<float, 4u, 2u>;
    using mat4x3 = dsga::basic_matrix<float, 4u, 3u>;
    using mat4x4 = dsga::basic_matrix<float, 4u, 4u>;

    using mat2 = dsga::basic_matrix<float, 2u, 2u>;
    using mat3 = dsga::basic_matrix<float, 3u, 3u>;
    using mat4 = dsga::basic_matrix<float, 4u, 4u>;

    // double matrices
    using dmat2x2 = dsga::basic_matrix<double, 2u, 2u>;
    using dmat2x3 = dsga::basic_matrix<double, 2u, 3u>;
    using dmat2x4 = dsga::basic_matrix<double, 2u, 4u>;
    using dmat3x2 = dsga::basic_matrix<double, 3u, 2u>;
    using dmat3x3 = dsga::basic_matrix<double, 3u, 3u>;
    using dmat3x4 = dsga::basic_matrix<double, 3u, 4u>;
    using dmat4x2 = dsga::basic_matrix<double, 4u, 2u>;
    using dmat4x3 = dsga::basic_matrix<double, 4u, 3u>;
    using dmat4x4 = dsga::basic_matrix<double, 4u, 4u>;

    using dmat2 = dsga::basic_matrix<double, 2u, 2u>;
    using dmat3 = dsga::basic_matrix<double, 3u, 3u>;
    using dmat4 = dsga::basic_matrix<double, 4u, 4u>;

    //
    // bring the vector and matrix free functions into the dsga namespace
    //

    using namespace dsga::functions;
}

```

## Vector Types

In [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), the various dimensions of the vectors and matrices are between 2 and 4, inclusive. For ```dsga```, this is true for the matrices, but for the vectors we also can have length be 1. We don't call them "vectors", but we suffix their type with "scal" for scalar, as opposed to a suffix of "vec1".

We have gone against the specification for a few reasons. Having length 1 vectors is a good way of dealing with how [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) has modifed the basic types, e.g., float, double, int. In GLSL, these basic types are not the same as they are in ```c++```. They behave as if they are vectors of length 1, including [swizzling](#swizzling). Since we can't change the basic types, we provide the "scalar" analog vector type that mimics what happens, e.g., ```bscal```, ```iscal```, ```fscal```, ```dscal```.

Since these "scalar" types are really vectors of length 1, they can use the vector functions. The functions don't discriminate based on size in dsga. In the cubic bezier evaluator example, we call the vector function ```mix()``` several times, and the last time was formerly on vectors of length 1, before we decided to pretend that it was "scalar" like fscal, so instead we are calling ```std::lerp()``` for the length 1 vector case.

### Swizzling

[Swizzling](https://en.wikipedia.org/wiki/Swizzling_(computer_graphics)) is the act of taking a vector and creating a new vector from it that is a specialized "view" on the original vector. The "swizzle" of a vector is itself a vector, although of a different sort, and can be mostly used like a non-swizzle. If there is ever a problem, just wrap up the swizzle in a vector constructor:

```c++
vec4 big_vec;
...
// if you have a problem using the swizzle directly, construct a new vector with it
vec3 smaller_vec(big_vec.zyx);
```

Swizzling uses dot notation, e.g., ```foo.xy, bar.zw, baz.xxyy```. This gives you a type of vector that is a view on the data of the original vector. The swizzles are part of the original vector, and they have the same lifetime. The "x" index means the first value in the vector, "y" means the second, "z" means the third, and "w" means the fourth, so "xyzw" are the possible values in a swizzle, depending on the size of the original vector. In [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), there are 3 different domains for swizzling: ```xyzw```, ```rgba```, and ```stpq```. We use "xyzw" when talking about spatial coordinates, "rgba" when talking about color coordinates, and "stpq" when talking about texture coordinates. Since dsga is intended for geometry and algebra, we only bothered supporting **xyzw** for swizzling.

A length 1 vector can only refer to "x", but it can do so up to 4 times in a swizzle:
```c++
fscal length_one_vec;
...
auto length_four_vec = vec4(length_one_vec.xxxx);
```

Similarly, length 2 vectors can refer to combinations of "xy", length 3 vectors can refer to combinations of "xyz", and length 4 vectors can refer to combinations of "xyzw". Since the maximum size of a vector is 4, that is the maximum number of swizzle characters you can use.

Swizzling to a size of 1 is another reason to allow vectors of length 1:
```c++
vec4 big_vec;
...
fscal z_val = big_vec.z;
```

We try to let these length 1 vectors straddle the line between a vector and scalar. We do as much as possible to treat it like a real scalar value, but at the same time we also allow these to use the vector functions and operations.

We can also assign to the swizzles if they meet certain criteria. If a swizzle uses an ordinate more than once, then that can't be assigned to, since we could potentially try to give that ordinate different values:
```c++
vec4 big_vec;
...
big_vec.xyx = vec3(1, 2, 3);    // error, trying to assign to position "x" more than once
big_vec.zyx = big_vec.xzz;      // ok, data destinations are all unique even if sources are not
```

### Tolerance Checking

GLSL doesn't provide functions for testing how close vectors are to each other within a tolerance. In our ```dsga``` implementation, we provide two types of tolerance checking:

* Euclidean Distance - this treats the vectors as mathematical vectors or points, and it compares the Euclidean distance between them to see if they are within the tolerance. The tolerance is a strictly less-than comparison. Tolerances need to be non-negative. The use of type ```vector_base``` is an advanced feature, and it is used to make sure it covers vectors and their swizzles. More can be learned about ```vector_base``` in the [details](DETAILS.md).
```c++
namespace dsga
{
    template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
    constexpr bool within_distance(const vector_base<W1, T, C, D1> &x,
                                   const vector_base<W2, T, C, D2> &y,
                                   T tolerance) noexcept;
}
```
* Bounding Box - this is a component-wise tolerance check. There is a version of this function that takes a vector of tolerances as well. All the vector elements must be within tolerance or the whole answer is false. The tolerance is a strictly less-than comparison. All tolerances need to be non-negative. This function also uses ```vector_base``` like the other tolerance functions, and for the same reasons.
```c++
namespace dsga
{
    template <bool W1, dimensional_scalar T, std::size_t C, typename D1, bool W2, typename D2>
    constexpr bool within_box(const vector_base<W1, T, C, D1> &x,
                              const vector_base<W2, T, C, D2> &y,
                              T tolerance) noexcept;
}
```

## Matrix Types

As noted above, matrices each have between 2-4 rows and 2-4 columns, giving 9 possible matrix sizes. The components of the matrices must be floating-point types. The matrices store things in column major order, and the type naming reflects that. It can be confusing to read since that is the opposite of the mathematical notation for matrices. The set of columns is represented as an array of floating point vectors. The columns of the matrix are accessible via array notation, i.e., ```operator []```. The rows of the matrix are accessible via the ```row()``` function. Any component of a matrix ```A``` can be accessed by two adjacent ```operator []``` calls, such as ```A[col_num][row_num]```.

For an example of GLSL type names vs. math notation, ```mat4x2``` is a matrix with 4 columns with 2 rows, but math notation specifies the number of rows first, followed by number of columns. So the GLSL type ```mat4x2``` is a 2x4 matrix using math notation ("m by n" which is "rows by columns").

The matrix types are very generic. One can pre-mulitply (matrix on left, vector on right), post-multiply (vector on left, matrix on right), treat square matrices that are meant to represent transformations as left-handed or right-handed, etc. There is no default preferred interpretation in ```dsga```, although users may have a preferred approach to using matrices.

# API

* Matrices and Vectors
   * [Iterators](#iterators)
   * [Tuple Interface](#tuple-interface)
   * [Low Level Pointer Access](#low-level-pointer-access)
* [Vector](#vector)
   * [Rule Of Six For Vectors](#rule-of-six-for-vectors)
   * [Vector Constructors](#vector-constructors)
   * [Vector Members](#vector-members)
   * [Vector Operators](#vector-operators)
   * [Vector Functions](#vector-functions)
* [Matrix](#matrix)
   * [Rule Of Six For Matrices](#rule-of-six-for-matrices)
   * [Matrix Constructors](#matrix-constructors)
   * [Matrix Members](#matrix-members)
   * [Matrix Operators](#matrix-operators)
   * [Matrix Functions](#matrix-functions)

It is difficult to give a straightforward list of all the functions in the vector and matrix structs. First, there are many different classes for different sized vectors, although each has roughly the same API. Second, we specialize the vectors and matrices based on size and type. Third, the function signatures are pretty difficult to read, as they:

* are generic, which means they usually use base classes
* use templates
* use concepts
* have many different versions

We have [enumerated all the specific classes](#types-and-functions) we support in the above section on types and functions, and there are a lot of them. GLSL has a bias towards the type ```float```, but we implemented the functions without that bias. If there is a ```float``` version of a function, then there is likely a ```double``` version. GLSL also does not provide support for the 64-bit integer types ```long long``` and ```unsigned long long```. So for the most part (but not in all cases), if there is function for ```int``` types, there should be a version for ```long long``` types. The same is true for ```unsigned int``` and ```unsigned long long```.

Please look at what is in the [GLSL spec](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), especially Section 5 and Section 8, for a thorough look at the API. We will summarize what was implemented and how we supplemented matrix and vector.

### Iterators

Both the vector and matrix structs support **begin/cbegin/rbegin/crbegin** and **end/cend/rend/crend** iterator creation functions in order to provide non-const and const iterators. The ```indexed_vector``` iterators pass the ```std::random_access_iterator``` concept. This gives us access to:

* Standard Library Algorithms
* [Range-based for loop](https://en.cppreference.com/w/cpp/language/range-for)

### Tuple Interface

Both the vector and matrix structs support **std::tuple_element<>**, **std::tuple_size<>** and **get<>** in order to provide basic ```std::tuple``` support. This gives us access to:

* Data structures in same manner as ```tuple```
* [Structured Binding](https://en.cppreference.com/w/cpp/language/structured_binding)

### Low-level Pointer Access

Both the vector and matrix structs support ```data()``` and ```size()``` in order to provide pointer access to the underlying data. The parameter pack returned by ```sequence()``` can also be helpful here to map the physical order from ```data()``` to the logical order (only useful for generic vector situations or when using ```indexed_vector```). *Hopefully*, no one wants to use pointer data to manipulate or access the data structures, but this approach exists if it is deemed appropriate:

* **T \*data()** gives a pointer to the underlying vector elements or matrix columns, in physical order.
* **std::size_t size()** gives the number of elements in the vector or number of columns in the matrix.
* **std::index_sequence<Is...> sequence()** (only for vectors) gives a parameter pack that maps the physical order to the logical order. For a ```basic_vector``` those are the same, but for an ```indexed_vector``` they are mostly not the same. Pack expansion and folding are tools that might help with the low-level pointer access for vectors.

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

These length 1 vectors are not officially in GLSL, but GLSL did change the fundamental types to behave as if they are. We try to automatically convert from these to the underlying scalar type when we can. If there is a problem, just cast it to the underlying type.

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

* **Variable Arguments** - any combination of scalar values, vectors, and matrices can be arguments to the constructor, as long as there is enough data to initialize all the vector elements, and as long as the types are convertible. It is fine if an argument has more data than necessary to complete the vector initialization, as long as some of the argument data is used. It is an error to pass unused arguments:

```c++
// variadic constructor of scalar and vector arguments
template <typename ... Args>
constexpr basic_vector(const Args & ...args) noexcept;
``` 

This approach is exactly what ```basic_matrix``` does.

### Vector Member Functions

These are the members that are not part of the [iterator interface](#iterators), the [tuple interface](#tuple-interface), or the [low-level interface](#low-level-pointer-access).

* **operator =** - assignment operator. The vector needs to be the same length and underlying types must be convertible.
* **int length()** - returns the number of elements in a vector. This is part of the spec, and is the same as ```size()``` except it has a different return type.
* **operator []** - a generic way to access vector data. the ```x``` value is index 0, the ```y``` value is index 1, the ```z``` value is index 2, and the ```w``` value is index 3, assuming the vector is long enough to access those index values. Can be used for both reading and writing, assuming it isn't const or otherwise not allowed for writing.
* **set()** - this is the way of setting all the values for the vector at the same time. It takes the same number of scalar arguments as there are vector elements. This function is helpful at preventing trouble when there are potential aliasing problems.

### Vector Operators
The vector operators all work component-wise.

* unary ```operator +```
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
* ```floatBitsToInt()``` - requires ```std::bit_cast<>```
* ```floatBitsToUint()``` - requires ```std::bit_cast<>```
* ```doubleBitsToLongLong()``` - not in GLSL - requires ```std::bit_cast<>```
* ```doubleBitsToUlongLong()``` - not in GLSL - requires ```std::bit_cast<>```
* ```intBitsToFloat()``` - requires ```std::bit_cast<>```
* ```uintBitsToFloat()``` - requires ```std::bit_cast<>```
* ```longLongBitsToDouble()``` - not in GLSL - requires ```std::bit_cast<>```
* ```ulongLongBitsToDouble()``` - not in GLSL - requires ```std::bit_cast<>```
* ```fma()```
* ```frexp()```
* ```ldexp()```

#### Geometric Functions
The geometric functions treat a vector as an entity instead of as a collection of components. The vector component type must be floating-point.

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
template <floating_point_dimensional_scalar U, std::size_t Cols, std::size_t Rows>
constexpr basic_matrix(const basic_matrix<U, Cols, Rows> &arg) noexcept;
```

* **Single Matrix Argument, different type** - for matrices of the same dimensions, where the matrix type is implicitly convertible to the type of matrix you are trying to construct.

```c++
template <floating_point_dimensional_scalar U>
requires implicitly_convertible_to<U, T>
constexpr basic_matrix(const basic_matrix<U, C, R> &arg) noexcept;
```

* **Variable Arguments** - any combination of scalar values, vectors, and matrices can be arguments to the constructor, as long as there is enough data to initialize all the matrix elements, and as long as the types are convertible. It is fine if an argument has more data than necessary to complete the matrix initialization, as long as some of the argument data is used. It is an error to pass unused arguments:

```c++
// variadic constructor of scalar and vector arguments
template <typename ... Args>
constexpr basic_matrix(const Args & ...args) noexcept;
``` 

### Matrix Member Functions

These are the members that are not part of the [iterator interface](#iterators), the [tuple interface](#tuple-interface), or the [low-level interface](#low-level-pointer-access).

* **operator =** - assignment operator. The matrix needs to be the same size and underlying types must be convertible.
* **int length()** - returns the number of columns as an ```int```. This is part of the spec, and is the same as ```size()``` except it has a different return type.
* **int column_length()** - like ```length()``` but for rows. Returns the number of rows as an ```int```.
* **std::size_t size()** - returns the number of columns as as a ```std::size_t```.
* **std::size_t column_size()** - like ```size()``` but for rows. Returns the number of rows as a ```std::size_t```.
* **operator []** - a generic way to access matrix data. The values returned by this operator are the columns of the matrix, which are of type ```basic_vector```. Can be used for both reading and writing, assuming it isn't const or otherwise not allowed for writing. Along with ```operator []``` in ```basic_vector```, we can access individual matrix elements with notation such as **auto val = my_matrix[col][row];**.
* **row()** - this returns a ```basic_vector``` that represents a row of the matrix.

### Matrix Operators
The matrix operators all work component-wise, except for ```operator *```, which performs linear algebraic functions with vectors and other matrices.

* unary ```operator +```
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
