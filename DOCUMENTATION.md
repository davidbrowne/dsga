# Documentation

Jump to [API](#api).

This single header library aims to provide the basic functionality of the types and functions in the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). We are only interested in the vector and matrix types, along with the corresponding functions that operate on these types. We don't support the language as a whole, just the data types, and that is so we can develop geometric algebraic algorithms. The types are very flexible, and they can be used for a natural rapid prototyping environment for ```c++```.

Here we provide the documentation on what is in the specification, and also some information on how we differ from the specification. To see more about how this works in the context of ```c++20```, have a look at the [details](DETAILS.md) for more information.

## Types and Functions

This is an approximation of how we "export" the specific types and function to the top-level namespace. These types are all different based on their sizes and the types of data they hold. They can be dealt with more generically, but for that information see the [details](DETAILS.md) page.

``` c++
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
// bring the vector and matrix free functions into the global namespace
//

using namespace dsga::functions;
```

## Vector Types

In [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), the various dimensions of the vectors and matrices are between 2 and 4, inclusive. For ```dsga```, this is true for the matrices, but for the vectors we also length 1. We don't call them "vectors", but we suffix their type with "scal" for scalar, as opposed to a suffix of "vec1".

We have gone against the specification for a few reasons. Having length 1 vectors is a good way of dealing with how [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) has modifed the basic types, e.g., float, double, int. In GLSL, these basic types are not the same as they are in ```c++```. They behave as if they are vectors of length 1, including [swizzling](#swizzling). Since we can't change the basic types, we provide the "scalar" analog vector type that mimics what happens, e.g., ```bscal```, ```iscal```, ```fscal```, ```dscal```.

Since these "scalar" types are really vectors of length 1, they can use the vector functions. The functions don't discriminate based on size in dsga. In the cubic bezier evaluator example, we call the vector function ```mix()``` several times, and the last time is on vectors of length 1. If we decided to not allow vector functions on "scalar" types like fscal, then in this situation we could have called ```std::lerp()``` for the length 1 vector case.

### Swizzling

[Swizzling](https://en.wikipedia.org/wiki/Swizzling_(computer_graphics)) is the act of taking a vector and creating a new vector from it that is a specialized "view" on the original vector. The "swizzle" of a vector is itself a vector, although of different sort, and can be mostly used like a non-swizzle. If there is ever a problem, just wrap up the swizzle in a vector contructor:

```c++
vec4 big_vec;
...
// if you have a problem using the swizzle directly, construct a new vector with it
vec3 smaller_vec(big_vec.zyx);
```

Swizzling uses dot notation, e.g., ```foo.xy, bar.zw, baz.xxyy```. This gives you a type of vector that is a view on the data of the original vector. The swizzles are part of the original vector, and they have the same lifetime. The "x" index means the first value in the vector, "y" means the second, "z" means the third, and "w" means the fourth, so "xyzw" are the possible values in a swizzle, depending on the size of the original vector. In [GLSL](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), there are 3 different domains for swizzling: ```xyzw```, ```rgba```, and ```stpq```. We use "xyzw" when talking about spatial coordinates, "rgba" when talking about color coordinates, and "stpq" when talking about texture coordinates. Since dsga is intended for geometry and algebra, we only bothered supporting **xyzw** for swizzling.

A 1-dimensional vector can only refer to "x", but it can do so up to 4 times in a swizzle:
```c++
fscal length_one_vec;
...
auto length_four_vec = vec4(length_one_vec.xxxx);
```

Similarly, 2-dimensional vectors can refer to combinations of "xy", 3-dimensional vectors can refer to combinations of "xyz", and 4-dimensional vectors can refer to combinations of "xyzw". Since the maximum size of a vector is 4, that is the maximum number of swizzle characters you can use.

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

## Matrix Types

As noted above, matrices each have between 2-4 rows and 2-4 columns, giving 9 possible matrix sizes. The components of the matrices must be floating point types. The matrices store things in column major order, and the type naming reflects that. It can be confusing to read since that is the opposite of the mathematical notation for matrices. The set of columns is represented as an array of floating point vectors. The columns of the matrix are accessible via array notation, i.e., ```operator []```. The rows of the matrix are accessible via the ```template row<N>()``` function. Any component of a matrix ```A``` can be accessed by two adjacent ```operator []``` calls, such as ```A[col_num][row_num]```.

For an example of GLSL type names vs. math notation, ```mat4x2``` is a matrix with 4 columns with 2 rows, but math notation specifies the number of rows first, followed by number of columns. So the GLSL type ```mat4x2``` is a 2x4 matrix using math notation ("m by n" which is "rows by columns").

The matrix types are very generic. One can pre-mulitply (matrix on left, vector on right), post-multiply (vector on left, matrix on right), treat square matrices that are meant to represent transformations as left-handed or right-handed, etc. There is no default preferred interpretation, although the user may have a preferred approach to using matrices.

## API

It is difficult to give a straightforward list of all the functions in the vector and matrix structs. First, there are many different classes for different sized vectors, although each has roughly the same API. Second, we specialize the vectors and matrices based on size and type. Third, the function signatures are pretty difficult to read, as they:

* are generic, which means they usually use base classes
* use templates
* use concepts
* have many different versions

We have gone to the trouble of [enumerating all the specific classes](#types-and-functions) we support in the above section on types and functions, and there are a lot of them. Some functions take as arguments or return as values some general types, e.g., floating-point vector, as opposed to saying vec2, vec3, vec4, dvec2, dvec3, or dvec4 (and possibly even fscal and dscal). So the question becomes how we represent the generic categories in the documentation for the API. We can follow what GLSL did, and that may be the best approach for the vector types:

* **genIType** - int, iscal, ivec2, ivec3, ivec4, long long, llscal, llvec2, llvec3, llvec4
* **genUType** - unsigned, uscal, uvec2, uvec3, uvec4, unsigned long long, ullscal, ullvec2, ullvec3, ullvec4
* **genBType** - bool, bscal, bvec2, bvec3, bvec4
* **genFType** - float, scal, vec2, vec3, vec4, fscal, fvec2, fvec3, fvec4
* **genDType** - double, dscal, dvec2, dvec3, dvec4

Sometimes we just want to say integral types or floating-point types. In those cases we will list options for multiple categories.

We don't want to repeat what is in the [GLSL spec](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf), but a brief description should be beneficial.

#### Iterators

Both the vector and matrix structs support **begin()/cbegin()** and **end()/cend()** in order to provide basic ```iterator``` support through non-const and const iterators. This gives us access to:

* Standard Library Algorithms
* [Range-based for loop](https://en.cppreference.com/w/cpp/language/range-for) support 

#### Tuple Interface

Both the vector and matrix structs support **std::tuple_element<>**, **std::tuple_size<>** and **get<>** in order to provide basic ```std::tuple``` support. This gives us access to:

* Data structures in same manner as ```tuple```s do
* [Structured Binding support](https://en.cppreference.com/w/cpp/language/structured_binding)

#### Low-level Pointer Access

Both the vector and matrix structs support **data()** and **size()** in order to provide pointer access to the underlying data. *Hopefully*, no one wants to use pointer data to manipulate or access the data structures, but this method exists if it is deemed appropriate.

* ```data()``` gives a pointer to the underlying vector elements or matrix columns
* ```size()``` gives the number of elements in the vector or number of columns in the matrix.

### Vector

```c++
// length 1 vectors/scalar hybrids
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

These length 1 vectors are not officially in GLSL, but they did change their fundamental types to behave as if they are. We try to automatically convert from these to the underlying scalar type when we can. If there is a problem, just cast it to the underlying type.

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

#### Constructors
* 
* 
* 
#### Operators
#### Functions

### Matrix

#### Constructors
* 
* 
* 
#### Operators
#### Functions
