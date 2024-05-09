# Motivation

I wanted to expand the point/vector class that we use at work. We have x, y, and z data members, but that is the only way to get at the data. I wanted something more flexible that would use contiguous memory like an array, but still be useful with x, y, z data member access. I specifically did *NOT* want data function access as the way to get x, y, and z data. I wanted data *member* access.

I stumbled across a blog article about this very type of issue, and once I better understood anonymous unions and the common initial sequence concept (since verified with ```std::is_corresponding_member<>```), I was ready to write this library. It ended up writing it for my own projects, as the stuff at work is too entrenched to change given our resources.

I decided that instead of limiting swizzling to just x, y, and z values, I would go all the way and try and get as much vector and matrix functionality I could with this approach. I have learned since that this was called ""array programming"". That is why in the end I decided to implement the vectors and matrices from the [OpenGL Shading Language 4.6 specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf). ```dsga```'s implementation doesn't care about data-packing or rendering.

I also wanted to learn more about ```c++20```. I was interested in learning ```git``` (been using ```subversion``` for around 20 years) and how to create a public repo. This project is the result.
