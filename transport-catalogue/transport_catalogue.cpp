
#include "input_reader.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <cassert>
#include <string_view>
#include <algorithm>
#include <string>
#include <unordered_set>

namespace transport_catalogue {

TransportCatalogue::TransportCatalogue(const std::vector<InputParser::Request>& requests) {
    RequestProcessing(requests);
}

void TransportCatalogue::ProcessOneRequest(const InputParser::Request& req) {

    if (req.req_type == InputParser::RequestType::ADD_STOP) {
        AddNewBusStop(req);
    } else if (req.req_type == InputParser::RequestType::ADD_ROUTE) {
        AddNewBusRoute(req);
    }
}

void TransportCatalogue::RequestProcessing(const std::vector<InputParser::Request>& requests) {

    for (auto i : requests) {
        if (i.req_type == InputParser::RequestType::ADD_STOP) {
            ProcessOneRequest(i);
        }
    }

    for (auto i : requests) {
        if (i.req_type == InputParser::RequestType::ADD_STOP) {
            ProcessDistanceInRequest(i);
        }
    }

    for (auto i : requests) {
        if (i.req_type == InputParser::RequestType::ADD_ROUTE) {
            ProcessOneRequest(i);
        }
    }
}

void TransportCatalogue::AddNewBusStop(const InputParser::Request& new_stop) {

    assert(new_stop.req_type == InputParser::RequestType::ADD_STOP); // check type of request
    assert(new_stop.req_body.size() >= 3); // check type of request

    BusStop stop(std::string(new_stop.req_body[0]),
                 std::stod(std::string(new_stop.req_body[1])),
                 std::stod(std::string(new_stop.req_body[2])));

    all_stops_.push_back(std::move(stop));
    BusStop* stop_ptr = &(all_stops_.back());

    stop_name_to_stop_[std::string_view(stop_ptr->bus_stop_name)] = stop_ptr;

    stop_name_to_route_set_[stop_ptr];

    stop_ptr->is_found = true;

}

void TransportCatalogue::ProcessDistanceInRequest(const InputParser::Request& new_stop) {

    assert(new_stop.req_type == InputParser::RequestType::ADD_STOP); // check type of request

    for (int i = 3; i < new_stop.req_body.size() - 1; i += 2) {    // filling the distances

        BusStopPtrPair  tmp = {stop_name_to_stop_[std::string_view(new_stop.req_body[0])],
                               stop_name_to_stop_[std::string_view(new_stop.req_body[i + 1])]};
        all_distances_[tmp] = std::stoi(new_stop.req_body[i]);

    }

}

void TransportCatalogue::AddNewBusRoute(const InputParser::Request& new_route) {

    assert(new_route.req_type == InputParser::RequestType::ADD_ROUTE); // check type of request
    assert(new_route.req_body.size() > 1); // check number of args

    BusRoute route(new_route.req_body[0]);

    route.is_found = true;

    all_routes_.push_back(std::move(route));
    BusRoute* route_ptr = &(all_routes_.back());

    route_name_to_route_[std::string_view(route_ptr->route_name)] = route_ptr;


    if (new_route.route_type == InputParser::RouteType::STRAIGHT) {
        for (auto i = 1; i < new_route.req_body.size(); ++i) {
            route_ptr->stops.push_back(stop_name_to_stop_.at(std::string_view(new_route.req_body[i])));
        }
        for (auto i = new_route.req_body.size() - 2 ; i >= 1 ; --i) {
            route_ptr->stops.push_back(stop_name_to_stop_.at(std::string_view(new_route.req_body[i])));
        }
    } else {
        for (auto i = 1; i < new_route.req_body.size(); ++i) {
            route_ptr->stops.push_back(stop_name_to_stop_.at(std::string_view(new_route.req_body[i])));
        }
    }

    for (auto i : route_ptr->stops) {
        stop_name_to_route_set_[i].insert(route_ptr);
    }

}

const std::pair<TransportCatalogue::BusStop*, std::unordered_set<TransportCatalogue::BusRoute*>> TransportCatalogue::SearchBusStop(const std::string& stop_name) const {

    if (stop_name_to_stop_.count(std::string_view(stop_name)) != 0) {

        auto stop_ptr = stop_name_to_stop_.at(std::string_view(stop_name));

        return {stop_ptr, stop_name_to_route_set_.at(stop_ptr)};

    } else {

        static const BusStop dummy_stop("dummy", 0, 0);
        return {const_cast<TransportCatalogue::BusStop*>(&dummy_stop), std::unordered_set<TransportCatalogue::BusRoute*>{}};

    }

}

const TransportCatalogue::BusRoute& TransportCatalogue::SearchBusRoute(const std::string& route_name) {

    if (route_name_to_route_.count(std::string_view(route_name)) != 0) {

        BusRoute& result = *route_name_to_route_.at(std::string_view(route_name));

        if (result.route_length == 0) {
            result.CalculateRouteLength();
        }
        if (result.route_size == 0) {
            result.CalculateRouteSize();
        }
        if (result.unique_stops == 0) {
            result.CalculateUniqueStops();
        }
        if (result.true_route_length == 0) {
            CalculateTrueRouteLengthForRoute(&result);
        }
        return const_cast<TransportCatalogue::BusRoute&>(result);

    } else {

        static const BusRoute dummy_route("dummy"s);
        return dummy_route;

    }
}

double TransportCatalogue::BusRoute::CalculateRouteLength() {

    for (auto i = 0 ; i < stops.size() - 1 ; ++i) {
        route_length += ComputeDistance(stops[i]->map_point, stops[i + 1]->map_point);
    }

    return route_length;
}

std::size_t TransportCatalogue::BusRoute::CalculateUniqueStops() {

    std::vector<std::string_view> tmp;

    for (auto i : stops) {
        auto str = std::string_view(i->bus_stop_name);
        if (std::count(tmp.begin(), tmp.end(), str) == 0) {
            tmp.push_back(str);
        }
    }

    unique_stops = tmp.size();

    return unique_stops;
}

bool TransportCatalogue::BusStop::operator==(const BusStop& other) {
    return bus_stop_name == other.bus_stop_name;
}


std::size_t TransportCatalogue::BusRoute::CalculateRouteSize() {
    route_size = stops.size();
    return stops.size();
}


int TransportCatalogue::CalculateTrueRouteLengthForRoute(BusRoute* route) {

    for (auto i = 0 ; i < route->stops.size() - 1 ; ++i) {

        auto tmp = BusStopPtrPair{route->stops[i], route->stops[i + 1]};

        if (all_distances_.count(tmp) != 0) {
            route->true_route_length += all_distances_.at(tmp);
        } else {
            route->true_route_length += all_distances_.at(BusStopPtrPair{route->stops[i + 1], route->stops[i]});
        }
    }

    return route->true_route_length;
}

} // end of namespace: transport_catalogue
