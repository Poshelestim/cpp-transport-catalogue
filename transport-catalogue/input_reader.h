#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <deque>

namespace parser
{

std::deque<std::string> parseInputData(std::istream &_input);

std::deque<std::string> parseInputData();

} //namespace parser

