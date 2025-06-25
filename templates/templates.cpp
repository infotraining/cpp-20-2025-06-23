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

    if constexpr (requires { container.push_back(std::forward<decltype(item)>(item)); })
        container.push_back(std::forward<decltype(item)>(item));
    else
        container.insert(std::forward<decltype(item)>(item));
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

template <double Factor, typename T>
auto scale(T x)
{
    return Factor * x;
}

TEST_CASE("NTTP - double")
{
    REQUIRE(scale<2.0>(8) == 16.0);
}

struct Tax
{
    double value;

    constexpr Tax(double v) : value{v}
    {}

    double get_value() const
    {
        return value;
    }
};

template <Tax Vat>
constexpr auto calc_gross_price(double net_price)
{
    return net_price + net_price * Vat.get_value();
}

TEST_CASE("NTTP - structs")
{
    constexpr Tax vat_pl{0.23};
    constexpr Tax vat_ger{0.19};

    REQUIRE(calc_gross_price<vat_pl>(100.0) == 123.0);
    REQUIRE(calc_gross_price<vat_ger>(100.0) == 119.0);
}

template <size_t N>
struct Str
{
    char text[N];

    constexpr Str(const char(&str)[N]) noexcept
    {
        std::copy(str, str + N, text);
    }

    friend std::ostream& operator<<(std::ostream& out, const Str& str)
    {
        out << str.text;

        return out;
    }

    auto operator<=>(const Str& other) const = default;
};

template <Str LogName>
class Logger
{
public:
    void log(std::string_view msg)
    {
        std::cout << LogName << ": " << msg << "\n";
    }
};

TEST_CASE("NTTP - strings")
{
    Logger<"main_logger"> logger_1;
    Logger<"backup_logger"> logger_2;

    logger_1.log("Start");
    logger_2.log("Stop");
}

template <std::invocable auto GetVat>
constexpr std::floating_point auto calc_gross_price_with_lambda(std::floating_point auto net_price)
{
    return net_price + net_price * GetVat();
}

TEST_CASE("NTTP - lambda")
{
    constexpr auto get_vat_pl = [] { return 0.23; };
    constexpr auto get_vat_ger = [] { return 0.19; };

    REQUIRE(calc_gross_price_with_lambda<get_vat_pl>(100.0) == 123.0);
    REQUIRE(calc_gross_price_with_lambda<get_vat_ger>(100.0) == 119.0);
}