# dsga : Data Structures for Geometric Algorithms

dsga is a c\+\+20 library that implements the vectors and matrices from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). This library was not intended to be used for rendering. My requirements in general are for things like 3D CAD/CAM applications and other geometric things.

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

Currently this is a single header library with a single file dependency, so I guess a two header library. All you need to do is include [dsga.hxx](https://raw.githubusercontent.com/davidbrowne/dsga/main/dsga.hxx). The functions are in the ```dsga``` namespace.

We now depend on [cxcm.hxx](https://raw.githubusercontent.com/davidbrowne/cxcm/main/cxcm.hxx) where the functions are in the [cxcm](https://github.com/davidbrowne/cxcm) namespace. A copy of this file is in this repository.

## Motivation

I wanted to expand the point/vector class that we use at work. We have x, y, and z data members, but that is the only way to get at the data. I wanted something more flexible that would use contiguous memory like an array, but still be useful with x, y, z data member access. I specifically did *NOT* want accessor functions as the way to get x, y, and z data. I wanted data member access.

After reading a [blog article](https://t0rakka.silvrback.com/simd-scalar-accessor) about this very type of issue, I started thinking more about this, and once I better understood anonymous unions and the common initial sequence concept (since verified with ```std::is_corresponding_member<>```), I was ready to write this library.

I decided that instead of limiting my swizzling to just x, y, and z values, I would go all the way and try and get as much vector and matrix functionality I could with this approach. That is why in the end I decided to implement the vectors and matrices from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). dsga's implementation doesn't care about data-packing or rendering.

I also wanted to learn more about c\+\+20. I was interested in learning git (been using subversion for 20 years) and how to create a public repo. This project is the result.

## Status

Current version: `v0.4.1`

* All the vector and matrix functionality is implemented.
* First pass at test coverage. Everything major has some test, but code coverage is not 100%. 

### The next steps
* Official single header release. ```cxcm.hxx``` has been merged into ```dsga.hxx```, but it needs some testing before we make the first official release of dsga.
* Documentation. Currently, the documentation that is offered is the source code and tests, this README page, and the GLSL specification.
* Example projects. Need small, medium, and large examples.

## Usage

Use it more or less like you would use vectors and matrices in a shader program, but not necessarily for shading.

The following types are pretty much what you expect, but there is a 1D version of a vector that we suffix as ```scal``` for scalar. It helps with keeping things interoperating, and it provides a way to swizzle a supposed "scalar" value:

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
```

## How The Vectors Work

There are really two vector classes: ```basic_vector``` and ```indexed_vector```. ```basic_vector``` is what you would normally think of as a contiguously stored vector/point representation. ```indexed_vector``` is a view on ```basic_vector```, which may only be modified under certain conditions.

A ```basic_vector``` has data members that provide "swizzling". These data members are of type ```indexed_vector```, and this is where they are a view on the owning ```basic_vector```. Only the ```indexed_vector```s that do not have duplicate indexes in the swizzle are modifiable, e.g., ```foo.xzy``` is modifiable, while ```foo.zzy``` is not modifiable. Either way, an ```indexed_vector``` from a swizzle has a lifetime tied to the lifetime of the ```basic_vector``` it came from.

We want to use both types of vectors in the same way, for constructors, equality comparison, assignment, operators, compound assignment, vector functions, etc. Instead of duplicating this effort, ```basic_vector``` and ```indexed_vector``` derive from a [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) base class called ```vector_base```, and this is now the foundation for constructors, equality comparison, assignment, operators, compound assignment, vector functions, etc:
![vec_base](./vec_base_uml.svg)

```vector_base``` carries the following information:
* Whether it can be used as an lvalue, i.e., is it writable
* The type of the data in the vector (some arithmetic type)
* How many elements are in the vector (1-4), i.e., the ```Count```
* The type of the derived class

It provides the following functions that can be used to generically manipulate and access vector data:
* ```set()``` - relies on ```init()```, which sets all the data in the vector to new values. Since this modifies the data, it is only enabled if it is writable. This function helps prevent aliasing issues that might occur otherwise, e.g., ```foo = foo.zyx;``` could have a problem with a naive implementation.
* ```operator[]``` - relies on ```at()```, which is a reference to a single data value. If writable then can use as an lvalue. The data is in logical order.
* ```data()``` - provides pointer to data access via ```raw_data()```. If it is writable, then can use pointer to write data. Pointer access is in physical order.
* ```sequence()``` - relies on ```make_sequence_pack()```. The physical order to logical order mapping in a parameter pack.
*  ```length()``` - relies on ```Count``` template parameter, and it returns type ```int```.
* ```size()``` - relies on ```Count``` template parameter, and it returns type ```std::size_t```.

## Testing

This project uses [doctest](https://github.com/onqtam/doctest) for testing. I occasionally use [nanobench](https://github.com/martinus/nanobench) for understanding implementation tradeoffs.

The tests have been run on:

* MSVC 2019 - v16.10

```
[doctest] doctest version is "2.4.6"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   79 |   79 passed | 0 failed | 0 skipped
[doctest] assertions: 1806 | 1806 passed | 0 failed |
[doctest] Status: SUCCESS!
```

The following run all the unit tests except where there is lack of support for ```std::is_corresponding_member<>``` or where there is lack of support for ```std::bit_cast<>()```, and these are protected with feature test macros:

* clang 12.0.0 on Windows, [official binaries](https://github.com/llvm/llvm-project/releases/tag/llvmorg-12.0.0), with MSVC installed (uses MSVC standard library, so has ```std::bit_cast<>()```)

```
[doctest] doctest version is "2.4.6"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   79 |   79 passed | 0 failed | 0 skipped
[doctest] assertions: 1790 | 1790 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* gcc 10.3 on Windows, [tdm-gcc](https://jmeubank.github.io/tdm-gcc/) distribution (no ```std::bit_cast<>()```), last successful build:

```
[doctest] doctest version is "2.4.6"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   79 |   79 passed | 0 failed | 0 skipped
[doctest] assertions: 1782 | 1782 passed | 0 failed |
[doctest] Status: SUCCESS!
```

## Similar Projects

It is a common pastime for people to write these kind of vector libraries. The three I wanted to mention here are:

* [glm](https://github.com/g-truc/glm) - popular long lived project that is similar in goals with respect to being based on OpenGL Shading Language specification, but is much more mature. It will work with c\+\+98, while dsga is for c\+\+20.
* [DirectXMath](https://github.com/microsoft/DirectXMath) - this is from Microsoft and basically performs the same role as glm, but with DirectX instead of OpenGL. It is also long lived and much more mature than dsga.
* mango (repo has been removed by owner) - this is the project that I read the blog about for vector component access and swizzling, so it is nice to have as another example. Again, more mature than dsga.

## License [![BSL](https://img.shields.io/badge/license-BSL-blue)](https://choosealicense.com/licenses/bsl-1.0/)

```
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
```

This project uses the [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/).

### Third Party Attribution 

The libraries we use (some just occasionally):

```
// cxcm - a c++20 library that provides constexpr versions of some <cmath> and related functions.
//
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
```

```
// doctest.h - the lightest feature-rich C++ single-header testing framework for unit tests and TDD
//
// Copyright (c) 2016-2021 Viktor Kirilov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
```

```
// Microbenchmark framework for C++11/14/17/20
// https://github.com/martinus/nanobench
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2021 Martin Ankerl <martin.ankerl@gmail.com>
```
