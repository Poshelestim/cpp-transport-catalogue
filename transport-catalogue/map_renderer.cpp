#include "map_renderer.h"

#include <stdexcept>

namespace renderer
{

const double LEFT_VALUE_INTERVAL = 0.0;
const double RIGHT_VALUE_INTERVAL = 100000.0;

void MapRenderer::setWidth(double width)
{
    if (width < LEFT_VALUE_INTERVAL || width > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid width value");
    }
    width_ = width;
}

void MapRenderer::setHeight(double height)
{
    if (height < LEFT_VALUE_INTERVAL || height > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid height value");
    }
    height_ = height;
}

void MapRenderer::setPadding(double padding)
{
    if (padding < LEFT_VALUE_INTERVAL || padding > std::min(width_, height_) / 2.0)
    {
        throw std::invalid_argument("invalid padding value");
    }
    padding_ = padding;
}

void MapRenderer::setLineWidth(double line_width)
{
    if (line_width < LEFT_VALUE_INTERVAL || line_width > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid line_width value");
    }
    line_width_ = line_width;
}

void MapRenderer::setStopRadius(double stop_radius)
{
    if (stop_radius < LEFT_VALUE_INTERVAL || stop_radius > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid stop_radius value");
    }
    stop_radius_ = stop_radius;
}

void MapRenderer::setBusLabelFontSize(int bus_label_font_size)
{
    if (bus_label_font_size < LEFT_VALUE_INTERVAL || bus_label_font_size > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid bus_label_font_size value");
    }
    bus_label_font_size_ = bus_label_font_size;
}

void MapRenderer::setBusLabelOffset(double x, double y)
{
    const double LEFT_VALUE_INTERVAL = -100000.0;
    if (x < LEFT_VALUE_INTERVAL || x > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid bus_label_offset_x value");
    }

    if (y < LEFT_VALUE_INTERVAL || y > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid bus_label_offset_y value");
    }

    bus_label_offset_ = {x, y};
}

void MapRenderer::setStopLabelFontSize(int stop_label_font_size)
{
    if (stop_label_font_size < LEFT_VALUE_INTERVAL || stop_label_font_size > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid stop_label_font_size value");
    }
    stop_label_font_size_ = stop_label_font_size;
}

void MapRenderer::setStopLabelOffset(double x, double y)
{
    const double LEFT_VALUE_INTERVAL = -100000.0;
    if (x < LEFT_VALUE_INTERVAL || x > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid stop_label_offset_x value");
    }

    if (y < LEFT_VALUE_INTERVAL || y > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid stop_label_offset_y value");
    }

    stop_label_offset_ = {x, y};
}

void MapRenderer::setUnderlayerColor(const svg::Color &color)
{
    underlayer_color_ = color;
}

void MapRenderer::setUnderlayerWidth(double width)
{
    if (width < LEFT_VALUE_INTERVAL || width > RIGHT_VALUE_INTERVAL)
    {
        throw std::invalid_argument("invalid underlayer_width value");
    }
    underlayer_width_ = width;
}

void MapRenderer::appendColorPalette(const svg::Color& color)
{
    color_palette_.emplace_back(color);
}

void MapRenderer::setColorPalette(const std::vector<svg::Color> &colors)
{
    if (colors.empty())
    {
        throw std::invalid_argument("colors is empty");
    }
    color_palette_ = colors;
}

void MapRenderer::setColorPalette(std::vector<svg::Color> &&colors) noexcept
{
    std::swap(color_palette_, colors);
    colors.clear();
}

void MapRenderer::setInitSetting(bool value)
{
    is_init = value;
}

bool MapRenderer::getInitSetting() const noexcept
{
    return is_init;
}

double MapRenderer::getWidht() const noexcept
{
    return width_;
}

double MapRenderer::getHeight() const noexcept
{
    return height_;
}

double MapRenderer::getPadding() const noexcept
{
    return padding_;
}

svg::Polyline MapRenderer::renderPolylineBusRoute(const std::deque<svg::Point> &stops_points,
                                                  size_t index_color) const
{
    svg::Polyline result;

    for (const auto &stop : stops_points)
    {
        result.AddPoint(stop);
    }

    result.SetStrokeColor(color_palette_.at(index_color % color_palette_.size()))
            .SetStrokeWidth(line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetFillColor(svg::NoneColor);

    return result;
}

svg::Text MapRenderer::renderTextBusRoute(svg::Point pos,
                                          std::string_view text,
                                          size_t index_color) const
{
    svg::Text result;
    result.SetData(text.data())
            .SetPosition(pos)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetOffset({bus_label_offset_})
            .SetFillColor(color_palette_.at(index_color % color_palette_.size()))
            .SetFontSize(static_cast<uint32_t>(bus_label_font_size_));
    return result;
}

svg::Text MapRenderer::renderTextUnderlayerBusRoute(svg::Point pos,
                                                    std::string_view text) const
{
    svg::Text underlayer;
    underlayer.SetFillColor(underlayer_color_)
            .SetStrokeColor(underlayer_color_)
            .SetStrokeWidth(underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetPosition(pos)
            .SetOffset(bus_label_offset_)
            .SetFontSize(static_cast<uint32_t>(bus_label_font_size_))
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(text.data());
    return underlayer;
}

svg::Circle MapRenderer::renderCircleStop(svg::Point pos) const
{
    svg::Circle circle;
    circle.SetCenter(pos)
            .SetRadius(stop_radius_)
            .SetFillColor("white");
    return circle;
}

svg::Text MapRenderer::renderTextStop(svg::Point pos, std::string_view text) const
{
    svg::Text result;
    result.SetData(text.data())
            .SetPosition(pos)
            .SetFontFamily("Verdana")
            .SetOffset(stop_label_offset_)
            .SetFillColor("black")
            .SetFontSize(static_cast<uint32_t>(stop_label_font_size_));
    return result;
}

svg::Text MapRenderer::renderTextUnderlayerStop(svg::Point pos, std::string_view text) const
{
    svg::Text underlayer;
    underlayer.SetFillColor(underlayer_color_)
            .SetStrokeColor(underlayer_color_)
            .SetStrokeWidth(underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetPosition(pos)
            .SetOffset(stop_label_offset_)
            .SetFontSize(static_cast<uint32_t>(stop_label_font_size_))
            .SetFontFamily("Verdana")
            .SetData(text.data());
    return underlayer;
}

bool IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

} //namespace renderer
