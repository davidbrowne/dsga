
//          Copyright David Browne 2020-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "dsga.hxx"
#include "stl.hxx"

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <bit>

//
// Example: convert a binary STL file to an ASCII STL file, recomputing the normal vectors.
// For this example, and in general, binary STL is assumed to be little-endian.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// data structure for facet data, including the given normal vector, the computed normal
// vector, and the three vertices of the facet
struct facet_data
{
	dsga::vec3 given_normal;
	dsga::vec3 computed_normal;
	dsga::vec3 vertex[3];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

// binary STL is little-endian, so if native is big-endian, convert float from native to little-endian and vice versa.
// the same operation will toggle the endianness in either direction, so make proper use of the semantic functions below.
// if native is little-endian, then this is a no-op.
constexpr float toggle_endianness(float f) noexcept
{
	if constexpr (std::endian::native == std::endian::big)
	{
		auto integral_float = std::bit_cast<unsigned int>(f);
		integral_float = dsga::byteswap(integral_float);
		return std::bit_cast<float>(integral_float);
	}
	else
	{
		return f;
	}
};

// binary STL is little-endian, so if native is big-endian, convert unsigned int from native to little-endian and vice versa.
// the same operation will toggle the endianness in either direction, so make proper use of the semantic functions below.
// if native is little-endian, then this is a no-op.
constexpr unsigned int toggle_endianness(unsigned int u) noexcept
{
	if constexpr (std::endian::native == std::endian::big)
	{
		return dsga::byteswap(u);
	}
	else
	{
		return u;
	}
};

// semantically, if used properly, one could consider this to be a one-way conversion
constexpr float le_to_native(float f) noexcept
{
	return toggle_endianness(f);
};

// semantically, if used properly, one could consider this to be a one-way conversion
constexpr float native_to_le(float f) noexcept
{
	return toggle_endianness(f);
};

// semantically, if used properly, one could consider this to be a one-way conversion
constexpr unsigned int le_to_native(unsigned int u) noexcept
{
	return toggle_endianness(u);
};

// semantically, if used properly, one could consider this to be a one-way conversion
constexpr unsigned int native_to_le(unsigned int u) noexcept
{
	return toggle_endianness(u);
};

// semantically, this is a one-way endian conversion for a 3D vertex or normal vector.
constexpr dsga::vec3 le_to_native_coordinate_triple(const dsga::vec3 &v) noexcept
{
	return dsga::vec3(le_to_native(v[0]), le_to_native(v[1]), le_to_native(v[2]));
};

// semantically, this is a one-way endian conversion for a 3D vertex or normal vector.
constexpr dsga::vec3 native_to_le_coordinate_triple(const dsga::vec3 &v) noexcept
{
	return dsga::vec3(native_to_le(v[0]), native_to_le(v[1]), native_to_le(v[2]));
};

// right-handed unit normal vector for a triangle facet,
// inputs are triangle vertices in counter-clockwise order
constexpr dsga::vec3 right_handed_normal(const dsga::vec3 &v1, const dsga::vec3 &v2, const dsga::vec3 &v3) noexcept
{
	return dsga::normalize(dsga::cross(v2 - v1, v3 - v1));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// iostream output operator for STL ASCII output -- need to set scientific and precision==9 on the stream (actually 8, because
// std::scientific format only counts digits *after* the decimal point.
// see https://www.zverovich.net/2023/06/04/printing-double.html
template <dsga::dimensional_scalar T, std::size_t Size>
inline std::ostream &operator<<(std::ostream &o, const dsga::basic_vector<T, Size> &v)
{
	o << std::scientific << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << " " << std::scientific << v[i];
	return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// read float value from binary file, converting from little-endian to native endianness if needed
bool read_binary(std::ifstream &some_file, float &float_val)
{
	if (!some_file.good())
		return false;

	constexpr auto float_size = 4u;
	some_file.read(reinterpret_cast<char *>(&float_val), float_size);

	// make sure we read the number of bytes we wanted
	if (some_file.gcount() != float_size)
		return false;

	float_val = le_to_native(float_val);

	return true;
}

// read unsigned int value from binary file, converting from little-endian to native endianness if needed
bool read_binary(std::ifstream &some_file, unsigned int &uint_val)
{
	if (!some_file.good())
		return false;

	constexpr auto uint_size = 4u;
	some_file.read(reinterpret_cast<char *>(&uint_val), uint_size);

	// make sure we read the number of bytes we wanted
	if (some_file.gcount() != uint_size)
		return false;

	uint_val = le_to_native(uint_val);

	return true;
}

//
bool maybe_binary_stl(std::ifstream &some_file, uintmax_t file_size, unsigned int &num_facets)
{
	constexpr uintmax_t facet_size = 50u;
	constexpr uintmax_t header_size = 80u;
	constexpr uintmax_t num_facets_size = 4u;
	bool maybe_val = false;
	num_facets = 0;

	// file too small to have bytes for number of facets
	if (file_size < (header_size + num_facets_size))
		return false;

	// skip possible header and read possible number of facets
	some_file.seekg(header_size);
	if (read_binary(some_file, num_facets))
	{
		uintmax_t hypothetical_size = ((num_facets * facet_size) + header_size + num_facets_size);
		maybe_val = (hypothetical_size == file_size);
	}

	return maybe_val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr auto solid_open  = "solid dsga_example\n";
constexpr auto facet_open  = "  facet normal ";					// the normal vector will be appended after this string
constexpr auto loop_open   = "    outer loop\n";
constexpr auto vertex_line = "      vertex ";					// a facet vertex will be appended after this string,
																// with three vertices per facet, each on its own line
constexpr auto loop_close  = "    endloop\n";
constexpr auto facet_close = "  endfacet\n";
constexpr auto solid_close = "endsolid dsga_example\n";

//
bool write_ascii_facet(std::ofstream &out_file, const dsga::vec3 &normal,
					   const dsga::vec3 &vertex1, const dsga::vec3 &vertex2, const dsga::vec3 &vertex3)
{
	out_file << facet_open << normal << "\n";
	out_file << loop_open;
	out_file << vertex_line << vertex1 << "\n";
	out_file << vertex_line << vertex2 << "\n";
	out_file << vertex_line << vertex3 << "\n";
	out_file << loop_close;
	out_file << facet_close;

	return out_file.good();
}

//
bool read_coordinate(std::ifstream &some_file, dsga::vec3 &triple)
{
	return
		read_binary(some_file, triple[0]) &&
		read_binary(some_file, triple[1]) &&
		read_binary(some_file, triple[2]);
}

//
bool read_binary_facet(std::ifstream &some_file, dsga::vec3 &vertex1, dsga::vec3 &vertex2, dsga::vec3 &vertex3)
{
	return
		read_coordinate(some_file, vertex1) &&
		read_coordinate(some_file, vertex2) &&
		read_coordinate(some_file, vertex3);
}

//
bool read_binary_facet_write_ascii(std::ifstream &some_file, std::ofstream &out_file)
{
	const std::streampos binary_facet_size = 50u;

	// remember where we started reading
	auto file_cursor = some_file.tellg();

	// if the input file is ok, set file position to the start of the next facet
	auto set_next_position = [&some_file, file_cursor, binary_facet_size]()
	{
		if (some_file.good())
		{
			some_file.seekg(file_cursor + binary_facet_size);
		}
	};

	// read normal vector and vertices for this facet
	dsga::vec3 given_normal{}, vertex1{}, vertex2{}, vertex3{};
	if (!read_coordinate(some_file, given_normal) || !read_binary_facet(some_file, vertex1, vertex2, vertex3))
	{
		set_next_position();
		return false;
	}

	// write facet to output file if data is valid -- we are being pretty relaxed about what we consider valid,
	// but we want to check for infinities and NaNs -- don't worry about non-zero first octant vertex location
	bool success = false;
	if (valid_vertex_relaxed(vertex1) && valid_vertex_relaxed(vertex2) && valid_vertex_relaxed(vertex3))
	{
		// for now, use the validated binary STL normal vector, but it may be better to always
		// re-compute the normal vector from the vertices and use that.
		if (valid_normal_vector(given_normal))
		{
			success = write_ascii_facet(out_file, given_normal, vertex1, vertex2, vertex3);;
		}
		else
		{
			auto computed_normal = right_handed_normal(vertex1, vertex2, vertex3);
			if (valid_normal_vector(computed_normal))
			{
				success = write_ascii_facet(out_file, computed_normal, vertex1, vertex2, vertex3);
			}
		}
	}

	set_next_position();
	return success;
}

// write summary of facet read results to output stream, mostly for error reporting
void report_facet_summary(std::ostream &out_file, unsigned int num_facets, unsigned int read_facets, unsigned int bad_facets)
{
	out_file << "expected number of facets:    " << num_facets << "\n";
	out_file << "number of good facets:        " << read_facets << "\n";
	out_file << "number of bad facets:         " << bad_facets << "\n";

	unsigned int processed_facets = read_facets + bad_facets;
	if (processed_facets != num_facets)
	{
		out_file << "number of unprocessed facets: " << num_facets - processed_facets << "\n";
	}
}

//
bool binary_stl_to_ascii(std::ifstream &some_file, std::ofstream &out_file, unsigned int num_facets, unsigned int &read_facets)
{
	// set file position to the start of the first facet
	constexpr auto header_size = 80u;
	constexpr auto num_facets_size = 4u;
	some_file.seekg(header_size + num_facets_size);

	// iostream ASCII STL float precision for format flag std::scientific
	out_file.precision(std::numeric_limits<float>::max_digits10 - 1);

	// write opening line for ASCII STL file
	out_file << solid_open;

	// convert input facets to output facets
	read_facets = 0;
	unsigned int bad_facets = 0;
	for (unsigned i = 0; i < num_facets; ++i)
	{
		if (!some_file.good() || !out_file.good())
		{
			report_facet_summary(std::cerr, num_facets, read_facets, bad_facets);
			break;
		}

		read_binary_facet_write_ascii(some_file, out_file) ? ++read_facets : ++bad_facets;
	}

	// write closing line for ASCII STL file
	if (out_file.good())
	{
		out_file << solid_close;
	}

	// report facet info if we had any bad facets or if we didn't read the expected number of facets
	unsigned int processed_facets = read_facets + bad_facets;
	if ((bad_facets > 0) || (processed_facets != num_facets))
	{
		report_facet_summary(std::cerr, num_facets, read_facets, bad_facets);
	}

	return (num_facets > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
int stl_main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	// check input arguments -- we want exactly two arguments, the source binary STL file and the destination ASCII STL file
	if (argc != 3)
	{
		std::cerr << "Convert binary STL file to ASCII STL file.\n";
		std::cerr << "Usage: " << argv[0] << " binary_src.stl ascii_dest.stl\n";
		return EXIT_FAILURE;
	}

	// check if input file appears to be a binary STL file
	auto binary_stl_path = std::filesystem::path(argv[1]);

	auto binary_exists = std::filesystem::exists(binary_stl_path);
	auto binary_regular = binary_exists ? std::filesystem::is_regular_file(binary_stl_path) : false;
	auto binary_size = binary_exists ? std::filesystem::file_size(binary_stl_path) : 0;

	// do file size and facet count make sense for this to be a binary STL file?
	unsigned int num_facets = 0;
	bool appears_to_be_binary_stl = false;
	if (binary_exists && binary_regular)
	{
		auto maybe_binary_stl_file = std::ifstream(binary_stl_path, std::ios::binary);
		if (maybe_binary_stl_file.is_open())
		{
			appears_to_be_binary_stl = maybe_binary_stl(maybe_binary_stl_file, binary_size, num_facets);
			maybe_binary_stl_file.close();
		}
	}

	// early exit if bad input file
	if (!appears_to_be_binary_stl)
	{
		std::cerr << std::filesystem::absolute(binary_stl_path) << " is not a valid binary STL file.\n";
		return EXIT_FAILURE;
	}

	// check if output file can be written as an ASCII STL file
	auto ascii_stl_path = std::filesystem::path(argv[2]);

	// can't have same source and destination
	if (std::filesystem::absolute(binary_stl_path) == std::filesystem::absolute(ascii_stl_path))
	{
		std::cerr << "Input file must be different from output file.\n";
		return EXIT_FAILURE;
	}

	auto ascii_exists = std::filesystem::exists(ascii_stl_path);
	auto ascii_regular = ascii_exists ? std::filesystem::is_regular_file(ascii_stl_path) : false;

	[[maybe_unused]] bool overwrite_destination = false;
	[[maybe_unused]] bool new_destination = false;
	if (ascii_exists)
	{
		if (ascii_regular)
		{
			std::string user_input{};
			std::cout << "Overwrite " << std::filesystem::absolute(ascii_stl_path) << "? [Y/n] ";
			std::getline(std::cin, user_input);
			if (user_input.empty() || user_input[0] == 'y' || user_input[0] == 'Y')
			{
				std::ofstream ascii_file(ascii_stl_path, std::ios::app);
				if (ascii_file.is_open())
				{
					overwrite_destination = true;
					ascii_file.close();
				}
			}
			else
			{
				std::cerr << std::filesystem::absolute(ascii_stl_path) << " will not be overwritten.\n";
			}
		}
		else
		{
			std::cerr << std::filesystem::absolute(ascii_stl_path) << " is a bad path for destination.\n";
		}
	}
	else
	{
		std::ofstream ascii_file(ascii_stl_path, std::ios::app);
		if (ascii_file.is_open())
		{
			new_destination = true;
			ascii_file.close();
		}
	}

	if (!overwrite_destination && !new_destination)
	{
		std::cerr << "Can't open destination file " << std::filesystem::absolute(ascii_stl_path) << "\n";
		return EXIT_FAILURE;
	}

	// ascii destination file verified ok
	constexpr auto facet_size = 50u;
	constexpr auto header_size = 80u;
	constexpr auto num_facets_size = 4u;

	auto estimated_num_facets = (binary_size - header_size - num_facets_size) / facet_size;
	if (estimated_num_facets != num_facets)
	{
		std::cerr << "Estimated number of facets based on file size (" << estimated_num_facets
				  << ") does not match number of facets read from file (" << num_facets << ").\n";
		return EXIT_FAILURE;
	}

	auto binary_stl = std::ifstream(binary_stl_path, std::ios::binary);
	auto ascii_stl = std::ofstream(ascii_stl_path);

	unsigned int read_facets{};
	bool success = binary_stl_to_ascii(binary_stl, ascii_stl, num_facets, read_facets);

	binary_stl.close();
	ascii_stl.close();

	if (!success)
	{
		std::cerr << "No good facets found.\n";
		return EXIT_FAILURE;
	}

	std::cout << "Successfully converted " << read_facets << " facets from binary STL to ASCII STL.\n";

	return EXIT_SUCCESS;
}
