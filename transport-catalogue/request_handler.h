#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H
#pragma once

#include <istream>
#include <optional>
#include <string>
#include <vector>

#include "json_reader.h"

class RequestHandler
{
public:

    using BusStat = domain::BusStat;
    using StopStat = domain::StopStat;

    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(std::string_view bus_name) const;


    // Возвращает маршруты, проходящие через
    //    const std::set<StopStat>* GetBusesByStop(const std::string_view& stop_name);

    StopStat getStopInfo(std::string_view _name) const;

    BusStat getBusInfo(std::string_view _name) const;

    void procRequests(const json::Document &_doc, std::ostream &_output) const;

    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;

    svg::Polyline AddRoute(const domain::Bus *bus);

    void createRoutePolylines(svg::Document &_doc,
                              const renderer::SphereProjector &_sp,
                              const std::vector<const domain::Bus *> &_buses) const;

    void createRouteTexts(svg::Document &_doc,
                          const renderer::SphereProjector &_sp,
                          const std::vector<const domain::Bus *> &_buses) const;
};


#endif // REQUESTHANDLER_H
