#include <iostream>
#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace transport_catalogue;

int main() {
    // Initialization: Parser for Catalogue;
    InputParser parser;

    // Initialization: Catalogue with parsing data from stream;
    TransportCatalogue catalogue(parser.ParseFromStream(std::cin));

    TransportCatalogue* ptr = &catalogue;

    // Initialization: Parser to process Queries to Catalogue;
    StatParser query_processor(ptr);

    // Processing Queries to Catalogue and output result;
    query_processor.ProcessQueries(std::cin);

    return 0;
}