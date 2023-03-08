#include "svg.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace svg
{

using namespace std::literals;

std::ostream& operator<<(std::ostream &out, StrokeLineCap line_cap)
{
    switch (line_cap)
    {
    case StrokeLineCap::BUTT:
        out << "butt";
        break;
    case StrokeLineCap::ROUND:
        out << "round";
        break;
    case StrokeLineCap::SQUARE:
        out << "square";
        break;
    default:
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, StrokeLineJoin line_join)
{
    switch (line_join)
    {
    case StrokeLineJoin::ARCS:
        out << "arcs";
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel";
        break;
    case StrokeLineJoin::MITER:
        out << "miter";
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip";
        break;
    case StrokeLineJoin::ROUND:
        out << "round";
        break;
    default:
        break;
    }
    return out;
}

svg::Rgb::Rgb(uint8_t _red, uint8_t _green, uint8_t _blue) :
    red(_red), green(_green), blue(_blue)
{

}

svg::Rgba::Rgba(uint8_t _red, uint8_t _green, uint8_t _blue, float _opacity) :
    red(_red), green(_green), blue(_blue), opacity(_opacity)
{

}

void ColorPrinter::operator()(std::monostate /*unused*/) const
{
    out << NoneColor;
}

void ColorPrinter::operator()(const std::string &_color) const
{
    out << _color;
}

void ColorPrinter::operator()(Rgb _color) const
{
    out << "rgb("sv <<
           std::to_string(_color.red) << ","sv <<
           std::to_string(_color.green) << ","sv <<
           std::to_string(_color.blue) << ")"sv;
}

void ColorPrinter::operator()(Rgba _color) const
{
    out << "rgba("sv <<
           std::to_string(_color.red) << ","sv <<
           std::to_string(_color.green) << ","sv <<
           std::to_string(_color.blue) << ","sv <<
           _color.opacity << ")"sv;
}

std::ostream &operator<<(std::ostream &out, const Color color)
{
    std::visit(ColorPrinter{out}, color);
    return out;
}

RenderContext::RenderContext(std::ostream &out)
    : out(out)
{

}

RenderContext::RenderContext(std::ostream &out, int indent_step, int indent) :
    out(out),
    indent_step(indent_step),
    indent(indent)
{

}

RenderContext RenderContext::Indented() const
{
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const
{
    for (int i = 0; i < indent; ++i)
    {
        out.put(' ');
    }
}

void Object::Render(const RenderContext &context) const
{
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << "\n"sv;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)
{
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)
{
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;
    out << R"(<circle cx=")" << center_.x
        << R"(" cy=")" << center_.y << R"(")"
        << R"( r=")" << radius_ << R"(")";

    RenderAttrs(context);

    out << "/>"sv;
}

Polyline &Polyline::AddPoint(Point point)
{
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;
    bool is_first = true;

    out << R"(<polyline points=")";
    for (const Point &point : points_)
    {
        if (is_first)
        {
            out << point.x << ","sv << point.y;
            is_first = false;
            continue;
        }

        out << " "sv << point.x << ","sv << point.y;

    }

    out << R"(")";

    RenderAttrs(context);

    out << R"(/>)";
}

Text &Text::SetPosition(Point pos)
{
    this->position_ = pos;
    return *this;
}

Text &Text::SetOffset(Point offset)
{
    this->offset_ = offset;
    return *this;
}

Text &Text::SetFontSize(uint32_t size)
{
    this->font_size_ = size;
    return *this;
}

Text &Text::SetFontFamily(std::string font_family)
{
    this->font_family_ = std::move(font_family);
    return *this;
}

Text &Text::SetFontWeight(std::string font_weight)
{
    this->font_weight_ = std::move(font_weight);
    return *this;
}

Text &Text::SetData(std::string data)
{
    this->data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;

    out << "<text"sv;

    RenderAttrs(context);
    out << R"( x=")" << position_.x << R"(")" <<
           R"( y=")" << position_.y << R"(")" <<
           R"( dx=")" << offset_.x << R"(")" <<
           R"( dy=")" << offset_.y << R"(")" <<
           R"( font-size=")" << font_size_ << R"(")";

    if(!font_family_.empty())
    {
        out << R"( font-family=")" << font_family_ << R"(")";
    }
    if(!font_weight_.empty())
    {
        out << R"( font-weight=")" << font_weight_ << R"(")";
    }

    out << ">"sv;

    out << data_;

    out << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object> &&obj)
{
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream &out) const
{
    RenderContext ctx(out, 2, 2);
    out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << "\n"sv;
    out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << "\n"sv;

    for (const auto &obj : objects_)
    {
        obj->Render(ctx);
    }

    out << "</svg>"sv;
}

}  // namespace svg

namespace shapes
{

Star::Star(svg::Point _center, double _outer_radius,
           double _inner_radius, int _num_rays) :
    center_(_center), outer_radius_(_outer_radius),
    inner_radius_(_inner_radius), num_rays_(_num_rays)
{

}

void Star::Draw(svg::ObjectContainer &container) const
{
    svg::Polyline polyline;

    for (int ray_id = 0; ray_id <= num_rays_; ++ray_id)
    {
        double angle = 2.0 * M_PI * (ray_id % num_rays_) / num_rays_;
        polyline.AddPoint({center_.x + outer_radius_ * std::sin(angle),
                           center_.y - outer_radius_ * std::cos(angle)});

        if (ray_id == num_rays_)
        {
            break;
        }

        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_radius_ * std::sin(angle),
                           center_.y - inner_radius_ * std::cos(angle)});
    }
    container.Add(polyline.SetFillColor("red").SetStrokeColor("black"));
}

Snowman::Snowman(svg::Point _head_center, double _head_radius) :
    head_center_(_head_center), head_radius_(_head_radius)
{

}

void Snowman::Draw(svg::ObjectContainer &container) const
{
    // Top circle
    svg::Point current_center{head_center_.x, head_center_.y};
    double current_radius = head_radius_;
    auto top = svg::Circle().SetCenter(current_center).SetRadius(current_radius);

    // Middle circle
    current_center.y += 2. * head_radius_;
    current_radius = 1.5 * head_radius_;
    auto middle = svg::Circle().SetCenter(current_center).SetRadius(current_radius);

    // Bottom circle
    current_center.y += 3. * head_radius_;
    current_radius = 2. * head_radius_;
    auto bottom = svg::Circle().SetCenter(current_center).SetRadius(current_radius);

    for (auto circle : {bottom, middle, top})
    {
        container.Add(std::move(circle.PathProps::SetFillColor("rgb(240,240,240)").SetStrokeColor("black")));
    }
}

Triangle::Triangle(svg::Point _p1, svg::Point _p2, svg::Point _p3) :
    p1_(_p1), p2_(_p2), p3_(_p3)
{

}

void Triangle::Draw(svg::ObjectContainer &container) const
{
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

} // namespace shapes
