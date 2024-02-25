
//          Copyright David Browne 2020-2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)


// The STL file format has been around for over three decades. Since the format is so
// simple and straightforward, it is pretty easy to implement STL file readers and writers.
// Over the years, many implementations of ASCII STL file writers are/were not very strict
// when it comes to following the loose specification. If one wants to be able to read STL
// files that are out in the wild, one needs to relax some of the specifications.
//
// For maximum flexibility, consider these suggestions and observations:
//
// - Don't rely on the ASCII data to be only floats. The data could be doubles. Read the
//   data, but then for output convert to float. Binary STL is always float.
// - Don't rely on numeric data being written with full precision. It depends on the format
//   specifiers that were used to write out the floating-point data.
// - Don't rely on the space character as the only whitespace separator.
// - Don't rely on the normal vector. Compute it yourself from the vertex data
// - Don't rely on only one solid in an ASCII STL file.
// - Don't rely on the data being in the all-positive octant (definite numbers that are
//   non-negative and nonzero).
// - Don't rely on the data being "water tight" or that facets share edges with other facets.
//   Even if the data is actually a regular water tight boundary model, there is no structure
//   in the file format to indicate topological relationships, such as a winged-triangle or
//   half-edge relationship. Treat the STL data as just a bag of triangles, and if needed,
//   post-process for possible topological relationships.
// - For binary STL, use the file size and facet count to determine the endianess of the data.
//   Binary STL is likely little endian, but it is not specified. When writing binary STL,
//   use little endian.
// - There might be NaNs or infinities (non-definite values) in the data.
// - Facet data may be degenerate (zero area triangle), for numeric or topological reasons.
//
// If one takes these suggestions to heart, the most pragmatic approach for parsing
// ASCII STL files is to skip everything except the vertex data of the form "vertex v1 v2 v3",
// where v1, v2, and v3 are floating point numbers (float or doubles). Every 3 vertices read
// gives a triangular facet. The facets can be checked for NaNs and infinities. The normal
// vector can be computed for each facet, and if the facet is "degenerate" (depending on the
// normal vector and/or the area of the facet), then the facet can be discarded.
//
// This pragmatic approach may be too loose, in case the number of "vertex" reads is not a
// multiple of 3. A more cautious approach may be to parse for "outer loop" and "endloop".
// If there are not 3 vertex reads between "outer loop" and "endloop", or if there aren't
// 3 numbers per vertex, then the facet could be discarded, and parsing continued.
// 
// When parsing a binary STL file, read the number of facets that are supposed to be in
// the file. Each facet is 50 bytes long, and there is an 80 byte file header, with 4 bytes
// for the facet count. If the math doesn't work out with the size of the binary file, flip
// the endianness of the facet count read and see if that works out. If so, then all data
// reads will need to take the other endianness into account. If not, something is wrong
// with the file, and there is no obvious work-around.


//
// This example is under construction. It will become an STL file converter, checking for
// data integrity, and it will be able to read and convert between both ASCII and binary
// STL formats.
//

extern int stl_main(int argc, char *argv[]);
