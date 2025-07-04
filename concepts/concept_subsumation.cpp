#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std::literals;

struct BoundingBox
{
    int w, h;
};

struct Color
{
    uint8_t r, g, b;
};

struct Rect
{
    int w, h;

    void draw() const
    {
        std::cout << "Rect::draw()\n";
    }

    BoundingBox box() const noexcept
    {
        return BoundingBox{w, h};
    }
};

struct ColorRect : Rect
{
    Color color;

    Color get_color() const noexcept
    {
        return color;
    }

    void set_color(Color new_color)
    {
        color = new_color;
    }
};

// clang-format off
template <typename T>
concept Shape = requires(const T& obj)
{
    { obj.box() } noexcept -> std::same_as<BoundingBox>;
    obj.draw();
};
// clang-format on

template <typename T>
concept ShapeWithColor = Shape<T> && requires(T obj, Color color) {
    { obj.get_color() } noexcept -> std::same_as<Color>;
    obj.set_color(color);
};

static_assert(Shape<Rect>);
static_assert(Shape<ColorRect>);
static_assert(ShapeWithColor<ColorRect>);
static_assert(not ShapeWithColor<Rect>);

template <Shape T>
void render(T& shp)
{
    std::cout << "render<Shape T>\n";
    shp.draw();
}

template <ShapeWithColor T>
void render(T& shp)
{
    std::cout << "render<ShapeWithColor T>\n";
    shp.set_color(Color{255, 255, 255});
    shp.draw();
}

TEST_CASE("concept subsumation")
{
    Rect r{10, 20};
    ColorRect cr{10, 20, {0, 255, 0}};

    render(r);
    render(cr);
}