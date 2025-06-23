#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

TEST_CASE("safe comparing integral numbers")
{
    int x = -42;
    unsigned int y = 665;

    SECTION("direct comparison")
    {
        //REQUIRE(x < y);
        REQUIRE(std::cmp_less(x, y));
    }

    SECTION("with generic functions")
    {
        auto my_comparer = [](auto a, auto b) { 
            if constexpr(std::is_integral_v<decltype(a)> && std::integral<decltype(b)>)
                return std::cmp_less(a, b);
            else
                return a < b;
        };

        REQUIRE(my_comparer(x, y));

        std::string str1 = "abc";
        std::string str2 = "def";

        REQUIRE(my_comparer(str1, str2));
    }

    SECTION("in_range")
    {
        REQUIRE(std::in_range<uint8_t>(x) == false);
        REQUIRE(std::in_range<uint8_t>(42) == true);
    }
} 