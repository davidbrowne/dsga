# Details

## How The Vectors Work

There are really two vector classes: **basic_vector** and **indexed_vector**. ```basic_vector``` is what you would normally think of as a contiguously stored vector/point representation. ```indexed_vector``` is a view on ```basic_vector```, which may only be modified under certain conditions.

A ```basic_vector``` has data members that provide [swizzling](https://en.wikipedia.org/wiki/Swizzling_(computer_graphics)). These data members are of type ```indexed_vector```, and this is where they are a view on the owning ```basic_vector```. Only the ```indexed_vector```s that do not have duplicate indexes in the swizzle are modifiable, e.g., ```foo.xzy``` is modifiable, while ```foo.zzy``` is not modifiable. Either way, an ```indexed_vector``` from a swizzle has a lifetime tied to the lifetime of the ```basic_vector``` it came from.

We want to use both types of vectors in the same way, for constructors, equality comparison, assignment, operators, compound assignment, vector functions, etc. Instead of duplicating this effort, ```basic_vector``` and ```indexed_vector``` derive from a [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) base class called **vector_base**, and this provides the generic foundation for constructors, equality comparison, assignment, operators, compound assignment, vector functions, etc:
![vec_base](./vec_base_uml.svg)

```vector_base``` assumes that its derived structs and classes implement the **vector duck type** interface, which is not a real code interface, just more of a conceptual [duck typing](https://en.wikipedia.org/wiki/Duck_typing) idea. Both ```basic_vector``` and ```indexed_vector``` implement this conceptual interface.

```vector_base``` carries the following information, via template parameters:
* Whether it can be used as an lvalue, i.e., **is it writable**
* The **type of the data** in the vector (some arithmetic type)
* How many elements are in the vector (1-4), i.e., the **Count**
* The **type of the derived class**

It provides the following functions that can be used to generically manipulate and access vector data:
* **set()** - relies on ```init()```, which sets all the data in the vector to new values. Since this modifies the data, it is only enabled if it is writable. This function helps prevent aliasing issues that might occur otherwise, e.g., ```foo = foo.zyx;``` could have a problem with a naive implementation.
* **operator[]** - relies on ```at()```, which is a reference to a single data value. If writable then can use as an lvalue. The data is in logical order.
* **data()** - provides pointer to data access via ```raw_data()```. If it is writable, then can use pointer to write data. Pointer access is in physical order.
* **sequence()** - relies on ```make_sequence_pack()```. The physical order to logical order mapping in a parameter pack.
*  **length()** - relies on ```Count``` template parameter, and it returns type ```int```.
* **size()** - relies on ```Count``` template parameter, and it returns type ```std::size_t```.

## Inside basic_vector

Since we wanted to use notation such as ```foo.xy``` to access swizzles, the swizzled items must obviously be data members; however, there are many possible swizzles, increasing dramatically for the vector length 4 case (340 swizzles just for xyzw!). We don't want the size of ```basic_vector``` to grow, and the only way to be able to access many data members without the class/struct size growing is through a union. Since we don't want to have an intermediate name for access, it must be an **anonymous union**.

To get what we want, we need for each member of the union to have a common intial sequence. The anonymous union for each struct is made up of one member of type **storage_wrapper** and many swizzled ```indexed_vector```s, the number depending on the length of the vector. We have tests that confirm that these all share a **common initial sequence**, using the function ```std::is_corresponding_member()```. Both ```storage_wrapper``` and the ```indexed_vector```s have the same storage type (for now it is a ```std::array```), and the size of the array is the size of the ```basic_vector```.

```basic_vector``` member functions and constructors work through ```storage_wrapper```. It is also used as the initialized anonymous union member. For constexpr variables with anonymous unions, one and only one member must be initialized and/or accessed at compile time, and for us it is the one of type ```storage_wrapper```.

Since we are using a union where everything has the same common initial sequence, the data is shared and accessible for all the union members, and everything has the same lifetime as the ```basic_vector``` they come from.

## Inside indexed_vector
