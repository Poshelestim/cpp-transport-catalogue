#pragma once
#include <istream>
#include <string>
#include <vector>

#include "transport_catalogue.h"

namespace statistics
{

struct Statistics
{
    explicit Statistics(TransportCatalogue *_catalogue);

    ~Statistics() = default;

    void printBusStatistics(std::string_view _name) const;

    void printStopStatistics(std::string_view _name) const;

    TransportCatalogue *catalogue_;
};

std::deque<std::string> parseQueries(std::istream &_input);

std::deque<std::string> parseQueries();

} //namespace statistics
