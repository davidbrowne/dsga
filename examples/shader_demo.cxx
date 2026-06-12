// SPDX-License-Identifier: BSL-1.0
// To the extent copyright is held by the author, this file is licensed
// under the Boost Software License 1.0. To the extent it is not,
// this file is dedicated to the public domain.
// 
// shader_demo.cxx
//
// A CPU-side "software shader" written entirely with dsga vec/mat types and
// functions, mimicking the structure of a GLSL fragment shader program.
//
// Scene: a sphere above a checkerboard ground plane, lit with Blinn-Phong
// shading (ambient + diffuse + specular), a hard shadow, and a sky gradient.
//
// Output: render.ppm  (plain-text Netpbm format, open in any image viewer)
//
// Build (C++20):
//   g++ -std=c++20 -O2 -I<path/to/dsga/include> -o shader_demo shader_demo.cxx
//   cl /std:c++20 /O2 /I<path\to\dsga\include> shader_demo.cxx
//
// Requires: dsga.hxx  (single-header, copy into your include path)

#include "dsga.hxx"

#include <array>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <numbers>

// ---------------------------------------------------------------------------
// Convenience aliases (mirrors common GLSL short-hand)
// ---------------------------------------------------------------------------

using vec2 = dsga::vec2;
using vec3 = dsga::vec3;
using vec4 = dsga::vec4;
using mat3 = dsga::mat3;
using mat4 = dsga::mat4;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

inline constexpr float PI  = std::numbers::pi_v<float>;
inline constexpr float INF = std::numeric_limits<float>::infinity();

// ---------------------------------------------------------------------------
// "Uniforms" — scene parameters set once, read by every fragment invocation
// ---------------------------------------------------------------------------

struct Uniforms
{
    // Camera
    vec3  cam_origin  { 0.f, 1.2f, 4.f };
    vec3  cam_target  { 0.f, 0.4f, 0.f };
    float cam_fov_deg { 60.f };

    // Sphere
    vec3  sphere_center { 0.f, 0.75f, 0.f };
    float sphere_radius { 0.75f };
    vec3  sphere_color  { 0.20f, 0.55f, 0.95f };  // blue-ish

    // Ground plane  (y = 0)
    vec3  checker_a { 0.90f, 0.90f, 0.90f };       // light tile
    vec3  checker_b { 0.25f, 0.25f, 0.25f };        // dark tile
    float checker_scale { 1.0f };

    // Directional light
    vec3  light_dir   { dsga::normalize(vec3{ 1.f, 2.f, 1.5f }) };
    vec3  light_color { 1.f, 0.95f, 0.85f };        // warm white
    vec3  ambient     { 0.08f, 0.10f, 0.14f };

    // Sky gradient colours
    vec3  sky_horizon { 0.70f, 0.82f, 1.00f };
    vec3  sky_zenith  { 0.25f, 0.50f, 0.95f };
};

// ---------------------------------------------------------------------------
// Ray
// ---------------------------------------------------------------------------

struct Ray
{
    vec3 origin;
    vec3 dir;      // unit length

    vec3 at(float t) const noexcept { return origin + t * dir; }
};

// ---------------------------------------------------------------------------
// Intersection helpers
// ---------------------------------------------------------------------------

// Returns the smallest positive t at which the ray hits the sphere,
// or INF if no hit.
float intersect_sphere(const Ray& r, vec3 center, float radius) noexcept
{
    vec3  oc = r.origin - center;
    float a  = dsga::dot(r.dir, r.dir);
    float b  = 2.f * dsga::dot(oc, r.dir);
    float c  = dsga::dot(oc, oc) - radius * radius;
    float d  = b * b - 4.f * a * c;

    if (d < 0.f) return INF;

    float sq = std::sqrt(d);
    float t0 = (-b - sq) / (2.f * a);
    float t1 = (-b + sq) / (2.f * a);

    if (t0 > 1e-4f) return t0;
    if (t1 > 1e-4f) return t1;
    return INF;
}

// Returns the positive t at which the ray hits the plane y == 0,
// or INF if no hit (ray parallel to or pointing away from plane).
float intersect_plane_y0(const Ray& r) noexcept
{
    if (std::abs(r.dir.y) < 1e-6f) return INF;
    float t = -r.origin.y / r.dir.y;
    return (t > 1e-4f) ? t : INF;
}

// ---------------------------------------------------------------------------
// Shading helpers (all pure-dsga, no raw float arrays)
// ---------------------------------------------------------------------------

// Blinn-Phong specular term
float specular(vec3 view_dir, vec3 normal, vec3 light_dir_to_light, float shininess) noexcept
{
    vec3  H     = dsga::normalize(view_dir + light_dir_to_light);
    float n_dot_h = dsga::max(dsga::dot(normal, H), 0.f);
    return std::pow(n_dot_h, shininess);
}

// Sky color based on ray direction — a simple zenith/horizon gradient
vec3 sky_color(vec3 dir, const Uniforms& u) noexcept
{
    float t = dsga::clamp(0.5f * (dir.y + 1.f), 0.f, 1.f);
    return dsga::mix(u.sky_horizon, u.sky_zenith, t);
}

// Checkerboard ground color at world-space XZ position
vec3 checker_color(vec3 hit, const Uniforms& u) noexcept
{
    // scale, floor, parity — done with dsga component-wise ops
    vec2  scaled   = vec2{ hit.x, hit.z } * u.checker_scale;
    // use std::floor on each component (dsga::floor maps directly to std::floor)
    vec2  floored  = dsga::floor(scaled);
    int   parity   = (static_cast<int>(floored.x) + static_cast<int>(floored.y)) & 1;
    return (parity == 0) ? u.checker_a : u.checker_b;
}

// ---------------------------------------------------------------------------
// Fragment function — called once per pixel, returns a linear-space RGB color
// ---------------------------------------------------------------------------

vec3 fragment(vec2 uv,          // normalized device coords in [-1, 1]
              const Uniforms& u) noexcept
{
    // ---- Build camera ray ------------------------------------------------
    // We construct an explicit camera basis matrix from look-at vectors.

    vec3  fwd   = dsga::normalize(u.cam_target - u.cam_origin);
    vec3  right = dsga::normalize(dsga::cross(fwd, vec3{ 0.f, 1.f, 0.f }));
    vec3  up    = dsga::cross(right, fwd);

    // mat3 whose columns are right, up, -fwd (a standard view-to-world basis)
    mat3  cam_basis{ right, up, -fwd };

    float half_h   = std::tan((u.cam_fov_deg * 0.5f) * PI / 180.f);
    vec3  ray_dir  = cam_basis * vec3{ uv.x * half_h, uv.y * half_h, -1.f };
    ray_dir        = dsga::normalize(ray_dir);

    Ray ray{ u.cam_origin, ray_dir };

    // ---- Intersect scene -------------------------------------------------

    float t_sphere = intersect_sphere(ray, u.sphere_center, u.sphere_radius);
    float t_plane  = intersect_plane_y0(ray);

    // ---- Shade -----------------------------------------------------------

    // Nothing hit → sky
    if (t_sphere >= INF && t_plane >= INF)
        return sky_color(ray_dir, u);

    vec3 color;
    vec3 hit_pos;
    vec3 normal;
    vec3 albedo;
    float shininess;

    if (t_sphere < t_plane)
    {
        // --- Sphere hit ---
        hit_pos   = ray.at(t_sphere);
        normal    = dsga::normalize(hit_pos - u.sphere_center);
        albedo    = u.sphere_color;
        shininess = 64.f;
    }
    else
    {
        // --- Ground plane hit ---
        hit_pos   = ray.at(t_plane);
        normal    = vec3{ 0.f, 1.f, 0.f };
        albedo    = checker_color(hit_pos, u);
        shininess = 12.f;
    }

    // ---- Shadow — re-trace a shadow ray toward the light -----------------

    Ray shadow_ray{ hit_pos + normal * 1e-3f, u.light_dir };
    float t_shadow = intersect_sphere(shadow_ray, u.sphere_center, u.sphere_radius);
    float shadow   = (t_shadow < INF) ? 0.f : 1.f;

    // ---- Blinn-Phong lighting --------------------------------------------

    vec3  view_dir  = dsga::normalize(u.cam_origin - hit_pos);
    float n_dot_l   = dsga::max(dsga::dot(normal, u.light_dir), 0.f);
    float spec      = specular(view_dir, normal, u.light_dir, shininess);

    vec3 ambient_term  = u.ambient * albedo;
    vec3 diffuse_term  = shadow * n_dot_l * u.light_color * albedo;
    vec3 specular_term = shadow * spec    * u.light_color * vec3{ 0.4f, 0.4f, 0.4f };

    color = ambient_term + diffuse_term + specular_term;

    // ---- Sky reflection on sphere (fake environment) --------------------

    if (t_sphere < t_plane)
    {
        vec3 refl_dir = dsga::reflect(-view_dir, normal);
        vec3 env      = sky_color(refl_dir, u);
        // mix in a little reflection (Schlick-inspired, not full PBR)
        float cos_i   = dsga::clamp(1.f - dsga::dot(view_dir, normal), 0.f, 1.f);
        float fresnel = 0.04f + 0.96f * cos_i * cos_i * cos_i * cos_i * cos_i;
        color         = dsga::mix(color, env, fresnel * 0.5f);
    }

    return color;
}

// ---------------------------------------------------------------------------
// Gamma correction and tone mapping (post-process pass over the framebuffer)
// ---------------------------------------------------------------------------

// Reinhard tone mapping followed by gamma encode
vec3 tone_map(vec3 linear_color) noexcept
{
    // Reinhard (per-channel)
    vec3 mapped = linear_color / (linear_color + vec3{ 1.f, 1.f, 1.f });

    // Gamma 2.2 encode with component-wise pow
    auto gamma_encode = [](float x) { return std::pow(dsga::clamp(x, 0.f, 1.f), 1.f / 2.2f); };
    return vec3{ gamma_encode(mapped.x), gamma_encode(mapped.y), gamma_encode(mapped.z) };
}

// ---------------------------------------------------------------------------
// main — "render loop" (rasterizer / dispatch, equivalent to gl_FragCoord loop)
// ---------------------------------------------------------------------------

int main()
{
    constexpr int WIDTH  = 800;
    constexpr int HEIGHT = 450;

    Uniforms u;   // all scene parameters live here — like a GLSL uniform block

    // Framebuffer stored as flat array of vec3 (linear light before tone-map)
    std::vector<vec3> framebuffer(static_cast<std::size_t>(WIDTH * HEIGHT));

    // ---- Fragment dispatch (equivalent to the GPU rasterizer issuing
    //      one fragment invocation per pixel) ----------------------------

    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            // gl_FragCoord → NDC, with Y flipped so +Y is up
            vec2 uv{
                (2.f * static_cast<float>(x) - static_cast<float>(WIDTH))  / static_cast<float>(HEIGHT),
                (static_cast<float>(HEIGHT) - 2.f * static_cast<float>(y)) / static_cast<float>(HEIGHT)
            };

            framebuffer[static_cast<std::size_t>(y * WIDTH + x)] = fragment(uv, u);
        }
    }

    // ---- Post-process: tone map + gamma encode --------------------------

    for (auto& pixel : framebuffer)
        pixel = tone_map(pixel);

    // ---- Write PPM -------------------------------------------------------

    std::ofstream out("render.ppm", std::ios::binary);
    if (!out)
    {
        std::cerr << "Error: could not open render.ppm for writing.\n";
        return 1;
    }

    out << "P3\n" << WIDTH << ' ' << HEIGHT << "\n255\n";

    for (const auto& pixel : framebuffer)
    {
        // Convert [0,1] float → [0,255] uint8 using dsga component access
        auto to_byte = [](float v) -> int
        {
            return static_cast<int>(dsga::clamp(v, 0.f, 1.f) * 255.f + 0.5f);
        };
        out << to_byte(pixel.x) << ' '
            << to_byte(pixel.y) << ' '
            << to_byte(pixel.z) << '\n';
    }

    std::cout << "Rendered " << WIDTH << "x" << HEIGHT
              << " pixels to render.ppm\n";
    return 0;
}
