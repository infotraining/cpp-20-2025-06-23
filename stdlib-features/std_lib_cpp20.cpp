#include <array>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <mdspan>
#include <numbers>
#include <print>
#include <span>
#include <string>
#include <vector>
#include <format>

using namespace std::literals;

void print(std::span<const int> data, std::string_view desc)
{
    std::cout << desc << ": [ ";
    for (const auto& item : data)
        std::cout << item << " ";
    std::cout << "]\n";
}

void zero(std::span<int> data, int default_value = 0)
{
    for (auto& item : data)
        item = default_value;
}

TEST_CASE("std::span")
{
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    SECTION("fixed extent")
    {
        std::span<int, 5> spn_1{vec.begin(), 5};
        print(spn_1, "spn_1");
    }

    SECTION("dynamic extent")
    {
        std::span<int> spn_2{vec};
        print(spn_2, "spn_2");

        spn_2 = std::span{vec.begin() + 2, 3};
        zero(spn_2);

        spn_2[0] = 665;

        print(vec, "vec");
    }
}

void print_as_bytes(const float f, const std::span<const std::byte> bytes)
{
#ifdef __cpp_lib_format
    std::cout << std::format("{:+6}", f) << " - { ";

    for (std::byte b : bytes)
    {
        std::cout << std::format("{:02X} ", std::to_integer<int>(b));
    }

    std::cout << "}\n";
#endif
}

TEST_CASE("float as span of bytes")
{
    float data[] = {std::numbers::pi_v<float>};

    std::span<const std::byte> const_bytes = std::as_bytes(std::span{data});
    print_as_bytes(data[0], const_bytes);

    std::span<std::byte> writeble_bytes = std::as_writable_bytes(std::span{data});
    writeble_bytes[3] |= std::byte{0b1000'0000};
    print_as_bytes(data[0], const_bytes);
}

TEST_CASE("why span")
{
    int tab[10] = {1, 2, 3, 4};
    print(tab, "tab");

    int* dynamic_tab = new int[10]{1, 2, 3, 4};
    print(std::span{dynamic_tab, 10}, "dynamic_tab");

    std::vector<int> vec{1, 2, 3, 4};
    print(vec, "vec");

    std::array<int, 10> arr = {1, 2, 3};
    print(arr, "arr");

    delete[] dynamic_tab;
}

//////////////////////////////////////////////////
// BEWARE DANGLING POINTERS

constexpr std::span<int> get_head(std::span<int> items, size_t head_size = 1)
{
    return items.first(head_size);
}

TEST_CASE("beware dangling pointers")
{
    SECTION("OK")
    {
        std::vector<int> vec = {1, 2, 3, 4, 5, 6};
        auto head = get_head(vec, 3);
        print(head, "head");
    }

    SECTION("UB")
    {
        std::vector<int> vec = {1, 2, 3, 4, 5, 6};
        auto head = get_head(vec, 3);

        vec.push_back(7);

        print(head, "head"); // UB
    }
}

TEST_CASE("mdspan")
{
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    // View data as contiguous memory representing 2 rows of 6 ints each
    auto ms2 = std::mdspan(v.data(), 2, 6);
    
    // View the same data as a 3D array 2 x 3 x 2
    auto ms3 = std::mdspan(v.data(), 2, 3, 2);

    // Write data using 2D view
    for (std::size_t i = 0; i != ms2.extent(0); i++)
        for (std::size_t j = 0; j != ms2.extent(1); j++)
            ms2[i, j] = i * 1000 + j;

    // Read back using 3D view
    for (std::size_t i = 0; i != ms3.extent(0); i++)
    {
        std::println("slice @ i = {}", i);
        for (std::size_t j = 0; j != ms3.extent(1); j++)
        {
            for (std::size_t k = 0; k != ms3.extent(2); k++)
                std::print("{} ", ms3[i, j, k]);
            std::println("");
        }
    }
}

static std::string txt = "Hello";

TEST_CASE("std::format - basics")
{
    std::cout << std::format("{} has {} chars\n", txt, txt.size());

    std::cout << std::format("Price:{:_>8.2f} PLN\n", 665.9);

    std::cout << std::format("'{0}' has value {0:02X} {0:+4d} {0:03o}\n", '?');
    std::cout << std::format("'{0}' has value {0:02X} {0:+4d} {0:03o}\n", 'a');
    std::cout << std::format("'{0}' has value {0:02X} {0:+4d} {0:03o}\n", 'A');
}

TEST_CASE("std::format_to_n - better performance")
{
    SECTION("add null at the end")
    {
        char buffer[128];

        auto result = std::format_to_n(buffer, std::size(buffer) - 1, "String '{}' has {} chars\n", txt, txt.size());
        *(result.out) = '\0'; // add null at the end

        std::cout << buffer;
    }

    SECTION("use array that is zeroed")
    {
        std::array<char, 128> buffer{};

        auto result = std::format_to_n(buffer.data(), buffer.size() - 1, "String '{}' has {} chars\n", txt, txt.size());

        std::cout << buffer.data();
    }
}

TEST_CASE("std::format_to - unlimited number of chars")
{
    SECTION("works with streams")
    {
        std::format_to(std::ostreambuf_iterator<char>{std::cout}, "{} has value {}\n", "Pi", std::numbers::pi);
    }

    SECTION("works with back inserters")
    {
        std::string str;
        std::format_to(std::back_inserter(str), "{} has value {}\n", "Pi", std::numbers::pi);

        std::cout << str;
    }
}