# dsga : Data Structures for Geometric Algorithms

**dsga** is a single header-only **C++20 library** that implements the **vectors** and **matrices** from the OpenGL Shading Language 4.6 specification ([pdf](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf) | [html](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.html)). It is inspired by the spec, but does deviate in some small ways, mostly to make it work well in C++20. It is intended to be used for [array programming](https://en.wikipedia.org/wiki/Array_programming) other than rendering. Our requirements in general are for things like 3D CAD/CAM applications and other geometric and algebraic things. See [motivation](docs/MOTIVATION.md) for more details. This library does _not_ use SIMD instructions or types under the hood, beyond whatever the compiler provides through optimization.

## Home
[https://github.com/davidbrowne/dsga](https://github.com/davidbrowne/dsga)

## Current Version
v2.1.4

## [Latest Major Changes](docs/CHANGELOG.md)
* v2.1.4
    * Moved slice() functions to their own example file.

* v2.1.3
    * Added ability to change the underlying data pointer of a ```basic_view```, which also affects ```view_wrapper``` and ```indexed_view```. Much refactoring was required for this, including having to allow the null pointer as the underlying data pointer (all functions that require use of the pointer will assert if the pointer is null).
    * Added a new constructor to ```basic_matrix``` that takes a raw pointer to an array of data. The constructor will throw if it is passed nullptr. The constructor does not wrap the pointer, rather it copies the array data into ```basic_matrix```'s own storage.
    * Added a ```slice()``` function, to give you a contiguous sub-range of the data object. It works for arguments of type ```basic_vector```, ```basic_view```, and ```vector_view```. It returns a ```basic_view``` on the argument vector or view. The lifetime of the returned object depends on the lifetime of the vector or view argument, so it will have a dangling-pointer if it lives beyond the argument. This function can throw if the template parameter argument for length and the offset argument would result in a buffer overrun.
* v2.1.2
    * Added ```get<>``` for ```view_wrapper```.
    * Added ```as_base()``` to ```vector_base``` for debugging and testing purposes.
* v2.1.1
    * Further experimental additions: adding a boolean ```Mutable``` template parameter to the classes for when they can be logically "const" (false means "const", true means "non const"), for both vectors (currently Mutable == true for the vectors) but mostly for the vector views that wrap an external storage pointer (const pointer vs non-const pointer). This is different from ```Writable```, which is used to determine if an indexed vector/indexed view is able to be an lvalue due to swizzle restrictions.
* v2.1.0
    * MAJOR EXPERIMENTAL ADDITION: there are now vector types, ```basic_view``` and ```indexed_view``` (and similarly ```view_wrapper```), that don't own their data. They are meant to work on a contiguous external data source, e.g., a slice of an array, instead of internal data storage, like the storage in a ```basic_vector```. ```view_vector``` is similar to ```basic_vector```, but it is a ```basic_view``` with an internal array for its data source.

## Tested Compilers
### Regularly Tested
* Microsoft Visual Studio 2022 v17.11.2
* gcc v14
* clang v18.1.8
* icx v2024.1.0

### Minimum Version
* Microsoft Visual Studio 2022 v17.x
* gcc v11.4
* clang v16.0.6
* icx v2023.1.0 - using [Compiler Explorer](https://compiler-explorer.com/) for basic compilation test, but test suite not run

## Contents
* [Some Quick Examples](#some-quick-examples)
* [Relevant GLSL Overview](#relevant-glsl-overview)
* [Implemented Interfaces](#implemented-interfaces)
* [General documentation](docs/DOCUMENTATION.md)
* [Detailed API documentation](docs/API.md)
* [```dsga``` Implementation Details](docs/DETAILS.md)
* [Vector and Matrix Types](#types)
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
auto angle_between(const dsga::vector_base<W1, T, C, D1> &v1,
                   const dsga::vector_base<W2, T, C, D2> &v2)
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

// arguments are of the vector_base class type, and this function will be used if any passed argument is of type indexed_vector
template <bool W1, dsga::floating_point_scalar T1, typename D1, bool W2, dsga::floating_point_scalar T2, typename D2>
[[nodiscard]] constexpr auto cross(const dsga::vector_base<W1, T1, 3, D1> &a,
                                   const dsga::vector_base<W2, T2, 3, D2> &b) noexcept
{
    // CTAD gets us the type and size for the vector
    return dsga::basic_vector((a[1] * b[2]) - (b[1] * a[2]),
                              (a[2] * b[0]) - (b[2] * a[0]),
                              (a[0] * b[1]) - (b[0] * a[1]));
}

// arguments are of type basic_vector, and there is a compact swizzled implementation
template <dsga::floating_point_scalar T1, dsga::floating_point_scalar T2>
[[nodiscard]] constexpr auto cross(const dsga::basic_vector<T1, 3> &a,
                                   const dsga::basic_vector<T2, 3> &b) noexcept
{
    return (a.yzx * b.zxy) - (a.zxy * b.yzx);
}
```

```c++
// simple example converting a 2D cartesian point (x, y) into polar coordinates (r, theta)
// using arrays instead of structures

// simple example - convert (x, y) to (r, theta)
template <dsga::floating_point_scalar T, std::size_t S>
void cartesian_to_polar(dsga::basic_view<true, T, S> r, dsga::basic_view<true, T, S> theta,
                        dsga::basic_view<false, T, S> x, dsga::basic_view<false, T, S> y)
{
    theta = dsga::atan(y, x);

    for (std::size_t i = 0; i < S; ++i)
        r[i] = dsga::length(dsga::basic_vector<T, 2>(x[i], y[i]));
}

// example that works with arrays of data for input and output instead of using point structure
void converter(std::vector<double> &radial_dist, std::vector<double> &polar_angle,
               const std::vector<double> &x, const std::vector<double> &y)
{
    constexpr std::size_t vector_size = 4;
    auto len = dsga::min(x.size(), y.size());
    auto div = len / vector_size;
    auto mod = len % vector_size;
    radial_dist.resize(len);
    polar_angle.resize(len);

    // chunk up the array into vectors of length 4 and process
    for (std::size_t i = 0; i < div; ++i)
    {
        cartesian_to_polar(dsga::dview4(radial_dist.data() + i * vector_size),
                           dsga::dview4(polar_angle.data() + i * vector_size),
                           dsga::cdview4(x.data() + i * vector_size),
                           dsga::cdview4(y.data() + i * vector_size));
    }

    // process the remainder of the array data (leftover data < 4 elements)
    switch (mod)
    {
        case 1:
            cartesian_to_polar(dsga::dview1(radial_dist.data() + div * vector_size),
                               dsga::dview1(polar_angle.data() + div * vector_size),
                               dsga::cdview1(x.data() + div * vector_size),
                               dsga::cdview1(y.data() + div * vector_size));
            break;

        case 2:
            cartesian_to_polar(dsga::dview2(radial_dist.data() + div * vector_size),
                               dsga::dview2(polar_angle.data() + div * vector_size),
                               dsga::cdview2(x.data() + div * vector_size),
                               dsga::cdview2(y.data() + div * vector_size));
            break;

        case 3:
            cartesian_to_polar(dsga::dview3(radial_dist.data() + div * vector_size),
                               dsga::dview3(polar_angle.data() + div * vector_size),
                               dsga::cdview3(x.data() + div * vector_size),
                               dsga::cdview3(y.data() + div * vector_size));
            break;

        case 0:
        default:
            break;
    }
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
    * vector only - these ordering facilities allow logical use of ```data```
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
    * ```query``` - not in ```std::valarray``` nor GLSL - like ```apply()``` but for boolean predicates
    * ```shift```
    * ```cshift```
    * ```min```
    * ```max```
    * ```sum```

## Types
The high level structs that are used are for vector types and matrix types.

### Vector Types

There are 3 primary vector types:
* Vectors, which are of type ```basic_vector```, and they have their own internal storage.
* Views, which are of type ```basic_view```, and they wrap a pointer from some external storage. They have the same interface as vectors.
* Constant views, which are just like the views, but they are internally const or immutable. This is for wrapping an external const pointer, or a non-const pointer that you want to treat as being internally const or immutable. They have the same interface as vectors.

Each of the vector types have between 1 and 4 elements, inclusive. For most situations, when dealing with a length 1 vector type, we treat it as a scalar. This is true for both the vector types and the vector swizzle types.

Below are the aliases for the three primary vector types:

| Size/Element Type | Vector | View | Const View |
|:---:|:---:|:---:|:---:|
| 1 bool | bscal | bview1 | cbview1 |
| 2 bool | bvec2 | bview2 | cbview2 |
| 3 bool | bvec3 | bview3 | cbview3 |
| 4 bool | bvec4 | bview4 | cbview4 |
| 1 int | iscal | iview1 | ciview1 |
| 2 int | ivec2 | iview2 | ciview2 |
| 3 int | ivec3 | iview3 | ciview3 |
| 4 int | ivec4 | iview4 | ciview4 |
| 1 unsigned int | uscal | uview1 | cuview1 |
| 2 unsigned int | uvec2 | uview2 | cuview2 |
| 3 unsigned int | uvec3 | uview3 | cuview3 |
| 4 unsigned int | uvec4 | uview4 | cuview4 |
| 1 long long | llscal | llview1 | cllview1 |
| 2 long long | llvec2 | llview2 | cllview2 |
| 3 long long | llvec3 | llview3 | cllview3 |
| 4 long long | llvec4 | llview4 | cllview4 |
| 1 unsigned long long | ullscal | ullview1 | cullview1 |
| 2 unsigned long long | ullvec2 | ullview2 | cullview2 |
| 3 unsigned long long | ullvec3 | ullview3 | cullview3 |
| 4 unsigned long long | ullvec4 | ullview4 | cullview4 |
| 1 float | scal | view1 | cview1 |
| 2 float | vec2 | view2 | cview2 |
| 3 float | vec3 | view3 | cview3 |
| 4 float | vec4 | view4 | cview4 |
| 1 float | fscal | fview1 | cfview1 |
| 2 float | fvec2 | fview2 | cfview2 |
| 3 float | fvec3 | fview3 | cfview3 |
| 4 float | fvec4 | fview4 | cfview4 |
| 1 double | dscal | dview1 | cdview1 |
| 2 double | dvec2 | dview2 | cdview2 |
| 3 double | dvec3 | dview3 | cdview3 |
| 4 double | dvec4 | dview4 | cdview4 |

There are two other types that are returned when swizzling a vector type:
* Vectors have swizzle types of struct ```indexed_vector```. They have most of the interface of a vector. They share the memory of the vector that they came from.
* Views have swizzle types of struct ```indexed_view```. They have most of the interface of a vector. They share the external pointer of the view that they came from.

You can mix and match the primary vector types and the swizzle vector types in expressions. They all inherit from the same base class, ```vector_base```.

You should not try to directly create instances of these helper struct types. Use them like you would use a vector or view.

### Matrix Types
The matrix types have elements of type float or double. Each of their dimensions is between 2 to 4, inclusive.

Recall that for glsl matrices, the numbers represent ***columns X rows***, as opposed to the mathemetical convention of ***rows X columns***.

| Size/Element Type | Matrix |
|:---:|:---:|
| 2x2 float | mat2x2 |
| 2x3 float | mat2x3 |
| 2x4 float | mat2x4 |
| 3x2 float | mat3x2 |
| 3x3 float | mat3x3 |
| 3x4 float | mat3x4 |
| 4x2 float | mat4x2 |
| 4x3 float | mat4x3 |
| 4x4 float | mat4x4 |
| 2x2 float | mat2 |
| 3x3 float | mat3 |
| 4x4 float | mat4 |
| 2x2 double | dmat2x2 |
| 2x3 double | dmat2x3 |
| 2x4 double | dmat2x4 |
| 3x2 double | dmat3x2 |
| 3x3 double | dmat3x3 |
| 3x4 double | dmat3x4 |
| 4x2 double | dmat4x2 |
| 4x3 double | dmat4x3 |
| 4x4 double | dmat4x4 |
| 2x2 double | dmat2 |
| 3x3 double | dmat3 |
| 4x4 double | dmat4 |





## Installation

This is a **single header library**, where you just need the file [dsga.hxx](include/dsga.hxx). Things are defined in the ```dsga``` namespace. The types provided by this library can be seen summarized in the [documentation](docs/DOCUMENTATION.md), [using directives](docs/DOCUMENTATION.md#types-and-functions).

Under the hood, we depend on the [cxcm](https://github.com/davidbrowne/cxcm) project for constexpr versions of some ```cmath``` functions. ```cxcm``` has been brought into ```dsga.hxx```, converted to a nested ```namespace cxcm``` under ```namespace dsga```, so we don't need to also include the files from ```cxcm```.

There are asserts in the codebase that can be disabled by defining the macro ```DSGA_DISABLE_ASSERTS```.

This may be a single header library, but if Visual Studio is being used, we recommend to also get the [dsga.natvis](VS2022/dsga.natvis) file for debugging and inspecting vectors and matrices in the IDE. While debugging this on Linux (WSL2: Windows Subsystem for Linux) with gcc in Visual Studio Code, we created a [.natvis](vscode/dsga-vscode.natvis) file for that too.

This is a c++20 library, so that needs to be the minimum standard that you tell the compiler to use.

## Status

Current version: `v2.1.4`

* Everything major has some tests (except for the recent view structs), but code coverage is not 100%.
* [Last Released: v2.0.0](https://github.com/davidbrowne/dsga/releases/tag/v2.0.0)
* [Change Log](docs/CHANGELOG.md)

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

* **MSVC 2022 - v17.11.2**

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2332 | 2332 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 13.2.0** on Windows, [MSYS2](https://www.msys2.org/) distribution:

Performs all unit tests except for gcc's ```std::is_trivial_v<>``` doesn't work for struct/classes with deleted default constructors.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2329 | 2329 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 18.1.8** on Windows, [official binaries](https://github.com/llvm/llvm-project/releases/tag/llvmorg-18.1.8):

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2303 | 2303 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **icpx 2024.1.0** on Windows, [official binaries](https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html#dpcpp-cpp):

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2303 | 2303 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 24.04 LTS running in WSL2 for Windows 11

* **gcc 14.0.1**

Performs all unit tests except for gcc's ```std::is_trivial_v<>``` doesn't work for struct/classes with deleted default constructors.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2329 | 2329 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 18.1.3**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2300 | 2300 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 22.04.3 LTS running in WSL2 for Windows 11

* **gcc 12.3.0**

Performs all unit tests except for gcc's ```std::is_trivial_v<>``` doesn't work for struct/classes with deleted default constructors.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2329 | 2329 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 11.4.0**

Performs all unit tests except for gcc's ```std::is_trivial_v<>``` doesn't work for struct/classes with deleted default constructors.

Also, does not perform the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2300 | 2300 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 16.0.6**

Performs all the unit tests except where there is lack of support for ```std::is_corresponding_member<>```, and this is protected with a feature test macro.

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  116 |  116 passed | 0 failed | 0 skipped
[doctest] assertions: 2300 | 2300 passed | 0 failed |
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
//          Copyright David Browne 2020-2024.
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
//          Copyright David Browne 2020-2024.
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
// Copyright (c) 2019-2020 Martin Ankerl <martin.ankerl@gmail.com>
```
