# Change Log

### Latest Changes
* Added two new examples
    * line_closest_points - finds the closest points between two lines in 3D space, and the distance between those points.
    * shader_demo - a simple shader demo that uses dsga for vector and matrix math, and demonstrates how to use the library in a shader-like context.
* Removed ```as_base()``` from **vec_interface**, since it was not really necessary.
* Added protected destructor to **vec_interface** to prevent deletion through a base class pointer.

### v3.1.0
* Minor version bump with some breaking changes to the API.
* Updated to doctest v2.5.2
* Renamed the 5 main classes, deprecating the old names.
    * **basic_vector** -> **vec**
    * **basic_matrix** -> **mat**
    * **vector_base** -> **vec_interface**
    * **indexed_vector** -> **swizzle_vec**
    * **storage_vector** -> **vec_storage**
* Microsoft Visual Studio 2022 will no longer be directly supported. Testing may possibly occur using the VS2022 toolset on VS2026.
* Removed deprecated function ```logicalNot()```, replaced by ```compNot()```.
* CMake support rewritten.
    * Install target with ```cmake --install``` support.
    * ```dsga::dsga``` imported ```INTERFACE``` target for ```target_link_libraries```.
    * CMake config file for ```find_package``` support.
    * CMake target for linking with other projects.
    * Compiler flags for MSVC, gcc, and clang.

### v3.0.0
* Major version bump with breaking changes to the API
* Removed the ```data()``` interface for all vector types and matrix types, in order to remove/mitigate the possibilty of pointer overruns.
* Minor refactoring.

### v2.2.16
* Refactored transform, tolerance, bezier, angle, and basic example functions.
* Refactored ```cross()```, ```clamp()```, ```outerProduct()```. ```transpose()```, ```diagonal_matrix()```, some ```basic_matrix``` constructors, and other functions.
* General refactoring and formatting.
* Updated to cxcm v1.2.1.

### v2.2.15
* The attributes ```[[ likely ]]``` and ```[[ unlikely ]]``` were sprinkled around (and some were removed).
* Refactored ```determinant()``` and ```inverse()``` for matrices.
* Refactored ```mix()``` (the version like ```lerp()```).

### v2.2.14
* Lots of formatting and many minor improvements.

### v2.2.13
* Added ```likely``` and ```unlikely``` attributes for branch prediction when we are explicitly doing a ```throw```.

### v2.2.12
* Moved detail lambdas out of ```dsga``` namespace to a sub-namespace.

### v2.2.11
* Matrix functions that used to be able to mix ```float``` and ```double``` based arguments will now only work with arguments based off the same floating-point type.
* Changed more functions where ```auto``` was the return type to where the return type is now explicitly specified.

### v2.2.10
* ```normalize()``` now returns a vector of zeroes if the length is zero (instead of the old way where it returned a vector of NaNs).
* Modified angle example by adding an alternate (supposedly less accurate) function for determining the angle between 2 vectors.
* For many functions where ```auto``` was the return type, changed them so the return type is now explicitly specified, where possible. This is an effort to reduce compile times.

### v2.2.9
* Reverted changes to the ```length()``` method and associated tests.

### v2.2.8
* Added transformation matrix functions example.
* Refactored ```operator ==()``` for the vector classes.
* Added support for matrices for the tolerance example functions.
* Creaded example view transform functions.
* Updated some copyright dates.

### No Version Commit
* Tweaks to the MSVC debugger visualizer (dsga.natvis).

### v2.2.7
* Reworked example tolerance-checking functions.
* Minor formatting and commenting changes.
* Upgraded to doctest v2.4.12.

### v2.2.6
* Minor refactoring.
* Tweaked ```dsga.natvis``` for better debugging experience.
* Changed usage of ```std::array::operator []``` to ```std::array::at()``` for bounds checking.
* Overhauled ```dsga::mix()``` to use a faster algorithm than ```std::lerp()```.

### v2.2.5
* Extracted ```dsga::invoke()``` to its own example header, and modified it to work on a wide range of inputs.
* Made all lambda captures specific.

### v2.2.4
* Added ```dsga::compXor()``` to perform xor operations on boolean values (as opposed to bitwise xor ^)
* Added ```dsga::invoke()``` which returns a vector created by invoking an operation element-wise to a variable number of vectors (there must be at least 1) that are all the same size, but might be of different types
* Upgraded to cxcm v1.2.0

### v2.2.3
* Upgraded to cxcm v1.1.10

### v2.2.1
* Replaced home-brew asserts with exceptions. Attempting to make safer through bounds checking and other input checking, enforced by throwing exceptions.
* Upgraded to cxcm v1.1.8

### v2.2.0
* Reverted major changes between v2.0.5 and v2.1.4, so we no longer have the view structs that wrap a pointer. Wrapping a pointer turned the data structures from owning to non-owning for the view structs, but we want the vector and matrix structs to be owning. The point of the view experiment was built on lack of insight on the nature of owning vs. non-owning and what this library was trying to achieve.

###  Ancillary Changes
* Updated/refacatored some examples.
* Minor README refactoring.

### v2.0.5
* Fixed wrong matrix type (reversed dimensions) being returned from ```outerProduct()```.

### v2.0.4
* Added bool return types for a few lambdas.
* Renamed ```logicalNot()``` to ```compNot()```. Deprecated ```logicalNot()```.
* Added ```compAnd()``` and ```compOr()``` functions to complement ```compNot()```.
* Added missing scalar versions of non-geometric vector functions.
* Added some asserts.

### v2.0.3
* Tolerance checking functions moved to examples/tolerance.hxx.

### v2.0.2
* Upgraded to cxcm v1.1.4.
* Updated example iostream formatters for boolalpha output when appropriate.
* Added Intel's icx/icpx compiler (for Windows) as one of the test compilers.
* Potentially breaking change: removed an implicit ```dsga::basic_matrix``` constructor, now requiring the use of a constructor that is explicit.
* Minor doc and repo maintenance.

### v2.0.1
* Added ```query()``` function (not in GLSL nor ```std::valarray```) to vector_base. It works like ```apply()```, but expects a boolean predicate, and returns a vector of boolean values instead of element type T.
* Upgraded to cxcm v1.1.3.
* Minor refactoring.

### Ancillary Changes
* Updated example ```iostream``` and ```std::format``` output to look like the c++23 std::format style for ranges.
* Updated the MSVC debugger visualizer (dsga.natvis) to look like the c++23 std::format style for ranges.
* Minor refactoring of some examples.

### v2.0.0
* Large __Breaking Change__ - minimized how vectors of length == 1 behave as vectors. Most dsga operations and functions treat length == 1 vectors as scalars, returning scalar results (mostly through refactoring the underlying execution machinery). Use of the non-GLSL types iscal, uscal, bscal, scal, fscal, dscal, etc., is generally discouraged.
* Small __Breaking Change__ - reverted/removed ```std::initializer_list``` constructors added in v1.5.0.
* Moved vector relational functions above the other vector functions (for use in assertions).
* Added ```within_tolerance()``` comparison functions, that fit well with ```within_distance()``` and ```within_box()```.
* Upgraded to cxcm v1.1.2.
* Minor type constraint (concepts) refactoring.
* Other minor refactoring.
* Added changelog.

### v1.5.0
* Small __Breaking Change__ - added ```std::initializer_list``` constructors to ```basic_vector``` and ```basic_matrix``` - if not enough components, then fill rest with zeros - if too many components, just use the components necessary to fill the vector or matrix.
* Fixed ```indexed_vector``` iterator classes to use signed types for indexing into storage (fixes iterator subtraction and ```reverse_iterator``` usage, as the iterators are random-access).

### v1.4.1
* Minor refactoring.
* Comment removal and/or updating.
* Removed Microsoft VS2019 support (latest version of VS2019 does not compile dsga).

### v1.4.0
* Minor type constraint (concepts) refactoring.
* Updating copyright dates.
