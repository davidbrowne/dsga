# Lambda Pack Wrapper

Imagine this flexible function that converts a ```std::array``` by applying a lambda to all its elements:

```c++
template <std::floating_point T, std::size_t S, typename UnaryOp>
constexpr auto apply_op(const std::array<T, S> &data, UnaryOp lambda) noexcept
{
    using return_type = decltype(UnaryOp()(std::declval<T>()));

    auto return_val = std::array<return_type, S>();

    for (std::size_t i = 0; i < S; ++i)
    {
        return_val[i] = lambda(data[i]);
    }

    return return_val;
}

// converted_array == std::array{ -3., -4., -5. }
auto converted_array = apply_op(std::array{ 3., 4., 5. },
                                [](auto val){ return -val; });
```

We can't use a ranged-for loop without somehow having an index variable too, so the good old for loop works better. As far as initializing a variable with the results of calling this function, the best case here is that we get Named Return Value Optimization (NRVO). At the very least we will get implicit move optimization.

We could instead use a library function to fill in all the data, but the point here is how to process data for types that are array-like in nature and also optionally return the data. By array-like, I mean that they are indexable starting at 0 through n-1, for something of size n, and the method of indexing is something like ```operator []``` or maybe a function like ```std::get<>```.

One different way of processing this data all at once is through variadic template pack expansion; however, where is the parameter pack? Usually to use parameter packs out of nowhere, we must use a two-function process, one function to introduce the parameter pack, the other to consume it:

```c++
template <std::floating_point T, std::size_t S, typename UnaryOp>
constexpr auto apply_op(const std::array<T, S> &data, UnaryOp lambda) noexcept
{
    return apply_op_impl(data, lambda, std::make_index_sequence<S>{});
}

template <std::floating_point T, std::size_t S, typename UnaryOp, std::size_t ...Is>
constexpr auto apply_op_impl(const std::array<T, S> &data, UnaryOp lambda, std::index_sequence<Is...>)
{
    using return_type = decltype(UnaryOp()(std::declval<T>()));

    return std::array<return_type, S>{ lambda(data[Is])... };
}

// converted_array == std::array{ -3., -4., -5. }
auto converted_array = apply_op(std::array{ 3., 4., 5. },
                                [](auto val){ return -val; });
```

So here we created a parameter pack as an argument for a function that does the implementation, and that function now has a variadic template pack that can be expanded and used as indexes into the array. This works just fine, and it will give us RVO. It is kind of clumsy to have to do it with two functions just so we can create a function that has a variadic parameter pack to do the expansion.

We can do the same thing without introducing an external function. We can instead create a lambda that does the same thing and immediately evaluate it all in one shot:

```c++
template <std::floating_point T, std::size_t S, typename UnaryOp>
constexpr auto apply_op(const std::array<T, S> &data, UnaryOp lambda) noexcept
{
    using return_type = decltype(UnaryOp()(std::declval<T>()));

    // lambda pack wrapper
    return [&]<std::size_t ...Is>(std::index_sequence<Is...>) noexcept
    {
        return std::array<return_type, S>{ lambda(data[Is])... };
    }(std::make_index_sequence<S>{});
}

// converted_array == std::array{ -3., -4., -5. }
auto converted_array = apply_op(std::array{ 3., 4., 5. },
                                [](auto val){ return -val; });
```

So we created a lambda with a std::index_sequence<> as its argument. Inside the lambda, we can do the pack expansion and return the result of what was going on with the pack expansion. The function itself just returns the result of the lambda execution. We will get RVO. In this case, the introduction of the parameter pack and its expansion is all taken care of in the lambda and its parameters. The variadic nature is self contained. I call this a **lambda pack wrapper**.

Imagine that we had an array-like type where we didn't have the ability to create a non-initialized instance, and we also couldn't fill it with data one piece at a time. Instead, everything must be initialized all at once in a function or constructor call. Parameter pack expansion works great in this case, filling the function call with the exact number of arguments needed. We couldn't use the approach in the first example at all.

I am also assuming that compiler generated pack expansion has good codegen.

To me this approach seems elegant. I am fond of the bezier example of this in the quick peek part of the [README](../README.md#a-quick-peek-at-some-examples), made even more generic and flexible in the [details](DETAILS.md#detailed-generic-example). Here, we pack expand all the rows, where each row represents a different set of ordinate values to evaluate, e.g., x, y, z, w. There are two functions, but the other function has nothing to do with pack expansion, it is all about computing the value for the pack expansions.

