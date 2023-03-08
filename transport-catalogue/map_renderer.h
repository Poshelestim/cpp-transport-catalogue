#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <queue>

#include "libs\geo.h"
#include "libs\svg.h"

#include "domain.h"

namespace renderer
{

inline const double EPSILON = 1e-6;

bool IsZero(double value);

class SphereProjector
{
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding) :
        padding_(padding)
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end)
        {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_))
        {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat))
        {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom)
        {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom)
        {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom)
        {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const
    {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer
{
public:

    MapRenderer() = default;

    void setWidth(double width);
    void setHeight(double height);
    void setPadding(double padding);
    void setLineWidth(double line_width);
    void setStopRadius(double stop_radius);
    void setBusLabelFontSize(int bus_label_font_size);
    void setBusLabelOffset(double x, double y);
    void setStopLabelFontSize(int stop_label_font_size);
    void setStopLabelOffset(double x, double y);
    void setUnderlayerColor(const svg::Color &color);
    void setUnderlayerWidth(double width);
    void appendColorPalette(const svg::Color& color);
    void setColorPalette(const std::vector<svg::Color> &colors);
    void setColorPalette(std::vector<svg::Color> &&colors) noexcept;
    void setInitSetting(bool value);

    [[nodiscard]] bool getInitSetting() const noexcept;
    double getWidht() const noexcept;
    double getHeight() const noexcept;
    double getPadding() const noexcept;

    [[nodiscard]] svg::Polyline renderPolylineBusRoute(const std::deque<svg::Point> &stops_points,
                                                       size_t index_color) const;

    [[nodiscard]] svg::Text renderTextBusRoute(svg::Point pos,
                                               std::string_view text,
                                               size_t index_color) const;

    [[nodiscard]] svg::Text renderTextUnderlayerBusRoute(svg::Point pos,
                                                         std::string_view text) const;

    [[nodiscard]] svg::Circle renderCircleStop(svg::Point pos) const;

    [[nodiscard]] svg::Text renderTextStop(svg::Point pos, std::string_view text) const;

    [[nodiscard]] svg::Text renderTextUnderlayerStop(svg::Point pos,
                                                     std::string_view text) const;

private:

    bool is_init = false;
    double width_ = 0.0;
    double height_ = 0.0;
    double padding_ = 0.0;
    double line_width_ = 0.0;
    double stop_radius_ = 0.0;
    int bus_label_font_size_ = 0;
    svg::Point bus_label_offset_ = {0.0, 0.0};
    int stop_label_font_size_ = 0;
    svg::Point stop_label_offset_ = {0.0, 0.0};
    svg::Color underlayer_color_;
    double underlayer_width_ = 0.0;
    std::vector<svg::Color> color_palette_;
};

} //namespace renderer

#endif // MAPRENDERER_H
