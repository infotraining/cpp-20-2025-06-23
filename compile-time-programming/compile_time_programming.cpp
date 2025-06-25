#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <ranges>
#include <algorithm>
#include <numeric>

using namespace std::literals;

int runtime_func(int x)
{
    return x * x;
}

constexpr int constexpr_func(int x)
{
    return x * x;
}

consteval int consteval_func(int x)
{
    return x * x;
}

TEST_CASE("runtime vs. constexpr vs. consteval")
{
    int result1 = runtime_func(42);
    int result2 = constexpr_func(result1); // runtime call - maybe optimized by the compiler

    constexpr int result3 = constexpr_func(42);

    int result4 = consteval_func(42);
    constexpr int result5 = consteval_func(result3);
}

void compile_time_error() // runtime func
{}
 
consteval int next_two_digit_value(int value)
{
    if (value < 9 || value >= 99)
    {
        //compile_time_error();
        throw std::out_of_range("Arg out of range");
    }
 
    return ++value;
}

TEST_CASE("consteval")
{
    auto square = [](int x) consteval { return x * x; };

    constexpr std::array squares = { square(2), square(3), square(4), square(9) };

    // constexpr auto number = next_two_digit_value(99);
    REQUIRE(next_two_digit_value(66) == 67);
    // REQUIRE(next_two_digit_value(99) == 67); // Compiler ERROR
}

constexpr int len(const char* s)
{
    if (std::is_constant_evaluated()) // C++20
    // if consteval // C++23
    {
        // compile-time friendly code
        int idx = 0;

        while (s[idx] != '\0')
            ++idx;
        return idx;
    }
    else
    {
        return std::strlen(s); // function called at runtime
    }
}

TEST_CASE("constexpr extensions")
{
    constexpr auto txt = "Hello World!!!";

    constexpr int txt_len = len(txt);

    std::cout << len("Hey") << "\n";
}

constexpr uintmax_t factorial(uintmax_t n)
{   
    if (n <= 1)
        return 1;
    
    return n * factorial(n - 1);
}

template <size_t N>
consteval auto create_factorial_lookup_table()
{
    std::array<uintmax_t, N> values{};
    
    auto factorial_values = std::views::iota(0)
        | std::views::take(N)
        | std::views::transform([](auto n) { return factorial(n); });

    std::ranges::copy(factorial_values, values.begin());

    return values;
}

TEST_CASE("lookup table at compile time")
{
    constexpr auto factorial_lookup_table = create_factorial_lookup_table<20>(); 
}

constexpr int with_dynamic_memory()
{
    int result{};

    int* ptr = new int[100]{42};
    
    std::vector<int> vec;
    vec.push_back(42);
    
    result = ptr[0];
    
    delete[] ptr;
    
    return result;
}

TEST_CASE("dynamic memory allocation")
{
    constexpr int r = with_dynamic_memory();
}

template <std::ranges::input_range... TRng_>
constexpr auto avg_for_unique(const TRng_&... rng)
{
    using TElement = std::common_type_t<std::ranges::range_value_t<TRng_>...>;

    std::vector<TElement> vec;                            // empty vector
    vec.reserve((rng.size() + ...));                      // reserve a buffer - fold expression C++17
    (vec.insert(vec.end(), rng.begin(), rng.end()), ...); // fold expression C++17

    // sort items
    std::ranges::sort(vec); // std::sort(vec.begin(), vec.end());

    // create span of unique_items
    auto new_end = std::unique(vec.begin(), vec.end());
    std::span unique_items{vec.begin(), new_end};

    // calculate sum of unique items
    auto sum = std::accumulate(unique_items.begin(), unique_items.end(), TElement{});

    return sum / static_cast<double>(unique_items.size());
}

TEST_CASE("avg for unique")
{
    constexpr std::array lst1 = {1, 2, 3, 4, 5};
    constexpr std::array lst2 = {5, 6, 7, 8, 9};

    constexpr auto avg = avg_for_unique(lst1, lst2);

    std::cout << "AVG: " << avg << "\n";
}

template <size_t FIB_AMOUNT = 100>
constexpr std::array<size_t, FIB_AMOUNT> generate_fibonacci()
{
    std::array<size_t, FIB_AMOUNT> fib{};

    fib[0] = 0;
    fib[1] = 1;

    for (size_t i = 2; i < FIB_AMOUNT; ++i)    {
        fib[i] = fib[i - 1] + fib[i - 2];
    }

    return fib;
}

template <size_t N, typename F>
consteval auto create_lookup_table(F func)
{
    std::array<uintmax_t, N> values{};
    
    auto factorial_values = std::views::iota(0)
        | std::views::take(N)
        | std::views::transform([func](auto n) { return func(n); });

    std::ranges::copy(factorial_values, values.begin());

    return values;
}

constexpr int fibonacci(int n)
{
    if (n <= 1)
        return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

TEST_CASE("Exercise - Fibonacci lookup table - 1..100")
{
    constexpr auto lookup_fibonacci = create_lookup_table<10>([](int n) {
        if (n <= 1)
            return n;
        return fibonacci(n-1) + fibonacci(n-2);
    });
}