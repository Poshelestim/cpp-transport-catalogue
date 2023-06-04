#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <filesystem>

#include <transport_catalogue.pb.h>

#include "map_renderer.h"
#include "transport_router.h"

namespace serialization
{

class Serialization final
{
public:
    using ProtoTransportCatalogue = proto_transport_catalogue::TransportCatalogue;

    Serialization(std::filesystem::path path);

    bool Serialize(const TransportCatalogue &catalogue,
                   const renderer::MapRenderer &render,
                   const TransportRouter &router);

    bool Deserialize(TransportCatalogue &catalogue,
                     renderer::MapRenderer &render,
                     TransportRouter &router);

private:
    void AddStopsInProto(const TransportCatalogue &catalogue);
    void ParseStopsFromProto(TransportCatalogue &catalogue);

    void AddRoutesInProto(const TransportCatalogue &catalogue);
    void ParseRoutesFromProto(TransportCatalogue &catalogue);

    void AddDistancesInProto(const TransportCatalogue &catalogue);
    void ParseDistancesFromProto(TransportCatalogue &catalogue) const;

    void AddRenderSettingsInProto(const renderer::MapRenderer &map_renderer);
    void ParseRenderSettingsFromProto(renderer::MapRenderer &map_renderer) const;

    void AddTransportRouterSettingsInProto(const TransportRouter &router);
    void ParseTransportRouterSettingsFromProto(TransportRouter &router) const;

    void AddTransportRouterInProto(const TransportRouter &router);
    void ParseTransportRouterFromProto(TransportRouter &router);

    void AddGraphInProto(const TransportRouter &router);
    void ParseGraphFromProto(TransportRouter &router);

    void AddInternalRouterInProto(const TransportRouter &router);
    void ParseInternalRouterFromProto(TransportRouter &router);

    std::filesystem::path path_;

    ProtoTransportCatalogue proto_catalogue_;

    std::unordered_map<size_t, std::string_view> stop_name_by_id_;
    std::unordered_map<std::string_view, size_t> stop_id_by_name_;
};


} // namespace serialization

#endif // SERIALIZATION_H
