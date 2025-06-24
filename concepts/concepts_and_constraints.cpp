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
        requires Pointer<T>          // requires clause
    auto max_value(T a, T b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return max_value(*a, *b);
    }
} // namespace Ver_2

template <typename T>
concept Pointer = requires(T ptr) {  // requires expression
    *ptr;
    ptr == nullptr;
    ptr != nullptr;
};

namespace Ver_3
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

namespace Ver_4
{
    // unconstrained template function
    auto max_value(auto a, auto b)
    {
        return a < b ? b : a;
    }

    // template function with constraints
    auto max_value(Pointer auto a, Pointer auto b)
        requires std::same_as<decltype(a), decltype(b)>
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }
} // namespace Ver_4

inline namespace Ver_5
{
    // unconstrained template function
    template <typename T1, typename T2>
    auto max_value(T1 a, T2 b)
    {
        return a < b ? b : a;
    }

    // template function with constraints
    template <Pointer T1, Pointer T2>
        requires std::same_as<T1, T2>
    auto max_value(T1 a, T2 b)
    {
        assert(a != nullptr);
        assert(b != nullptr);
        return *a < *b ? *b : *a;
    }
} // namespace Ver_5

TEST_CASE("constraints")
{
    int x = 10;
    int y = 20;

    // CHECK(max_value(x, y) == 20);
    CHECK(max_value(&x, &y) == 20);

    const int* ptr_to_const_1 = &x;
    const int* ptr_to_const_2 = &y;
    CHECK(max_value(ptr_to_const_1, ptr_to_const_2) == 20);

    auto sptr1 = std::make_shared<int>(20);
    auto sptr2 = std::make_shared<int>(42);
    CHECK(max_value(sptr1, sptr2) == 42);
}

template<typename T>
concept HasOutOperator = requires(T obj, std::ostream& out)
{
    out << obj;
};

static_assert(HasOutOperator<int>);
static_assert(HasOutOperator<double>);
static_assert(HasOutOperator<std::string>);
static_assert(not HasOutOperator<std::vector<int>>); 

template <typename T>
struct Wrapper
{
    T value;

    void print() const
        requires HasOutOperator<T>
    {
        std::cout << "value: " << value << "\n";
    }

    void print() const
        requires std::ranges::range<T> && HasOutOperator<std::ranges::range_value_t<T>>
    {
        std::cout << "values: [ ";
        for (const auto& item : value)
            std::cout << item << " ";
        std::cout << "]\n";
    }
};

TEST_CASE("concepts")
{
    Wrapper<int> w1{42};
    w1.print();

    Wrapper w2{std::vector{1, 2, 3}};
    w2.print();
}

template <typename T>
class ScopedPtr
{
    T* ptr_;
public:
    explicit ScopedPtr(T* ptr) noexcept : ptr_{ptr}
    {}

    ~ScopedPtr() noexcept
    {
        delete ptr_;
    }

    ScopedPtr(const ScopedPtr&) = delete;
    ScopedPtr& operator=(const ScopedPtr&) = delete;
    
    T& operator*() const noexcept
    {
        return *ptr_;
    }

    T* operator->() const noexcept
        requires (!std::is_fundamental_v<T>)
    {
        return ptr_;
    }
};

struct Data
{
    int id;
    std::string name;
};

TEST_CASE("using unique_ptr")
{
    ScopedPtr<int> uptr1{new int(42)};
    REQUIRE(*uptr1 == 42);

    ScopedPtr<Data> uptr2{new Data{42, "forty-two"}};
    REQUIRE(uptr2->id == 42);
}

std::unsigned_integral auto gen_id()
{
    static uint32_t id = 0;
    return ++id;
}

TEST_CASE("auto + concepts")
{
    std::convertible_to<uint32_t> auto id = gen_id();  // std::convertible_to<decltype(a), uint32_t>
}

///////////////////////////////////////
// requires expression

template <typename T>
concept LeanPointer = requires(T ptr) {
    *ptr;
    ptr == nullptr;
    requires sizeof(T) == sizeof(void*); // forced evaluation to bool
};

template <typename T>
concept ContainerLike = requires()
{
    typename T::value_type;
    typename T::iterator;
    typename T::const_iterator;
};

template <typename T>
concept Hashable = requires(T obj) {
    { std::hash<T>{}(obj) } -> std::convertible_to<uint32_t>;
};

TEST_CASE("requires expression")
{
    static_assert(LeanPointer<int*>);
    static_assert(LeanPointer<std::unique_ptr<int>>);
    static_assert(not LeanPointer<std::shared_ptr<int>>);

    static_assert(ContainerLike<std::vector<int>>);
    static_assert(not ContainerLike<int[32]>);

    static_assert(Hashable<int>);
    static_assert(Hashable<std::string>);
    static_assert(not Hashable<std::vector<int>>);
}

template <typename T>
concept HasCompareThreeWayCompareNoExcept = requires(T a, T b) {
    { a <=> b } noexcept;
};

template <typename T>
void check_spaceship_noexcept(T a, T b)
{
    static_assert(noexcept(a <=> b), "Spaceship potentially throws");
}

struct SpaceshipDefault
{
    int id;
    std::string name;

    auto operator<=>(const SpaceshipDefault& other) const = default;
};

struct SpaceshipCustom
{
    SpaceshipDefault sd;
    double price;

    auto operator<=>(const SpaceshipCustom& other) const noexcept
    {
        if (auto result = sd <=> other.sd; result == 0)
            return std::strong_order(price, other.price);
        else
            return result;
    }
};

TEST_CASE("has <=> as noexcept")
{
    static_assert(HasCompareThreeWayCompareNoExcept<int>);
    static_assert(HasCompareThreeWayCompareNoExcept<SpaceshipDefault>);
    //static_assert(HasCompareThreeWayCompareNoExcept<SpaceshipCustom>);

    check_spaceship_noexcept(42, 42);
    check_spaceship_noexcept(SpaceshipDefault{42, "ft"}, SpaceshipDefault{42, "ft"});
    check_spaceship_noexcept(SpaceshipCustom{ {42, "ft"}, 3.33 },SpaceshipCustom{ {42, "ft"}, 3.33 });
}