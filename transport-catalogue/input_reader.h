#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <deque>

#include "transport_catalogue.h"

struct Bus;
struct Stop;

namespace parser
{

std::deque<std::string> parseInputData(std::istream &_input);

std::deque<std::string> parseInputData();

Bus parseBus(std::string_view _query);

std::pair<Stop, std::vector<std::pair<std::string_view, double> > > parseStop(std::string_view _query);

} //namespace parser

