#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg
{

struct Point
{
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0.0;
    double y = 0.0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream &out, StrokeLineCap line_cap);

std::ostream& operator<<(std::ostream &out, StrokeLineJoin line_join);

struct Rgb
{
    Rgb() = default;

    Rgb(uint8_t _red, uint8_t _green, uint8_t _blue);

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba
{
    Rgba() = default;

    Rgba(uint8_t _red, uint8_t _green, uint8_t _blue, float _opacity);

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    float opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};

struct ColorPrinter
{
    std::ostream &out;

    void operator()(std::monostate) const;
    void operator()(const std::string& _color) const;
    void operator()(Rgb _color) const;
    void operator()(Rgba _color) const;
};

std::ostream &operator<<(std::ostream& out, const Color color);

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext
{
    RenderContext(std::ostream& out);

    RenderContext(std::ostream& out, int indent_step, int indent = 0);

    RenderContext Indented() const;

    void RenderIndent() const;

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

template <typename Owner>
class PathProps
{
public:

    Owner& SetFillColor(Color color);

    Owner& SetStrokeColor(Color color);

    Owner& SetStrokeWidth(double width);

    Owner& SetStrokeLineCap(StrokeLineCap line_cap);

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream &out) const;

    void RenderAttrs(const RenderContext &context) const;

private:
    Owner& AsOwner();

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

template<typename Owner>
Owner &PathProps<Owner>::SetFillColor(Color color)
{
    fill_color_ = std::move(color);
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeColor(Color color)
{
    stroke_color_ = std::move(color);
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeWidth(double width)
{
    stroke_width_ = width;
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap)
{
    line_cap_ = line_cap;
    return AsOwner();
}

template<typename Owner>
Owner &PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join)
{
    line_join_ = line_join;
    return AsOwner();
}

template<typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream &out) const
{
    using namespace std::literals;

    if (fill_color_)
    {
        out << R"( fill=")" << *fill_color_ << R"(")";
    }

    if (stroke_color_)
    {
        out << R"( stroke=")" << *stroke_color_ << R"(")";
    }

    if (stroke_width_)
    {
        out << R"( stroke-width=")" << *stroke_width_ << R"(")";
    }

    if (line_cap_)
    {
        out << R"( stroke-linecap=")" << *line_cap_ << R"(")";
    }

    if (line_join_)
    {
        out << R"( stroke-linejoin=")" << *line_join_ << R"(")";
    }
}

template<typename Owner>
void PathProps<Owner>::RenderAttrs(const RenderContext &context) const
{
    RenderAttrs(context.out);
}

template<typename Owner>
Owner &PathProps<Owner>::AsOwner()
{
    // static_cast безопасно преобразует *this к Owner&,
    // если класс Owner — наследник PathProps
    return static_cast<Owner&>(*this);
}

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object
{
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle>
{
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final: public Object, public PathProps<Polyline>
{
public:

    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:

    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final: public Object, public PathProps<Text>
{
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:

    void RenderObject(const RenderContext& context) const override;

    Point position_ = {};
    Point offset_ = {};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer
{
public:

    virtual ~ObjectContainer() = default;

    template<typename Obj>
    void Add(Obj obj);

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:

    std::vector<std::shared_ptr<Object>> objects_;
};

class Document final : public ObjectContainer
{
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};

template<typename Obj>
void ObjectContainer::Add(Obj obj)
{
    objects_.emplace_back(std::move(std::make_unique<Obj>(obj)));
}

class Drawable
{
public:
    virtual ~Drawable() = default;

    virtual void Draw(svg::ObjectContainer &container) const = 0;
};

}  // namespace svg

namespace shapes
{

class Star : public svg::Drawable
{
public:

    Star(svg::Point _center, double _outer_radius,
         double _inner_radius, int _num_rays);

    void Draw(svg::ObjectContainer &container) const override;

private:

    svg::Point center_;
    double outer_radius_ = 0.0;
    double inner_radius_ = 0.0;
    int num_rays_ = 0;
};

class Snowman : public svg::Drawable
{
public:

    Snowman(svg::Point _head_center, double _head_radius);

    void Draw(svg::ObjectContainer &container) const override;

private:
    svg::Point head_center_;
    double head_radius_;
};

class Triangle : public svg::Drawable
{
public:

    Triangle(svg::Point _p1, svg::Point _p2, svg::Point _p3);

    void Draw(svg::ObjectContainer &container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

}  // namespace shapes
