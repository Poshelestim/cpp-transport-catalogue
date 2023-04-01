#include "json_reader.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>

#include "json_builder.h"

namespace reader
{

JsonReader::JsonReader(TransportCatalogue &_catalogue,
                       renderer::MapRenderer &map_renderer) :
    catalogue_(_catalogue),
    render_(map_renderer)
{

}

json::Document JsonReader::readJson(std::istream &_input)
{
    return json::Load(_input);
}

domain::Bus parseBus(const json::Dict &_data)
{
    domain::Bus new_bus;
    new_bus.name_ = _data.at("name").AsString();
    new_bus.is_circul_ = _data.at("is_roundtrip").AsBool();
    const auto &stops = _data.at("stops").AsArray();
    new_bus.route_.reserve(stops.size());
    for (const auto &stop : stops)
    {
        new_bus.route_.emplace_back(stop.AsString());
    }
    return new_bus;
}

std::pair<domain::Stop, Distances> parseStop(const json::Dict &_data)
{
    domain::Stop new_stop;
    std::vector<std::pair<std::string_view, double>> distances;

    new_stop.name_ = _data.at("name").AsString();
    new_stop.latitude_ = _data.at("latitude").AsDouble();
    new_stop.longitude_ = _data.at("longitude").AsDouble();
    distances.reserve(_data.at("road_distances").AsDict().size());
    for (const auto &stop : _data.at("road_distances").AsDict())
    {
        distances.emplace_back(stop.first, stop.second.AsDouble());
    }
    return {std::move(new_stop), distances};
}

void JsonReader::parseBaseRequests(const json::Document &_doc)
{
    if (!_doc.GetRoot().IsDict())
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    const auto& query_map = _doc.GetRoot().AsDict();
    if (query_map.count("base_requests") == 0U)
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    for (const auto &query : query_map.at("base_requests").AsArray())
    {
        if (query.AsDict().at("type").AsString() == "Stop")
        {
            auto [new_stop, distances] = parseStop(query.AsDict());
            catalogue_.addStop(std::move(new_stop), distances);
        }
    }

    for (const auto &query : query_map.at("base_requests").AsArray())
    {
        if (query.AsDict().at("type").AsString() == "Bus")
        {
            auto new_bus = parseBus(query.AsDict());
            catalogue_.addBus(std::move(new_bus));
        }
    }
}

std::vector<TypeRequest> JsonReader::parseRequests(const json::Document &_doc)
{
    if (!_doc.GetRoot().IsDict())
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    const auto& query_map = _doc.GetRoot().AsDict();

    std::vector<TypeRequest> queries;
    queries.reserve(query_map.size());

    for (const auto &query : query_map.at("stat_requests").AsArray())
    {
        if (query.AsDict().at("type").AsString() == "Stop")
        {
            queries.emplace_back(TypeRequest{static_cast<uint32_t>(query.AsDict().at("id").AsInt()),
                                             TypeRequest::STOP,
                                             query.AsDict().at("name").AsString()});
            continue;
        }

        if (query.AsDict().at("type").AsString() == "Bus")
        {
            queries.emplace_back(TypeRequest{static_cast<uint32_t>(query.AsDict().at("id").AsInt()),
                                             TypeRequest::BUS,
                                             query.AsDict().at("name").AsString()});
            continue;
        }

        if (query.AsDict().at("type").AsString() == "Map")
        {
            queries.emplace_back(TypeRequest{static_cast<uint32_t>(query.AsDict().at("id").AsInt()),
                                             TypeRequest::MAP,
                                             ""});
            continue;
        }
    }

    return queries;
}

svg::Color parseColor(const json::Node& node)
{
    if (node.IsString())
    {
        return node.AsString();
    }

    if (node.IsArray())
    {
        const auto& color = node.AsArray();

        if (color.size() == 3)
        {
            return svg::Rgb(color.at(0).AsInt(),
                            color.at(1).AsInt(),
                            color.at(2).AsInt());
        }

        if (color.size() == 4)
        {
            return svg::Rgba(
                        color.at(0).AsInt(),
                        color.at(1).AsInt(),
                        color.at(2).AsInt(),
                        color.at(3).AsDouble()
                        );
        }
    }
    throw std::invalid_argument("Can't read color from JSON");
}

void JsonReader::parseRenderSettings(const json::Document &_doc)
{
    if (!_doc.GetRoot().IsDict())
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    if (_doc.GetRoot().AsDict().count("render_settings") == 0U)
    {
        render_.setInitSetting(false);
        return;
        throw std::invalid_argument("Incorrect JSON");
    }

    const auto& settings = _doc.GetRoot().AsDict().at("render_settings").AsDict();

    render_.setWidth(settings.at("width").AsDouble());
    render_.setHeight(settings.at("height").AsDouble());
    render_.setPadding(settings.at("padding").AsDouble());
    render_.setLineWidth(settings.at("line_width").AsDouble());
    render_.setStopRadius(settings.at("stop_radius").AsDouble());
    render_.setBusLabelFontSize(settings.at("bus_label_font_size").AsInt());
    render_.setBusLabelOffset(settings.at("bus_label_offset").AsArray().front().AsDouble(),
                              settings.at("bus_label_offset").AsArray().back().AsDouble());
    render_.setStopLabelFontSize(settings.at("stop_label_font_size").AsInt());
    render_.setStopLabelOffset(settings.at("stop_label_offset").AsArray().front().AsDouble(),
                               settings.at("stop_label_offset").AsArray().back().AsDouble());
    render_.setUnderlayerColor(parseColor(settings.at("underlayer_color")));
    render_.setUnderlayerWidth(settings.at("underlayer_width").AsDouble());
    for (const auto& color : settings.at("color_palette").AsArray())
    {
        render_.appendColorPalette(parseColor(color));
    }
    render_.setInitSetting(true);
}

json::Node JsonReader::writeStopStat(const domain::StopStat &_statisics, uint32_t _id)
{
    using namespace std::literals::string_literals;
    json::Builder builder;

    auto dist = builder.StartDict();

    if (_statisics.is_exist)
    {
        auto array = dist.Key("buses"s).StartArray();

        for (auto bus : _statisics.buses_)
        {
            array.Value(bus.data());
        }

        array.EndArray().Key("request_id"s).Value(static_cast<int>(_id));
    }
    else
    {
        dist.Key("request_id"s).Value(static_cast<int>(_id)).
                Key("error_message"s).Value("not found"s);
    }

    dist.EndDict();

    return builder.Build();
}

json::Node JsonReader::writeBusStat(const domain::BusStat &_statisics, uint32_t _id)
{
    using namespace std::literals::string_literals;

    json::Builder builder;

    auto dist = builder.StartDict();

    if (_statisics.number_stops_ != 0)
    {
        dist.Key("curvature"s).Value(_statisics.curvature_).
                Key("request_id"s).Value(static_cast<int>(_id)).
                Key("route_length"s).Value(_statisics.route_length_).
                Key("stop_count"s).Value(static_cast<int>(_statisics.number_stops_)).
                Key("unique_stop_count"s).Value(static_cast<int>(_statisics.number_unique_stops_));
    }
    else
    {
        dist.Key("request_id"s).Value(static_cast<int>(_id)).
                Key("error_message"s).Value("not found"s);
    }

    dist.EndDict();

    return builder.Build();
}

json::Node JsonReader::writeMap(const svg::Document &_doc, uint32_t _id)
{
    using namespace std::literals::string_literals;
    std::stringstream doc_stream;
    _doc.Render(doc_stream);

    json::Builder builder;

    auto dist = builder.StartDict();

    auto key = dist.Key("map"s);

    std::string formed_text = doc_stream.str();

//    char symbol = '\0';
//    while (doc_stream.get(symbol))
//    {
//        if (symbol == '\n')
//        {
//            formed_text += R"(\n)";
//        }
//        else if (symbol == '\r')
//        {
//            formed_text += R"(\r)";
//        }
//        else if (symbol == '\t')
//        {
//            formed_text += R"(\t)";
//        }
//        else if (symbol == '\\')
//        {
//            formed_text += R"(\\)";
//        }
//        else if (symbol == '"')
//        {
//            formed_text += R"(\")";
//        }
//        else
//        {
//            formed_text += symbol;
//        }
//    }

    key.Value(formed_text);

    dist.Key("request_id"s).Value(static_cast<int>(_id));

    dist.EndDict();

    return builder.Build();
}

} // namespace reader
