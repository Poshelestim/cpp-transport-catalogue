#include <utility>

#include "input_reader.h"

#include "log_duration.h"

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

} // namespace parser
