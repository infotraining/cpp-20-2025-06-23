#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <source_location>

using namespace std::literals;

struct Person
{
    int id;
    std::string name;
    double height;
    int age;
};

TEST_CASE("aggregates")
{
    Person p1{.id = 665, .name = "Jan", .age = 33};
    Person p2(42, "Adam", 1.78, 34);

    //Person p3(); // Most Vexing Parse - still a problem

    auto ptr = std::make_unique<Person>(55, "Ewa", 1.66, 23);

    int tab[](1, 2, 3, 4);
}

template <typename T1, typename T2>
struct Pair
{
    T1 fst;
    T2 snd;
};

// deduction guide - since C++20 provided by default by the compiler
// template <typename T1, typename T2>
// Pair(T1, T2) -> Pair<T1, T2>; 

TEST_CASE("CTAD - aggregates")
{
    Pair p1{42, 4.2343}; 
    Pair p2{54, "text"};
}

template <typename T>
void foo_location(T value)
{
    auto sl = std::source_location::current();
 
    std::cout << "file: " << sl.file_name() << "\n";
    std::cout << "function: " << sl.function_name() << "\n";
    std::cout << "line/col: " << sl.line() << "\n";
}

TEST_CASE("source location")
{
    foo_location(42);
}