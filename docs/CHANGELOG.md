# Change Log

### v2.0.0 (no version change as no change to dsga.hxx)
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
