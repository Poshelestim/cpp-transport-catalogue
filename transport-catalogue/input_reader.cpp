#include <utility>

#include "input_reader.h"

namespace parser
{

std::deque<std::string> parseInputData(std::istream &_input)
{
    using namespace std;
    deque<string> queries;

    size_t count_queries = 0;
    _input >> count_queries;
    _input.ignore();
    string line;

    while (count_queries != 0U)
    {
        getline(_input, line);

        if (!line.empty())
        {
            if (line == "\n")
            {
                continue;
            }

            int64_t pos = line.find_first_not_of(' ');
            int64_t space = line.find(' ', pos);
            --count_queries;

            if (line.substr(pos, space - pos) == "Stop")
            {
                queries.emplace_front(std::move(line));
                continue;
            }

            if (line.substr(pos, space - pos) == "Bus")
            {
                queries.emplace_back(std::move(line));
                continue;
            }
        }
    }

    return queries;
}

std::deque<std::string> parseInputData()
{
    return parseInputData(std::cin);
}

Bus parseBus(std::string_view _query)
{
    Bus parsed_bus;

    uint64_t pos = _query.find_first_not_of(' ');
    uint64_t space = _query.find(' ', pos);
    _query.remove_prefix(space);

    uint64_t colon = _query.rfind(':');
    if (colon != std::string_view::npos)
    {
        parsed_bus.name_ = _query.substr(0, colon);
        parsed_bus.name_.remove_prefix(1);
        _query.remove_prefix(colon + 1);
        char sep = '>';

        if (_query.find('-') != std::string_view::npos)
        {
            parsed_bus.is_circul_ = true;
            sep = '-';
        }

        uint64_t sep_pos = _query.find_first_of(sep);
        while (!_query.empty())
        {
            std::string_view namestop;
            if (sep_pos != std::string_view::npos)
            {
                namestop = _query.substr(_query.find_first_not_of(' '), sep_pos - 1);
                if (namestop[namestop.size() - 1] == ' ')
                {
                    namestop.remove_suffix(1);
                }
                parsed_bus.route_.emplace_back(namestop);
                _query = _query.substr(_query.find_first_of(sep) + 1);
                sep_pos = _query.find_first_of(sep);
                continue;
            }
            namestop = _query.substr(_query.find_first_not_of(' '));
            if (namestop[namestop.size() - 1] == ' ')
            {
                namestop.remove_suffix(1);
            }
            parsed_bus.route_.emplace_back(namestop);
            _query = "";
        }
    }
    return parsed_bus;
}

std::pair<Stop, std::vector<std::pair<std::string_view, double> > > parseStop(std::string_view _query)
{
    Stop new_stop;

    uint64_t pos = _query.find_first_not_of(' ');
    uint64_t space = _query.find(' ', pos);
    _query.remove_prefix(space);

    uint64_t colon = _query.rfind(':');
    if (colon != std::string_view::npos)
    {
        std::string_view base_info = _query;
        new_stop.name_ = base_info.substr(0, colon);
        new_stop.name_.remove_prefix(1);
        base_info.remove_prefix(colon + 1);
        base_info = base_info.substr(base_info.find_first_not_of(' '));
        colon = base_info.find(',');
        if (colon != std::string_view::npos)
        {
            std::string_view latitude = base_info.substr(0, colon);
            new_stop.latitude_ = std::stod({latitude.data(), latitude.size()});
        }
        base_info = base_info.substr(base_info.find(','));
        base_info.remove_prefix(1);
        std::string_view longitude = base_info.substr(base_info.find_first_not_of(' '));
        new_stop.longitude_ = std::stod({longitude.data(), longitude.size()});
    }

    std::string_view distances = _query;
    uint64_t comma = distances.rfind(',');
    bool has_stops = comma != distances.find(',');

    std::vector<std::pair<std::string_view, double> > result_distances;

    if (has_stops)
    {
        distances = distances.substr(0, comma);
        _query = _query.substr(_query.find(',') + 1);
        _query = _query.substr(_query.find(',') + 1);
        while (!_query.empty())
        {
            std::string_view distance = _query.substr(1, _query.find('m') - 1);
            std::string_view namestop = _query.substr(_query.find("to") + 3);
            namestop = namestop.substr(0, namestop.find_first_of(','));

            result_distances.push_back({namestop, std::stol(distance.data())});
            comma = _query.find(',');
            if (comma != std::string_view::npos)
            {
                _query = _query.substr(comma + 1);
                continue;
            }
            _query = "";
        }
    }

    return {new_stop, result_distances};
}

} // namespace parser
