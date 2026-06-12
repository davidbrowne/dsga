// SPDX-License-Identifier: BSL-1.0
// To the extent copyright is held by the author, this file is licensed
// under the Boost Software License 1.0. To the extent it is not,
// this file is dedicated to the public domain.
// 
// line_closest_points.cxx
//
// dsga example: given two lines each described by a line segment, find the
// closest point on each line to the other line.
//
// Lines in 3D are generally skew (non-intersecting, non-parallel), so there
// is a unique pair of closest points connected by the common perpendicular.
// Special cases handled:
//   - Parallel (or anti-parallel) lines  -- infinitely many solutions;
//     returns the point on line 1 nearest to the midpoint of segment 2,
//     and its corresponding closest point on line 2.
//   - Intersecting lines                 -- both closest points are the
//     same intersection point.
//
// The algorithm is the classic parametric approach:
//   P(s) = A + s * d1       (line 1, direction d1 = B - A)
//   Q(t) = C + t * d2       (line 2, direction d2 = D - C)
//
// Minimize |P(s) - Q(t)|^2 by solving the 2x2 linear system that results
// from setting the partial derivatives to zero.
//
// Reference: "Distance Between Lines" -- Dan Sunday, softsurfer.com
//
// Build (C++20):
//   g++ -std=c++20 -O2 -I<path/to/dsga/include> -o line_closest_points line_closest_points.cxx
//   cl /std:c++20 /O2 /I<path\to\dsga\include> line_closest_points.cxx

#include "dsga.hxx"

#include <cmath>
#include <iostream>
#include <optional>
#include <string>

// ---------------------------------------------------------------------------
// Type aliases
// ---------------------------------------------------------------------------

using vec3 = dsga::vec3;
using mat2 = dsga::mat2;

// ---------------------------------------------------------------------------
// Result type
// ---------------------------------------------------------------------------

struct ClosestPointsResult
{
    vec3  point_on_line1;   // closest point on line 1 to line 2
    vec3  point_on_line2;   // closest point on line 2 to line 1
    float distance;         // distance between the two closest points
    bool  parallel;         // true when lines are parallel / anti-parallel
};

// ---------------------------------------------------------------------------
// closest_points_on_lines
//
// Inputs: two line segments [a, b] and [c, d].  Each segment defines the
// infinite line that passes through its endpoints -- the segment endpoints
// themselves are NOT the clamped closest points; they are only used to
// establish the line direction and an anchor point.
// ---------------------------------------------------------------------------

ClosestPointsResult closest_points_on_lines(
    vec3 a, vec3 b,   // segment defining line 1
    vec3 c, vec3 d)   // segment defining line 2
{
    constexpr float EPSILON = 1e-7f;

    vec3  d1     = b - a;   // direction of line 1 (not necessarily unit)
    vec3  d2     = d - c;   // direction of line 2
    vec3  r      = a - c;   // vector between anchor points

    float e      = dsga::dot(d1, d1);   // |d1|^2
    float f      = dsga::dot(d2, d2);   // |d2|^2
    float g      = dsga::dot(d1, r);
    float h      = dsga::dot(d1, d2);
    float k      = dsga::dot(d2, r);

    // Coefficient matrix of the 2x2 parametric system:
    //   | e  -h | | s |   | -g |
    //   | h  -f | | t | = | -k |
    //
    // mat2 is column-major, so mat2{ {e,h}, {-h,-f} } gives:
    //   col 0 = (e, h),  col 1 = (-h, -f)
    dsga::mat2 M { dsga::vec2{e, h}, dsga::vec2{-h, -f} };
    float denom = dsga::determinant(M);

    // denom == 0  <=>  lines are parallel (includes anti-parallel)
    bool parallel = (std::abs(denom) < EPSILON * e * f);

    float s, t;

    if (parallel)
    {
        // Infinitely many solutions -- anchor s to the midpoint of segment 2
        // projected onto line 1, then find the corresponding t.
        vec3  mid_cd = (c + d) * 0.5f;
        s = dsga::dot(mid_cd - a, d1) / e;
        t = dsga::dot(a + s * d1 - c, d2) / f;
    }
    else
    {
        // Cramer's rule: replace each column with the RHS vector (-g, -k)
        // to get the determinant numerators for s and t.
        dsga::mat2 Ms { dsga::vec2{-g, -k}, dsga::vec2{-h, -f} };  // col 0 replaced
        dsga::mat2 Mt { dsga::vec2{ e,  h}, dsga::vec2{-g, -k} };  // col 1 replaced
        s = dsga::determinant(Ms) / denom;
        t = dsga::determinant(Mt) / denom;
    }

    vec3  p1 = a + s * d1;
    vec3  p2 = c + t * d2;
    float dist = dsga::length(p1 - p2);

    return { p1, p2, dist, parallel };
}

// ---------------------------------------------------------------------------
// Helpers for printing
// ---------------------------------------------------------------------------

void print_vec3(const std::string& label, vec3 v)
{
    std::cout << "  " << label
        << " = (" << v.x << ", " << v.y << ", " << v.z << ")\n";
}

void print_result(const std::string& title,
                  vec3 a, vec3 b,
                  vec3 c, vec3 d)
{
    std::cout << title << "\n";
    print_vec3("Line 1  A", a);
    print_vec3("Line 1  B", b);
    print_vec3("Line 2  C", c);
    print_vec3("Line 2  D", d);

    auto r = closest_points_on_lines(a, b, c, d);

    std::cout << "  --> Parallel: " << (r.parallel ? "yes" : "no") << "\n";
    print_vec3("--> Closest point on line 1", r.point_on_line1);
    print_vec3("--> Closest point on line 2", r.point_on_line2);
    std::cout << "  --> Distance between closest points: " << r.distance << "\n";

    // Verify perpendicularity (should be ~0 for non-parallel case)
    if (!r.parallel)
    {
        vec3  d1     = b - a;
        vec3  d2     = d - c;
        vec3  bridge = r.point_on_line2 - r.point_on_line1;
        float dot1   = dsga::dot(dsga::normalize(bridge), dsga::normalize(d1));
        float dot2   = dsga::dot(dsga::normalize(bridge), dsga::normalize(d2));
        std::cout << "  --> Perpendicularity check (should be ~0):\n";
        std::cout << "      bridge . d1 = " << dot1 << "\n";
        std::cout << "      bridge . d2 = " << dot2 << "\n";
    }

    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    std::cout << std::fixed;
    std::cout.precision(6);

    // ---- Case 1: Typical skew lines ----------------------------------------
    // Line 1 runs along X at y=0, z=0
    // Line 2 runs along Y at x=1, z=2
    print_result(
        "Case 1: Skew lines (X-axis vs Y-axis, offset)",
        vec3{ -3.f, 0.f, 0.f }, vec3{  3.f, 0.f, 0.f },   // line 1: X-axis
        vec3{  1.f,-3.f, 2.f }, vec3{  1.f, 3.f, 2.f });   // line 2: Y-axis at x=1,z=2
    // Expected: p1=(1,0,0), p2=(1,0,2), distance=2

    // ---- Case 2: Intersecting lines ----------------------------------------
    // Both lines lie in the XY plane and cross at the origin
    print_result(
        "Case 2: Intersecting lines (cross at origin)",
        vec3{ -2.f,-2.f, 0.f }, vec3{  2.f, 2.f, 0.f },   // line 1: y=x
        vec3{ -2.f, 2.f, 0.f }, vec3{  2.f,-2.f, 0.f });   // line 2: y=-x
    // Expected: p1=p2=(0,0,0), distance=0

    // ---- Case 3: Parallel lines --------------------------------------------
    // Both lines run along X, separated by 3 units in Z
    print_result(
        "Case 3: Parallel lines (both along X, z=0 vs z=3)",
        vec3{ -1.f, 0.f, 0.f }, vec3{  4.f, 0.f, 0.f },   // line 1: z=0
        vec3{  0.f, 0.f, 3.f }, vec3{  2.f, 0.f, 3.f });   // line 2: z=3
    // Expected: distance=3, parallel=true

    // ---- Case 4: Skew lines at arbitrary angles ----------------------------
    print_result(
        "Case 4: Skew lines at arbitrary angles",
        vec3{  0.f, 0.f, 0.f }, vec3{  1.f, 1.f, 0.f },   // line 1: direction (1,1,0)
        vec3{  0.f, 1.f, 1.f }, vec3{  1.f, 1.f, 2.f });   // line 2: direction (1,0,1)

    // ---- Case 5: Nearly parallel lines (stress test) -----------------------
    print_result(
        "Case 5: Nearly parallel lines",
        vec3{  0.f, 0.f, 0.f }, vec3{ 10.f, 0.f, 0.f },
        vec3{  0.f, 1.f, 0.f }, vec3{ 10.f, 1.0001f, 0.f });

    return 0;
}
