## dsga : Data Structures for Geometric Algorithms

**dsga** is a single header-only **C++20 library** that implements the **vectors** and **matrices** from the OpenGL Shading Language 4.6 specification ([pdf](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) | [html](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html)). It is inspired by the spec, but does deviate in some small ways, mostly to make it work well in C++20. It is intended to be used for [array programming](https://en.wikipedia.org/wiki/Array_programming) other than rendering. Our requirements in general are for things like 3D CAD/CAM applications and other geometric and algebraic things. See [motivation](docs/MOTIVATION.md) for more details. This library does _not_ use SIMD instructions or types under the hood, beyond whatever the compiler provides through optimization.

### Home
[https://github.com/davidbrowne/dsga](https://github.com/davidbrowne/dsga)

### Current Version
v3.3.1

### Usage

Use it more or less like you would use vectors and matrices in a shader program, but not necessarily for shading. We hope to be able to use it for rapid development of geometric algorithms. See the [examples](examples) directory.

The [documentation](docs/DOCUMENTATION.md) explains more about how the vector and matrix classes work. API is in [API](docs/API.md).

More in depth explanation can be found in the [details](docs/DETAILS.md).

### Some Quick Examples

``` c++
// get a 2D vector that is perpendicular (rotated 90 degrees counter-clockwise)
// to a 2D vector in the plane
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular(const dsga::vec<T, 2> &some_vec) noexcept
{
    auto cos90 = 0.0f;
    auto sin90 = 1.0f;

    // rotation matrix -- components in column major order
    return dsga::mat<T, 2, 2>(cos90, sin90, -sin90, cos90) * some_vec;
}

// same as above, different implementation
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular(const dsga::vec<T, 2> &some_vec) noexcept
{
    return dsga::vec<T, 2>(-1, 1) * some_vec.yx;
}
```

``` c++
// gives closest projection point from point to a line made from line segment p1 <=> p2
constexpr auto project_to_line(const dsga::dvec3 &point,
                               const dsga::dvec3 &p1,
                               const dsga::dvec3 &p2) noexcept
{
    auto hypot = point - p1;
    auto v1 = p2 - p1;
    auto t = dsga::dot(hypot, v1) / dsga::dot(v1, v1);

    return p1 + (t * v1);
}

// same as above, different implementation
constexpr auto project_to_line(const dsga::dvec3 &point,
                               const dsga::dvec3 &p1,
                               const dsga::dvec3 &p2) noexcept
{
    auto hypot = point - p1;
    auto v1 = p2 - p1;
    return p1 + dsga::outerProduct(v1, v1) * hypot / dsga::dot(v1, v1);
}
```

``` c++
//
// find the minimum positive angle between 2 vectors.
// 2D or 3D only.
//

template <dsga::floating_point_scalar T, std::size_t C>
requires ((C == 2) || (C == 3))
auto angle_between(const dsga::vec<T, C> &v1,
                   const dsga::vec<T, C> &v2)
{
    auto a = v1 * dsga::length(v2);
    auto b = v2 * dsga::length(v1);
    auto numerator = dsga::length(a - b);
    auto denominator = dsga::length(a + b);

    return T(2) * std::atan2(numerator, denominator);
}
```

```c++
//
// cross product
//

// cross product of vectors of length 3
template <dsga::floating_point_scalar T>
[[nodiscard]] constexpr auto cross(const dsga::vec<T, 3> &a,
                                   const dsga::vec<T, 3> &b) noexcept
{
    // CTAD gets us the type and size for the vector
    return dsga::vec((a[1] * b[2]) - (b[1] * a[2]),
                     (a[2] * b[0]) - (b[2] * a[0]),
                     (a[0] * b[1]) - (b[0] * a[1]));
}

// another approach using swizzles of vec type arguments
template <dsga::floating_point_scalar T>
[[nodiscard]] constexpr auto cross(const dsga::vec<T, 3> &a,
                                   const dsga::vec<T, 3> &b) noexcept
{
    return (a.yzx * b.zxy) - (a.zxy * b.yzx);
}
```

### Installation

This is a **single header library**, where you just need the file [dsga.hxx](include/dsga.hxx). You can just copy dsga.hxx, or you can get it installed via CMake. Things are defined in the ```dsga``` namespace. The types provided by this library can be seen summarized in the [documentation](docs/DOCUMENTATION.md).

Under the hood, we depend on the [cxcm](https://github.com/davidbrowne/cxcm) project for constexpr versions of some ```cmath``` functions. ```cxcm``` has been brought into ```dsga.hxx```, converted to a nested ```namespace detail::cxcm``` under ```namespace dsga```, so we don't need to also include the files from ```cxcm```.

This may be a single header library, but if Visual Studio is being used, we recommend to also get the [dsga.natvis](VS2026/dsga.natvis) file for debugging and inspecting vectors and matrices in the IDE.

This is a c++20 library, so that needs to be the minimum standard that you tell the compiler to use.

### Status

Current version: `v3.3.1`

* Everything major has some tests, but code coverage is not 100%.
* [Last Release: v3.0.0](https://github.com/davidbrowne/dsga/releases)
* [Change Log](CHANGELOG.md)

### Tested Compilers
#### Regularly Tested
* Microsoft Visual Studio 2026 v18.7
* gcc v16.1
* clang v22.1

#### Minimum Version
* Microsoft Visual Studio 2022 v17.x
* gcc v11.4
* clang v16.0

### Testing

All tests are currently 100% PASSING on all the testing platforms and compilers. See [PLATFORMS.md](PLATFORMS.md) for more details.

This project uses [doctest](https://github.com/onqtam/doctest) for testing. We occasionally use [nanobench](https://github.com/martinus/nanobench) for understanding implementation tradeoffs.

### [Latest Major Changes](CHANGELOG.md)
* v3.3.0
    * Removed `Writable` from classes/structs and member functions where it is always true.
    * Renamed concept `dimensional_size` to `vec_dimension`.
    * Created new concept `mat_dimension` for matrix row and column sizes.
    * Added new concepts `vec_scalar` and `vec_size` for vector element type and vector size.
    * Added new concept `vec_like` for vector-like types, which includes both `vec` and `swizzle_vec`.
* v3.2.0
    * Upgraded to cxcm v1.3.0.
    * Moved cxcm namespace from ```cxcm``` to ```detail::cxcm```.
    * Added two new examples
        * line_closest_points - finds the closest points between two lines in 3D space, and the distance between those points.
        * shader_demo - a simple shader demo that uses dsga for vector and matrix math, and demonstrates how to use the library in a shader-like context.
    * Removed ```as_base()``` from **vec_interface**, since it was not really necessary.
    * Added protected destructor to **vec_interface** to prevent deletion through a base class pointer.
* v3.1.0
    * Minor version bump with some breaking changes to the API.
    * Updated to doctest v2.5.2
    * Renamed the 5 main classes, deprecating the old names.
         * **basic_vector** -> **vec**
         * **basic_matrix** -> **mat**
         * **vector_base** -> **vec_interface**
         * **indexed_vector** -> **swizzle_vec**
         * **storage_vector** -> **vec_storage**
    * Microsoft Visual Studio 2022 will no longer be directly supported or updated. Testing may possibly occur using the VS2022 toolset on VS2026.
    * Removed deprecated function ```logicalNot()```, replaced by ```compNot()```.
    * CMake support rewritten.
         * Install target with ```cmake --install``` support.
         * ```dsga::dsga``` imported ```INTERFACE``` target for ```target_link_libraries```.
         * CMake config file for ```find_package``` support.
         * CMake target for linking with other projects.
         * Compiler flags for MSVC, gcc, and clang.
* v3.0.0
    * Major version bump with breaking changes to the API.
    * Removed the ```data()``` interface for all vector types and matrix types, in order to remove/mitigate the possibilty of pointer overruns.
    * Minor refactoring.

### License
[![BSL](https://img.shields.io/badge/license-BSL-blue)](https://choosealicense.com/licenses/bsl-1.0/)

```
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
```

This project uses the [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/).

#### Third Party Attribution 

The libraries we use (some just occasionally):

```
// cxcm - a c++20 library that provides constexpr versions of some <cmath> and related functions.
// https://github.com/davidbrowne/cxcm
//
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
```

```
// QD
// https://www.davidhbailey.com/dhbsoftware/
//
// Modified BSD 3-Clause License
//
// This work was supported by the Director, Office of Science, Division
// of Mathematical, Information, and Computational Sciences of the
// U.S. Department of Energy under contract number DE-AC03-76SF00098.
//
// Copyright (c) 2000-2007
//
// 1. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//   (1) Redistributions of source code must retain the copyright notice, this list of conditions and the following disclaimer.
//
//   (2) Redistributions in binary form must reproduce the copyright notice, this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//
//   (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory, U.S. Dept. of Energy nor the names of its contributors
//       may be used to endorse or promote products derived from this software without specific prior written permission.
//
// 2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
//    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
//    IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
//    OF THE POSSIBILITY OF SUCH DAMAGE.
//
// 3. You are under no obligation whatsoever to provide any bug fixes, patches, or upgrades to the features, functionality or performance of the
//    source code ("Enhancements") to anyone; however, if you choose to make your Enhancements available either publicly, or directly to Lawrence
//    Berkeley National Laboratory, without imposing a separate written license agreement for such Enhancements, then you hereby grant the following
//    license: a non-exclusive, royalty-free perpetual license to install, use, modify, prepare derivative works, incorporate into other computer
//    software, distribute, and sublicense such enhancements or derivative works thereof, in binary and source code form.
```

```
// doctest.h - the lightest feature-rich C++ single-header testing framework for unit tests and TDD
// https://github.com/doctest/doctest
//
// Copyright (c) 2016-2023 Viktor Kirilov
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
// Copyright (c) 2019-2023 Martin Leitner-Ankerl <martin.ankerl@gmail.com>
```
