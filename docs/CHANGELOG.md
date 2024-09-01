# Change Log

### v2.1.4
* Moved slice() functions to their own example file.

### v2.1.3
* Added ability to change the underlying data pointer of a ```basic_view```, which also affects ```view_wrapper``` and ```indexed_view```. This will lead to asserts in various functions when a view is wrapping the null pointer.
* When using ```swap``` with views that have aliasing overlap, e.g., ```view_wrapper``` or ```basic_view```, an exception will be thrown.
* Much refactoring of view structs/classes related to the above changes, including having to allow the null pointer as the underlying data pointer (with all functions that require use of the pointer will assert if it is null).
* Added a new constructor to ```basic_matrix``` that takes a raw pointer to an array of data. The constructor will throw if it is passed nullptr. The constructor does not wrap the pointer, rather it copies the array data into ```basic_matrix```'s own storage.
* Added a ```slice()``` function, to give you a contiguous sub-range of the data object. It works for arguments of type ```basic_vector```, ```basic_view```, and ```vector_view```. The function template argument is the length of the sub-range. The function will return a ```basic_view``` on the argument vector or view. The second argument is the offset into that vector or view, which is where the sub-range starts. The lifetime of the returned object depends on the lifetime of the vector or view argument, so it will have a dangling-pointer if it lives beyond the argument. This function can throw if the template parameter argument for length and the offset argument would result in a buffer overrun.

### v2.1.2
* Added ```get<>``` for ```view_wrapper```.
* Added ```as_base()``` to ```vector_base``` for debugging and testing purposes.

### v2.1.1
* Further experimental additions: adding a boolean ```Mutable``` template parameter to the classes for when they can be logically "const" (false means "const", true means "non const"), for both vectors (currently Mutable == true for the vectors) but mostly for the vector views that wrap an external storage pointer (const pointer vs non-const pointer). This is different from ```Writable```, which is used to determine if an indexed vector/indexed view is able to be an lvalue due to swizzle restrictions

### v2.1.0
* MAJOR EXPERIMENTAL ADDITION: there are now vector types, ```basic_view``` and ```indexed_view``` (and similarly ```view_wrapper```), that don't own their data. They are meant to work on a contiguous external data source, e.g., a slice of an array, instead of internal data storage, e.g., the storage in a ```basic_vector```. ```view_vector``` is similar to ```basic_vector```, but it is a ```basic_view``` with an internal array for its data source.
* Upgraded to cxcm v1.1.5.

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
