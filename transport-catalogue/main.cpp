#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"



int main() {

    transport_catalogue::TransportCatalogue catalogue;
    
    JsonReader reader(&catalogue);

    reader.ReadJSON(std::cin);

    reader.FillCatalogue();

    reader.ProcessRequests();

    reader.PrintResponses(std::cout);
    
}