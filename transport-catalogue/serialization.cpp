#include "serialization.h"

#include <fstream>
#include <utility>

namespace serialization
{

Serialization::Serialization(std::filesystem::path path) :
    path_(std::move(path))
{

}

bool Serialization::Serialize(const TransportCatalogue &catalogue,
                              const renderer::MapRenderer &render,
                              const TransportRouter &router)
{
    std::ofstream ofs(path_, std::ios::binary);
    if (!ofs.is_open())
    {
        return false;
    }

    AddStopsInProto(catalogue);
    AddRoutesInProto(catalogue);

    AddRenderSettingsInProto(render);

    AddTransportRouterInProto(router);

    proto_catalogue_.SerializeToOstream(&ofs);
    return true;
}

bool Serialization::Deserialize(TransportCatalogue &catalogue,
                                renderer::MapRenderer &render,
                                TransportRouter &router)
{
    std::ifstream ifs(path_, std::ios::binary);
    if (!ifs.is_open() || !proto_catalogue_.ParseFromIstream(&ifs))
    {
        return false;
    }

    ParseStopsFromProto(catalogue);
    ParseRoutesFromProto(catalogue);

    ParseRenderSettingsFromProto(render);

    ParseTransportRouterFromProto(router);
    return true;
}

void Serialization::AddStopsInProto(const TransportCatalogue &catalogue)
{
    const auto &stops = catalogue.getAllStops();
    for (const auto &stop : stops)
    {
        proto_transport_catalogue::Stop proto_stop;
        proto_stop.set_id(stop.id_);
        proto_stop.set_name(stop.name_.data());
        proto_stop.set_latitude(stop.latitude_);
        proto_stop.set_longitude(stop.longitude_);
        *proto_catalogue_.mutable_catalogue()->add_stops() = std::move(proto_stop);

        stop_id_by_name_.insert({stop.name_, stop.id_});
        stop_name_by_id_.insert({stop.id_, stop.name_});
    }

    AddDistancesInProto(catalogue);
}

void Serialization::ParseStopsFromProto(TransportCatalogue &catalogue)
{
    const auto &proto_stops = proto_catalogue_.catalogue().stops();
    for (auto it = proto_stops.cbegin(); it != proto_stops.cend(); ++it)
    {
        domain::Stop new_stop;

        new_stop.name_ = it->name();
        new_stop.latitude_ = it->latitude();
        new_stop.longitude_ = it->longitude();

        catalogue.addStop(std::move(new_stop), {});

        stop_name_by_id_.insert({it->id(), it->name()});
        stop_id_by_name_.insert({it->name(), it->id()});
    }

    ParseDistancesFromProto(catalogue);
}

void Serialization::AddRoutesInProto(const TransportCatalogue &catalogue)
{
    const auto &routes = catalogue.getAllBuses();
    for (const auto &route : routes)
    {
        proto_transport_catalogue::Route proto_route;
        proto_route.set_name(route.name_.data());
        proto_route.set_is_circul(route.is_circul_);

        for (const auto &stop : route.route_)
        {
            proto_route.add_stop_ids(catalogue.findStop(stop)->id_);
        }
        *proto_catalogue_.mutable_catalogue()->add_routes() = std::move(proto_route);
    }
}

void Serialization::ParseRoutesFromProto(TransportCatalogue &catalogue)
{
    const auto &proto_routes = proto_catalogue_.catalogue().routes();
    for (auto it = proto_routes.cbegin(); it != proto_routes.cend(); ++it)
    {
        domain::Bus new_bus;
        new_bus.name_ = it->name();
        new_bus.is_circul_ = it->is_circul();
        const auto &stops = it->stop_ids();
        new_bus.route_.reserve(stops.size());
        for (const auto &stop : stops)
        {
            new_bus.route_.emplace_back(stop_name_by_id_.at(stop));
        }

        catalogue.addBus(std::move(new_bus));
    }
}

void Serialization::AddDistancesInProto(const TransportCatalogue &catalogue)
{
    const auto &distances = catalogue.getAllDistances();
    for (const auto &[stops, distance] : distances)
    {
        proto_transport_catalogue::Distance proto_distance;
        proto_distance.set_stop_id_from(stop_id_by_name_.at(stops.first));
        proto_distance.set_stop_id_to(stop_id_by_name_.at(stops.second));
        proto_distance.set_distance(distance);
        *proto_catalogue_.mutable_catalogue()->add_distances() = std::move(proto_distance);
    }
}

void Serialization::ParseDistancesFromProto(TransportCatalogue &catalogue) const
{
    auto proto_distances = proto_catalogue_.catalogue().distances();
    for (auto it = proto_distances.cbegin(); it != proto_distances.cend(); ++it)
    {
        catalogue.appendDistancesBetweenStops({stop_name_by_id_.at(it->stop_id_from()),
                                               stop_name_by_id_.at(it->stop_id_to())},
                                              it->distance());
    }
}

proto_svg::Point makeProtoPoint(const svg::Point &point)
{
    proto_svg::Point result;
    result.set_x(point.x);
    result.set_y(point.y);
    return result;
}

proto_svg::Color makeProtoColor(const svg::Color &color)
{
    proto_svg::Color result;
    if (std::holds_alternative<std::string>(color))
    {
        result.set_string_color(std::get<std::string>(color));
    }
    else if (std::holds_alternative<svg::Rgb>(color))
    {
        const auto &rgb = std::get<svg::Rgb>(color);
        auto *p_color = result.mutable_rgb_color();
        p_color->set_r(rgb.red);
        p_color->set_g(rgb.green);
        p_color->set_b(rgb.blue);
    }
    else if (std::holds_alternative<svg::Rgba>(color))
    {
        const auto &rgba = std::get<svg::Rgba>(color);
        auto *p_color = result.mutable_rgba_color();
        p_color->set_r(rgba.red);
        p_color->set_g(rgba.green);
        p_color->set_b(rgba.blue);
        p_color->set_o(rgba.opacity);
    }
    return result;
}

void Serialization::AddRenderSettingsInProto(const renderer::MapRenderer &map_renderer)
{
    auto *proto_settings = proto_catalogue_.mutable_render_settings();
    const auto &render_settings = map_renderer.getSettings();

    proto_settings->set_height(render_settings.height_);
    proto_settings->set_width(render_settings.width_);

    proto_settings->set_padding(render_settings.padding_);

    proto_settings->set_line_width(render_settings.line_width_);
    proto_settings->set_stop_radius(render_settings.stop_radius_);

    proto_settings->set_bus_label_font_size(render_settings.bus_label_font_size_);
    *proto_settings->mutable_bus_label_offset() =
            makeProtoPoint(render_settings.bus_label_offset_);

    proto_settings->set_stop_label_font_size(render_settings.stop_label_font_size_);
    *proto_settings->mutable_stop_label_offset() =
            makeProtoPoint(render_settings.stop_label_offset_);

    *proto_settings->mutable_underlayer_color() =
            makeProtoColor(render_settings.underlayer_color_);
    proto_settings->set_underlayer_width(render_settings.underlayer_width_);

    for (const auto &color : render_settings.color_palette_)
    {
        *proto_settings->add_color_palette() = makeProtoColor(color);
    }
}

svg::Color makeSvgColor(const proto_svg::Color &color)
{
    svg::Color result;
    switch (color.color_case())
    {
    case proto_svg::Color::kStringColor:
    {
        result = color.string_color();
    } break;
    case proto_svg::Color::kRgbColor :
    {
        const auto &p_rgb = color.rgb_color();
        result = svg::Rgb(p_rgb.r(), p_rgb.g(), p_rgb.b());
    } break;
    case proto_svg::Color::kRgbaColor :
    {
        const auto &p_rgba = color.rgba_color();
        result = svg::Rgba(p_rgba.r(), p_rgba.g(), p_rgba.b(), p_rgba.o());
    } break;
    default:
    {
        result = svg::NoneColor;
    } break;
    }
    return result;
}

void Serialization::ParseRenderSettingsFromProto(renderer::MapRenderer &map_renderer) const
{
    if (!proto_catalogue_.has_render_settings())
    {
        return;
    }

    const auto &proto_settings = proto_catalogue_.render_settings();

    map_renderer.setWidth(proto_settings.width());
    map_renderer.setHeight(proto_settings.height());
    map_renderer.setPadding(proto_settings.padding());
    map_renderer.setLineWidth(proto_settings.line_width());
    map_renderer.setStopRadius(proto_settings.stop_radius());
    map_renderer.setBusLabelFontSize(proto_settings.bus_label_font_size());
    map_renderer.setBusLabelOffset(proto_settings.bus_label_offset().x(),
                                   proto_settings.bus_label_offset().y());
    map_renderer.setStopLabelFontSize(proto_settings.stop_label_font_size());
    map_renderer.setStopLabelOffset(proto_settings.stop_label_offset().x(),
                                    proto_settings.stop_label_offset().y());
    map_renderer.setUnderlayerColor(makeSvgColor(proto_settings.underlayer_color()));
    map_renderer.setUnderlayerWidth(proto_settings.underlayer_width());
    for (const auto &color : proto_settings.color_palette())
    {
        map_renderer.appendColorPalette(makeSvgColor(color));
    }
    map_renderer.setInitSetting(true);
}

void Serialization::AddTransportRouterSettingsInProto(const TransportRouter &router)
{
    auto proto_settings = proto_catalogue_.mutable_router()->mutable_settings();

    auto [wait_time, velocity] = router.getSettings();

    proto_settings->set_wait_time(wait_time);
    proto_settings->set_velocity(velocity);
}

void Serialization::ParseTransportRouterSettingsFromProto(TransportRouter &router) const
{
    const auto &proto_settings = proto_catalogue_.router().settings();

    router.setWaitTime(proto_settings.wait_time()).
            setVelocity(proto_settings.velocity()).
            setInitSetting(true);
}

void Serialization::AddTransportRouterInProto(const TransportRouter &router)
{
    AddTransportRouterSettingsInProto(router);
    AddGraphInProto(router);
    AddInternalRouterInProto(router);

    auto *proto_vertexes = proto_catalogue_.mutable_router()->mutable_vertexes();
    for (const auto &[stop_name, id_vertex] : router.getVertexes())
    {
        proto_transport_router::VertexIds proto;
        proto.set_waiting(id_vertex.waiting);
        proto.set_moving(id_vertex.moving);
        (*proto_vertexes)[stop_id_by_name_.at(stop_name)] = std::move(proto);
    }

    auto *proto_wait_edges = proto_catalogue_.mutable_router()->mutable_wait_edges();
    for (const auto &[id_edge, wait_info] : router.getWaitEdges())
    {
        proto_transport_router::WaitInfo proto;
        proto.set_stop_id(stop_id_by_name_.at(wait_info.name));
        proto.set_time(wait_info.time);
        (*proto_wait_edges)[id_edge] = std::move(proto);
    }

    auto *proto_bus_edges = proto_catalogue_.mutable_router()->mutable_bus_edges();
    for (const auto &[id_edge, route_info] : router.getBusEdges())
    {
        proto_transport_router::BusRouteInfo proto;
        proto.set_name(route_info.name.data());
        proto.set_span_count(route_info.span_count);
        proto.set_time(route_info.time);
        (*proto_bus_edges)[id_edge] = std::move(proto);
    }
}

void Serialization::ParseTransportRouterFromProto(TransportRouter &router)
{
    ParseTransportRouterSettingsFromProto(router);
    ParseGraphFromProto(router);

    const auto &proto_vertexes = proto_catalogue_.router().vertexes();
    auto &router_vertexes = router.getVertexes();
    for (const auto &[id_stop, id_vertex] : proto_vertexes)
    {
        router_vertexes[stop_name_by_id_.at(id_stop)].moving = id_vertex.moving();
        router_vertexes[stop_name_by_id_.at(id_stop)].waiting = id_vertex.waiting();
    }

    const auto &proto_wait_edges = proto_catalogue_.router().wait_edges();
    auto &router_wait_edges = router.getWaitEdges();
    for (const auto &[id_edge, wait_info] : proto_wait_edges)
    {
        router_wait_edges[id_edge].name = stop_name_by_id_.at(wait_info.stop_id());
        router_wait_edges[id_edge].time = wait_info.time();
    }

    const auto &proto_bus_edges = proto_catalogue_.router().bus_edges();
    auto &router_bus_edges = router.getBusEdges();
    for (const auto &[id_edge, route_info] : proto_bus_edges)
    {
        router_bus_edges[id_edge].name = route_info.name();
        router_bus_edges[id_edge].span_count = route_info.span_count();
        router_bus_edges[id_edge].time = route_info.time();
    }
}

void Serialization::AddGraphInProto(const TransportRouter &router)
{
    auto *p_graph = proto_catalogue_.mutable_router()->mutable_graph();
    const auto &graph = router.getGraph();

    for (size_t edge_id = 0; edge_id < graph.GetEdgeCount(); ++edge_id)
    {
        proto_graph::Edge p_edge;
        p_edge.set_from(graph.GetEdge(edge_id).from);
        p_edge.set_to(graph.GetEdge(edge_id).to);
        p_edge.set_weight(graph.GetEdge(edge_id).weight);
        *p_graph->add_edges() = std::move(p_edge);
    }

    p_graph->set_vertex_count(graph.GetVertexCount());
}

void Serialization::ParseGraphFromProto(TransportRouter &router)
{
    const auto &p_graph = proto_catalogue_.router().graph();
    auto &graph = router.getGraph();

    {
        graph::DirectedWeightedGraph<double> buf(p_graph.vertex_count());
        std::swap(buf, graph);
    }

    for (auto edge_id = 0; edge_id < p_graph.edges_size(); ++edge_id)
    {
        const auto &p_edge = p_graph.edges(edge_id);
        graph.AddEdge({p_edge.from(), p_edge.to(), p_edge.weight()});
    }

    ParseInternalRouterFromProto(router);
}

void Serialization::AddInternalRouterInProto(const TransportRouter &router)
{
    auto *proto_router = proto_catalogue_.mutable_router()->mutable_router();

    for (const auto &data : router.getInternalRouter()->GetRoutesInternalData())
    {
        proto_graph::RoutesInternalData proto_data;
        for (const auto &internal : data)
        {
            proto_graph::OptionalRouteInternalData proto_internal;
            if (internal.has_value())
            {
                const auto &value = internal.value();
                auto *p_value = proto_internal.mutable_route_internal_data();
                p_value->set_weight(value.weight);
                if (value.prev_edge.has_value())
                {
                    p_value->set_prev_edge(value.prev_edge.value());
                }
            }
            *proto_data.add_routes_internal_data() = std::move(proto_internal);
        }
        *proto_router->add_routes_internal_data() = std::move(proto_data);
    }
}

void Serialization::ParseInternalRouterFromProto(TransportRouter &router)
{
    const auto &proto_router = proto_catalogue_.router().router();

    router.setRouterWithNewGraph();

    auto &routes_internal_data = router.getInternalRouter()->GetRoutesInternalData();

    for (int index = 0; index < proto_router.routes_internal_data_size(); ++index)
    {
        const auto &proto_internal_data = proto_router.routes_internal_data(index);
        auto internal_data_count = proto_internal_data.routes_internal_data_size();
        for (int ind = 0; ind < internal_data_count; ++ind)
        {
            const auto &proto_optional_data = proto_internal_data.routes_internal_data(ind);
            if ( proto_optional_data.optional_route_internal_data_case() ==
                 proto_graph::OptionalRouteInternalData::kRouteInternalData )
            {
                graph::Router<double>::RouteInternalData data;
                const auto &proto_data = proto_optional_data.route_internal_data();
                data.weight = proto_data.weight();
                if ( proto_data.optional_prev_edge_case() ==
                     proto_graph::RouteInternalData::kPrevEdge )
                {
                    data.prev_edge = proto_data.prev_edge();
                }
                else
                {
                    data.prev_edge = std::nullopt;
                }
                routes_internal_data[index][ind] = data;
            }
            else
            {
                routes_internal_data[index][ind] = std::nullopt;
            }
        }
    }
}

} // namespace serialization
