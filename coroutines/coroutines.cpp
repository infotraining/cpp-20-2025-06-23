#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <coroutine>
#include <syncstream>

using namespace std::literals;

class TaskResumer
{
public:
    struct promise_type;
    using CoroHandle = std::coroutine_handle<promise_type>;
    
    struct promise_type
    {
        TaskResumer get_return_object()
        {
            return TaskResumer{ CoroHandle::from_promise(*this) };
        }

        auto initial_suspend()
        {
            std::cout << "Initial suspension point..." << std::endl;
            return std::suspend_always{};
        }

        auto final_suspend() noexcept
        {
            std::cout << "Final suspension point..." << std::endl;
            return std::suspend_always{};
        }

        void return_void()
        {
            std::cout << "Returning void" << std::endl;
        }

        void unhandled_exception() 
        {
            std::terminate();
        }
    };

    TaskResumer(CoroHandle coro_hndl) : coro_hndl_{coro_hndl}
    {}

    TaskResumer(const TaskResumer&) = delete;
    TaskResumer& operator=(const TaskResumer&) = delete;

    ~TaskResumer()
    {
        if (coro_hndl_)
            coro_hndl_.destroy();
    }

    bool resume() const
    {
        if (!coro_hndl_ || coro_hndl_.done())
            return false;

        coro_hndl_.resume();

        return !coro_hndl_.done();
    }
private:
    CoroHandle coro_hndl_;
};

TaskResumer simplest_coroutine()
{
    std::cout << "Simplest_coroutine_started..." << std::endl;

    int i = 0;
    ++i;

    co_await std::suspend_always();

    std::cout << "Step: " << i << std::endl;
    ++i;

    std::cout << "Step: " << i << std::endl;
    ++i;

    co_await std::suspend_always();

    std::cout << "End of simplest_coroutine\n";
}

TEST_CASE("first coroutine")
{
    TaskResumer task = simplest_coroutine();

    while (task.resume())
        std::cout << "Caller!!!" << std::endl;
}

//////////////////////////////////////////////////////////
// Awaiter concept

template <typename T, typename... Ts>
concept OneOf = (std::same_as<T, Ts> || ...);

static_assert(OneOf<int, bool, double, int>);
static_assert(OneOf<int, int, double>);
static_assert(not OneOf<int, short, double, char>);

template <typename, template <typename...> class>
constexpr bool IsInstanceOf_v = false;

template <typename... As, template <typename...> class T>
constexpr bool IsInstanceOf_v<T<As...>, T> = true;

template <typename A, template <typename...> class T>
concept IsInstanceOf = IsInstanceOf_v<A, T>;

static_assert(IsInstanceOf<std::tuple<int>, std::tuple>);
static_assert(IsInstanceOf<std::coroutine_handle<void>, std::coroutine_handle>);

template <typename T>
concept AwaitSuspendResult = OneOf<T, void, bool> || IsInstanceOf<T, std::coroutine_handle>;

static_assert(AwaitSuspendResult<void>);
static_assert(AwaitSuspendResult<bool>);
static_assert(AwaitSuspendResult<std::coroutine_handle<void>>);
static_assert(not AwaitSuspendResult<int>);

template <typename T, typename... Promise>
concept Awaiter = requires(T obj, std::coroutine_handle<Promise...> coro_hndl) {
    { obj.await_ready() } -> std::convertible_to<bool>;
    { obj.await_suspend(coro_hndl) } -> AwaitSuspendResult;
    obj.await_resume();
};

static_assert(Awaiter<std::suspend_always>);
static_assert(Awaiter<std::suspend_never>);

inline auto sync_out(std::ostream& out = std::cout)
{
    return std::osyncstream{out};
}

class FireAndForget
{
public:
    class promise_type
    {
    public:
        FireAndForget get_return_object()
        {
            return {};
        }

        auto initial_suspend()
        {
            sync_out() << "...Initial suspension point...\n";
            return std::suspend_never{};
        }

        auto final_suspend() noexcept
        {
            sync_out() << "...Final suspension point...\n";
            return std::suspend_never{};
        }

        void return_void()
        {}

        void unhandled_exception()
        {
            std::terminate();
        }
    };
};

struct ResumeOnNewThreadAwaiter 
{
    bool await_ready() noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coro_hndl)
    {
        std::thread([coro_hndl] { coro_hndl.resume(); }).detach();
    }

    std::thread::id await_resume() const noexcept
    {
        return std::this_thread::get_id();
    }
};

static_assert(Awaiter<ResumeOnNewThreadAwaiter>);

auto resume_on_new_thread()
{
    return ResumeOnNewThreadAwaiter{};
}

FireAndForget fire_and_forget_test()
{
    std::cout << "Start on thread#" << std::this_thread::get_id() << "..." << std::endl;

    co_await resume_on_new_thread();

    std::cout << "Continue on thread#" << std::this_thread::get_id() << "..." << std::endl;

    std::thread::id thd_id = co_await resume_on_new_thread();

    std::cout << "Finish on thread#" << std::this_thread::get_id() << "..." << std::endl;
}

TEST_CASE("fire and forget")
{
    auto fire_on_threads = fire_and_forget_test();

    std::this_thread::sleep_for(2s);
}