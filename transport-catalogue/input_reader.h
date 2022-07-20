#pragma once

#include <iostream>
#include <vector>
#include <string_view>

#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue {

class InputParser {

public:

    enum RequestType {
        ADD_STOP,
        ADD_ROUTE,
        NULL_REQUEST
    };

    enum RouteType {
        STRAIGHT,
        CIRCLE,
        NOT_ROUTE
    };

    struct Request {
        RouteType route_type = RouteType::NOT_ROUTE;
        RequestType req_type = RequestType::NULL_REQUEST;
        std::vector<std::string> req_body;
    };

    explicit InputParser(::transport_catalogue::TransportCatalogue* catalogue_ptr);

    std::vector<Request> ParseFromStream(std::istream& input);

    void ProcessRequests(const std::vector<Request>& requests);

private:

    TransportCatalogue* catalogue_ptr_;

    Request ParseLine(std::string_view line);

};

} // end of namespace: transport_catalogue
