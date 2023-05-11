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
    TransportRouter router;

    RequestHandler handler{catalogue, render, router};
    reader::JsonReader reader(catalogue, render, router);

    json::Document json_input = json::Load(cin);
    reader.parseBaseRequests(json_input);
    reader.parseRenderSettings(json_input);
    reader.parseRoutingSettings(json_input);

    router.createGraph(catalogue);

    handler.procRequests(json_input, cout);
    return 0;
}
