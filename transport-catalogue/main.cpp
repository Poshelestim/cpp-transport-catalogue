#include <algorithm>
#include <fstream>
#include <sstream>

#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"

void PrintUsage(std::ostream& stream = std::cerr)
{
    using namespace std::literals;
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
    using namespace std;

    if (argc != 2)
    {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);


    if (mode == "make_base"sv)
    {
        std::ifstream ifs("./input_make_base.json");

        TransportCatalogue catalogue;
        renderer::MapRenderer render;
        TransportRouter router;

        RequestHandler handler{catalogue, render, router};
        reader::JsonReader reader(catalogue, render, router);

        json::Document json_input = json::Load(ifs);

        reader.parseBaseRequests(json_input);
        reader.parseRenderSettings(json_input);
        reader.parseRoutingSettings(json_input);

        router.createGraph(catalogue);

        auto path = reader.parseSerializationSettings(json_input);
        if (path.has_value())
        {
            serialization::Serialization serialization(path.value());
            serialization.Serialize(catalogue, render, router);
        }
    }
    else if (mode == "process_requests"sv)
    {
        std::ifstream ifs("./input_process_requests.json");
        std::ofstream ofs("./output_process_requests.json");

        TransportCatalogue catalogue;
        renderer::MapRenderer render;
        TransportRouter router;

        RequestHandler handler{catalogue, render, router};
        reader::JsonReader reader(catalogue, render, router);

        json::Document json_input = json::Load(ifs);

        auto path = reader.parseSerializationSettings(json_input);
        if (path.has_value())
        {
            serialization::Serialization serialization(path.value());
            serialization.Deserialize(catalogue, render, router);

            handler.procRequests(json_input, ofs);
        }
    }
    else
    {
        PrintUsage();
        return 1;
    }

    //    TransportCatalogue catalogue;
    //    renderer::MapRenderer render;
    //    TransportRouter router;

    //    RequestHandler handler{catalogue, render, router};
    //    reader::JsonReader reader(catalogue, render, router);

    //    json::Document json_input = json::Load(cin);
    //    reader.parseBaseRequests(json_input);
    //    reader.parseRenderSettings(json_input);
    //    reader.parseRoutingSettings(json_input);

    //    router.createGraph(catalogue);

    //    handler.procRequests(json_input, cout);

    return 0;
}
