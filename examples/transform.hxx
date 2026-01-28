#pragma once

//          Copyright David Browne 2025-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include "../examples/tolerance.hxx"

//
// These functions are for "proper orthonormal matrices", embedded in a matrix a dimension higher, along
// with an embedded translation (origin). The proper orthonormal matrices that will be used here are
// considered rotation matrices. The combination of a rotation matrix, a translation, 0 vector, and 1 are 
// called transformation matrices. Transformations matrices can be either 2D or 3D transforms.
// For proper orthonormal rotation matrices, inverse(rotation_matrix) == transpose(rotation_matrix).
// Our definition of a transformation matrix does NOT include shearing, scaling, stretching, squeezing,
// or reflection. The rotations are column vector based, as in multiply matrixes with a column vector, e.g.,
// "Ax = y", where A is the transformation matrix, and x and y are column vectors. If you want to multiply
// with the matrix on the right, then we have row vectors that multiply the matrix, e.g., "zB = w",
// where B is the transformation matrix, and z and w are row vectors. If you take the transpose of the
// matrix and the multiplicand (point/vector), you should get the same answer, as a row vector.
// The transpose of vectors changes the vectors from a row to column, or the other way around.
// 
// For equations of the type "Ax = b", A is known as a right multiplication matrix or a column transformation matrix.
// For equations of the type "zB = w", B is known as a left multiplication matrix or a row transformation matrix.
// 
// Ax = b => B = transpose(A), z = transpose(x), w = transpose(y), leads to zB = w.
// 
// So you can use matrix multiplication as either "point * transform" or "transform * point", depending
// on your preference.
// 
// The dsga library has column major matrices and vectors as the default understanding, so Ax = b is
// more natural, but either way is supported. Ax = b is the natural form used in mathematics, physics,
// robotics, OpenGL, etc. yB = w is primarily used in some computer graphics software, like Direct X, although
// I think you can use it with column vectors as well.
//

// 
// Rotation matrix => proper orthonormal matrix, R => determinant(R) == +1
// 
// 2D
// 
// || R | T ||              | a  b |                        | x |
// ||---+---|| , where R => | c  d | , 0 => | 0  0 | , T => | y | , and 1 => | 1 |
// || 0 | 1 ||
// 
// 3D
// 
// || R | T ||              | a  b  c |                               | x |
// ||---+---|| , where R => | d  e  f | , and 0 => | 0  0  0 | , T => | y | , and 1 => | 1 |
// || 0 | 1 ||              | g  h  i |                               | z |
// 
// Inverse
// 
// || transpose(R) | -transpose(R) * T |
// ||--------------+-------------------|
// ||      0       |        1          |
//

namespace dsga
{
	// get the inverse of a 2D transformation matrix -- other matrix types won't give the proper inverse
	
	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 3, 3> transform_inverse(const basic_matrix<T, 3, 3> &arg) noexcept
	{
		// transpose of rotation part of arg is the inverse of the rotation part
		auto rot = basic_matrix<T, 2, 2>(arg);
		auto rot_transpose = transpose(rot);

		// create inverse matrix with initially only the rotation inverse part
		auto inv = basic_matrix<T, 3, 3>(rot_transpose);

		// invert the translation part (origin) of arg and put it in inverse matrix
		inv[2].xy = -(rot_transpose * arg[2].xy);

		return inv;
	}

	// get the inverse of a 3D transformation matrix -- other matrix types won't give the proper inverse

	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> transform_inverse(const basic_matrix<T, 4, 4> &arg) noexcept
	{
		// transpose of rotation part of arg is the inverse of the rotation part,
		// due to orthonormal rotation matrix where determinant == +1
		auto rot = basic_matrix<T, 3, 3>(arg);
		auto rot_transpose = transpose(rot);

		// create inverse matrix with initially only the rotation inverse part
		auto inv = basic_matrix<T, 4, 4>(rot_transpose);

		// invert the translation part (origin) of arg and put it in inverse matrix
		inv[3].xyz = -(rot_transpose * arg[3].xyz);

		return inv;
	}

	// check if a matrix is a transformation matrix
	template <floating_point_scalar T, std::size_t C, std::size_t R>
	[[nodiscard]] constexpr bool is_transformation_matrix(const basic_matrix<T, C, R> &arg,
														  T tolerance) noexcept
	{
		if constexpr (C != R)				// not a square matrix, so cannot be a transformation matrix
		{
			return false;
		}
		else if constexpr (C < 3)			// only applies to 3x3 (2D) and 4x4 (3D) matrices
		{
			return false;
		}

		// check if last row is [0, 0, ..., 1], if the rotation part has a determinant close to 1.0,
		// if the columns of the rotation part are unit vectors, and if the columns are all orthogonal

		// is the last row close to [0, 0, ..., 1]
		auto last_row = basic_vector<T, C>(T(0));
		last_row[C - 1] = T(1.0);																// set the last element to 1.0, the rest are 0.0
		bool last_row_success = all(within_tolerance(last_row - arg.row(C - 1), tolerance));	// check if the last row is close to [0, 0, ..., 1]

		// is the determinant close to 1.0
		auto rotation_part = basic_matrix<T, C - 1, C - 1>(arg);
		bool determinant_success = within_tolerance(determinant(rotation_part) - T(1.0), tolerance);

		// are all rotation columns unit vectors
		bool unit_columns_success = [&rotation_part, &tolerance]() noexcept
		{
			return [&rotation_part, &tolerance]<std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
			{
				return (within_tolerance(length(rotation_part[Is]) - T(1.0), tolerance) && ...);
			}(std::make_index_sequence<C - 1>{});
		}();

		// do all columns have a dot product with the other columns returning a value close to 0.0,
		// i.e., are the columns orthogonal
		bool orthogonal_columns_success = [&rotation_part, &tolerance]() noexcept
		{
			return [&rotation_part, &tolerance]<std::size_t ...Is>(std::index_sequence<Is ...>) noexcept
			{
				return ([&rotation_part, &tolerance]<std::size_t ...Js>(std::index_sequence<Js ...>, std::size_t col) noexcept
				{
					return (within_tolerance(dot(rotation_part[col], rotation_part[Js + col + 1]), tolerance) && ...);
				}(std::make_index_sequence<C - 2 - Is>{}, Is) && ...);
			}(std::make_index_sequence<C - 2>{});
		}();

		// is the cross product of the first two columns of the rotation part (of a 4x4) close to the third column of the rotation part
		bool cross_product_success = true;
		if constexpr (C == 4)
		{
			auto cross_product = cross(rotation_part[0], rotation_part[1]);
			cross_product_success = all(within_tolerance(cross_product - rotation_part[2], tolerance));
		}

		return last_row_success && determinant_success && unit_columns_success && orthogonal_columns_success && cross_product_success;
	}

	//
	template <bool W, floating_point_scalar T, typename D>
	[[nodiscard]] constexpr basic_matrix<T, 3, 3> translation_matrix(const vector_base<W, T, 2, D> &translation) noexcept
	{
		auto trans_mat = identity_matrix<T, 3>();			// create an identity matrix
		trans_mat[2].xy = translation;						// set the translation part
		return trans_mat;
	}

	//
	template <bool W, floating_point_scalar T, typename D>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> translation_matrix(const vector_base<W, T, 3, D> &translation) noexcept
	{
		auto trans_mat = identity_matrix<T, 4>();			// create an identity matrix
		trans_mat[3].xyz = translation;						// set the translation part
		return trans_mat;
	}

	// make a 2D rotation matrix for a given theta.
	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 3, 3> rot_2d(T theta) noexcept
	{
		auto cos_theta = cos(theta);
		auto sin_theta = sin(theta);

		auto rot_mat = identity_matrix<T, 3>();
		rot_mat[0].xy = basic_vector( cos_theta, sin_theta);
		rot_mat[1].xy = basic_vector(-sin_theta, cos_theta);

		return rot_mat;
	}

	// make a 3D matrix for rotating about the X axis for a given theta.
	// also known as an A axis rotation.
	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> rot_x_axis(T theta) noexcept
	{
		auto cos_theta = cos(theta);
		auto sin_theta = sin(theta);

		auto rot_mat = identity_matrix<T, 4>();
		rot_mat[1].yz = basic_vector( cos_theta, sin_theta);
		rot_mat[2].yz = basic_vector(-sin_theta, cos_theta);

		return rot_mat;
	}

	// make a 3D matrix for rotating about the Y axis for a given theta.
	// also known as an B axis rotation.
	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> rot_y_axis(T theta) noexcept
	{
		auto cos_theta = cos(theta);
		auto sin_theta = sin(theta);

		auto rot_mat = identity_matrix<T, 4>();
		rot_mat[0].xyz = basic_vector(cos_theta, T(0.0), -sin_theta);
		rot_mat[2].xyz = basic_vector(sin_theta, T(0.0),  cos_theta);

		return rot_mat;
	}

	// make a 3D matrix for rotating about the Z axis for a given theta.
	// also known as an C axis rotation.
	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> rot_z_axis(T theta) noexcept
	{
		auto cos_theta = cos(theta);
		auto sin_theta = sin(theta);

		auto rot_mat = identity_matrix<T, 4>();
		rot_mat[0].xy = basic_vector( cos_theta, sin_theta);
		rot_mat[1].xy = basic_vector(-sin_theta, cos_theta);

		return rot_mat;
	}
	
	// make a 3D matrix for rotating about the k axis for a given theta.
	// k must be a 3D unit vector, i.e., length(k) == 1.0.
	// the tranlation part of the matrix is [0, 0, 0]
	template <bool W1, floating_point_scalar T, typename D1, bool W2, typename D2>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> rot_any_axis(T theta,
															   const vector_base<W1, T, 3, D1> &axis,
															   const vector_base<W2, T, 3, D2> &origin) noexcept
	{
		auto c_t = cos(theta);
		auto s_t = sin(theta);
		auto v_t = T(1.0) - c_t;

		auto&& [k_x, k_y, k_z] = normalize(axis);					// decompose the normalized axis into its components

		// calculate the rotation matrix components
		auto rot_mat = identity_matrix<T, 4>();
		rot_mat[0].xyz = basic_vector(k_x * k_x * v_t +		  c_t,
									  k_x * k_y * v_t + k_z * s_t,
									  k_x * k_z * v_t - k_y * s_t);
		rot_mat[1].xyz = basic_vector(k_x * k_y * v_t - k_z * s_t,
									  k_y * k_y * v_t +		  c_t,
									  k_y * k_z * v_t + k_x * s_t);
		rot_mat[2].xyz = basic_vector(k_x * k_z * v_t + k_y * s_t,
									  k_y * k_z * v_t - k_x * s_t,
									  k_z * k_z * v_t +		  c_t);
		rot_mat[3].xyz = origin;									// origin is the translation part of the matrix

		return rot_mat;
	}

	// prettify a transformation matrix to take care of the rotation part to recover from any drift away from an orthonormal matrix
	template <floating_point_scalar T>
	[[nodiscard]] constexpr basic_matrix<T, 4, 4> renormalize_transformation_matrix(const basic_matrix<T, 4, 4> &mat, T tol) noexcept
	{
		auto snap_to_zero = [tol](T x) noexcept { return abs(x) <= tol ? T(0) : x; };
		auto fix_for_zeros = [&snap_to_zero](const basic_vector<T, 3> &v) noexcept { return v.apply(snap_to_zero); };

		// create a copy of the matrix, so that we can modify it
		auto fixed_mat = identity_matrix<T, 4>();

		// clean up the first 2 columns of the rotation matrix - round to zero for tiny values -- make it a unit vector
		fixed_mat[0].xyz = normalize(fix_for_zeros(mat[0].xyz));
		fixed_mat[1].xyz = normalize(fix_for_zeros(mat[1].xyz));

		// create the third column of rotation matrix - will be orthonormal to first 2 axes
		fixed_mat[2].xyz = cross(fixed_mat[0].xyz, fixed_mat[1].xyz);

		// guarantee all the columns orthonormal
		fixed_mat[0].xyz = cross(fixed_mat[1].xyz, fixed_mat[2].xyz);

		// leave translation values alone, or should we fix_for_zeros(mat[3].xyz) here too?
		fixed_mat[3].xyz = mat[3].xyz;

		return fixed_mat;
	}

}	// namespace dsga

// test that transform_inverse() works same as dsga::inverse() when
// the argument is a transformation matrix.
inline bool transform_inverse_example()
{
	auto angle = std::numbers::sqrt3_v<double> / 2.0;	// 0.8660254037844386
	auto d1 = dsga::dvec4(dsga::cos(angle), dsga::sin(angle), 0, 0);
	auto d2 = dsga::dvec4(-dsga::sin(angle), dsga::cos(angle), 0, 0);
	auto d3 = dsga::dvec4(0, 0, 1, 0);
	auto d4 = dsga::dvec4(3, 5, 7, 1);

	auto m1 = dsga::dmat4(d1, d2, d3, d4);
	[[ maybe_unused ]] auto m1_is_xform_mat = dsga::is_transformation_matrix(m1, 1e-10);
	auto m1inv1 = dsga::inverse(m1);
	auto m1inv2 = dsga::transform_inverse(m1);

	return dsga::within_box(m1inv1, m1inv2, 1e-10);
}
