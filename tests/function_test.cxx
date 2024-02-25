
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <numbers>
#include "dsga.hxx"
using namespace dsga;

#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

constexpr auto single_ordinate_cubic_bezier_eval(vec4 cubic_control_points, float t) noexcept
{
	auto quadratic_control_points = mix(cubic_control_points.xyz, cubic_control_points.yzw, t);
	auto linear_control_points = mix(quadratic_control_points.xy, quadratic_control_points.yz, t);
	return mix((float)linear_control_points.x, (float)linear_control_points.y, t);
}

constexpr auto simple_cubic_bezier_eval(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) noexcept
{
	auto AoS = mat4x2(p0, p1, p2, p3);

	return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
	{
		return vec2(single_ordinate_cubic_bezier_eval(AoS.row(Is), t)...);
	}(std::make_index_sequence<2u>{});
}

// functions are implemented without regard to any specific dimension (except geometric functions), so we can test generically
TEST_SUITE("test functions")
{
	TEST_CASE("vector angle and trigonometry functions")
	{
		constexpr float my_pi = std::numbers::pi_v<float>;
		fvec3 degs(30, 45, 60);
		fvec3 rads(my_pi / 6, my_pi / 4, my_pi / 3);

		SUBCASE("radians and degrees")
		{
			// radians()
			auto my_rads = radians(degs);
			CHECK_EQ(my_rads, rads);
			CHECK_EQ(fvec3(radians(30.f), radians(45.f), radians(60.f)), rads);

			// degrees()
			auto my_degs = degrees(rads);
			CHECK_EQ(my_degs, degs);
			CHECK_EQ(fvec3(degrees(my_pi / 6), degrees(my_pi / 4), degrees(my_pi / 3)), degs);
		}

		SUBCASE("basic trig")
		{
			// sin()
			CHECK_EQ(sin(radians(fvec2(30, 90))), fvec2(0.5f, 1));
			CHECK_EQ(fvec2(dsga::sin(radians(30.f)), dsga::sin(radians(90.f))), sin(radians(fvec2(30, 90))));

			// cos()
			CHECK_EQ(cos(radians(fvec2(0, 180))), fvec2(1, -1));
			CHECK_EQ(fvec2(dsga::cos(radians(0.f)), dsga::cos(radians(180.f))), cos(radians(fvec2(0, 180))));

			// tan()
			CHECK_EQ(tan(radians(fvec2(45, 0))), fvec2(1, 0));
			CHECK_EQ(fvec2(dsga::tan(radians(45.f)), dsga::tan(radians(0.f))), tan(radians(fvec2(45, 0))));

			// asin()
			CHECK_EQ(asin(fvec2(0.5f, 1)), radians(fvec2(30, 90))) ;
			CHECK_EQ(fvec2(dsga::asin(0.5f), dsga::asin(1.f)), asin(fvec2(0.5f, 1)));

			// acos()
			CHECK_EQ(acos(fvec2(1, -1)), radians(fvec2(0, 180)));
			CHECK_EQ(fvec2(dsga::acos(1.f), dsga::acos(-1.f)), acos(fvec2(1, -1)));

			// atan() (both 1 arg and 2 arg)
			CHECK_EQ(atan(fvec2(1, 0)), radians(fvec2(45, 0)));
			CHECK_EQ(fvec2(dsga::atan(1.f), dsga::atan(0.f)), atan(fvec2(1, 0)));

			CHECK_EQ(atan(fvec2(1, -1), fvec2(-1, -1)), radians(fvec2(135, -135)));
			CHECK_EQ(fvec2(dsga::atan(1.f, -1.f), dsga::atan(-1.f, -1.f)), atan(fvec2(1, -1), fvec2(-1, -1)));

		}

		SUBCASE("hyperbolic trig")
		{
			dvec3 vals(-2, 0, 2);

			// sinh()
			auto sinhs = sinh(vals);
			CHECK_EQ(dvec3(dsga::sinh((double)vals.x), dsga::sinh((double)vals.y), dsga::sinh((double)vals.z)), sinhs);

			// cosh()
			auto coshs = cosh(vals);
			CHECK_EQ(dvec3(dsga::cosh((double)vals.x), dsga::cosh((double)vals.y), dsga::cosh((double)vals.z)), coshs);

			// asinh()
			auto asinhs = asinh(sinhs);
			CHECK_EQ(vals, asinhs);
			CHECK_EQ(dvec3(dsga::asinh((double)sinhs.x), dsga::asinh((double)sinhs.y), dsga::asinh((double)sinhs.z)), asinhs);

			// acosh()
			auto acoshs = acosh(coshs);
			CHECK_EQ(dvec3(2, 0, 2), acoshs);
			CHECK_EQ(dvec3(dsga::acosh((double)coshs.x), dsga::acosh((double)coshs.y), dsga::acosh((double)coshs.z)), acoshs);

			// tanh(), atanh()
			dvec3 more_vals(-1, 0, 1);
			auto atanhs = atanh(tanh(more_vals));
			CHECK_EQ(dvec3(dsga::atanh(dsga::tanh((double)more_vals.x)), dsga::atanh(dsga::tanh((double)more_vals.y)), dsga::atanh(dsga::tanh((double)more_vals.z))), atanhs);
			CHECK_EQ(atanh(sinh(more_vals) / cosh(more_vals)), atanhs);
			CHECK_EQ(dvec3(dsga::atanh(dsga::sinh((double)more_vals.x) / dsga::cosh((double)more_vals.x)), dsga::atanh(dsga::sinh((double)more_vals.y) / dsga::cosh((double)more_vals.y)), dsga::atanh(dsga::sinh((double)more_vals.z) / dsga::cosh((double)more_vals.z))), atanh(sinh(more_vals) / cosh(more_vals)));
		}
	}

	TEST_CASE("vector exponential functions")
	{
		SUBCASE("non-sqrt related")
		{
			// pow(), e.g., pow(2, 10) = 2^10 = 1024
			vec3 pow_bases(2, std::numbers::e_v<float>, 10);
			vec3 pow_exps(std::numbers::log2e_v<float>, std::numbers::ln10_v<float>, 3);
			vec3 pow_result = pow(pow_bases, pow_exps);
			CHECK_EQ(pow_result, vec3(std::numbers::e_v<float>, 10, 1000));
			CHECK_EQ(vec3(dsga::pow(2.f, std::numbers::log2e_v<float>), dsga::pow(std::numbers::e_v<float>, std::numbers::ln10_v<float>), dsga::pow(10.f, 3.f)), pow_result);

			// exp(), e.g., e^x = exp(x)
			vec2 exp_vals(std::numbers::ln10_v<float>, std::numbers::ln2_v<float>);
			vec2 exp_result = exp(exp_vals);
			CHECK_EQ(exp_result, vec2(10, 2));
			CHECK_EQ(vec2(dsga::exp(std::numbers::ln10_v<float>), dsga::exp(std::numbers::ln2_v<float>)), exp_result);

			// log(), e.g., log(x) = ln(x) = log<base e>(x)
			vec2 log_vals(10, 2);
			vec2 log_result = log(log_vals);
			CHECK_EQ(log_result, vec2(std::numbers::ln10_v<float>, std::numbers::ln2_v<float>));
			CHECK_EQ(vec2(dsga::log(10.f), dsga::log(2.f)), log_result);

			// exp2(), e.g., 2^x = exp2(x)
			vec3 exp2_vals(std::numbers::log2e_v<float>, 2, 10);
			vec3 exp2_result = exp2(exp2_vals);
			CHECK_EQ(exp2_result, vec3(std::numbers::e_v<float>, 4, 1024));
			CHECK_EQ(vec3(dsga::exp2(std::numbers::log2e_v<float>), dsga::exp2(2.f), dsga::exp2(10.f)), exp2_result);

			// log2(), e.g., log2(x) = lb(x) = log<base 2>(x)
			vec3 log2_vals(std::numbers::e_v<float>, 4, 1024);
			vec3 log2_result = log2(log2_vals);
			CHECK_EQ(log2_result, vec3(std::numbers::log2e_v<float>, 2, 10));
			CHECK_EQ(vec3(dsga::log2(std::numbers::e_v<float>), dsga::log2(4.f), dsga::log2(1024.f)), log2_result);
		}

		SUBCASE("sqrt related")
		{
			//
			// cxcm constexpr versions of these may be off by an ulp from standard library.
			// Because these implementation results do not exactly match the MSVC's standard,
			// you have to explicitly opt-in to these constexpr versions of sqrt() and
			// inversesqrt() via define macro CXCM_APPROXIMATIONS_ALLOWED (prior to
			// including cxcm.hxx or dsga.hxx).
			//
			// we need a better analysis of how the constexpr version differs from std::sqrt().
			//

			// sqrt()
			auto vals = vec3(4, 16, 64);
			auto sqrtvals = sqrt(vals);
			CHECK_EQ(sqrtvals, vec3(2, 4, 8));
			CHECK_EQ(vec3(dsga::sqrt(4.f), dsga::sqrt(16.f), dsga::sqrt(64.f)), sqrtvals);
			CHECK_EQ(dsga::sqrt(dvec2(2.0, 3.0)), dvec2(std::numbers::sqrt2_v<double>, std::numbers::sqrt3_v<double>));
			CHECK_EQ(dsga::sqrt(vec2(2.0f, 3.0f)), vec2(std::numbers::sqrt2_v<float>, std::numbers::sqrt3_v<float>));
			CHECK_EQ(std::numbers::phi_v<double>, (1.0 + dsga::sqrt(5.0)) / 2.0);
			CHECK_EQ(std::numbers::phi_v<float>, (1.0f + dsga::sqrt(5.0f)) / 2.0f);

			// inversesqrt()
			auto invsqrtvals = inversesqrt(vals);
			CHECK_EQ(invsqrtvals, vec3(0.5, 0.25, 0.125));
			CHECK_EQ(vec3(inversesqrt(4.f), inversesqrt(16.f), inversesqrt(64.f)), invsqrtvals);
			CHECK_EQ(dsga::inversesqrt(dvec2(3.0, std::numbers::pi_v<double>)), dvec2(std::numbers::inv_sqrt3_v<double>, std::numbers::inv_sqrtpi_v<double>));
			CHECK_EQ(dsga::inversesqrt(fscal(3.0f)), fscal(std::numbers::inv_sqrt3_v<float>));

			// off by 1 ulp
//			CHECK_EQ(std::numbers::inv_sqrtpi_v<float>, dsga::inversesqrt(std::numbers::pi_v<float>));

			// fast_inversesqrt()
			CHECK_EQ(dsga::fast_inversesqrt(std::numbers::pi_v<double>), std::numbers::inv_sqrtpi_v<double>);
			CHECK_EQ(dsga::fast_inversesqrt(fscal(3.0f)), fscal(std::numbers::inv_sqrt3_v<float>));
		}
	}

	TEST_CASE("vector common functions")
	{
		SUBCASE("most common")
		{
			ivec3 int_data(-10, 0, 9);
			vec4 float_data(-1.75, -0.25, 0.5, 1.0);
			dvec4 double_data(11.5, 12.5, -11.5, -12.5);

			// abs()
			auto abs_vals = abs(int_data);
			CHECK_EQ(abs_vals, ivec3(10, 0, 9));
			CHECK_EQ(abs(float_data), fvec4(1.75f, 0.25f, 0.5f, 1.0f));
			CHECK_EQ(abs(double_data), dvec4(11.5, 12.5, 11.5, 12.5));
			CHECK_EQ(ivec3(dsga::abs(-10), dsga::abs(0), dsga::abs(9)), abs_vals);

			// sign()
			auto sign_vals = sign(int_data);
			CHECK_EQ(sign_vals, ivec3(-1, 0, 1));
			CHECK_EQ(sign(float_data), fvec4(-1.f, -1.f, 1.f, 1.f));
			CHECK_EQ(sign(double_data), dvec4(1., 1., -1., -1.));
			CHECK_EQ(ivec3(sign(-10), sign(0), sign(9)), sign_vals);

			// floor()
			auto floor_vals = floor(float_data);
			CHECK_EQ(floor_vals, vec4(-2, -1, 0, 1));
			CHECK_EQ(vec4(dsga::floor((float)float_data.x), dsga::floor((float)float_data.y), dsga::floor((float)float_data.z), dsga::floor((float)float_data.w)), floor_vals);

			// trunc()
			auto trunc_vals = trunc(float_data);
			CHECK_EQ(trunc_vals, vec4(-1, 0, 0, 1));
			CHECK_EQ(vec4(dsga::trunc((float)float_data.x), dsga::trunc((float)float_data.y), dsga::trunc((float)float_data.z), dsga::trunc((float)float_data.w)), trunc_vals);

			// round()
			auto round_vals = round(float_data);
			CHECK_EQ(round_vals, vec4(-2, 0, 1, 1));
			CHECK_EQ(vec4(dsga::round((float)float_data.x), dsga::round((float)float_data.y), dsga::round((float)float_data.z), dsga::round((float)float_data.w)), round_vals);

			// roundEven()
			auto re_float_data = roundEven(float_data);
			CHECK_EQ(re_float_data, vec4(-2, 0, 0, 1));
			auto re_double_data = roundEven(double_data);
			CHECK_EQ(re_double_data, dvec4(12, 12, -12, -12));
			CHECK_EQ(dvec4(dsga::roundEven((double)double_data.x), dsga::roundEven((double)double_data.y), dsga::roundEven((double)double_data.z), dsga::roundEven((double)double_data.w)), re_double_data);

			// ceil()
			auto ceil_vals = ceil(float_data);
			CHECK_EQ(ceil_vals, vec4(-1, 0, 1, 1));
			CHECK_EQ(vec4(dsga::ceil((float)float_data.x), dsga::ceil((float)float_data.y), dsga::ceil((float)float_data.z), dsga::ceil((float)float_data.w)), ceil_vals);

			// fract()
			auto fract_vals = fract(float_data);
			CHECK_EQ(fract_vals, vec4(0.25, 0.75, 0.5, 0));
			CHECK_EQ(vec4(dsga::fract((float)float_data.x), dsga::fract((float)float_data.y), dsga::fract((float)float_data.z), dsga::fract((float)float_data.w)), fract_vals);

			vec4 mod_x_data(7.75, -12.25, 4, -0.5);
			vec4 mod_y_data(2.25, -2.5, 3.125, -0.75);

			// mod()
			auto mod_vals_vector = mod(mod_x_data, mod_y_data);
			auto mod_vals_scalar = mod(mod_x_data, 1.625f);

			CHECK_EQ(mod_vals_vector, vec4(1, -2.25, 0.875, -0.5));
			CHECK_EQ(mod_vals_scalar, vec4(1.25, 0.75, 0.75, 1.125));
			CHECK_EQ(vec4(dsga::mod((float)mod_x_data.x, (float)mod_y_data.x), dsga::mod((float)mod_x_data.y, (float)mod_y_data.y), dsga::mod((float)mod_x_data.z, (float)mod_y_data.z), dsga::mod((float)mod_x_data.w, (float)mod_y_data.w)), mod_vals_vector);

			// modf()
			vec4 modf_int_part;
			auto modf_vals = modf(mod_y_data, modf_int_part);
			CHECK_EQ(modf_int_part, vec4(2, -2, 3, -0));
			CHECK_EQ(modf_vals, vec4(0.25, -0.5, 0.125, -0.75));
			std::array<float, 4> modf_int_part_scalars;
			CHECK_EQ(vec4(dsga::modf((float)mod_y_data.x, modf_int_part_scalars[0]), dsga::modf((float)mod_y_data.y, modf_int_part_scalars[1]), dsga::modf((float)mod_y_data.z, modf_int_part_scalars[2]), dsga::modf((float)mod_y_data.w, modf_int_part_scalars[3])), modf_vals);
			CHECK_EQ(to_vector(modf_int_part_scalars), modf_int_part);
		}

		SUBCASE("in range functions")
		{
			vec4 x(10, -8, 4, 0);
			vec4 y(7, -9, 4, 1);
			float boundary{ 0.5f };

			// min()
			auto min_vals = min(x, y);
			CHECK_EQ(min_vals, vec4(7, -9, 4, 0));
			auto min_boundary_vals = min(x, boundary);
			CHECK_EQ(min_boundary_vals, vec4(0.5f, -8, 0.5f, 0));
			CHECK_EQ(vec4(min(10.f, 7.f), min(-8.f, -9.f), min(4.f, 4.f), min(0.f, 1.f)), min_vals);

			// max()
			auto max_vals = max(x, y);
			CHECK_EQ(max_vals, vec4(10, -8, 4, 1));
			auto max_boundary_vals = max(x, boundary);
			CHECK_EQ(max_boundary_vals, vec4(10, 0.5f, 4, 0.5f));
			CHECK_EQ(vec4(max(10.f, 7.f), max(-8.f, -9.f), max(4.f, 4.f), max(0.f, 1.f)), max_vals);

			// clamp()
			dvec4 more_vals(-4, 3, 2, -2.5);
			dvec4 high_vec(0, 2, 4, -3);
			dvec4 low_vec(-3, 0, 1, -4);
			double high_scalar{ 2.5 };
			double low_scalar{ -3 };

			auto vector_clamp = clamp(more_vals, low_vec, high_vec);
			CHECK_EQ(vector_clamp, dvec4(-3, 2, 2, -3));
			auto scalar_clamp = clamp(more_vals, low_scalar, high_scalar);
			CHECK_EQ(scalar_clamp, dvec4(-3, 2.5, 2, -2.5));
			CHECK_EQ(dvec4(clamp(-4., -3., 0.), clamp(3., 0., 2.), clamp(2., 1., 4.), clamp(-2.5, -4., -3.)), vector_clamp);

			// mix()
			
			// first version
			vec4 vector_mix_steps(0, .5, .75, 1);
			float scalar_mix_step{0.25};
			
			auto vector_mix_vals = mix(x, y, vector_mix_steps);
			CHECK_EQ(vector_mix_vals, vec4(10, -8.5, 4, 1));
			auto scalar_mix_vals = mix(x, y, scalar_mix_step);
			CHECK_EQ(scalar_mix_vals, vec4(9.25, -8.25, 4, 0.25));
			CHECK_EQ(vec4(mix(10.f, 7.f, 0.f), mix(-8.f, -9.f, 0.5f), mix(4.f, 4.f, 0.75f), mix(0.f, 1.f, 1.f)), vector_mix_vals);

			// second version
			bvec4 mix_toggles(true, false, true, false);
			auto bool_mix_vals = mix(x, y, mix_toggles);
			CHECK_EQ(bool_mix_vals, vec4(7, -8, 4, 0));
			CHECK_EQ(vec4(mix(10.f, 7.f, true), mix(-8.f, -9.f, false), mix(4.f, 4.f, true), mix(0.f, 1.f, false)), bool_mix_vals);

			// step()
			auto vector_step_vals = step(x, y);
			CHECK_EQ(vector_step_vals, vec4(0, 0, 1, 1));
			CHECK_EQ(vec4(step(10.f, 7.f), step(-8.f, -9.f), step(4.f, 4.f), step(0.f, 1.f)), vector_step_vals);

			float edge{ 3.75 };
			auto scalar_step_vals = step(edge, x);
			CHECK_EQ(scalar_step_vals, vec4(1, 0, 1, 0));

			// smoothstep()
			vec4 smoothstep_edge0(-3, 0, 1, -4);
			vec4 smoothstep_edge1(0, 2, 7, -3);

			auto vector_smoothstep_vals = smoothstep(smoothstep_edge0, smoothstep_edge1, x);
			CHECK_EQ(vector_smoothstep_vals, vec4(1, 0, 0.5, 1));
			CHECK_EQ(vec4(smoothstep(-3.f, 0.f, 10.f), smoothstep(0.f, 2.f, -8.f), smoothstep(1.f, 7.f, 4.f), smoothstep(-4.f, -3.f, 0.f)), vector_smoothstep_vals);

			float scalar_edge0{ 2 };
			float scalar_edge1{ 6 };

			auto scalar_smoothstep_vals = smoothstep(scalar_edge0, scalar_edge1, x);
			CHECK_EQ(scalar_smoothstep_vals, vec4(1, 0, 0.5, 0));

			//
			// real-world example of using mix() and other functions -- cubic bezier evaluation
			//

			vec2 p0(2, 2);
			vec2 p1(5, 4);
			vec2 p2(3, 5);
			vec2 p3(8, 3);
			float t1{ 0.25f };
			float t2{ 0.75f };

			auto val1 = simple_cubic_bezier_eval(p0, p1, p2, p3, t1);
			CHECK_EQ(val1, vec2(3.5, 3.28125));

			auto val2 = simple_cubic_bezier_eval(p0, p1, p2, p3, t2);
			CHECK_EQ(val2, vec2(5.375, 3.96875));
		}

		SUBCASE("bit changing functions")
		{
			// floatBitsToInt()
			auto floatToInt = floatBitsToInt(vec4(123.125, 6967.0e+4, -654.0, std::numbers::pi_v<float>));
			CHECK_EQ(floatToInt, ivec4(1123434496, 1283777166, -1004306432, 1078530011));
			CHECK_EQ(ivec4(floatBitsToInt(123.125f), floatBitsToInt(6967.0e+4f), floatBitsToInt(-654.0f), floatBitsToInt(std::numbers::pi_v<float>)), floatToInt);

			// floatBitsToUint()
			auto floatToUint = floatBitsToUint(vec4(87.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrt3_v<float>));
			CHECK_EQ(floatToUint, uvec4(645673883u, 1283777166u, 3290660864u, 1058262330u));
			CHECK_EQ(uvec4(floatBitsToUint(87.5e-17f), floatBitsToUint(6967.0e+4f), floatBitsToUint(-654.0f), floatBitsToUint(std::numbers::inv_sqrt3_v<float>)), floatToUint);

			// doubleBitsToLongLong()
			auto doubleToLongLong = doubleBitsToLongLong(dvec4(123.125, 6967.0e+4, -654.0, std::numbers::phi_v<double>));
			CHECK_EQ(doubleToLongLong, llvec4(4638364568563744768ll, 4724447884039159808ll, -4574408176199270400ll, 4609965796441453736ll));
			CHECK_EQ(llvec4(doubleBitsToLongLong(123.125), doubleBitsToLongLong(6967.0e+4), doubleBitsToLongLong(-654.0), doubleBitsToLongLong(std::numbers::phi_v<double>)), doubleToLongLong);

			// doubleBitsToUlongLong()
			auto doubleToUlongLong = doubleBitsToUlongLong(dvec4(1.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrtpi_v<double>));
			CHECK_EQ(doubleToUlongLong, ullvec4(4355345018344775537ull, 4724447884039159808ull, 13872335897510281216ull, 4603256987541740397ull));
			CHECK_EQ(ullvec4(doubleBitsToUlongLong(1.5e-17), doubleBitsToUlongLong(6967.0e+4), doubleBitsToUlongLong(-654.0), doubleBitsToUlongLong(std::numbers::inv_sqrtpi_v<double>)), doubleToUlongLong);

			// intBitsToFloat()
			auto intToFloat = intBitsToFloat(ivec4(1123434496, 1283777166, -1004306432, 1078530011));
			CHECK_EQ(intToFloat, vec4(123.125, 6967.0e+4, -654.0, std::numbers::pi_v<float>));
			CHECK_EQ(vec4(intBitsToFloat(1123434496), intBitsToFloat(1283777166), intBitsToFloat(-1004306432), intBitsToFloat(1078530011)), intToFloat);

			// uintBitsToFloat()
			auto uintToFloat = uintBitsToFloat(uvec4(645673883u, 1283777166u, 3290660864u, 1058262330u));
			CHECK_EQ(uintToFloat, vec4(87.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrt3_v<float>));
			CHECK_EQ(vec4(uintBitsToFloat(645673883u), uintBitsToFloat(1283777166u), uintBitsToFloat(3290660864u), uintBitsToFloat(1058262330u)), uintToFloat);

			// longLongBitsToDouble()
			auto longLongToDouble = longLongBitsToDouble(llvec4(4638364568563744768ll, 4724447884039159808ll, -4574408176199270400ll, 4609965796441453736ll));
			CHECK_EQ(longLongToDouble, dvec4(123.125, 6967.0e+4, -654.0, std::numbers::phi_v<double>));
			CHECK_EQ(dvec4(longLongBitsToDouble(4638364568563744768ll), longLongBitsToDouble(4724447884039159808ll), longLongBitsToDouble(-4574408176199270400ll), longLongBitsToDouble(4609965796441453736ll)), longLongToDouble);

			// ulongLongBitsToDouble()
			auto ulongLongToDouble = ulongLongBitsToDouble(ullvec4(4355345018344775537ull, 4724447884039159808ull, 13872335897510281216ull, 4603256987541740397ull));
			CHECK_EQ(ulongLongToDouble, dvec4(1.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrtpi_v<double>));
			CHECK_EQ(dvec4(ulongLongBitsToDouble(4355345018344775537ull), ulongLongBitsToDouble(4724447884039159808ull), ulongLongBitsToDouble(13872335897510281216ull), ulongLongBitsToDouble(4603256987541740397ull)), ulongLongToDouble);
		}

		SUBCASE("other common functions")
		{
			dvec4 bad_nums(std::numeric_limits<double>::quiet_NaN(),
						   0.0,
						   std::numeric_limits<double>::infinity(),
						   -std::numeric_limits<double>::infinity());

			// isnan()
			auto isnan_vals = isnan(bad_nums); 
			CHECK_EQ(isnan_vals, bvec4(true, false, false, false));
			CHECK_EQ(bvec4(isnan(std::numeric_limits<double>::quiet_NaN()), isnan(0.0), isnan(std::numeric_limits<double>::infinity()), isnan(-std::numeric_limits<double>::infinity())), isnan_vals);

			// isinf()
			auto isinf_vals = isinf(bad_nums);
			CHECK_EQ(isinf_vals, bvec4(false, false, true, true));
			CHECK_EQ(bvec4(isinf(std::numeric_limits<double>::quiet_NaN()), isinf(0.0), isinf(std::numeric_limits<double>::infinity()), isinf(-std::numeric_limits<double>::infinity())), isinf_vals);

			// fma()
			dvec3 a(2, 4, 6);
			dvec3 b(3, 5, 7);
			dvec3 c(20, 40, 60);

			auto fma_vals = fma(a, b, c);
			CHECK_EQ(fma_vals, dvec3(26, 60, 102));
			CHECK_EQ(dvec3(dsga::fma(2., 3., 20.), dsga::fma(4., 5., 40.), dsga::fma(6., 7., 60.)), fma_vals);

			// frexp()
			ivec3 exps(0);
			dvec3 frexp_data(10, 20, 30);

			auto frexp_vals = frexp(frexp_data, exps);

			CHECK_EQ(frexp_vals, dvec3(0.625, 0.625, 0.9375));
			CHECK_EQ(exps, ivec3(4, 5, 5));
			std::array<int, 3> exp_array{};
			CHECK_EQ(dvec3(dsga::frexp(10., exp_array[0]), dsga::frexp(20., exp_array[1]), dsga::frexp(30., exp_array[2])), frexp_vals);
			CHECK_EQ(to_vector(exp_array), exps);

			// ldexp()
			dvec3 ld_data(0.625, 0.625, 0.9375);
			ivec3 ld_exps(4, 5, 5);

			auto ldexp_vals = ldexp(ld_data, ld_exps);

			CHECK_EQ(ldexp_vals, dvec3(10, 20, 30));
			CHECK_EQ(dvec3(dsga::ldexp(0.625, 4), dsga::ldexp(0.625, 5), dsga::ldexp(0.9375, 5)), ldexp_vals);
		}
	}

	TEST_CASE("vector geometric functions")
	{
		SUBCASE("generic vector functions")
		{
			// length()
			auto length_val = scal(-12.75f);
			vec2 length_vals1 = vec2(3, 4);
			vec2 length_vals2 = vec2(5, 12);
			auto length0 = length(length_val);
			auto length1 = length(length_vals1);
			auto length2 = length(length_vals2);

			CHECK_EQ(length0, 12.75f);
			CHECK_EQ(length1, 5.f);
			CHECK_EQ(length2, 13.f);

			// distance()
			vec2 p1(12, 23);
			vec2 p2(4, 38);
			auto dist = distance(p1, p2);
			CHECK_EQ(dist, 17.f);

			// dot()
			constexpr float my_pi = std::numbers::pi_v<float>;
			auto dot_val = dot(vec2(1, 0), vec2(cos(my_pi / 4), sin(my_pi / 4)));
			CHECK_EQ(dot_val, std::numbers::sqrt2_v<float> / 2);

			// cross()
			auto x_axis = dvec3(1, 0, 0);
			auto y_axis = dvec3(0, 1, 0);
			auto z_axis = dvec3(0, 0, 1);
			CHECK_EQ(z_axis, cross(x_axis, y_axis));
			CHECK_EQ(x_axis, cross(y_axis, z_axis));
			CHECK_EQ(y_axis, -cross(x_axis, z_axis));

			auto cross_val = cross(vec3(1, 0, 0), vec3(cos(my_pi / 4), sin(my_pi / 4), 0));
			CHECK_EQ(cross_val, vec3(0, 0, std::numbers::sqrt2_v<float> / 2));

			// normalize()
			auto pre_norm = vec4(4, -4, 4, -4);
			auto normed = normalize(pre_norm);
			CHECK_EQ(normed, vec4(0.5, -0.5, 0.5, -0.5));

			auto zeros = dvec4(0.0);
			auto nan_normed = normalize(zeros);
			CHECK_UNARY(all(isnan(nan_normed)));

			auto single_val = scal(-3.5);
			auto single_normed = normalize(single_val);
			CHECK_EQ(single_normed, scal(1));

			auto single_zero = dscal(0.0);
			auto single_nan_normed = normalize(single_zero);
			CHECK_UNARY(all(isnan(single_nan_normed)));
		}

		SUBCASE("'normal' vector functions")
		{
			// reflect()
			auto norm = vec3(0, 0, 1);
			auto I = normalize(vec3(std::numbers::sqrt2_v<float> / 2, -std::numbers::sqrt2_v<float> / 2, 1));
			auto reflect_vec = reflect(I, norm);
			CHECK_EQ(reflect_vec, vec3(0.5, -0.5, -std::numbers::sqrt2_v<float> / 2));

			// faceforward()
			auto ff = faceforward(norm, I, norm);
			CHECK_EQ(ff, -norm);

			// refract()
			auto refract_val = refract(I, norm, 1.0f);
			CHECK_EQ(refract_val, reflect_vec);
		}
	}

	TEST_CASE("vector relational functions")
	{
		SUBCASE("standard relational functions")
		{
			auto v1 = vec3(1, 1, 5);
			auto v2 = vec3(0, 1, 6);

			// lessThan()
			auto lt_vec = lessThan(v1, v2);
			CHECK_EQ(lt_vec, bvec3(false, false, true));

			// lessThanEqual()
			auto lte_vec = lessThanEqual(v1, v2);
			CHECK_EQ(lte_vec, bvec3(false, true, true));

			// greaterThan()
			auto gt_vec = greaterThan(v1, v2);
			CHECK_EQ(gt_vec, bvec3(true, false, false));

			// greaterThanEqual()
			auto gte_vec = greaterThanEqual(v1, v2);
			CHECK_EQ(gte_vec, bvec3(true, true, false));

			// equal()
			auto eq_vec = equal(v1, v2);
			CHECK_EQ(eq_vec, bvec3(false, true, false));

			auto bool_eq_vec = equal(bvec3(true, false, true), bvec3(true, false, false));
			CHECK_EQ(bool_eq_vec, bvec3(true, true, false));

			// notEqual()
			auto neq_vec = notEqual(v1, v2);
			CHECK_EQ(neq_vec, bvec3(true, false, true));

			auto bool_neq_vec = notEqual(bvec3(true, false, true), bvec3(true, false, false));
			CHECK_EQ(bool_neq_vec, bvec3(false, false, true));
		}

		SUBCASE("reduced relational functions")
		{
			auto v0 = vec3(1, 1, 5);
			auto v1 = vec3(1, 4, 8);
			auto v2 = vec3(0, 1, 6);

			// any()
			auto any_true = any(lessThan(v0, v2));
			CHECK_UNARY(any_true);
			auto any_not_true = any(lessThan(v1, v2));
			CHECK_UNARY_FALSE(any_not_true);

			// all()
			auto all_true = all(greaterThan(v1, v2));
			CHECK_UNARY(all_true);
			auto all_not_true = all(greaterThan(v0, v2));
			CHECK_UNARY_FALSE(all_not_true);

			// none()
			auto none_of_them = none(lessThan(v1, v2));
			CHECK_UNARY(none_of_them);
			auto some_of_them = none(greaterThan(v0, v2));
			CHECK_UNARY_FALSE(some_of_them);

			// not() - c++ doesn't allow not(), so logicalNot()
			auto not_true = all(logicalNot(lessThan(v1, v2)));
			CHECK_UNARY(not_true);
			auto not_not_true = all(logicalNot(lessThan(v0, v2)));
			CHECK_UNARY_FALSE(not_not_true);
		}
	}

	TEST_CASE("other vector functions")
	{
		// byteswap()

		dsga::uvec3 uv(0xFFEEDDCC, 0xBBAA9988, 0x01234567);
		auto swapped = dsga::byteswap(uv);
		CHECK_EQ(swapped, dsga::uvec3(0xCCDDEEFF, 0x8899AABB, 0x67452301));

		// swizzle()

		dvec4 v(1, 2, 3, 4);
		ivec4 indexes(0, 1, 2, 3);
		auto runtime_swizzle = swizzle(v, 3, 2, 1);
		auto member_swizzle_swizzle = swizzle(v.wzyx, indexes.w, indexes.z, indexes.y);
		auto runtime_swizzle1 = swizzle(v, 3, 3, 3);
		auto runtime_swizzle2 = swizzle(v, 2);

		CHECK_EQ(runtime_swizzle, basic_vector(4.0, 3.0, 2.0));
		CHECK_EQ(member_swizzle_swizzle, basic_vector(1.0, 2.0, 3.0));
		CHECK_EQ(runtime_swizzle1, basic_vector(4.0, 4.0, 4.0));
		CHECK_EQ(runtime_swizzle2, basic_vector(3.0));
	}

	TEST_CASE("general utility functions")
	{
		// to_underlying()
		enum class bool_flag : bool {};
		enum class int_flag : int {};

		auto bool_arg = bool_flag{true};
		auto int_arg = int_flag{42};

		auto bool_value = dsga::to_underlying(bool_arg);
		auto int_value = dsga::to_underlying(int_arg);

		CHECK_EQ(bool_value, true);
		CHECK_EQ(int_value, 42);
	}

	TEST_CASE("matrix functions")
	{
		// matrixCompMult() - equivalent to a component-wise "matrix/matrix binary operator *" if there was one
		const mat2x3 A(1, 2, 3, 4, 5, 6);
		const mat2x3 B(5, 10, 15, 20, 25, 30);
		const auto mcm = matrixCompMult(A, B);
		CHECK_EQ(mcm, mat2x3(5, 20, 45, 80, 125, 180));

		// outerProduct()
		const auto op = outerProduct(dvec3(3, 5, 7), dvec3(2, 4, 6));
		CHECK_EQ(op, dmat3(6, 10, 14, 12, 20, 28, 18, 30, 42));

		// transpose()
		const dmat4 nums(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
		const dmat4 transposed_nums = transpose(nums);
		CHECK_EQ(transposed_nums, dmat4(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));

		// inverse()
		const auto some4x4 = dmat4(dvec4(1, 0, 2, 2), dvec4(0, 2, 1, 0), dvec4(0, 1, 0, 1), dvec4(1, 2, 1, 4));
		const auto inverse4 = inverse(some4x4);

		CHECK_EQ(inverse4, dmat4(dvec4(-2, 1, -8, 3), dvec4(-0.5, 0.5, -1, 0.5), dvec4(1, 0, 2, -1), dvec4(0.5, -0.5, 2, -0.5)));
		CHECK_EQ(some4x4 * inverse4, dmat4(1));
		CHECK_EQ(inverse4 * some4x4, dmat4(1));

		// determinant()
		const auto det = determinant(some4x4);
		CHECK_EQ(det, 2);

		// cross_matrix() - different ways to get cross product
		const auto u = dvec3(255429.53125, -139725.125, 140508.53125);
		const auto v = dvec3(10487005., 8066347., -11042884.);

		auto uv = cross(u, v);
		auto uv1 = cross_matrix(u) * v;
		auto uv2 = u * cross_matrix(v);

		CHECK_EQ(uv, uv1);
		CHECK_EQ(uv, uv2);
		CHECK_EQ(uv1, uv2);

		// diagonal_matrix()
		auto diagonal_vec = dsga::dvec4(0, 1, 2, 3);
		auto dm1 = dsga::diagonal_matrix(diagonal_vec);
		auto dm2 = dsga::diagonal_matrix(diagonal_vec.wwyz);

		CHECK_EQ(dm1, dsga::dmat4(dsga::dvec4(0, 0, 0, 0),
								  dsga::dvec4(0, 1, 0, 0),
								  dsga::dvec4(0, 0, 2, 0),
								  dsga::dvec4(0, 0, 0, 3)));
		CHECK_EQ(dm2, dsga::dmat4(dsga::dvec4(3, 0, 0, 0),
								  dsga::dvec4(0, 3, 0, 0),
								  dsga::dvec4(0, 0, 1, 0),
								  dsga::dvec4(0, 0, 0, 2)));
	}
}

// T func(const T&)
double apply_test_func(const double &val)
{
	return val * 25;
}

TEST_SUITE("valarray-style vector member functions")
{
	TEST_CASE("vector apply()")
	{
		auto apply_input = dsga::dvec4(0, 1, 2, 3);

		// T func(T)
		CHECK_EQ(apply_input.apply([](double val) -> double { return val * 5; }), dsga::dvec4(0, 5, 10, 15));
		CHECK_EQ(apply_input.xwyz.apply([](double val) -> double { return val * 5; }), dsga::dvec4(0, 15, 5, 10));
		CHECK_EQ(apply_input.apply([](double val) -> double { return std::sin(val); }), dsga::sin(apply_input));

		// T func(const T&)
		CHECK_EQ(apply_input.apply([](const double &val) -> double { return val * 25; }), dsga::dvec4(0, 25, 50, 75));
		CHECK_EQ(apply_input.apply(apply_test_func), dsga::dvec4(0, 25, 50, 75));
	}

	TEST_CASE("vector shift()")
	{
		auto shift_input = dsga::ivec4(1, 2, 3, 4);

		CHECK_EQ(shift_input.shift(13), dsga::ivec4(0));
		CHECK_EQ(shift_input.shift(11), dsga::ivec4(0));
		CHECK_EQ(shift_input.shift(3), dsga::ivec4(4, 0, 0, 0));
		CHECK_EQ(shift_input.shift(1), dsga::ivec4(2, 3, 4, 0));
		CHECK_EQ(shift_input.shift(0), shift_input);
		CHECK_EQ(shift_input.shift(-1), dsga::ivec4(0, 1, 2, 3));
		CHECK_EQ(shift_input.shift(-3), dsga::ivec4(0, 0, 0, 1));
		CHECK_EQ(shift_input.shift(-11), dsga::ivec4(0));
		CHECK_EQ(shift_input.shift(-13), dsga::ivec4(0));
	}

	TEST_CASE("vector cshift()")
	{
		auto cshift_input = dsga::ivec4(1, 2, 3, 4);

		CHECK_EQ(cshift_input.cshift(13), dsga::ivec4(2, 3, 4, 1));
		CHECK_EQ(cshift_input.cshift(11), dsga::ivec4(4, 1, 2, 3));
		CHECK_EQ(cshift_input.cshift(3), dsga::ivec4(4, 1, 2, 3));
		CHECK_EQ(cshift_input.cshift(1), dsga::ivec4(2, 3, 4, 1));
		CHECK_EQ(cshift_input.cshift(0), cshift_input);
		CHECK_EQ(cshift_input.cshift(-1), dsga::ivec4(4, 1, 2, 3));
		CHECK_EQ(cshift_input.cshift(-3), dsga::ivec4(2, 3, 4, 1));
		CHECK_EQ(cshift_input.cshift(-11), dsga::ivec4(2, 3, 4, 1));
		CHECK_EQ(cshift_input.cshift(-13), dsga::ivec4(4, 1, 2, 3));
	}

	TEST_CASE("vector min()")
	{
		CHECK_EQ(dsga::ivec4(-1, 10, 2, -8).min(), -8);

		CHECK_UNARY(std::isnan(dsga::basic_vector{std::numeric_limits<double>::quiet_NaN(), 2., 3., 4.}.min()));
		CHECK_EQ(dsga::basic_vector{1., std::numeric_limits<double>::quiet_NaN(), 3., 4.}.min(), 1.);
		CHECK_EQ(dsga::basic_vector{1., 2., 3., std::numeric_limits<double>::quiet_NaN()}.min(), 1.);
	}

	TEST_CASE("vector max()")
	{
		CHECK_EQ(dsga::ivec4(-1, 10, 2, -8).max(), 10);

		CHECK_UNARY(std::isnan(dsga::basic_vector{std::numeric_limits<double>::quiet_NaN(), 2., 3., 4.}.max()));
		CHECK_EQ(dsga::basic_vector{1., std::numeric_limits<double>::quiet_NaN(), 3., 4.}.max(), 4.);
		CHECK_EQ(dsga::basic_vector{1., 2., 3., std::numeric_limits<double>::quiet_NaN()}.max(), 3.);
	}

	TEST_CASE("vector sum()")
	{
		CHECK_EQ(dsga::ivec4(-1, 10, 2, -8).sum(), 3);

		CHECK_UNARY(std::isnan(dsga::basic_vector{std::numeric_limits<double>::quiet_NaN(), 2., 3.}.sum()));
		CHECK_UNARY(std::isnan(dsga::basic_vector{1., std::numeric_limits<double>::quiet_NaN(), 3.}.sum()));
		CHECK_UNARY(std::isnan(dsga::basic_vector{1., 2., std::numeric_limits<double>::quiet_NaN()}.sum()));
	}
}