#ifndef JSONREADER_H
#define JSONREADER_H
#pragma once
#include <iostream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "libs/json.h"

namespace domain
{
struct Bus;
struct Stop;
}

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
    };

    uint32_t id;
    TypeStatRequest type;
    std::string_view name;
};

json::Document readJson(std::istream &_input);

void parseBaseRequests(const json::Document &_doc,
                       TransportCatalogue &_catalogue);

std::vector<TypeRequest> parseRequests(const json::Document &_doc);

void writeStopStat(const domain::StopStat &_stat, uint32_t _id, std::ostream &_output);

void writeBusStat(const domain::BusStat &_stat, uint32_t _id, std::ostream &_output);

void parseRenderSettings(const json::Document &_doc, renderer::MapRenderer &render);

void writeMap(const svg::Document &_doc, uint32_t _id, std::ostream &_output);

} //namespace reader

#endif // JSONREADER_H
