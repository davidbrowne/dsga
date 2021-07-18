
//          Copyright David Browne 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"


#if defined(__clang__)
// clang 10.0 does not like colors on windows (link problems with isatty and fileno)
#define DOCTEST_CONFIG_COLORS_NONE
#endif

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// operators are implemented without regard to any specific dimension, so we can test generically
TEST_SUITE("test operators")
{
	TEST_CASE("vector angle and trigonometry functions")
	{
		const float my_pi = std::numbers::pi_v<float>;
		fvec3 degs(30, 45, 60);
		fvec3 rads(my_pi / 6, my_pi / 4, my_pi / 3);

		SUBCASE("radians and degrees")
		{
			// radians()
			
			auto my_rads = radians(degs);
			CHECK_EQ(my_rads, rads);

			// degrees()

			auto my_degs = degrees(rads);
			CHECK_EQ(my_degs, degs);
		}

		SUBCASE("basic trig")
		{
			// sin()

			CHECK_EQ(sin(radians(fvec2(30, 90))), fvec2(0.5f, 1));

			// cos()

			CHECK_EQ(cos(radians(fvec2(0, 180))), fvec2(1, -1));

			// tan()
			CHECK_EQ(tan(radians(fvec2(45, 0))), fvec2(1, 0));

			// asin()
			CHECK_EQ(asin(fvec2(0.5f, 1)), radians(fvec2(30, 90))) ;

			// acos()
			CHECK_EQ(acos(fvec2(1, -1)), radians(fvec2(0, 180)));

			// atan() (both 1 arg and 2 arg)
			CHECK_EQ(atan(fvec2(1, 0)), radians(fvec2(45, 0)));

			CHECK_EQ(atan(fvec2(1, -1), fvec2(-1, -1)), radians(fvec2(135, -135)));

		}

		SUBCASE("hyperbolic trig")
		{
			dvec3 vals(-2, 0, 2);

			// sinh()
			auto sinhs = sinh(vals);

			// cosh()
			auto coshs = cosh(vals);

			// tanh()
			auto tanhs = tanh(vals);

			// asinh()
			auto asinhs = asinh(sinhs);
			CHECK_EQ(vals, asinhs);

			// acosh()
			auto acoshs = acosh(coshs);
			CHECK_EQ(dvec3(2, 0, 2), acoshs);

			// atanh()
			auto atanhs = atanh(tanhs);
			CHECK_EQ(atanh(sinhs / coshs), atanhs);
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

			// exp(), e.g., e^x = exp(x)
			vec2 exp_vals(std::numbers::ln10_v<float>, std::numbers::ln2_v<float>);
			vec2 exp_result = exp(exp_vals);
			CHECK_EQ(exp_result, vec2(10, 2));

			// log(), e.g., log(x) = ln(x) = log<base e>(x)
			vec2 log_vals(10, 2);
			vec2 log_result = log(log_vals);
			CHECK_EQ(log_result, vec2(std::numbers::ln10_v<float>, std::numbers::ln2_v<float>));

			// exp2(), e.g., 2^x = exp2(x)
			vec3 exp2_vals(std::numbers::log2e_v<float>, 2, 10);
			vec3 exp2_result = exp2(exp2_vals);
			CHECK_EQ(exp2_result, vec3(std::numbers::e_v<float>, 4, 1024));

			// log2(), e.g., log2(x) = lb(x) = log<base 2>(x)
			vec3 log2_vals(std::numbers::e_v<float>, 4, 1024);
			vec3 log2_result = log2(log2_vals);
			CHECK_EQ(log2_result, vec3(std::numbers::log2e_v<float>, 2, 10));

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

			// inversesqrt()
			auto invsqrtvals = inversesqrt(vals);
			CHECK_EQ(invsqrtvals, vec3(0.5, 0.25, 0.125));
		}
	}

	TEST_CASE("vector common functions")
	{
		SUBCASE("most common")
		{
			ivec3 int_data(-10, 0, 9);

			// abs()
			auto abs_vals = abs(int_data);
			CHECK_EQ(abs_vals, ivec3(10, 0, 9));

			// sign()
			auto sign_vals = sign(int_data);
			CHECK_EQ(sign_vals, ivec3(-1, 0, 1));

			vec4 float_data(-1.75, -0.25, 0.5, 1.0);
			dvec4 double_data(11.5, 12.5, -11.5, -12.5);

			// floor()
			auto floor_vals = floor(float_data);
			CHECK_EQ(floor_vals, vec4(-2, -1, 0, 1));

			// trunc()
			auto trunc_vals = trunc(float_data);
			CHECK_EQ(trunc_vals, vec4(-1, 0, 0, 1));

			// round()
			auto round_vals = round(float_data);
			CHECK_EQ(round_vals, vec4(-2, 0, 1, 1));

			// roundEven()
			auto re_float_data = roundEven(float_data);
			CHECK_EQ(re_float_data, vec4(-2, 0, 0, 1));
			auto re_double_data = roundEven(double_data);
			CHECK_EQ(re_double_data, dvec4(12, 12, -12, -12));

			// ceil()
			auto ceil_vals = ceil(float_data);
			CHECK_EQ(ceil_vals, vec4(-1, 0, 1, 1));

			// fract()
			auto fract_vals = fract(float_data);
			CHECK_EQ(fract_vals, vec4(0.25, 0.75, 0.5, 0));

			vec4 mod_x_data(7.75, -12.25, 4, -0.5);
			vec4 mod_y_data(2.25, -2.5, 3.125, -0.75);

			// mod()
			auto mod_vals_vector = mod(mod_x_data, mod_y_data);
			auto mod_vals_scalar = mod(mod_x_data, 1.625f);

			CHECK_EQ(mod_vals_vector, vec4(1, -2.25, 0.875, -0.5));
			CHECK_EQ(mod_vals_scalar, vec4(1.25, 0.75, 0.75, 1.125));

			// modf()
			vec4 modf_int_part;
			auto modf_vals = modf(mod_y_data, modf_int_part);
			CHECK_EQ(modf_int_part, vec4(2, -2, 3, -0));
			CHECK_EQ(modf_vals, vec4(0.25, -0.5, 0.125, -0.75));
		}

		SUBCASE("in range functions")
		{
			// min()

			// max()

			// clamp()

			// mix()

			// step()

			// smoothstep()

		}

		SUBCASE("bit changing functions")
		{
#if defined(__cpp_lib_bit_cast)

			// floatBitsToInt()
			auto floatToInt = floatBitsToInt(vec4(123.125, 6967.0e+4, -654.0, std::numbers::pi_v<float>));
			CHECK_EQ(floatToInt, ivec4(1123434496, 1283777166, -1004306432, 1078530011));

			// floatBitsToUint()
			auto floatToUint = floatBitsToUint(vec4(87.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrt3_v<float>));
			CHECK_EQ(floatToUint, uvec4(645673883u, 1283777166u, 3290660864u, 1058262330u));

			// doubleBitsToLongLong()
			auto doubleToLongLong = doubleBitsToLongLong(dvec4(123.125, 6967.0e+4, -654.0, std::numbers::phi_v<double>));
			CHECK_EQ(doubleToLongLong, llvec4(4638364568563744768ll, 4724447884039159808ll, -4574408176199270400ll, 4609965796441453736ll));

			// doubleBitsToUlongLong()
			auto doubleToUlongLong = doubleBitsToUlongLong(dvec4(1.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrtpi_v<double>));
			CHECK_EQ(doubleToUlongLong, ullvec4(4355345018344775537ull, 4724447884039159808ull, 13872335897510281216ull, 4603256987541740397ull));

			// intBitsToFloat()
			auto intToFloat = intBitsToFloat(ivec4(1123434496, 1283777166, -1004306432, 1078530011));
			CHECK_EQ(intToFloat, vec4(123.125, 6967.0e+4, -654.0, std::numbers::pi_v<float>));

			// uintBitsToFloat()
			auto uintToFloat = uintBitsToFloat(uvec4(645673883u, 1283777166u, 3290660864u, 1058262330u));
			CHECK_EQ(uintToFloat, vec4(87.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrt3_v<float>));

			// longLongBitsToDouble()
			auto longLongToDouble = longLongBitsToDouble(llvec4(4638364568563744768ll, 4724447884039159808ll, -4574408176199270400ll, 4609965796441453736ll));
			CHECK_EQ(longLongToDouble, dvec4(123.125, 6967.0e+4, -654.0, std::numbers::phi_v<double>));

			// ulongLongBitsToDouble()
			auto ulongLongToDouble = ulongLongBitsToDouble(ullvec4(4355345018344775537ull, 4724447884039159808ull, 13872335897510281216ull, 4603256987541740397ull));
			CHECK_EQ(ulongLongToDouble, dvec4(1.5e-17, 6967.0e+4, -654.0, std::numbers::inv_sqrtpi_v<double>));

#endif
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

			// isinf()
			auto isinf_vals = isinf(bad_nums);
			CHECK_EQ(isinf_vals, bvec4(false, false, true, true));

			// fma()
			dvec3 a(2, 4, 6);
			dvec3 b(3, 5, 7);
			dvec3 c(20, 40, 60);

			auto fma_vals = fma(a, b, c);
			CHECK_EQ(fma_vals, dvec3(26, 60, 102));

			// frexp()
			ivec3 exps(0);
			dvec3 frexp_data(10, 20, 30);

			auto frexp_vals = frexp(frexp_data, exps);

			CHECK_EQ(frexp_vals, dvec3(0.625, 0.625, 0.9375));
			CHECK_EQ(exps, ivec3(4, 5, 5));

			// ldexp()
			dvec3 ld_data(0.625, 0.625, 0.9375);
			ivec3 ld_exps(4, 5, 5);

			auto ldexp_vals = ldexp(ld_data, ld_exps);

			CHECK_EQ(ldexp_vals, dvec3(10, 20, 30));
		}
	}

	TEST_CASE("vector geometric functions")
	{
		SUBCASE("generic vector functions")
		{
			// length()
			vec2 length_vals1 = vec2(3, 4);
			vec2 length_vals2 = vec2(5, 12);
			auto length1 = length(length_vals1);
			auto length2 = length(length_vals2);

			CHECK_EQ(length1, 5.f);
			CHECK_EQ(length2, 13.f);

			// distance()
			vec2 p1(12, 23);
			vec2 p2(4, 38);
			auto dist = distance(p1, p2);
			CHECK_EQ(dist, 17.f);

			// dot()
			const float my_pi = std::numbers::pi_v<float>;
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

			// notEqual()
			auto neq_vec = notEqual(v1, v2);
			CHECK_EQ(neq_vec, bvec3(true, false, true));
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

			// not() - c++ doesn't allow not(), so logicalNot()
			auto not_true = all(logicalNot(lessThan(v1, v2)));
			CHECK_UNARY(not_true);
			auto not_not_true = all(logicalNot(lessThan(v0, v2)));
			CHECK_UNARY_FALSE(not_not_true);
		}
	}

	TEST_CASE("matrix functions")
	{
		// matrixCompMult() - equivalent to a component-wise "matrix/matrix binary operator *" if there was one
		mat2x3 A(1, 2, 3, 4, 5, 6);
		mat2x3 B(5, 10, 15, 20, 25, 30);
		auto mcm = matrixCompMult(A, B);
		CHECK_EQ(mcm, mat2x3(5, 20, 45, 80, 125, 180));

		// outerProduct()
		auto op = outerProduct(dvec3(3, 5, 7), dvec3(2, 4, 6));
		CHECK_EQ(op, dmat3(6, 10, 14, 12, 20, 28, 18, 30, 42));

		// transpose()
		dmat4 nums(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
		dmat4 transposed_nums = transpose(nums);
		CHECK_EQ(transposed_nums, dmat4(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));

		// inverse()
		auto some4x4 = dmat4(dvec4(1, 0, 2, 2), dvec4(0, 2, 1, 0), dvec4(0, 1, 0, 1), dvec4(1, 2, 1, 4));
		auto inverse4 = inverse(some4x4);

		CHECK_EQ(inverse4, dmat4(dvec4(-2, 1, -8, 3), dvec4(-0.5, 0.5, -1, 0.5), dvec4(1, 0, 2, -1), dvec4(0.5, -0.5, 2, -0.5)));
		CHECK_EQ(some4x4 * inverse4, dmat4(1));
		CHECK_EQ(inverse4 * some4x4, dmat4(1));

		// determinant()
		auto det = determinant(some4x4);
		CHECK_EQ(det, 2);
	}
}
