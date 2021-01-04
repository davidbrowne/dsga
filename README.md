# dsga: Data Structures for Geometric Algorithms

dsga is a c\+\+20 library that implements/will implement the vectors and matrices from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). This library was not intended to be used for rendering. My requirements in general are for things like 3D CAD/CAM applications.

## Quick Peek

``` c++
auto perpendicular(const vec2 &some_vec)
{
    return some_vec.yx * vec2(-1, 1);
}

auto my_dot(const dvec3 &v1, const dvec3 &v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

// no coincident point or collinear checking
auto project_point_to_line(const dvec3 &p0, const dvec3 &p1, const dvec3 &point_in_space)
{
    auto v1 = p1 - p0;
    auto vq = point_in_space - p0;
    auto t = my_dot(vq, v1) / my_dot(v1, v1);

    return p1 + t * v1;
}

```

## Installation

Currently this is a single header library. All you need to do is include [dsga.hxx](https://raw.githubusercontent.com/davidbrowne/dsga/main/dsga.hxx). The functions are in the ```dsga``` namespace.

It will eventually depend on [cxcm.hxx](https://raw.githubusercontent.com/davidbrowne/cxcm/main/cxcm.hxx) where the functions are in the [cxcm](https://github.com/davidbrowne/cxcm) namespace. This dependency will occur once the vector and matrix functions are implemented.


## Motivation

I wanted to expand the point/vector class that we use at work. We have x, y, and z data members, but that is the only way to get at the data. I wanted something more flexible that would use contiguous memory like an array, but still be useful with x, y, z data member access. I specifically did *NOT* want accessor functions as the way to get x, y, and z, using those names.

After reading a [blog](https://t0rakka.silvrback.com/simd-scalar-accessor) about this very type of issue, I started thinking more about this, and how to deal with unions and their common initial sequence. I saw some confusion about the common initial sequence, and I based my implementation on what I read in these discussions:
* [Language lawyers: unions and "common initial sequence"](https://www.reddit.com/r/cpp_questions/comments/7ktrrj/language_lawyers_unions_and_common_initial/)
* [Union common initial sequence with primitive](https://stackoverflow.com/questions/43655657/union-common-initial-sequence-with-primitive)
* [Do scalar members in a union count towards the common initial sequence?](https://stackoverflow.com/questions/48209179/do-scalar-members-in-a-union-count-towards-the-common-initial-sequence)
* [Are there any guarantees for unions that contain a wrapped type and the type itself?](https://stackoverflow.com/questions/48058545/are-there-any-guarantees-for-unions-that-contain-a-wrapped-type-and-the-type-its)

Once I understood these issues, I decided I wanted to try and get as much vector and matrix functionality I could with this approach. That is why in the end I decided to implement the vectors and matrices from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). dsga's implementation doesn't care about data-packing or rendering.

I also wanted to learn more about c\+\+20. I was interested in learning git (been using subversion for 20 years) and how to create a public repo. This project is the result.

## Status

Current version: `v0.1.0`

Implemented:

Almost all of the vector class support is implemented. Still missing:

* Tests for operators, assignments, and compound assignments
* Some constructor test cases
* The vector functions implementation and their tests

Implementation not yet started:

* The matrix classes, their interactions with vectors, and their tests.

## Usage

Use it more or less like you would use vectors in a shader program, but not necessarily for shading.

The following types are pretty much what you expect, but there is a 1D version of a vector that we suffix as "scal" for scalar. It helps with keeping things interoperating, and it provides a way to swizzle a supposed "scalar" value:

``` c++
//
// specialized using types
//

// this 1D vector is a swizzlable scalar -- analog to glsl primitive types
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

// also float vectors, but using the common naming convention (not in glsl)
using fscal = vectype1<float>;
using fvec2 = vectype2<float>;
using fvec3 = vectype3<float>;
using fvec4 = vectype4<float>;

// double vectors
using dscal = vectype1<double>;
using dvec2 = vectype2<double>;
using dvec3 = vectype3<double>;
using dvec4 = vectype4<double>;
```

## How It Works

There are really two vector classes: ```basic_vector``` and ```indexed_vector```. ```basic_vector``` is what you would normally think of as a contiguously stored vector/point representation. ```indexed_vector``` is a view on ```basic_vector```, which may only be modified under certain conditions.

A ```basic_vector``` has data members that provide "swizzling". These data members are of type ```indexed_vector```, and this is where they are a view on the owning ```basic_vector```. Only the ```indexed_vector```s that do not have duplicate indexes in the swizzle are modifiable, e.g., ```foo.xzy``` is modifiable, while ```foo.zzy``` is not modifiable. Either way, an ```indexed_vector``` from a swizzle has a lifetime tied to the lifetime of the ```basic_vector``` it came from.

We want to use both types of vectors in the same way, for constructors, equality comparison, assignment, operators, compound assignment, vector functions, etc. Instead of duplicating this effort, ```basic_vector``` and ```indexed_vector``` derive from a [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) base class called ```vec_base```, and this is now the foundation for constructors, equality comparison, assignment, operators, compound assignment, vector functions, etc:
![vec_base](./vec_base_uml.svg)

```vec_base``` carries the following information:
* Whether it can be used as an lvalue, i.e., is it writable
* The type of the data in the vector (some arithmetic type)
* How many elements are in the vector (1-4)
* The type of the derived class

It provides 3 functions that can be used to build up the constructors, equality comparisons, assignment operators, binary and unary operators, compound assignment operators, vector functions, etc:
* ```set()``` - calls ```static_cast<Derived *>(this)->init()```, which sets all the data in the vector to new values. Since this modifies the data, it is only enabled if it can be used as an lvalue. This function helps prevent aliasing issues that might occur otherwise, e.g., ```foo = foo.zyx;``` could have a problem with a naive implementation.
* ```operator[]``` - calls ```static_cast<Derived *>(this)->at()```, which is an lvalue reference to a single data value. Since this can modify the data, it is only enabled if it can be used as an lvalue.
* ```operator[] const``` - calls ```static_cast<Derived *>(this)->at() const```, which is a const lvalue reference to a single data value. This is read-only, so it is always available.


## Testing

This project uses [doctest](https://github.com/onqtam/doctest) for testing. The tests have been run on:

* MSVC 2019 - v16.9 Preview 1 or higher (required for constexpr with anonymous unions)

```
===============================================================================
[doctest] test cases:   54 |   54 passed | 0 failed | 0 skipped
[doctest] assertions: 1210 | 1210 passed | 0 failed |
[doctest] Status: SUCCESS!
```

The official tests have not been run with these compilers, but the project compiled without error with these (via [Compiler Explorer](https://godbolt.org/)):

* gcc 10.2
* clang 11.0

It could work on earlier versions of gcc and clang, and it certainly should work on later versions. MSVC requires v16.9 Preview 1 for VS2019 at a minimum.

## Similar Projects

It is a common pastime for people to write these kind of vector libraries. The three I wanted to mention here are:

* [glm](https://github.com/g-truc/glm) - popular long lived project that is similar in goals with respect to being based on OpenGL Shading Language specification, but is much more mature. It will work with c\+\+98, while dsga is for c\+\+20.
* [DirectXMath](https://github.com/microsoft/DirectXMath) - this is from Microsoft and basically performs the same role as glm, but with DirectX instead of OpenGL. It is also long lived and much more mature than dsga.
* [mango](https://github.com/t0rakka/mango) - this is the project that I read the blog about for vector component access and swizzling, so it is nice to have as another example. Again, more mature than dsga.

## License [![BSL](https://img.shields.io/badge/license-BSL-blue)](https://choosealicense.com/licenses/bsl-1.0/)

This project uses the [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/).