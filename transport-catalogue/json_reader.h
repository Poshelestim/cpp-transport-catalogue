#ifndef JSONREADER_H
#define JSONREADER_H
#pragma once
#include <iostream>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "libs/json.h"

namespace domain
{
struct Bus;
struct Stop;
} // namespace domain

namespace reader
{
using Distances = std::vector<std::pair<std::string_view, double> >;
//using RenderSettings = std::unordered_map<std::string_view,

struct TypeRequest
{
    enum TypeStatRequest
    {
        BUS = 0,
        STOP,
        MAP,
        ROUTE,
    };

    uint32_t id;
    TypeStatRequest type;
    std::string_view name;
    std::string_view from;
    std::string_view to;
};

class JsonReader
{
public:

    using RouteStat = std::optional<std::pair<double, std::vector<TransportRouter::RouteItem>>>;

    JsonReader(TransportCatalogue &_catalogue,
               renderer::MapRenderer &_map_renderer,
               TransportRouter &_router);

    ~JsonReader() = default;

    JsonReader(const JsonReader& _other) = delete;

    static json::Document readJson(std::istream &_input);

    void parseBaseRequests(const json::Document &_doc);

    static std::vector<TypeRequest> parseStatRequests(const json::Document &_doc);

    void parseRenderSettings(const json::Document &_doc);

    void parseRoutingSettings(const json::Document &_doc);

    static json::Node writeStopStat(const domain::StopStat &_statisics, uint32_t _id);

    static json::Node writeBusStat(const domain::BusStat &_statisics, uint32_t _id);

    static json::Node writeMap(const svg::Document &_doc, uint32_t _id);

    static json::Node writeRoute(const RouteStat &_statisics, uint32_t _id);

private:
    TransportCatalogue &catalogue_;
    renderer::MapRenderer &render_;
    TransportRouter &router_;
};

} //namespace reader

#endif // JSONREADER_H
