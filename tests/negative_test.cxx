#include "dsga.hxx"
#include <array>
#include <vector>

//
// test cases for dsga concepts
//

// positive cases — things that SHOULD satisfy the concepts
static_assert(dsga::vec_like<dsga::vec3>, "dsga::vec3 should satisfy vec_like");
static_assert(dsga::vec_like<dsga::vec4>, "dsga::vec4 should satisfy vec_like");
static_assert(dsga::writable_vec_like<dsga::vec3>, "dsga::vec3 should satisfy writable_vec_like");

// negative cases — things that should NOT satisfy vec_like
static_assert(!dsga::vec_like<std::array<float, 3>>, "std::array<float, 3> should not satisfy vec_like");
static_assert(!dsga::vec_like<std::vector<float>>, "std::vector<float> should not satisfy vec_like");
static_assert(!dsga::vec_like<int>, "int should not satisfy vec_like");
static_assert(!dsga::vec_like<float>, "float should not satisfy vec_like");

// const vec3 should be vec_like but NOT writable_vec_like
static_assert(dsga::vec_like<const dsga::vec3>, "const vec3 should satisfy vec_like");
static_assert(!dsga::writable_vec_like<const dsga::vec3>, "const vec3 should not satisfy writable_vec_like");

// a minimal look-alike struct with no dsga trait specializations
struct fake_vec
{
	float data[3];
	float operator[](std::size_t i) const { return data[i]; }
	static constexpr std::size_t size() { return 3; }
};
static_assert(!dsga::vec_like<fake_vec>, "fake_vec should not satisfy vec_like");
