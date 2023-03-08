#include "request_handler.h"

#include <iomanip>
#include <iostream>
#include <utility>

RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer& renderer) :
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
    const auto queries = reader::parseRequests(_doc);
    _output << R"([)" << std::endl;
    for (auto query = queries.begin(); query != queries.end(); ++query)
    {
        switch (query->type)
        {
        case reader::TypeRequest::STOP :
            reader::writeStopStat(getStopInfo(query->name), query->id, _output);
            break;
        case reader::TypeRequest::BUS :
            reader::writeBusStat(getBusInfo(query->name), query->id, _output);
            break;
        case reader::TypeRequest::MAP :
            reader::writeMap(RenderMap(), query->id, _output);
            break;
        default:
            break;
        }

        if (query != queries.end() - 1)
        {
            _output << ',' << std::endl;
        }
    }
    _output << std::endl << R"(])" << std::endl;

}

svg::Document RequestHandler::RenderMap() const
{
    svg::Document doc;
    if (!renderer_.getInitSetting())
    {
        return doc;
    }

    std::deque<geo::Coordinates> geo_points;

    const auto stops = catalogue_.getSortedUsedStops();

    for (const auto *stop: stops)
    {
        geo_points.push_back({ stop->latitude_, stop->longitude_ });
    }

    const renderer::SphereProjector sp(geo_points.begin(),
                                       geo_points.end(),
                                       renderer_.getWidht(),
                                       renderer_.getHeight(),
                                       renderer_.getPadding());

    ///отрисовка маршрутов
    {
        const auto buses = catalogue_.getSortedBuses();
        createRoutePolylines(doc, sp, buses);
        createRouteTexts(doc, sp, buses);
    }
    ///отрисовка остановок
    {
        ///отрисовка кругов остановок
        {
            for (const auto *stop : stops)
            {
                doc.Add(renderer_.renderCircleStop(sp({ stop->latitude_,
                                                        stop->longitude_ })));
            }
        }
        ///отрисовка названий остановок
        {
            for (const auto *stop : stops)
            {
                doc.Add(renderer_.renderTextUnderlayerStop(sp({ stop->latitude_,
                                                                stop->longitude_ }),
                                                           stop->name_));
                doc.Add(renderer_.renderTextStop(sp({ stop->latitude_,
                                                      stop->longitude_ }),
                                                 stop->name_));
            }
        }
    }

    return doc;
}

void RequestHandler::createRoutePolylines(svg::Document &_doc,
                                          const renderer::SphereProjector &_sp,
                                          const std::vector<const domain::Bus *> &_buses) const
{
    size_t COUNTER_BUSES = 0;
    for (const auto *bus_ptr : _buses)
    {
        std::deque<svg::Point> stops_points;

        for (const auto &stop : bus_ptr->route_)
        {
            const auto *ptr = catalogue_.findStop(stop);
            stops_points.push_back(_sp({ ptr->latitude_, ptr->longitude_ }));
        }

        if (!bus_ptr->is_circul_)
        {
            auto it = bus_ptr->route_.rbegin() + 1;

            while (it != bus_ptr->route_.rend())
            {
                const auto *ptr = catalogue_.findStop(*it);
                stops_points.push_back(_sp({ ptr->latitude_, ptr->longitude_ }));
                ++it;
            }
        }

        _doc.Add(renderer_.renderPolylineBusRoute(stops_points, COUNTER_BUSES));
        ++COUNTER_BUSES;
    }
}

void RequestHandler::createRouteTexts(svg::Document &_doc,
                                      const renderer::SphereProjector &_sp,
                                      const std::vector<const domain::Bus *> &_buses) const
{
    size_t COUNTER_BUSES = 0;
    {
        for (const auto *bus_ptr : _buses)
        {
            const auto *stop_begin = catalogue_.findStop(*bus_ptr->route_.begin());
            _doc.Add(renderer_.renderTextUnderlayerBusRoute(
                         _sp({ stop_begin->latitude_, stop_begin->longitude_ }),
                         bus_ptr->name_));
            _doc.Add(renderer_.renderTextBusRoute(_sp({ stop_begin->latitude_,
                                                        stop_begin->longitude_ }),
                                                  bus_ptr->name_, COUNTER_BUSES));
            if (!bus_ptr->is_circul_)
            {
                const auto *stop_end = catalogue_.findStop(*(bus_ptr->route_.end() - 1));
                if (stop_end->name_ != stop_begin->name_)
                {
                    _doc.Add(renderer_.renderTextUnderlayerBusRoute(
                                 _sp({ stop_end->latitude_, stop_end->longitude_ }),
                                 bus_ptr->name_));
                    _doc.Add(renderer_.renderTextBusRoute(_sp({ stop_end->latitude_,
                                                                stop_end->longitude_ }),
                                                          bus_ptr->name_, COUNTER_BUSES));
                }
            }
            ++COUNTER_BUSES;
        }
    }
}
