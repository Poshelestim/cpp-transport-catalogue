#include "transport_router.h"

#include <memory>

TransportRouter::TransportRouter() :
    graph_(0)
{

}

void TransportRouter::setInitSetting(bool value)
{
    this->is_init_ = value;
}

TransportRouter &TransportRouter::setWaitTime(int time)
{
    this->wait_time_ = static_cast<double>(time);
    return *this;
}

TransportRouter &TransportRouter::setVelocity(int velocity)
{
    static const double km_per_hour_to_m_per_min = 1000.0 / 60.0;
    this->velocity_ = velocity * km_per_hour_to_m_per_min;
    return *this;
}

void TransportRouter::createGraph(const TransportCatalogue &_catalogue)
{
    if (!is_init_)
    {
        return;
    }

    const std::vector<const domain::Stop *> sorted_used_stops =
            _catalogue.getSortedUsedStops();

    {
        graph::DirectedWeightedGraph<double> buf(sorted_used_stops.size() * 2);
        std::swap(buf, this->graph_);
    }

    for (const auto *stop : sorted_used_stops)
    {
        vertexes_[stop->name_].waiting = vertexes_counter_++;
        vertexes_[stop->name_].moving = vertexes_counter_++;
        const graph::EdgeId idx =
                graph_.AddEdge({vertexes_.at(stop->name_).waiting,
                                vertexes_.at(stop->name_).moving,
                                wait_time_});
        wait_edges_.insert({idx, {stop->name_, wait_time_} });
    }

    const std::vector<const domain::Bus *> buses = _catalogue.getSortedBuses();

    for (const auto *bus : buses)
    {
        createEdgeBetweenStops(bus->route_.begin(), bus->route_.end(),
                               bus->name_, _catalogue);
        if (!bus->is_circul_)
        {
            createEdgeBetweenStops(bus->route_.rbegin(), bus->route_.rend(),
                                   bus->name_, _catalogue);
        }
    }

    router_ = std::make_unique<graph::Router<double>>(graph::Router<double>(this->graph_));
}

std::optional<std::pair<double, std::vector<TransportRouter::RouteItem> > >
TransportRouter::buildRoute(std::string_view _from, std::string_view _to) const
{
    if (vertexes_.count(_from) == 0U || vertexes_.count(_to) == 0U)
    {
        return {};
    }

    const graph::VertexId from_vertex = vertexes_.at(_from).waiting;
    const graph::VertexId to_vertex = vertexes_.at(_to).waiting;

    std::optional<graph::Router<double>::RouteInfo> route_info =
            router_->BuildRoute(from_vertex, to_vertex);

    if (!route_info.has_value())
    {
        return {};
    }

    std::pair<double, std::vector<RouteItem>> output;
    output.first = route_info->weight;

    auto& items = output.second;

    for (const auto& edge_id : route_info->edges)
    {
        if (bus_edges_.count(edge_id) > 0)
        {
            items.emplace_back(bus_edges_.at(edge_id));
            continue;
        }

        if (wait_edges_.count(edge_id) > 0)
        {
            items.emplace_back(wait_edges_.at(edge_id));
            continue;
        }
    }

    return output;
}

