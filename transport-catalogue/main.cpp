#include <algorithm>
#include <fstream>
#include <sstream>

#include "json_reader.h"
#include "request_handler.h"

int main()
{
    using namespace std;

    TransportCatalogue catalogue;
    renderer::MapRenderer render;
    RequestHandler handler{catalogue, render};
    reader::JsonReader reader(catalogue, render);
    json::Document json_input = json::Load(std::cin);
    reader.parseBaseRequests(json_input);
    reader.parseRenderSettings(json_input);
    handler.procRequests(json_input, std::cout);
    return 0;
}
