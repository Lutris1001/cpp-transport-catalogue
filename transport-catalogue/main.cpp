#include <iostream>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace transport_catalogue;

int main() {

    // Initialization: Empty catalogue;
    TransportCatalogue catalogue;

    TransportCatalogue* ptr = &catalogue;

    // Initialization: Parser for Catalogue;
    InputParser parser(ptr);
    auto requsts = parser.ParseFromStream(std::cin);
    parser.ProcessRequests(requsts);

    // Initialization: Parser to process Queries to Catalogue;
    StatParser query_processor(ptr);

    // Processing Queries to Catalogue and output result;
    query_processor.ProcessQueries(std::cin);

    return 0;
}