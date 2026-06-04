# dsga : Data Structures for Geometric Algorithms

**dsga** is a single header-only **C++20 library** that implements the **vectors** and **matrices** from the OpenGL Shading Language 4.6 specification ([pdf](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) | [html](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html)). It is inspired by the spec, but does deviate in some small ways, mostly to make it work well in C++20. It is intended to be used for [array programming](https://en.wikipedia.org/wiki/Array_programming) other than rendering. Our requirements in general are for things like 3D CAD/CAM applications and other geometric and algebraic things. See [motivation](docs/MOTIVATION.md) for more details. This library does _not_ use SIMD instructions or types under the hood, beyond whatever the compiler provides through optimization.

## Home
[https://github.com/davidbrowne/dsga](https://github.com/davidbrowne/dsga)

## Current Version
v3.1.0

## Note
Use v3.0.0 for the latest stable release version. The next stable release will be v4.0.0. The versions between v3.0.0 and v4.0.0 are for testing out breaking changes to the API and for working on the documentation, so they may be unstable. Suggest that users either get v3.0.0 or wait till the release version v.4.0.0 and not update to the versions inbetween.

## Future Plans
* Doxygen documentation.
* Removal of API.md once we have the Doxygen documentation.
* Additional example classes built on top of the core library.
* Release v4.0.0 with more breaking changes to the API and the addition of the new documentation.
* Add more tests and try to get close to 100% code coverage.

## [Latest Major Changes](docs/CHANGELOG.md)
* v3.1.0
    * Minor version bump with some breaking changes to the API.
    * Updated to doctest v2.5.2
    * Renamed the 5 main classes, deprecating the old names.
         * **basic_vector** -> **vec**
         * **basic_matrix** -> **mat**
         * **vector_base** -> **vec_interface**
         * **indexed_vector** -> **swizzle_vec**
         * **storage_vector** -> **vec_storage**
    * Microsoft Visual Studio 2022 will no longer be directly supported. Testing may possibly occur using the VS2022 toolset on VS2026.
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

## Tested Compilers
### Regularly Tested
* Microsoft Visual Studio 2026 v18.6
* gcc v15.2
* clang v22.1

### Minimum Version
* Microsoft Visual Studio 2022 v17.x
* gcc v11.4
* clang v16.0

## Contents
* [Some Quick Examples](#some-quick-examples)
* [Relevant GLSL Overview](#relevant-glsl-overview)
* [Implemented Interfaces](#implemented-interfaces)
* [General documentation](docs/DOCUMENTATION.md)
* [Detailed API documentation](docs/API.md)
* [```dsga``` Implementation Details](docs/DETAILS.md)
* [Installation](#installation)
* [Status](#status)
* [Usage and Documentation](#usage)
* [Testing](#testing)
* [Similar Projects](#similar-projects)
* [License](#license)
* [Third Party Attribution](#third-party-attribution)

## Some Quick Examples

``` c++
// get a 2D vector that is perpendicular (rotated 90 degrees counter-clockwise)
// to a 2D vector in the plane
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular1(const dsga::vec<T, 2> &some_vec) noexcept
{
    auto cos90 = 0.0f;
    auto sin90 = 1.0f;

    // rotation matrix -- components in column major order
    return dsga::mat<T, 2, 2>(cos90, sin90, -sin90, cos90) * some_vec;
}

// same as above, different implementation
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular2(const dsga::vec<T, 2> &some_vec) noexcept
{
    return dsga::vec<T, 2>(-1, 1) * some_vec.yx;
}
```

``` c++
// gives closest projection point from point to a line made from line segment p1 <=> p2
constexpr auto project_to_line1(const dsga::dvec3 &point,
                                const dsga::dvec3 &p1,
                                const dsga::dvec3 &p2) noexcept
{
    auto hyp = point - p1;
    auto v1 = p2 - p1;
    auto t = dsga::dot(hyp, v1) / dsga::dot(v1, v1);

    return p1 + (t * v1);
}

// same as above, different implementation
constexpr auto project_to_line2(const dsga::dvec3 &point,
                                const dsga::dvec3 &p1,
                                const dsga::dvec3 &p2) noexcept
{
    auto hyp = point - p1;
    auto v1 = p2 - p1;
    return p1 + dsga::outerProduct(v1, v1) * hyp / dsga::dot(v1, v1);
}
```

``` c++
//
// evaluate a 2D cubic bezier curve at t
//

#if LINEAR_INTERPOLATE

// cubic bezier linear interpolation, one ordinate at a time, e.g., x, y, z, or w
// very slow implementation (de Casteljau algorithm), but illustrates the library
constexpr auto single_ordinate_cubic_bezier_eval(const dsga::vec4 &cubic_control_points, float t) noexcept
{
    auto quadratic_control_points = dsga::mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
    auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
    return dsga::mix(linear_control_points.x, linear_control_points.y, t);
}

#else

// ~10-25x faster - Bernstein polynomials
constexpr auto single_ordinate_cubic_bezier_eval(const dsga::vec4 &cubic_control_points, T t) noexcept
{
    auto t_complement = T(1) - t;
    return
        t_complement * t_complement * t_complement * cubic_control_points[0] +
        T(3) * t * t_complement * t_complement * cubic_control_points[1] +
        T(3) * t * t * t_complement * cubic_control_points[2] +
        t * t * t * cubic_control_points[3];
}

#endif

// main cubic bezier eval function -- takes 2D control points with float values.
// returns the 2D point on the curve at t
constexpr auto simple_cubic_bezier_eval(dsga::vec2 p0, dsga::vec2 p1, dsga::vec2 p2, dsga::vec2 p3, float t) noexcept
{
    // each control point is a column of the matrix.
    // the rows represent x coords and y coords.
    auto AoS = dsga::mat4x2(p0, p1, p2, p3);

    // lambda pack wrapper -- would be better solution if vector size was generic
    return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
    {
        return dsga::vec2(single_ordinate_cubic_bezier_eval(AoS.row(Is), t)...);
    }(std::make_index_sequence<2>{});
}
```

``` c++
//
// find the minimum positive angle between 2 vectors and/or indexed vectors (swizzles).
// Uses base class for vector types to be inclusive to both types.
// 2D or 3D only.
//

template <bool W1, dsga::floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C > 1) && (C < 4))
auto angle_between(const dsga::vec_interface<W1, T, C, D1> &v1,
                   const dsga::vec_interface<W2, T, C, D2> &v2)
{
    auto a = v1 * dsga::length(v2);
    auto b = v2 * dsga::length(v1);
    auto numerator = dsga::length(a - b);
    auto denominator = dsga::length(a + b);

    if (numerator == T(0))
        return T(0);
    else if (denominator == T(0))
        return std::numbers::pi_v<T>;

    return T(2) * std::atan(numerator / denominator);
}
```

``` c++
//
// STL file format read/write helpers
//

// make sure data has no infinities or NaNs
constexpr bool definite_coordinate_triple(const dsga::vec3 &data) noexcept
{
    return !(dsga::any(dsga::isinf(data)) || dsga::any(dsga::isnan(data)));
}

// make sure normal vector has no infinities or NaNs and is not the zero-vector { 0, 0, 0 }
constexpr bool valid_normal_vector(const dsga::vec3 &normal) noexcept
{
    return definite_coordinate_triple(normal) && dsga::any(dsga::notEqual(normal, dsga::vec3(0)));
}

// not checking for positive-only first octant data -- we are allowing zeros and negative values
constexpr bool valid_vertex_relaxed(const dsga::vec3 &vertex) noexcept
{
    return definite_coordinate_triple(vertex);
}

// strict version where all vertex coordinates must be positive-definite
constexpr bool valid_vertex_strict(const dsga::vec3 &vertex) noexcept
{
    return definite_coordinate_triple(vertex) && dsga::all(dsga::greaterThan(vertex, dsga::vec3(0)));
}

// right-handed unit normal vector for a triangle facet,
// inputs are triangle vertices in counter-clockwise order
constexpr dsga::vec3 right_handed_normal(const dsga::vec3 &v1, const dsga::vec3 &v2, const dsga::vec3 &v3) noexcept
{
    return dsga::normalize(dsga::cross(v2 - v1, v3 - v1));
}
```

```c++
//
// cross product
//

// arguments are of the vec_interface class type, so both swizzle_vec and vec types (of length 3) can be used.
template <bool W1, dsga::floating_point_scalar T1, typename D1, bool W2, dsga::floating_point_scalar T2, typename D2>
[[nodiscard]] constexpr auto cross(const dsga::vec_interface<W1, T1, 3, D1> &a,
                                   const dsga::vec_interface<W2, T2, 3, D2> &b) noexcept
{
    // CTAD gets us the type and size for the vector
    return dsga::vec((a[1] * b[2]) - (b[1] * a[2]),
                     (a[2] * b[0]) - (b[2] * a[0]),
                     (a[0] * b[1]) - (b[0] * a[1]));
}

// another approach using swizzles of vec type arguments
template <dsga::floating_point_scalar T1, dsga::floating_point_scalar T2>
[[nodiscard]] constexpr auto cross(const dsga::vec<T1, 3> &a,
                                   const dsga::vec<T2, 3> &b) noexcept
{
    return (a.yzx * b.zxy) - (a.zxy * b.yzx);
}
```
## Relevant GLSL Overview

Our programming environment is ```C++20```, not a GLSL shader program, so the entire GLSL Shading language specification is a super-set of what we are trying to achieve. We really just want the vector and matrix data structures (and their corresponding functions and behavior) to be usable in a ```C++20``` environment. Another term for this type of programming is [array programming](https://en.wikipedia.org/wiki/Array_programming).

The following links to the shading specification should help with understanding what we are trying to implement with this header-only library.

* [Variables and Types](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#variables-and-types)
    * [Basic Types](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#basic-types): we have added support for vectors to also hold values of type ```std::size_t```, ```unsigned long long``` (which is what ```std::size_t``` really is for x64), and ```signed long long```.
    * [Vectors](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vectors): GLSL does not have 1-dimensional vectors, but we do, which we have using directives to give them names that describe them as scalars and not as vectors, e.g., ```dsga::iscal```, ```dsga::dscal```, ```dsga::bscal```. We support 1-dimensional vectors because GLSL does something special with the fundamental types, allowing them to be swizzled. We use the 1-dimensional vectors to mimic that ability.
      ```glsl
      // glsl
      double value = 10.0;
      dvec3 swizzled_value = value.xxx; 

      // dsga
      // dscal is an alias for dsga::vec<double, 1>
      dsga::dscal value = 10.0;
      dsga::dvec3 swizzled_value = value.xxx; 
      ```

      1-dimensional vectors types are also the return type for single component swizzles, e.g., ```val.x```, ```val.y```, ```val.z```, ```val.w```. They are designed to be easily convertible to the underlying type of the vector elements.
    * [Matrices](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#matrices)
* [Operators and Expressions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#operators-and-expressions)
    * [Vector and Matrix Constructors](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vector-and-matrix-constructors)
    * [Vector and Scalar Components and Length](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vector-components): we only allow swizzling with the ```{ x, y, z, w }``` component names. Support for ```{ r, g, b, a }``` and ```{ s, t, p , q }``` has not been implemented.

      In addition, you cannot swizzle a swizzle. I am currently unclear if this is a constraint of the specification, but it is a constraint of dsga's implementation:
      ```c++
      auto my_vec = dsga::vec3(10, 20, 30);
      auto double_swiz = my_vec.zxy.x;           // error: no such data member x
      auto swiz = my_vec.zxy;                    // swizzle type is not dsga::vec3
      auto swiz_again = swiz.x;                  // error: no such data member x
      auto try_swiz_again = dsga::vec3(swiz).x;  // wrapping with dsga::vec3 works
      dsga::vec3 swiz_reborn = my_vec.zxy;       // dsga::vec3 constructor from swizzle
      auto and_swiz_again = swiz_reborn.x;       // works
      ```
    * [Matrix Components](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#matrix-components)
    * [Assignments](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#assignments)
    * [Expressions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#expressions)
    * [Vector and Matrix Operations](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vector-and-matrix-operations)
 
* [Built-In Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#built-in-functions): we support the additional types ```std::size_t```, ```unsigned long long```, and ```signed long long``` in the functions where appropriate. We also added bit conversion functions between these 64-bit integral types and ```double```.

  We also support using ```double``` for all the functions where ```float``` is supported, with the exception of the bit conversion functions for ```float``` with 32-bit integral types, and ```double``` with the 64-bit integral types.
    * [Angle and Trigonometry Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#angle-and-trigonometry-functions): there are also scalar versions of these functions, but where c++ does the same thing, it might be easier to use the ```std::``` version instead of the ```dsga::``` version.
    * [Exponential Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#exponential-functions): there are also scalar versions of these functions, but where c++ does the same thing, it might be easier to use the ```std::``` version instead of the ```dsga::``` version.

    * [Common Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#common-functions): there are also scalar versions of these functions, but where c++ does the same thing, it might be easier to use the ```std::``` version instead of the ```dsga::``` version.
    * [Geometric Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#geometric-functions): ```ftransform()``` is not implemented as it is only for GLSL vertex shader programs.
    * [Matrix Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#matrix-functions)
    * [Vector Relational Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vector-relational-functions): GLSL has a vector function ```not()```, but ```not``` is a c++ keyword. Instead of naming this function ```not()```, we name it ```compNot()```.

      In addition, we have added the non-GLSL convenience function ```none()```, which returns ```!any()```.

## Implemented Interfaces

To make the vectors and matrices as useful as possible in a C++ context, various C++ customization points were implemented or interfaces partially emulated, e.g., ```std::valarray<>```. There are many options for data access. For ```dsga``` vectors and matrices, we have:

* Swizzle access like GLSL (vec only, not swizzle_vec)
    * Only from the set of { x, y, z, w }, e.g., ```foo.wyxz```
* ```std::tuple``` protocol, structured bindings
    * ```get```
    * ```tuple_size```
    * ```tuple_element```
* Iterator access, ranges, range-for loop
    * ```begin```
    * ```cbegin```
    * ```rbegin```
    * ```crbegin```
    * ```end```
    * ```cend```
    * ```rend```
    * ```crend```
* Index access (logical)
    * ```operator []```
    * ```size```
    * ```length```
* Type Conversions
    * ```to_vector``` - from both ```std::array``` and C style arrays
    * ```to_matrix``` - from both ```std::array``` and C style arrays
    * ```to_array``` - from both ```dsga::mat``` and ```dsga::vec_interface``` to ```std::array```
    * [```std::span``` example](examples/span_convert.hxx)
* Text output
    * [```std::ostream``` example](examples/ostream_output.hxx)
    * [```std::formatter``` example](examples/format_output.hxx)
* ```std::valarray``` API (vector only)
    * ```apply```
    * ```query``` - not in ```std::valarray``` nor GLSL - like ```apply()``` but for boolean predicates
    * ```shift```
    * ```cshift```
    * ```min```
    * ```max```
    * ```sum```

## Installation

This is a **single header library**, where you just need the file [dsga.hxx](include/dsga.hxx). You can just copy dsga.hxx, or you can get it installed via CMake. Things are defined in the ```dsga``` namespace. The types provided by this library can be seen summarized in the [documentation](docs/DOCUMENTATION.md), [using directives](docs/DOCUMENTATION.md#types-and-functions).

Under the hood, we depend on the [cxcm](https://github.com/davidbrowne/cxcm) project for constexpr versions of some ```cmath``` functions. ```cxcm``` has been brought into ```dsga.hxx```, converted to a nested ```namespace cxcm``` under ```namespace dsga```, so we don't need to also include the files from ```cxcm```.

This may be a single header library, but if Visual Studio is being used, we recommend to also get the [dsga.natvis](VS2026/dsga.natvis) file for debugging and inspecting vectors and matrices in the IDE.

This is a c++20 library, so that needs to be the minimum standard that you tell the compiler to use.

## Status

Current version: `v3.0.0`

* Everything major has some tests, but code coverage is not 100%.
* [Last Release: v3.0.0](https://github.com/davidbrowne/dsga/releases)
* [Change Log](docs/CHANGELOG.md)

## Usage

Use it more or less like you would use vectors and matrices in a shader program, but not necessarily for shading. We hope to be able to use it for rapid development of geometric algorithms. See the [examples](examples) directory.

The [documentation](docs/DOCUMENTATION.md) explains more about how the vector and matrix classes work, and describes the API.

More in depth explanation can be found in the [details](docs/DETAILS.md).

## Testing

This project uses [doctest](https://github.com/onqtam/doctest) for testing. We occasionally use [nanobench](https://github.com/martinus/nanobench) for understanding implementation tradeoffs.

All tests are currently 100% PASSING on all the testing platforms and compilers.

The tests have been most recently run on:

### Windows 11 Native

* **MSVC 2026 v18.6**

```
[doctest] doctest version is "2.5.2"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 15.2** on Windows, [MSYS2](https://www.msys2.org/) distribution:

```
[doctest] doctest version is "2.5.2"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 22.1** on Windows, [semi-official binaries](https://github.com/llvm/llvm-project/releases):

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.5.2"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  110 |  110 passed | 0 failed | 1 skipped
[doctest] assertions: 2168 | 2168 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 26.04 LTS running in WSL for Windows 11

* **gcc 16.0**

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  109 |  109 passed | 0 failed | 0 skipped
[doctest] assertions: 2177 | 2177 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 22.1**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  108 |  108 passed | 0 failed | 1 skipped
[doctest] assertions: 2161 | 2161 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 24.04 LTS running in WSL for Windows 11

* **gcc 14.2**

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  109 |  109 passed | 0 failed | 0 skipped
[doctest] assertions: 2177 | 2177 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 20.1**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  108 |  108 passed | 0 failed | 1 skipped
[doctest] assertions: 2161 | 2161 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 22.04.3 LTS running in WSL for Windows 11

* **gcc 12.3**

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  109 |  109 passed | 0 failed | 0 skipped
[doctest] assertions: 2177 | 2177 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 11.4**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  108 |  108 passed | 0 failed | 1 skipped
[doctest] assertions: 2161 | 2161 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 16.0**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.5.1"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  108 |  108 passed | 0 failed | 1 skipped
[doctest] assertions: 2161 | 2161 passed | 0 failed |
[doctest] Status: SUCCESS!
```

## License
[![BSL](https://img.shields.io/badge/license-BSL-blue)](https://choosealicense.com/licenses/bsl-1.0/)

```
//          Copyright David Browne 2020-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
```

This project uses the [Boost Software License 1.0](https://choosealicense.com/licenses/bsl-1.0/).

### Third Party Attribution 

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
