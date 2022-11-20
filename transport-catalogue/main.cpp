#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"


int main() {

    transport_catalogue::TransportCatalogue catalogue;
    
    JsonReader reader(&catalogue);

    reader.ReadJSON(std::cin);

    reader.FillCatalogue();

    graph::DirectedWeightedGraph<double> route_graph(catalogue.GetStopsCount());
    catalogue.FillGraph(&route_graph);

    graph::Router<double> router{route_graph};
    catalogue.SetRouter(&router);

    reader.ProcessRequests();

    reader.PrintResponses(std::cout);
    
}