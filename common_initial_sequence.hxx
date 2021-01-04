#pragma once

// for our vector and swizzling, we need to rely on union and the common initial sequence.
// anything written in the union via a union member that shares a common initial sequence with
// another union member can be referenced via any of the shared common initial sequence union
// members, regardless of whether the member last wrote to the union or not. this is the exception
// to the normal rule, where you can't read from a union member unless that was the last one that
// wrote through to the union.
//
// for our vector and swizzling, we want all union data members to share the same common intial
// sequence. it is unclear to me if we just want something there representing the type that isn't
// really used for normal reference. I have seen many unions that want to have the common intial
// sequence benefit and have the first union member be a dummy member of the common type. the
// problem lies in that the other union members use that type inside them, as they are often a
// struct with the data member in it. this first thing is not in a struct, so it should not be
// considered a part of the shared common initial sequence. to make it be a part of it, then it
// too needs to be in a struct.
//
// this information comes from what I have gathered from these links:
//
// https://www.reddit.com/r/cpp_questions/comments/7ktrrj/language_lawyers_unions_and_common_initial/
// https://stackoverflow.com/questions/43655657/union-common-initial-sequence-with-primitive
// https://stackoverflow.com/questions/48209179/do-scalar-members-in-a-union-count-towards-the-common-initial-sequence
// https://stackoverflow.com/questions/48058545/are-there-any-guarantees-for-unions-that-contain-a-wrapped-type-and-the-type-its

template <typename T>
struct common_initial_sequence_wrapper
{
	T value;
};
