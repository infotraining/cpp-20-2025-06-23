#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace std::literals;

auto add(auto a, auto b)
{
    return a + b;
}

namespace Explain
{
    template <typename T1, typename T2>
    auto add(T1 a, T2 b)
    {
        return a + b;
    }
} // namespace Explain

void add_to(auto& container, auto&& item)
{
    //undefined(); // phase 1
    //container.undefined(item); // phase 2

    container.push_back(std::forward<decltype(item)>(item));
}

namespace AlternativeTake
{
    template <typename TItem>
    void add_to(auto& container, TItem&& item)
    {
        container.push_back(std::forward<TItem>(item));
    }
}

namespace Explain
{
    template <typename T1, typename T2>
    void add_to(T1& container, T2&& item)
    {
        container.push_back(std::forward<decltype(item)>(item));
    }
} // namespace Explain

TEST_CASE("functions with auto params")
{
    int x = 12;
    int y = 777;
    REQUIRE(add(x, y) == 789);

    std::vector<int> vec = {1, 2, 3, 4};
    add_to(vec, 5);

    // REQUIRE(vec == std::vector{1, 2, 3, 4, 5});
}

TEST_CASE("templates & lambda expressions")
{
    REQUIRE(true);
}