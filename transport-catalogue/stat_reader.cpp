#include "stat_reader.h"

#include <iomanip>
#include <iostream>
#include <utility>

namespace statistics
{

std::ostream &operator<<(std::ostream &os, const TransportCatalogue::BusInfo &stat)
{
    using namespace std::literals::string_literals;
    os << std::setprecision(6);

    if (stat.number_stops_ != 0)
    {
        os << "Bus "s << stat.name_ << ": "s <<
              stat.number_stops_ << " stops on route, "s <<
              stat.number_unique_stops_ << " unique stops, "s <<
              stat.route_length_ << " route length, "s <<
              stat.curvature_ << " curvature" << std::endl;
    }
    else
    {
        os << "Bus "s << stat.name_ << ": not found"s << std::endl;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, const TransportCatalogue::StopInfo &stat)
{
    using namespace std::literals::string_literals;

    os << "Stop "s << stat.name_ << ": "s;
    if (stat.is_exist)
    {
        if (stat.buses_.empty())
        {
            os << "no buses"s << std::endl;
        }
        else
        {
            os << "buses";
            for (const std::string_view &bus : stat.buses_)
            {
                os << " " << bus;
            }
            os << std::endl;
        }
    }
    else
    {
        os << "not found"s << std::endl;
    }

    return os;
}

Statistics::Statistics(TransportCatalogue *_catalogue) :
    catalogue_(_catalogue)
{

}

void Statistics::printBusStatistics(std::string_view _name) const
{
    std::cout << catalogue_->getBusInfo(_name);
}

void Statistics::printStopStatistics(std::string_view _name) const
{
    std::cout << catalogue_->getStopInfo(_name);
}

std::deque<std::string> parseQueries(std::istream &_input)
{
    using namespace std;
    deque<string> queries;

    size_t count_queries = 0;
    _input >> count_queries;
    _input.ignore();
    string line;
    //    getline(_input, line);

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

            if (line.substr(pos, space - pos) == "Bus")
            {
                queries.emplace_back(std::move(line));
                continue;
            }

            if (line.substr(pos, space - pos) == "Stop")
            {
                queries.emplace_back(std::move(line));
                continue;
            }
        }
    }

    return queries;
}

std::deque<std::string> parseQueries()
{
    return parseQueries(std::cin);;
}

} //namespace statistics
