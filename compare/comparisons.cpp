#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace std::literals;

struct Point
{
    int x;
    int y;

    friend std::ostream& operator<<(std::ostream& out, const Point& p)
    {
        return out << std::format("Point({},{})", p.x, p.y);
    }

    bool operator==(const Point& other) const = default;
    //friend bool operator==(const Point& lhs, const Point& rhs) = default;

    bool operator==(const std::pair<int, int>& other) const
    {
        return x == other.first && y == other.second;
    }
};

struct Point3D : Point
{
    int z;

    constexpr Point3D(int x, int y, int z) : Point{x, y}, z{z}
    {}

    bool operator==(const Point3D& other) const = default;
};

TEST_CASE("Point - operator ==")
{
    SECTION("Point")
    {
        Point p1{1, 2};
        Point p2{1, 2};
        Point p3{2, 1};

        CHECK(p1 == p2);
        CHECK(p1 != p3); // !(p1 == p2) - rewriting expression

        SECTION("comparisons with std::pair<int>")
        {
            std::pair<int, int> pair_1{1, 2};
            CHECK(p1 == pair_1);  // p1.operator==(pair_1);
            CHECK(pair_1 == p1);  // p1 == pair_1 - rewriting expression
        }
    }

    SECTION("Point3D")
    {
        constexpr Point3D p1{1, 2, 3};
        constexpr Point3D p2{1, 2, 3};
        Point3D p3{1, 2, 4};

        static_assert(p1 == p2);
        CHECK(p1 != p3);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Comparisons
{
    struct Money
    {
        int dollars;
        int cents;

        constexpr Money(int dollars, int cents)
            : dollars(dollars)
            , cents(cents)
        {
            if (cents < 0 || cents > 99)
            {
                throw std::invalid_argument("cents must be between 0 and 99");
            }
        }

        constexpr Money(double amount)
            : dollars(static_cast<int>(amount))
            , cents(static_cast<int>(amount * 100) % 100)
        { }

        friend std::ostream& operator<<(std::ostream& out, const Money& m)
        {
            return out << std::format("${}.{}", m.dollars, m.cents);
        }

        auto operator<=>(const Money& other) const  = default;

        //bool operator==(const Money& other) const = default; - implicitly declared
    };

    namespace Literals
    {
        // clang-format off
        constexpr Money operator""_USD(long double amount)
        {
            return Money(amount);
        }
        // clang-format on
    } // namespace Literals
} // namespace Comparisons

TEST_CASE("Money - operator <=>")
{
    using Comparisons::Money;
    using namespace Comparisons::Literals;

    Money m1{42, 50};
    Money m2{42, 50};

    SECTION("comparison operators are synthetized")
    {
        CHECK((m1 <=> m2 == 0));

        CHECK(m1 == m2);
        CHECK(m1 == Money(42.50));
        CHECK(m1 == 42.50_USD);
        CHECK(m1 != 42.51_USD);
        CHECK(m1 < 42.51_USD);  // synthetized as: m1 <=> 42.51_USD < 0
        CHECK(m1 <= 42.51_USD);
        CHECK(m1 > 0.99_USD);
        CHECK(m1 >= 0.99_USD);

        static_assert(Money{42, 50} == 42.50_USD);
    }

    SECTION("sorting")
    {
        std::vector<Money> wallet{42.50_USD, 13.37_USD, 0.99_USD, 100.00_USD, 0.01_USD};
        std::ranges::sort(wallet);
        CHECK(std::ranges::is_sorted(wallet));
    }
}

struct Temperature
{
    double value;

    std::strong_ordering operator<=>(const Temperature& other) const
    {
        return std::strong_order(value, other.value);
    }

    bool operator==(const Temperature& other) const = default;
};

TEST_CASE("operator <=>")
{
    SECTION("primitive types")
    {
        int x = 42;
        int y = 665;

        REQUIRE((x <=> y < 0));
        REQUIRE((x <=> 42 == 0));

        std::strong_ordering result = x <=> y;
        REQUIRE(result == std::strong_ordering::less);
        REQUIRE((result < 0));
    }

    SECTION("custom types")
    {
        Temperature t1{42.2};
        Temperature t2{42.23};
        
        SECTION("result is a comparison category")
        {            
            auto result = t1 <=> t2;
            REQUIRE((result == std::partial_ordering::less));
        }

        SECTION("operators <, >, <=, >= are synthetized")
        {
            REQUIRE(t1 < t2);   
        }

        SECTION("operator ==")
        {
            REQUIRE(t1 == Temperature{42.2});
        }
    }
}

class Car
{
    std::string licence_plate_;
    int milage_;
public:
    Car(const std::string& licence_plate, int milage) : licence_plate_{licence_plate}, milage_{milage}
    {}

    std::string get_licence_plate() const
    {
        return licence_plate_;
    }

    int get_milage()
    {
        return milage_;
    }

    void drive(int distance)
    {
        milage_ += distance;
    }

    std::weak_ordering operator<=>(const Car& other) const
    {
        return licence_plate_ <=> other.licence_plate_;
    }

    bool operator==(const Car& other) const
    {
        return licence_plate_ == other.licence_plate_;
    }
};

TEST_CASE("comparison categories")
{
    SECTION("strong ordering")
    {
        SECTION("integral types")
        {
            int x = 42;
            int y = 665;

            auto result = x <=> y;
            static_assert(std::same_as<decltype(result), std::strong_ordering>);
        }

        SECTION("strings")
        {
            std::string str1 = "abc";
            std::string str2 = "abc";

            auto result = str1 <=> str2;
            static_assert(std::same_as<decltype(result), std::strong_ordering>);
        }
    }

    SECTION("partial ordering")
    {
        double dx = 0.01;
        double dy = 0.001;

        auto result = dx <=> dy;
        CHECK((result == std::partial_ordering::greater));
        CHECK((result > 0));

        result = dx <=> std::numeric_limits<double>::quiet_NaN();
        CHECK((result == std::partial_ordering::unordered));
    }

    SECTION("weak ordering")
    {
        Car c1{"KR 11114", 0};

        Car c2 = c1;
        c2.drive(100);

        auto result = c1 <=> c2;
        REQUIRE(( result == 0));
    }
}

struct PreCpp20
{
    int value;

    bool operator==(const PreCpp20& other) const
    {
        return value == other.value;
    }

    bool operator<(const PreCpp20& other) const
    {
        return value < other.value;
    }
};

struct PostCpp20
{
    int x;
    PreCpp20 y;

    std::strong_ordering operator<=>(const PostCpp20& other) const = default;
};

TEST_CASE("Pre & Post C++20")
{
    PostCpp20 p1{1, PreCpp20{2}};
    PostCpp20 p2{1, PreCpp20{3}};

    CHECK((p1 <=> p2 < 0));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Comparisons
{
    class Data
    {
        int* buffer_;
        size_t size_;

    public:
        Data(std::initializer_list<int> values)
            : buffer_(new int[values.size()])
            , size_(values.size())
        {
            std::copy(values.begin(), values.end(), buffer_);
        }

        ~Data()
        {
            delete[] buffer_;
        }

        auto operator<=>(const Data& other) const
        {
            return std::lexicographical_compare_three_way(buffer_, buffer_ + size_, other.buffer_, other.buffer_ + other.size_);
        }

        bool operator==(const Data& other) const
        {
            if (size_ != other.size_)
                return false;
            
            return std::equal(buffer_, buffer_ + size_, other.buffer_);
        }
    };
} // namespace Comparisons

TEST_CASE("lexicographical_compare_three_way")
{
    using Comparisons::Data;

    Data data1{1, 2, 3};
    Data data2{1, 2, 3};
    Data data3{1, 2, 4};

    CHECK(data1 == data2);
    CHECK(data1 <= data2);
}