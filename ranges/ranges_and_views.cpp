#include <catch2/catch_test_macros.hpp>
#include <helpers.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std::literals;

struct Person
{
    int id;
    std::string name;
};

TEST_CASE("ranges", "[ranges]")
{
    auto data = helpers::create_numeric_dataset<20>(42);
    helpers::print(data, "data");

    std::vector words = {"one"s, "two"s, "three"s, "four"s, "five"s, "six"s, "seven"s, "eight"s, "nine"s, "ten"s, 
                         "eleven"s, "twelve"s, "thirteen"s, "fourteen"s, "fifteen"s, "sixteen"s, "seventeen"s, "eighteen"s, "nineteen"s, "twenty"s};
    helpers::print(words, "words");

    SECTION("algorithms")
    {
        std::sort(words.begin(), words.end());
        std::ranges::sort(words, std::greater{});

        helpers::print(words, "words");
    }

    SECTION("projections")
    {
        //std::sort(words.begin(), words.end(), [](const auto& left, const auto& right) { return left.size() < right.size(); });
        std::ranges::sort(words, std::less{}, &std::string::size);
        helpers::print(words, "words");

        std::vector<Person> people = { Person{42, "Jan"}, {55, "Adam"}, {88, "Zenon"} };
        std::ranges::sort(people, std::less{}, &Person::name); 
        for(const auto& p : people)
            std::cout << p.name << " ";
        std::cout << "\n";
    }

    SECTION("concepts & tools")
    {
        std::vector vec = {1, 2, 3};

        using T = std::ranges::range_value_t<decltype(vec)>;
        using Iterator = std::ranges::iterator_t<decltype(vec)>;

        std::sort(std::ranges::begin(vec), std::ranges::end(vec));
    }
}

template <auto Value>
struct EndValue
{
    bool operator==(auto it) const
    {
        return *it == Value;
    }
};

TEST_CASE("sentinels", "[ranges]")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 9, 8, 11, 10, 7};
    std::ranges::sort(data.begin(), EndValue<42>{});
    helpers::print(data, "data");

    auto pos = std::ranges::find(data.begin(), std::unreachable_sentinel, 42);
    REQUIRE(*pos == 42);

    char txt[] = { 'a', 'b', 'c', '\0', 'e', 'f' };
    std::ranges::sort(std::ranges::begin(txt), EndValue<'\0'>{}, std::greater{});
    helpers::print(txt, "txt");
}

TEST_CASE("views")
{
    std::vector data = {2, 3, 4, 1, 5, 42, 6, 7, 8, 9, 10};

    SECTION("all")
    {
        auto all_view = std::views::all(data);
        auto target = all_view; // O(1)
        helpers::print(target, "target");
    }

    SECTION("subrange - iterator & sentinel as a view")
    {
        auto head = std::ranges::subrange{data.begin(), EndValue<42>{}};
        std::ranges::sort(head);
        helpers::print(data, "data");

        for(auto& item : head)
            item = 0;

        head[3] = 665;
        helpers::print(data, "data");
    }

    SECTION("counted")
    {        
    }

    SECTION("iota")
    {
    }

    SECTION("pipes |")
    {
    }
}

TEST_CASE("views - reference semantics")
{    
    std::vector data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    auto evens_view = data | std::views::filter([](int i) { return i % 2 == 0; });
    helpers::print(data, "data");

    // TODO - set all even numbers to 0 using evens_view

    helpers::print(data, "data");
}