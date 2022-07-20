#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <vector>
#include <string_view>
#include <unordered_set>

namespace transport_catalogue {

class StatParser {

public:

    enum QueryType {
        SEARCH_STOP,
        SEARCH_ROUTE,
        NULL_QUERY
    };

    struct Query {
        QueryType query_type;
        std::vector<std::string> query_body;
    };

    explicit StatParser(TransportCatalogue* ptr);

    std::vector<Query> ReadFromStream(std::istream &input);

    std::istream& ProcessQueries(std::istream &input);

private:

    TransportCatalogue* catalogue_ptr_; // Ptr to catalogue to make request

    void PrintRoute(const std::string& name) const;

    void PrintStop(const std::string& name) const;

    Query ParseLine(std::string_view line);

    void ProcessOneQuery(Query);

};

} // end of namespace: transport_catalogue
