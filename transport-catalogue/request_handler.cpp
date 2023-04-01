#include "request_handler.h"

#include "json_builder.h"

#include <iomanip>
#include <iostream>
#include <utility>

RequestHandler::RequestHandler(const TransportCatalogue &db,
                               const renderer::MapRenderer& renderer) :
    catalogue_(db),
    renderer_(renderer)
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
        stopInfo.is_exist = true;
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

void RequestHandler::procRequests(const json::Document &_doc, std::ostream &_output) const
{
    const auto queries = reader::JsonReader::parseRequests(_doc);

    json::Builder builder;
    auto array = builder.StartArray();
    for (const auto &query : queries)
    {
        switch (query.type)
        {
        case reader::TypeRequest::STOP :
            array.Value(reader::JsonReader::writeStopStat(getStopInfo(query.name), query.id));
            break;
        case reader::TypeRequest::BUS :
            array.Value(reader::JsonReader::writeBusStat(getBusInfo(query.name), query.id));
            break;
        case reader::TypeRequest::MAP :
            array.Value(reader::JsonReader::writeMap(RenderMap(), query.id));
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

