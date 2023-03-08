#include "json_reader.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>

namespace reader
{

inline const std::string offset(4, ' ');
inline const std::string double_offset(8, ' ');

json::Document readJson(std::istream &_input)
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
    distances.reserve(_data.at("road_distances").AsMap().size());
    for (const auto &stop : _data.at("road_distances").AsMap())
    {
        distances.emplace_back(stop.first, stop.second.AsDouble());
    }
    return {std::move(new_stop), distances};
}

void parseBaseRequests(const json::Document &_doc,
                       TransportCatalogue &_catalogue)
{
    if (!_doc.GetRoot().IsMap())
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    const auto& query_map = _doc.GetRoot().AsMap();
    if (query_map.count("base_requests") == 0U)
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    for (const auto &query : query_map.at("base_requests").AsArray())
    {
        if (query.AsMap().at("type").AsString() == "Stop")
        {
            auto [new_stop, distances] = reader::parseStop(query.AsMap());
            _catalogue.addStop(std::move(new_stop), distances);
        }
    }

    for (const auto &query : query_map.at("base_requests").AsArray())
    {
        if (query.AsMap().at("type").AsString() == "Bus")
        {
            auto new_bus = reader::parseBus(query.AsMap());
            _catalogue.addBus(std::move(new_bus));
        }
    }
}

std::vector<TypeRequest> parseRequests(const json::Document &_doc)
{
    if (!_doc.GetRoot().IsMap())
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    const auto& query_map = _doc.GetRoot().AsMap();
    //    if (query_map.count("stat_requests") == 0U)
    //    {
    //        throw std::invalid_argument("Incorrect JSON");
    //    }

    std::vector<TypeRequest> queries;
    queries.reserve(query_map.size());

    for (const auto &query : query_map.at("stat_requests").AsArray())
    {
        if (query.AsMap().at("type").AsString() == "Stop")
        {
            queries.emplace_back(TypeRequest{static_cast<uint32_t>(query.AsMap().at("id").AsInt()),
                                             TypeRequest::STOP,
                                             query.AsMap().at("name").AsString()});
            continue;
        }

        if (query.AsMap().at("type").AsString() == "Bus")
        {
            queries.emplace_back(TypeRequest{static_cast<uint32_t>(query.AsMap().at("id").AsInt()),
                                             TypeRequest::BUS,
                                             query.AsMap().at("name").AsString()});
            continue;
        }

        if (query.AsMap().at("type").AsString() == "Map")
        {
            queries.emplace_back(TypeRequest{static_cast<uint32_t>(query.AsMap().at("id").AsInt()),
                                             TypeRequest::MAP,
                                             ""});
            continue;
        }
    }

    return queries;
}

void writeStopStat(const domain::StopStat &_stat, uint32_t _id, std::ostream &_output)
{
    using namespace std::literals::string_literals;

    _output << offset << "{" << std::endl;

    if (_stat.is_exist)
    {
        if (!_stat.buses_.empty())
        {
            _output << double_offset << R"("buses": [)" << std::endl;
            size_t current_index = 0;
            for (auto bus : _stat.buses_)
            {
                ++current_index;
                _output << double_offset << ' ' << R"(")" << bus << R"(")";
                if (current_index < _stat.buses_.size())
                {
                    _output << ", "s;
                }
            }
            _output << std::endl;
            _output << double_offset << R"(],)" << std::endl;
        }
        else
        {
            _output << double_offset << R"("buses": [ ],)" << std::endl;
        }
        _output << double_offset << R"("request_id": )" << _id << std::endl;
    }
    else
    {
        _output << double_offset << R"("request_id": )" << _id << R"(,)" << std::endl <<
                   double_offset << R"("error_message": "not found")" << std::endl;
    }

    _output << offset << "}";
}

void writeBusStat(const domain::BusStat &_stat, uint32_t _id, std::ostream &_output)
{
    using namespace std::literals::string_literals;
    _output << std::setprecision(6);

    _output << offset << "{" << std::endl;

    if (_stat.number_stops_ != 0)
    {
        _output << double_offset << R"("curvature": )" << _stat.curvature_ << R"(,)" << std::endl <<
                   double_offset << R"("request_id": )" << _id << R"(,)" << std::endl <<
                   double_offset << R"("route_length": )" << _stat.route_length_ << R"(,)" << std::endl <<
                   double_offset << R"("stop_count": )" << _stat.number_stops_ << R"(,)" << std::endl <<
                   double_offset << R"("unique_stop_count": )" << _stat.number_unique_stops_ << std::endl;
    }
    else
    {
        _output << double_offset << R"("request_id": )" << _id << R"(,)" << std::endl <<
                   double_offset << R"("error_message": "not found")" << std::endl;
    }

    _output << offset << "}";
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

void parseRenderSettings(const json::Document &_doc, renderer::MapRenderer &render)
{
    if (!_doc.GetRoot().IsMap())
    {
        throw std::invalid_argument("Incorrect JSON");
    }

    if (_doc.GetRoot().AsMap().count("render_settings") == 0U)
    {
        render.setInitSetting(false);
        return;
        throw std::invalid_argument("Incorrect JSON");
    }

    const auto& settings = _doc.GetRoot().AsMap().at("render_settings").AsMap();

    render.setWidth(settings.at("width").AsDouble());
    render.setHeight(settings.at("height").AsDouble());
    render.setPadding(settings.at("padding").AsDouble());
    render.setLineWidth(settings.at("line_width").AsDouble());
    render.setStopRadius(settings.at("stop_radius").AsDouble());
    render.setBusLabelFontSize(settings.at("bus_label_font_size").AsInt());
    render.setBusLabelOffset(settings.at("bus_label_offset").AsArray().front().AsDouble(),
                             settings.at("bus_label_offset").AsArray().back().AsDouble());
    render.setStopLabelFontSize(settings.at("stop_label_font_size").AsInt());
    render.setStopLabelOffset(settings.at("stop_label_offset").AsArray().front().AsDouble(),
                              settings.at("stop_label_offset").AsArray().back().AsDouble());
    render.setUnderlayerColor(parseColor(settings.at("underlayer_color")));
    render.setUnderlayerWidth(settings.at("underlayer_width").AsDouble());
    for (const auto& color : settings.at("color_palette").AsArray())
    {
        render.appendColorPalette(parseColor(color));
    }
    render.setInitSetting(true);
}

void writeMap(const svg::Document &_doc, uint32_t _id, std::ostream &_output)
{
    using namespace std::literals::string_literals;
    std::stringstream  doc_stream;
    _doc.Render(doc_stream);
    _output << offset << "{" << std::endl;

    _output << double_offset << R"("map": ")";
    char ch;
    while (doc_stream.get(ch))
    {
        if (ch == '\n')
        {
            _output << "\\n";
        }
        else if (ch == '\r')
        {
            _output << "\\r";
        }
        else if (ch == '\t')
        {
            _output << "\\t";
        }
        else if (ch == '\\')
        {
            _output << "\\\\";
        }
        else if (ch == '"')
        {
            _output << "\\\"";
        }
        else
        {
            _output << ch;
        }
    }
    _output << R"(",)" << std::endl;
    _output << double_offset << R"("request_id": )" << _id << std::endl;
    _output << offset << "}";
}

} // namespace reader
