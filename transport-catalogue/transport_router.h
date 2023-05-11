#ifndef TRANSPORTROUTER_H
#define TRANSPORTROUTER_H

#include <memory>
#include <variant>

#include "router.h"
#include "transport_catalogue.h"

class TransportRouter
{
public:
    struct VertexIds
    {
        graph::VertexId waiting = 0;
        graph::VertexId moving = 0;
    };

    using RouteItem = std::variant<std::monostate, domain::WaitInfo, domain::BusRouteInfo>;

    TransportRouter();

    void setInitSetting(bool value);

    TransportRouter &setWaitTime(int time);

    TransportRouter &setVelocity(int velocity);

    void createGraph(const TransportCatalogue &_catalogue);

    std::optional<std::pair<double, std::vector<RouteItem>>>
    buildRoute(std::string_view _from, std::string_view _to) const;

private:

    bool is_init_ = false;
    double wait_time_ = 0.0;
    double velocity_ = 0.0;

    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;
    std::unordered_map<std::string_view, VertexIds> vertexes_;

    graph::VertexId vertexes_counter_ = 0;

    std::unordered_map<graph::EdgeId, domain::WaitInfo> wait_edges_;

    std::unordered_map<graph::EdgeId, domain::BusRouteInfo> bus_edges_;

    template <typename It>
    void createEdgeBetweenStops(It begin, It end,
                                std::string_view _bus_name,
                                const TransportCatalogue &_catalogue);
};

template<typename It>
void TransportRouter::createEdgeBetweenStops(It begin, It end,
                                             std::string_view _bus_name,
                                             const TransportCatalogue &_catalogue)
{
    for (auto from_it = begin; from_it != std::prev(end); ++from_it)
    {
        double weight = 0.0;
        int span_count = 0;

        for (auto to_it = std::next(from_it); to_it != end; ++to_it)
        {
            std::string_view from_name = *from_it;
            const graph::VertexId from_idx = vertexes_.at(from_name).moving;

            std::string_view to_name = *to_it;
            const graph::VertexId to_idx = vertexes_.at(to_name).waiting;

            weight += _catalogue.
                    getDistancesBetweenStops({*prev(to_it), *(to_it)}).value() / this->velocity_;
            ++span_count;

            auto bus_edge_id = graph_.AddEdge({from_idx, to_idx, weight});

            bus_edges_[bus_edge_id] = {_bus_name, span_count, weight};
        }
    }
}

#endif // TRANSPORTROUTER_H
