#include "request_handler.h"

#include "json_builder.h"

#include <iomanip>
#include <iostream>
#include <utility>

RequestHandler::RequestHandler(const TransportCatalogue &db,
                               const renderer::MapRenderer& renderer,
                               const TransportRouter &router) :
    catalogue_(db),
    renderer_(renderer),
    router_(router)
{

}

std::optional<RequestHandler::BusStat> RequestHandler::GetBusStat(std::string_view bus_name) const
{
    return getBusInfo(bus_name);
}

RequestHandler::StopStat RequestHandler::getStopInfo(std::string_view _name) const
{
    StopStat stopInfo;
    domain::Stop *ptr_stop = catalogue_.findStop(_name);
    stopInfo.name_ = _name;
    if (ptr_stop != nullptr)
    {
        stopInfo.buses_ = catalogue_.getNameBuses(_name);
        stopInfo.is_exist_ = true;
    }
    return stopInfo;
}

RequestHandler::BusStat RequestHandler::getBusInfo(std::string_view _name) const
{
    domain::Bus *ptr_bus = catalogue_.findBus(_name);
    BusStat busInfo;
    busInfo.name_ = _name;
    if (ptr_bus != nullptr)
    {
        busInfo.number_stops_ = ptr_bus->is_circul_ ?
                    ptr_bus->route_.size() : ptr_bus->route_.size() * 2 - 1;
        busInfo.number_unique_stops_ = ptr_bus->number_unique_stops_;
        busInfo.route_length_ = ptr_bus->route_length_;
        busInfo.is_circul_ = ptr_bus->is_circul_;
        busInfo.curvature_ = ptr_bus->curvature_;
    }
    return busInfo;
}
RequestHandler::RouteStat RequestHandler::getRouteInfo(std::string_view _from, std::string_view _to) const
{
    const domain::Stop *stop_from = catalogue_.findStop(_from);
    const domain::Stop *next_to = catalogue_.findStop(_to);

    if (stop_from == nullptr || next_to == nullptr)
    {
        throw std::domain_error("createGraph(): findStop returned nullptr");
    }

    std::optional<std::pair<double, std::vector<TransportRouter::RouteItem>>> route =
            router_.buildRoute(stop_from->name_, next_to->name_);

    if (!route.has_value())
    {
        return std::nullopt;
    }

    return route;
}

void RequestHandler::procRequests(const json::Document &_doc, std::ostream &_output) const
{
    using namespace reader;

    const auto queries = JsonReader::parseStatRequests(_doc);

    json::Builder builder;
    auto array = builder.StartArray();
    for (const auto &query : queries)
    {
        switch (query.type)
        {
        case TypeRequest::STOP :
            array.Value(JsonReader::writeStopStat(getStopInfo(query.name), query.id));
            break;
        case TypeRequest::BUS :
            array.Value(JsonReader::writeBusStat(getBusInfo(query.name), query.id));
            break;
        case TypeRequest::MAP :
            array.Value(JsonReader::writeMap(RenderMap(), query.id));
            break;
        case TypeRequest::ROUTE :
            array.Value(JsonReader::writeRoute(getRouteInfo(query.from, query.to), query.id));
            break;
        default:
            break;
        }
    }

    array.EndArray();

    json::Print(json::Document{builder.Build()}, _output);
}

svg::Document RequestHandler::RenderMap() const
{
    return renderer_.render(catalogue_);
}
