# dsga : Data Structures for Geometric Algorithms

**dsga** is a single header-only **c++20 library** that implements the **vectors** and **matrices** from the OpenGL Shading Language 4.6 specification ([pdf](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) | [html](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html)). It is inspired by the spec, but does deviate in some small ways, mostly to make it work well in c++20. It is not intended to be used for rendering, just for sharing the fundamental data structures and associated functions. Our requirements in general are for things like 3D CAD/CAM applications and other geometric and algebraic things. See [motivation](docs/MOTIVATION.md) for more details. This library does not use SIMD instructions or types under the hood, beyond whatever the compiler provides through optimization.

## Current Version
v1.2.8

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
constexpr auto get_perpendicular1(const dsga::basic_vector<T, 2> &some_vec) noexcept
{
    auto cos90 = 0.0f;
    auto sin90 = 1.0f;

    // rotation matrix -- components in column major order
    return dsga::basic_matrix<T, 2, 2>(cos90, sin90, -sin90, cos90) * some_vec;
}

// same as above, different implementation
template <dsga::floating_point_scalar T>
constexpr auto get_perpendicular2(const dsga::basic_vector<T, 2> &some_vec) noexcept
{
    return dsga::basic_vector<T, 2>(-1, 1) * some_vec.yx;
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

// cubic bezier linear interpolation, one ordinate at a time, e.g., x, y, z, or w
constexpr auto single_ordinate_cubic_bezier_eval(dsga::vec4 cubic_control_points, float t) noexcept
{
    auto quadratic_control_points = dsga::mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
    auto linear_control_points = dsga::mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
    return dsga::mix(linear_control_points.x, linear_control_points.y, t);
}

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
    }(std::make_index_sequence<2u>{});
}
```

``` c++
//
// find the minimum positive angle between 2 vectors and/or indexed vectors (swizzles).
// Uses base class for vector types to be inclusive to both types.
// 2D or 3D only.
//

template <bool W1, dsga::floating_point_scalar T, std::size_t C, class D1, bool W2, class D2>
requires ((C > 1u) && (C < 4u))
auto angle_between(const dsga::vector_base<W1, T, C, D1> &v1,
                   const dsga::vector_base<W2, T, C, D2> &v2)
{
    auto v1_mag = dsga::length(v1);
    auto v2_mag = dsga::length(v2);
    auto numerator = dsga::length(v1 * v2_mag - v2 * v1_mag);
    auto denominator = dsga::length(v1 * v2_mag + v2 * v1_mag);

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
## Relevant GLSL Overview

Our programming environment is ```c++20```, not a GLSL shader program, so the entire GLSL Shading language specification is a super-set of what we are trying to achieve. We really just want the vector and matrix data structures (and their corresponding functions and behavior) to be usable in a ```c++20``` environment.

The following links to the shading specification should help with understanding what we are trying to implement with this header-only library.

* [Variables and Types](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#variables-and-types)
    * [Basic Types](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#basic-types): we have added support for vectors to also hold values of type ```std::size_t```, ```unsigned long long``` (which is what ```std::size_t``` really is for x64), and ```signed long long```.
    * [Vectors](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vectors): GLSL does not have 1-dimensional vectors, but we do, which we have using directives to give them names that describe them as scalars and not as vectors, e.g., ```dsga::iscal```, ```dsga::dscal```, ```dsga::bscal```. We support 1-dimensional vectors because GLSL does something special with the fundamental types, allowing them to be swizzled. We use the 1-dimensional vectors to mimic that ability.
      ```glsl
      // glsl
      double value = 10.0;
      dvec3 swizzled_value = value.xxx; 

      // dsga
      // dscal is an alias for dsga::basic_vector<double, 1>
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
    * [Out-of-Bounds Accesses](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#out-of-bounds-accesses): we have asserts for operator[] vectors and matrices, which help with debug runtimes and constexpr variable validity. These asserts (and others in the library) can be disabled by adding the following prior to including the header ```dsga.hxx```:
      ```c++
      #define DSGA_DISABLE_ASSERTS
      #include "dsga.hxx"
      ```
 
* [Built-In Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#built-in-functions): we support the additional types ```std::size_t```, ```unsigned long long```, and ```signed long long``` in the functions where appropriate. We also added bit conversion functions between these 64-bit integral types and ```double```.

  We also support using ```double``` for all the functions where ```float``` is supported, with the exception of the bit conversion functions for ```float``` with 32-bit integral types, and ```double``` with the 64-bit integral types.
    * [Angle and Trigonometry Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#angle-and-trigonometry-functions): there are also scalar versions of these functions, but where c++ does the same thing, it might be easier to use the ```std::``` version instead of the ```dsga::``` version.
    * [Exponential Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#exponential-functions): there are also scalar versions of these functions, but where c++ does the same thing, it might be easier to use the ```std::``` version instead of the ```dsga::``` version.

    * [Common Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#common-functions): there are also scalar versions of these functions, but where c++ does the same thing, it might be easier to use the ```std::``` version instead of the ```dsga::``` version.
    * [Geometric Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#geometric-functions): ```ftransform()``` is not implemented as it is only for GLSL vertex shader programs.
    * [Matrix Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#matrix-functions)
    * [Vector Relational Functions](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html#vector-relational-functions): GLSL has a vector function ```not()```, but ```not``` is a c++ keyword. Instead of naming this function ```not()```, we name it ```logicalNot()```.

      In addition, we have added the non-GLSL convenience function ```none()```, which returns ```!any()```.

## Implemented Interfaces

To make the vectors and matrices as useful as possible in a C++ context, various C++ customization points were implemented or interfaces partially emulated, e.g., ```std::valarray<>```. There are many options for data access. For ```dsga``` vectors and matrices, we have:

* Swizzle access like GLSL (vector only)
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
* Pointer access (physical), ```std::span``` (for contiguous range types ```dsga::basic_vector``` and ```dsga::basic_matrix```)
    * ```data```
        * vector - pointer to scalars of concept type ```dsga::dimensional_scalar```
        * matrix - pointer to column vectors whose scalars are of concept type ```dsga::floating_point_scalar```
    * ```size```
    * vector only - these allow logical use of ```data```
        * ```offsets```
        * ```sequence```
* Type Conversions
    * ```to_vector``` - from both ```std::array``` and C style arrays
    * ```to_matrix``` - from both ```std::array``` and C style arrays
    * ```to_array``` - from both ```dsga::basic_matrix``` and ```dsga::vector_base``` to ```std::array```
    * [```std::span``` example](examples/span_convert.hxx)
* Text output
    * [```std::ostream``` example](examples/ostream_output.hxx)
    * [```std::formatter``` example](examples/format_output.hxx)
* ```std::valarray``` API (vector only)
    * ```apply```
    * ```shift```
    * ```cshift```
    * ```min```
    * ```max```
    * ```sum```

## Installation

This is a **single header library**, where you just need the file [dsga.hxx](include/dsga.hxx). Things are defined in the ```dsga``` namespace. The types provided by this library can be seen summarized in the [documentation](docs/DOCUMENTATION.md), [using directives](docs/DOCUMENTATION.md#types-and-functions).

Under the hood, we depend on the [cxcm](https://github.com/davidbrowne/cxcm) project for constexpr versions of some ```cmath``` functions. ```cxcm``` has been brought into ```dsga.hxx```, converted to a nested ```namespace cxcm``` under ```namespace dsga```, so we don't need to also include the files from ```cxcm```.

There are asserts in the codebase that can be disabled by defining the macro ```DSGA_DISABLE_ASSERTS```.

This may be a single header library, but if Visual Studio is being used, we recommend to also get the [dsga.natvis](VS2022/dsga.natvis) file for debugging and inspecting vectors and matrices in the IDE. While debugging this on Linux (WSL2: Windows Subsystem for Linux) with gcc in Visual Studio Code, we created a [.natvis](vscode/dsga-vscode.natvis) file for that too.

This is a c++20 library, so that needs to be the minimum standard that you tell the compiler to use.

## Status

Current version: `v1.2.8`

* Breaking Change with v1.2.0
  * ```size``` and ```column_size``` functions in the various classes are now static member functions, tied to static data members of type ```std::integral_constant```
* Breaking Change with v1.1.0
  * Removed ```default_comparison_weights()```, ```weighted_compare()```, and related functions
  * Removed ```operator <=>``` for vector and matrix structs
  * Functionality is still available in [```examples/compare.hxx```](examples/compare.hxx)

* Everything major has some tests, but code coverage is not 100%.
* [Last Released: v1.1.0](https://github.com/davidbrowne/dsga/releases/tag/v1.1.0)

### The next steps
* Refining API documentation.
* Working on better ```cmake``` support.
* Add more tests.

## Usage

Use it more or less like you would use vectors and matrices in a shader program, but not necessarily for shading. We hope to be able to use it for rapid development of geometric algorithms. See the [examples](examples) directory.

The [documentation](docs/DOCUMENTATION.md) explains more about how the vector and matrix classes work, and describes the API.

More in depth explanation can be found in the [details](docs/DETAILS.md).

## Testing

This project uses [doctest](https://github.com/onqtam/doctest) for testing. We occasionally use [nanobench](https://github.com/martinus/nanobench) for understanding implementation tradeoffs.

All tests are currently 100% PASSING on all the testing platforms and compilers.

The tests have been most recently run on:

### Windows 11 Native

* **MSVC 2022 - v17.7.4**

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2115 | 2115 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 13.2.0** on Windows, [MSYS2](https://www.msys2.org/) distribution:

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2115 | 2115 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 17.0.2** on Windows, [official binaries](https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.2), with MSVC and/or gcc v13.2.0 installed:

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2099 | 2099 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu Mantic Minotaur preview running in WSL2 for Windows 11

* **gcc 13.2.0**

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2115 | 2115 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 17.0.2**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2099 | 2099 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 22.04 running in WSL2 for Windows 11

* **gcc 12.3.0**

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2115 | 2115 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 16.0.6**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  106 |  106 passed | 0 failed | 0 skipped
[doctest] assertions: 2099 | 2099 passed | 0 failed |
[doctest] Status: SUCCESS!
```

## Similar Projects

It is a common pastime for people to write these kind of vector libraries. The three we wanted to mention here are:

* [glm](https://github.com/g-truc/glm) - popular long lived project that is similar in goals with respect to being based on OpenGL Shading Language specification, but is much more mature. It will work with c\+\+98, while dsga is for c\+\+20.
* [DirectXMath](https://github.com/microsoft/DirectXMath) - this is from Microsoft and basically performs the same role as glm, but with DirectX instead of OpenGL. It is also long lived and much more mature than dsga.
* mango (repo has been removed by owner) - this is the project that I read the blog about for vector component access and swizzling, so it is nice to have as another example. Again, more mature than dsga.

## License
[![BSL](https://img.shields.io/badge/license-BSL-blue)](https://choosealicense.com/licenses/bsl-1.0/)

```
//          Copyright David Browne 2020-2023.
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
//          Copyright David Browne 2020-2023.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
```

```
// qdouble - quad-double precision floating point number
// https://github.com/janm31415/qdouble

// MIT License
//
// Copyright (c) 2022 Jan Maes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// https://opensource.org/licenses/MIT
```

```
// doctest.h - the lightest feature-rich C++ single-header testing framework for unit tests and TDD
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
// Copyright (c) 2019-2020 Martin Ankerl <martin.ankerl@gmail.com>
```
