#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H
#pragma once

#include <istream>
#include <string>
#include <vector>

#include "json_reader.h"

class RequestHandler
{
public:

    using BusStat = domain::BusStat;
    using StopStat = domain::StopStat;
    using RouteStat = std::optional<std::pair<double, std::vector<TransportRouter::RouteItem>>>;

    RequestHandler(const TransportCatalogue& db,
                   const renderer::MapRenderer& renderer,
                   const TransportRouter& router);

    // Возвращает информацию о маршруте (запрос Bus)
    [[nodiscard]] std::optional<BusStat> GetBusStat(std::string_view bus_name) const;

    [[nodiscard]] StopStat getStopInfo(std::string_view _name) const;

    [[nodiscard]] BusStat getBusInfo(std::string_view _name) const;

    [[nodiscard]] RouteStat getRouteInfo(std::string_view _from, std::string_view _to) const;

    void procRequests(const json::Document &_doc, std::ostream &_output) const;

    [[nodiscard]] svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;
    const TransportRouter& router_;

    svg::Polyline AddRoute(const domain::Bus *bus);

    void createRoutePolylines(svg::Document &_doc,
                              const renderer::SphereProjector &_sp,
                              const std::vector<const domain::Bus *> &_buses) const;

    void createRouteTexts(svg::Document &_doc,
                          const renderer::SphereProjector &_sp,
                          const std::vector<const domain::Bus *> &_buses) const;
};


#endif // REQUESTHANDLER_H
