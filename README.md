# dsga : Data Structures for Geometric Algorithms

**dsga** is a **c++20 library** that implements the **vectors** and **matrices** from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). It is inspired by the spec, but does deviate in some small ways, mostly to make it work well in c++20. It is not intended to be used for rendering, just for sharing the **fundamental data structures** and associated functions. Our requirements in general are for things like 3D CAD/CAM applications and other **geometric and algebraic things**. See [motivation](docs/MOTIVATION.md) for more details. This library does not use SIMD instructions or types under the hood, beyond whatever the compiler provides through optimization.

## A Quick Peek At Some Examples

``` c++
using namespace dsga;

// get a 2D vector that is perpendicular (rotated 90 degrees counter-clockwise)
// to a 2D vector in the plane
constexpr auto get_perpendicular1(const vec2 &some_vec) noexcept
{
    auto cos90 = 0.0f;
    auto sin90 = 1.0f;

    // rotation matrix -- components in column major order
    return mat2(cos90, sin90, -sin90, cos90) * some_vec;
}

// same as above, different implementation
constexpr auto get_perpendicular2(const vec2 &some_vec) noexcept
{
    return vec2(-1, 1) * some_vec.yx;
}

// project a point to a line in 3D -- there is no coincident or collinear point checking
constexpr auto project_to_line(const dvec3 &p0, const dvec3 &p1, const dvec3 &point_in_space) noexcept
{
    auto v1 = p1 - p0;
    auto vq = point_in_space - p0;
    auto t = dot(vq, v1) / dot(v1, v1);

    return p1 + t * v1;
}
```

``` c++
using namespace dsga;

//
// evaluate a 2D cubic bezier curve at t
//

// cubic bezier linear interpolation, one ordinate at a time, e.g., x, y, z, or w
constexpr auto single_ordinate_cubic_bezier_eval(vec4 cubic_control_points, float t) noexcept
{
    auto quadratic_control_points = mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
    auto linear_control_points = mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
    return std::lerp(linear_control_points.x, linear_control_points.y, t);
}

// main cubic bezier eval function -- takes 2D control points with float values.
// returns the 2D point on the curve at t
constexpr auto simple_cubic_bezier_eval(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) noexcept
{
    // each control point is a column of the matrix.
    // the rows represent x coords and y coords.
    auto AoS = mat4x2(p0, p1, p2, p3);

    // lambda pack wrapper -- would be better solution if vector size was generic
    return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
    {
        return vec2(single_ordinate_cubic_bezier_eval(AoS.row(Is), t)...);
    }(std::make_index_sequence<2u>{});
}
```

``` c++
using namespace dsga;

//
// find the minimum positive angle between 2 vectors and/or indexed vectors (swizzles).
// 2D or 3D only
//

template <bool W1, floating_point_dimensional_scalar T, std::size_t C,
          class D1, bool W2, class D2>
requires ((C > 1u) && (C < 4u))
auto angle_between(const vector_base<W1, T, C, D1> &v1,
                   const vector_base<W2, T, C, D2> &v2)
{
    auto v1_mag = length(v1);
    auto v2_mag = length(v2);
    auto numerator = length(v1 * v2_mag - v2 * v1_mag);
    auto denominator = length(v1 * v2_mag + v2 * v1_mag);

    if (numerator == T(0))
        return T(0);
    else if (denominator == T(0))
        return std::numbers::pi_v<T>;

    return T(2) * std::atan(numerator / denominator);
}
```

``` c++
using namespace dsga;

//
// get the signed volume for connected triangular mesh
//

// for a watertight connected triangular mesh, get the signed volume.
// vertices are 3d points, triangles have the indices to the vertices.
// follows right-hand rule for the sign of the volume.
//
// preconditions: watertight mesh, non-degenerate triangles
double signed_volume_watertight_mesh(const std::vector<dvec3> &vertices,
                                     const std::vector<uvec3> &triangles)
{
    double volume_times_six{ 0 };

    // each triangle contributes a tetrahedronal volume using the origin as the 4th "vertex"
    for (const auto &tri : triangles)
        volume_times_six += determinant(mat3(vertices[tri[0]], vertices[tri[1]], vertices[tri[2]]));

    return volume_times_six / 6.0;
}

// triangular mesh of a cube with sides length 2.
// not worried about topology, e.g., adjacent triangles, for this example.
void cube_mesh(std::vector<dvec3> &vertices,
               std::vector<uvec3> &triangles)
{
    vertices.clear();
    vertices.reserve(8);
    vertices.emplace_back(1, 1, 1);
    vertices.emplace_back(-1, 1, 1);
    vertices.emplace_back(-1, 1, -1);
    vertices.emplace_back(1, 1, -1);
    vertices.emplace_back(1, -1, 1);
    vertices.emplace_back(-1, -1, 1);
    vertices.emplace_back(-1, -1, -1);
    vertices.emplace_back(1, -1, -1);

    triangles.clear();
    triangles.reserve(12);
    triangles.emplace_back(1, 0, 2);
    triangles.emplace_back(3, 2, 0);
    triangles.emplace_back(0, 1, 4);
    triangles.emplace_back(1, 2, 5);
    triangles.emplace_back(2, 3, 6);
    triangles.emplace_back(3, 0, 7);
    triangles.emplace_back(5, 4, 1);
    triangles.emplace_back(6, 5, 2);
    triangles.emplace_back(7, 6, 3);
    triangles.emplace_back(4, 7, 0);
    triangles.emplace_back(6, 7, 5);
    triangles.emplace_back(4, 5, 7);
}

double get_cube_volume()
{
    std::vector<dvec3> vertices;
    std::vector<uvec3> triangles;
    cube_mesh(vertices, triangles);

    return signed_volume_watertight_mesh(vertices, triangles);
}
```

## Installation

This is a **single header library**, where you just need the file [dsga.hxx](include/dsga.hxx). Things are defined in the ```dsga``` namespace. The types provided by this library can be seen summarized in the [documentation](docs/DOCUMENTATION.md), [using directives](docs/DOCUMENTATION.md#types-and-functions).

Under the hood, we depend on the [cxcm](https://github.com/davidbrowne/cxcm) project for constexpr versions of some ```cmath``` functions. ```cxcm``` has been brought into ```dsga.hxx```, converted to a nested ```namespace cxcm``` under ```namespace dsga```, so we don't need to also include the files from ```cxcm```.

Since the constexpr versions of ```dsga::cxcm::sqrt()``` and ```dsga::cxcm::rsqrt()``` from [cxcm](https://github.com/davidbrowne/cxcm) do not have identical results with std::sqrt(), if you want the approximate but constexpr version of those or ```dsga::sqrt()``` or ```dsga::rsqrt()```, you have to opt-in by defining the macro ```DSGA_CXCM_CONSTEXPR_APPROXIMATIONS_ALLOWED```.

There are asserts in the codebase, especially in the indexed_vector iterator structs, that can be disabled by defining the macro ```DSGA_DISABLE_ASSERTS```.

This may be a single header library, but if Visual Studio is being used, we recommend to also get the [dsga.natvis](VS2022/dsga.natvis) file for debugging and inspecting vectors and matrices in the IDE.

## Status

Current version: `v0.8.11`

* **All the vector and matrix functionality is implemented.**
* First pass at test coverage. Everything major has some tests, but code coverage is not 100%.
* [Released v0.8.0](https://github.com/davidbrowne/dsga/releases/tag/v0.8.0)

### The next steps
* Example projects: need small, medium, and large examples. The quick peek at the top of this page is a start, as is a [more detailed generic version of the example](docs/DETAILS.md#detailed-generic-example).

## Usage

Use it more or less like you would use vectors and matrices in a shader program, but not necessarily for shading. We hope to be able to use it for rapid development of geometric algorithms.

The [documentation](docs/DOCUMENTATION.md) explains more about how the vector and matrix classes work, and describes the API.

More in depth explanation can be found in the [details](docs/DETAILS.md).

## Testing

This project uses [doctest](https://github.com/onqtam/doctest) for testing. We occasionally use [nanobench](https://github.com/martinus/nanobench) for understanding implementation tradeoffs.

Both MSVC and gcc (for Windows and on Ubuntu on WSL2) pass all the tests. clang for Windows passes, but there are 2 assertions out of 1869 that fail for clang-15 on Ubuntu, which appears to have a problem with ```std::is_trivial_v<>```.

The tests have been most recently run on:

### Windows 11 Native

* **MSVC 2022 - v17.4**

```
[doctest] doctest version is "2.4.9"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   83 |   83 passed | 0 failed | 0 skipped
[doctest] assertions: 1885 | 1885 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 12.2.0** on Windows, [MinGW](https://github.com/niXman/mingw-builds-binaries) distribution:

```
[doctest] doctest version is "2.4.9"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   83 |   83 passed | 0 failed | 0 skipped
[doctest] assertions: 1885 | 1885 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 15.0.7** on Windows, [official binaries](https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.7), with MSVC installed:

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.9"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   83 |   83 passed | 0 failed | 0 skipped
[doctest] assertions: 1869 | 1869 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 22.04 running in WSL2 for Windows 11

* **gcc 12.1.0**

```
[doctest] doctest version is "2.4.9"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:   83 |   83 passed | 0 failed | 0 skipped
[doctest] assertions: 1885 | 1885 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 15.0.7**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.9"
[doctest] run with "--help" for options
===============================================================================
dsga/tests/swizzle_test.cxx:1711:
TEST SUITE: test swizzling applications
TEST CASE:  type traits tests
  type traits for basic_vector

dsga/tests/swizzle_test.cxx:1744: ERROR: CHECK_UNARY( std::is_trivial_v<dvec4> ) is NOT correct!
  values: CHECK_UNARY( false )

===============================================================================
dsga/tests/swizzle_test.cxx:1711:
TEST SUITE: test swizzling applications
TEST CASE:  type traits tests
  type traits for basic_matrix

dsga/tests/swizzle_test.cxx:1849: ERROR: CHECK_UNARY( std::is_trivial_v<dmat4> ) is NOT correct!
  values: CHECK_UNARY( false )

===============================================================================
[doctest] test cases:   83 |   82 passed | 1 failed | 0 skipped
[doctest] assertions: 1869 | 1867 passed | 2 failed |
[doctest] Status: FAILURE!
```

## Similar Projects

It is a common pastime for people to write these kind of vector libraries. The three we wanted to mention here are:

* [glm](https://github.com/g-truc/glm) - popular long lived project that is similar in goals with respect to being based on OpenGL Shading Language specification, but is much more mature. It will work with c\+\+98, while dsga is for c\+\+20.
* [DirectXMath](https://github.com/microsoft/DirectXMath) - this is from Microsoft and basically performs the same role as glm, but with DirectX instead of OpenGL. It is also long lived and much more mature than dsga.
* mango (repo has been removed by owner) - this is the project that I read the blog about for vector component access and swizzling, so it is nice to have as another example. Again, more mature than dsga.

## License [![BSL](https://img.shields.io/badge/license-BSL-blue)](https://choosealicense.com/licenses/bsl-1.0/)

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
// Copyright (c) 2019-2020 Martin Ankerl <martin.ankerl@gmail.com>
```
