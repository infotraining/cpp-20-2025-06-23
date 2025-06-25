#include <catch2/catch_test_macros.hpp>
#include <helpers.hpp>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>
#include <map>
#include <tuple>

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
        auto tail = std::views::counted(data.rbegin(), 3);

        helpers::print(tail, "tail");
    }

    SECTION("iota")
    {
        auto iota_v = std::views::iota(1, 10);

        helpers::print(iota_v, "iota_v");
    }

    SECTION("single")
    {
        for(const auto& item : std::views::single(42))
        {
            std::cout << item << " ";
        }
        std::cout << "\n";
    }

    SECTION("pipes |")
    {
        auto evens = data | std::views::filter([](int x) { return x % 2 == 0; });

        for(const auto& item : evens)
        {
            std::cout << "even: " << item << "\n";
        }

        auto data_gathered = 
            std::views::iota(1)
                | std::views::take(10)
                | std::views::transform([](int n) { return n * n; })
                | std::views::filter([](int x) { return x % 2 == 0; })
                | std::views::reverse
                | std::views::common;

        std::vector<int> data_vec(data_gathered.begin(), data_gathered.end());

        helpers::print(data_gathered, "data_gathered");
    }
}

template <std::ranges::range T>
    requires requires(std::ranges::range_value_t<T> obj, std::ostream& out) {  out << obj; }
void classic_print(T&& items)
{
    for(const auto& item : items)
        std::cout << item << " ";
    std::cout << "\n"; 
}

TEST_CASE("views - reference semantics")
{    
    std::vector data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    auto evens_view = data | std::views::filter([](int i) { return i % 2 == 0; });
    classic_print(evens_view);

    helpers::print(data, "data");

    for(auto& item : evens_view)
    {
        item = 0;
    }

    helpers::print(data, "data");
}

TEST_CASE("maps & ranges")
{
    std::map<int, std::string> dict = { {1, "one"}, {2, "two"} };

    helpers::print(dict | std::views::keys, "keys");
    helpers::print(dict | std::views::values, "values");

    auto keys_of_dict = dict | std::views::elements<0>;
    helpers::print(keys_of_dict, "keys_of_dict");
}

TEST_CASE("split")
{
    std::string text = "abc def ghi";

    auto tokens_view = std::views::split(text, " ") | std::views::transform([](auto token) { return std::string_view(token); });

    helpers::print(tokens_view, "tokens");
}