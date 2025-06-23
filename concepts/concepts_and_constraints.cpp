#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std::literals;

// type trait - IsPointer_v

template <typename T>
struct IsPointer : std::false_type
{
    // static constexpr bool value = false;
};

template <typename T>
struct IsPointer<T*> : std::true_type
{
    // static constexpr bool value = true;
};

// template variable
template <typename T>
constexpr inline bool IsPointer_v = IsPointer<T>::value;

TEST_CASE("type traits")
{
    using T1 = int;
    using T2 = int*;
    using T3 = const int*;
    using T4 = volatile int*;

    static_assert(not IsPointer<T1>::value);
    static_assert(IsPointer_v<T2>);
    static_assert(std::is_pointer_v<T2>);
    static_assert(IsPointer_v<T3>);
    static_assert(IsPointer_v<T4>);
}

template <typename TContainer>
void print(TContainer&& c, std::string_view prefix = "items")
{
    std::cout << prefix << ": [ ";
    for (const auto& item : c)
        std::cout << item << " ";
    std::cout << "]\n";
}

namespace Ver_1
{
    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires IsPointer_v<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return max_value(*a, *b);
    }
} // namespace Ver_1

namespace Ver_2
{
    template <typename T>
    concept Pointer = IsPointer_v<T>;

    static_assert(IsPointer_v<int*>);
    static_assert(Pointer<int*>);
    static_assert(Pointer<const int*>);
    static_assert(not Pointer<int>);

    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <typename T>
        requires Pointer<T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return max_value(*a, *b);
    }
} // namespace Ver_2

template <typename T>
concept Pointer = requires(T ptr) {
    *ptr;
    ptr == nullptr;
    ptr != nullptr;
};

inline namespace Ver_3
{
    template <typename T>
    T max_value(T a, T b)
    {
        return a < b ? b : a;
    }

    template <Pointer T>
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return max_value(*a, *b);
    }
} // namespace Ver_3

// inline namespace Ver_4
// {
//     template <typename T>
//     T max_value(T a, T b)
//     {
//         return a < b ? b : a;
//     }

//     auto max_value(auto a, auto b) -> decltype(*a)
//         requires (Pointer<decltype(a)> && Pointer<decltype(b)> && std::same_as<decltype(a), decltype(b)>)
//     {
//         assert(a != nullptr);
//         assert(b != nullptr);
//         return *a < *b ? *b : *a;
//     }
// }

// template <Pointer T1, Pointer T2>
//     requires std::same_as<T1, T2>
// auto max_value(T1 a, T2 b)
// {
//     assert(a != nullptr);
//     assert(b != nullptr);
//     return max_value(*a, *b);
// }


TEST_CASE("constraints")
{
    int x = 10;
    int y = 20;

    //CHECK(max_value(x, y) == 20);
    CHECK(max_value(&x, &y) == 20);

    const int* ptr_to_const_1 = &x;
    const int* ptr_to_const_2 = &y;
    CHECK(max_value(ptr_to_const_1, ptr_to_const_2) == 20);

    auto sptr1 = std::make_shared<int>(20);
    auto sptr2 = std::make_shared<int>(42);
    CHECK(max_value(sptr1, sptr2) == 42);
}

template <typename T>
struct Wrapper
{
    T value;

    void print() const
    {
        std::cout << "value: " << value << "\n";
    }
};

TEST_CASE("concepts")
{
    Wrapper<int> w1{42};
    w1.print();
}