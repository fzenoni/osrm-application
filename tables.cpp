// #include "osrm/match_parameters.hpp"
// #include "osrm/nearest_parameters.hpp"
// #include "osrm/route_parameters.hpp"
#include "osrm/table_parameters.hpp"
// #include "osrm/trip_parameters.hpp"

#include "osrm/coordinate.hpp"
#include "osrm/engine_config.hpp"
#include "osrm/json_container.hpp"

#include "osrm/osrm.hpp"
#include "osrm/status.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <utility>

// my additions
#include <iterator>
#include <vector>
#include <algorithm>

#include <cstdlib>

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " data.osrm\n";
        return EXIT_FAILURE;
    }

    using namespace osrm;

    // Configure based on a .osrm base path, and no datasets in shared mem from osrm-datastore
    EngineConfig config;

    config.storage_config = {argv[1]};
    config.use_shared_memory = false;

    // We support two routing speed up techniques:
    // - Contraction Hierarchies (CH): requires extract+contract pre-processing
    // - Multi-Level Dijkstra (MLD): requires extract+partition+customize pre-processing
    //
    config.algorithm = EngineConfig::Algorithm::CH;
    // config.algorithm = EngineConfig::Algorithm::MLD;

    // Routing machine with several services (such as Route, Table, Nearest, Trip, Match)
    const OSRM osrm{config};

    // The following shows how to use the Route service; configure this service
    // RouteParameters params;
    // The following attempts to use the Table service; configure this service
    TableParameters params;

    // Table in milan
    params.coordinates.push_back({util::FloatLongitude{9.1919}, util::FloatLatitude{45.4641}});
    params.coordinates.push_back({util::FloatLongitude{9.2919}, util::FloatLatitude{45.2641}});
    params.coordinates.push_back({util::FloatLongitude{9.2043}, util::FloatLatitude{45.4859}});
    params.coordinates.push_back({util::FloatLongitude{9.2143}, util::FloatLatitude{45.4959}});

    // Define sources and destination by providing indexes
    params.sources.push_back(0);
    params.sources.push_back(1);
    params.destinations.push_back(2);
    params.destinations.push_back(3);

    // Response is in JSON format
    json::Object result;

    // Execute routing request, this does the heavy lifting
    // const auto status = osrm.Route(params, result);
    const auto status = osrm.Table(params, result);

    if (status == Status::Ok)
    {
        // auto &routes = result.values["routes"].get<json::Array>();
        auto &tables = result.values["durations"].get<json::Array>();

        // Loop over tables values
        for(int i = 0; i < tables.values.size(); i++) {
            auto &table = tables.values.at(i).get<json::Array>();
            for(int j = 0; j < table.values.size(); j++) {
                const auto duration = table.values.at(j).get<json::Number>().value; 

                // Warn users if extract does not contain the default coordinates from above
                if (duration == 0)
                {
                    std::cout << "Note: distance or duration is zero. ";
                    std::cout << "You are probably doing a query outside of the OSM extract.\n\n";
                }

                std::cout << "From origin " << i << " to destination " << j << ": " << duration
                    << " seconds." << std::endl;
            }
        }

        return EXIT_SUCCESS;
    }
    else if (status == Status::Error)
    {
        const auto code = result.values["code"].get<json::String>().value;
        const auto message = result.values["message"].get<json::String>().value;

        std::cout << "Code: " << code << "\n";
        std::cout << "Message: " << code << "\n";
        return EXIT_FAILURE;
    }
}
