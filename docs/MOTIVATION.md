# Motivation

I wanted to expand the point/vector class that we use at work. We have x, y, and z data members, but that is the only way to get at the data. I wanted something more flexible that would use contiguous memory like an array, but still be useful with x, y, z data member access. I specifically did *NOT* want accessor functions as the way to get x, y, and z data. I wanted data member access.

After reading a blog article about this very type of issue, I started thinking more about this, and once I better understood anonymous unions and the common initial sequence concept (since verified with ```std::is_corresponding_member<>```), I was ready to write this library, but for my own projects, as the work stuff is too entrenched, and we were using ```c++14``` at work when I started, with this project being written in ```c++20```.

I decided that instead of limiting my swizzling to just x, y, and z values, I would go all the way and try and get as much vector and matrix functionality I could with this approach. That is why in the end I decided to implement the vectors and matrices from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). ```dsga```'s implementation doesn't care about data-packing or rendering.

I also wanted to learn more about ```c++20```. I was interested in learning ```git``` (been using ```subversion``` for around 20 years) and how to create a public repo. This project is the result.
