
//          Copyright David Browne 2020-2023.
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

namespace fs = std::filesystem;

//
// Example: convert a binary STL file to an ASCII STL file, recomputing the normal vectors.
// For this example, and in general, binary STL is assumed to be little-endian.
//

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
	o << v[0];
	for (int i = 1; i < v.length(); ++i)
		o << " " << std::scientific << v[i];
	return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool maybe_binary_stl(std::ifstream &some_file, uintmax_t size)
{
	constexpr uintmax_t facet_size = 50u;
	constexpr uintmax_t header_size = 80u;
	constexpr uintmax_t num_facets_size = 4u;
	bool maybe_val = false;
	unsigned int num_facets{};

	// file too small to have bytes for number of facets
	if (size < (header_size + num_facets_size))
		return false;

	// skip possible header and read possible number of facets
	some_file.seekg(header_size);
	some_file.read(reinterpret_cast<char *>(&num_facets), num_facets_size);

	if constexpr (std::endian::native == std::endian::big)
		num_facets = dsga::byteswap(num_facets);

	uintmax_t hypothetical_size = ((num_facets * facet_size) + header_size + num_facets_size);
	maybe_val = (hypothetical_size == size);

	return maybe_val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr auto solid_open  = "solid dsga_example\n";
constexpr auto facet_open  = "  facet normal ";
constexpr auto loop_open   = "    outer loop\n";
constexpr auto vertex_line = "      vertex ";
constexpr auto loop_close  = "    endloop\n";
constexpr auto facet_close = "  endfacet\n";
constexpr auto solid_close = "endsolid dsga_example\n";

void write_ascii_facet(std::ofstream &out_file, const dsga::vec3 &computed_normal, const dsga::vec3 &vertex1, const dsga::vec3 &vertex2, const dsga::vec3 &vertex3)
{
	out_file << facet_open << computed_normal << "\n";
	out_file << loop_open;
	out_file << vertex_line << vertex1 << "\n";
	out_file << vertex_line << vertex2 << "\n";
	out_file << vertex_line << vertex3 << "\n";
	out_file << loop_close;
	out_file << facet_close;
}

bool read_binary_stl_float(std::ifstream &some_file, float &float_val)
{
	constexpr auto float_size = 4u;
	some_file.read(reinterpret_cast<char *>(&float_val), float_size);

	// make sure we read the number of bytes we wanted
	if (some_file.gcount() != float_size)
		return false;

	if constexpr (std::endian::native == std::endian::big)
		float_val = dsga::byteswap(float_val);

	return true;
}

bool read_coordinate_triple(std::ifstream &some_file, dsga::vec3 &triple)
{
	return
		read_binary_stl_float(some_file, triple[0]) &&
		read_binary_stl_float(some_file, triple[1]) &&
		read_binary_stl_float(some_file, triple[2]);
}

bool read_facet_vertices(std::ifstream &some_file, dsga::vec3 &vertex1, dsga::vec3 &vertex2, dsga::vec3 &vertex3)
{
	return
		read_coordinate_triple(some_file, vertex1) &&
		read_coordinate_triple(some_file, vertex2) &&
		read_coordinate_triple(some_file, vertex3);
}

bool read_binary_facet_write_ascii_facet(std::ifstream &some_file, std::ofstream &out_file)
{
	const std::streampos facet_size = 50;
	constexpr auto normal_size = 12u;

	// remember where we started reading
	auto file_cursor = some_file.tellg();

	bool success = false;

	// skip over normal vector in file -- we don't trust it -- we recompute it from vertices
	some_file.ignore(normal_size);

	dsga::vec3 vertex1;
	dsga::vec3 vertex2;
	dsga::vec3 vertex3;
	bool valid_vertices = read_facet_vertices(some_file, vertex1, vertex2, vertex3);

	// check for infinities and NaNs -- don't worry about non-zero first octant vertex location
	if (valid_vertices && valid_vertex_relaxed(vertex1) && valid_vertex_relaxed(vertex2) && valid_vertex_relaxed(vertex3))
	{
		auto computed_normal = right_handed_normal(vertex1, vertex2, vertex3);
		if (valid_normal_vector(computed_normal))
		{
			write_ascii_facet(out_file, computed_normal, vertex1, vertex2, vertex3);
			success = true;
		}
		else
		{
			success = false;		// don't need this here, but it is explicit for reading purposes
		}
	}

	// set position to next binary facet
	some_file.seekg(file_cursor + facet_size);
	return success;
}

bool convert_binary_stl_to_ascii(std::ifstream &some_file, std::ofstream &out_file, unsigned num_facets)
{
	constexpr auto header_size = 80u;
	constexpr auto num_facets_size = 4u;

	some_file.seekg(header_size + num_facets_size);

	// iostream ASCII STL float precision for format flag std::scientific
	out_file.precision(std::numeric_limits<float>::max_digits10 - 1);

	// convert input to output
	out_file << solid_open;
	unsigned bad_facets = 0;
	for (unsigned i = 0; i < num_facets; ++i)
	{
		if (!read_binary_facet_write_ascii_facet(some_file, out_file))
			++bad_facets;
	}
	out_file << solid_close;

	if (bad_facets)
		std::cerr << "number of bad facets: " << bad_facets << std::endl;

	return bad_facets != num_facets;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stl_main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Convert binary STL file to ASCII STL file.\n";
		std::cout << "Usage: " << argv[0] << " binary_src.stl ascii_dest.stl\n";
		return EXIT_FAILURE;
	}
	else
	{
		// check if input file appears to be a binary STL file
		auto binary_stl_path = fs::path(argv[1]);

		auto binary_exists = fs::exists(binary_stl_path);
		auto binary_regular = binary_exists ? fs::is_regular_file(binary_stl_path) : false;
		auto binary_size = binary_exists ? fs::file_size(binary_stl_path) : 0;

		// do file size and facet count make sense for this to be a binary STL file?
		bool appears_to_be_binary_stl = false;
		if (binary_exists && binary_regular)
		{
			auto maybe_binary_stl_file = std::ifstream(binary_stl_path, std::ios::binary);
			if (maybe_binary_stl_file.is_open())
			{
				appears_to_be_binary_stl = maybe_binary_stl(maybe_binary_stl_file, binary_size);
				maybe_binary_stl_file.close();
			}
		}

		// early exit if bad input file
		if (!appears_to_be_binary_stl)
		{
			std::cerr << fs::absolute(binary_stl_path) << " is not a valid binary STL file.\n";
			return EXIT_FAILURE;
		}

		// check if output file can be written as an ASCII STL file
		auto ascii_stl_path = fs::path(argv[2]);

		// can't have same source and destination
		if (fs::absolute(binary_stl_path) == fs::absolute(ascii_stl_path))
		{
			std::cerr << "Input file must be different from output file.\n";
			return EXIT_FAILURE;
		}

		auto ascii_exists = fs::exists(ascii_stl_path);
		auto ascii_regular = ascii_exists ? fs::is_regular_file(ascii_stl_path) : false;

		[[maybe_unused]] bool overwrite_destination = false;
		[[maybe_unused]] bool new_destination = false;
		if (ascii_exists)
		{
			if (ascii_regular)
			{
				std::string user_input{};
				std::cout << "Overwrite " << fs::absolute(ascii_stl_path) << "? [Y/n] ";
				std::getline(std::cin, user_input);
				if (user_input.empty() || user_input[0] == 'y' || user_input[0] == 'Y')
				{
					std::ofstream ascii_file(ascii_stl_path);
					if (ascii_file.is_open())
					{
						overwrite_destination = true;
						ascii_file.close();
					}
				}
				else
				{
					std::cerr << fs::absolute(ascii_stl_path) << " will not be overwritten.\n";
				}
			}
			else
			{
				std::cerr << fs::absolute(ascii_stl_path) << " is a bad path for destination.\n";
			}
		}
		else
		{
			std::ofstream ascii_file(ascii_stl_path);
			if (ascii_file.is_open())
			{
				new_destination = true;
				ascii_file.close();
			}
		}

		// ascii destination file verified ok
		if (overwrite_destination || new_destination)
		{
			constexpr auto facet_size = 50u;
			constexpr auto header_size = 80u;
			constexpr auto num_facets_size = 4u;

			auto num_facets = (binary_size - header_size - num_facets_size) / facet_size;

			auto binary_stl = std::ifstream(binary_stl_path, std::ios::binary);
			auto ascii_stl = std::ofstream(ascii_stl_path);
			bool success = convert_binary_stl_to_ascii(binary_stl, ascii_stl, static_cast<unsigned>(num_facets));
			binary_stl.close();
			ascii_stl.close();

			if (!success)
			{
				std::cerr << "No good facets found.\n";
				return EXIT_FAILURE;
			}
		}
		else
		{
			std::cerr << "Can't open destination file " << fs::absolute(ascii_stl_path) << "\n";
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
