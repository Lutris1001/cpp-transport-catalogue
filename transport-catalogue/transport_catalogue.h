#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <unordered_set>

#include "input_reader.h"
#include "geo.h"


namespace transport_catalogue {

class TransportCatalogue {

    friend class StatParser;

struct BusStop {
    BusStop(const std::string& name, double x, double y)
        : bus_stop_name(name), map_point(Coordinates{x, y})
    {
        assert(double(-90.0) <= x && x <= double(90.0));
        assert(double(-180.0) <= y && y <= double(180.0));
    }

    bool operator==(const BusStop& other);

    bool is_found = false;
    std::string bus_stop_name;
    Coordinates map_point;
};

struct BusRoute {

    BusRoute(const std::string& name)
        : route_name(name)
    {
        assert(!name.empty());
    }

    bool is_found = false;
    std::vector<BusStop*> stops;
    std::string route_name;

    // These parameters will be calculated only if needed
    double route_length = 0;
    int true_route_length = 0;
    std::size_t unique_stops = 0;
    std::size_t route_size = 0;

    double CalculateRouteLength();
    std::size_t CalculateUniqueStops();
    std::size_t CalculateRouteSize();

};

struct BusStopPtrHash {
    size_t operator()(const std::pair<BusStop*, BusStop*>& p) const {
        auto hash1 = std::hash<const void *>{}(p.first);
        auto hash2 = std::hash<const void *>{}(p.second);

        if (hash1 != hash2) {
            return hash1 ^ hash2;
        }

        return hash1;
    }
};

    using BusStopPtrPair = std::pair<BusStop*, BusStop*>;

class BusStopPtrPairEqualKey { // EqualTo class for all_distances_
public:
    constexpr bool operator()(const BusStopPtrPair &lhs, const BusStopPtrPair &rhs) const
    {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

};

public:

    explicit TransportCatalogue(const std::vector<InputParser::Request>& requests);

    void RequestProcessing(const std::vector<InputParser::Request>& requests);

private:

    std::deque<BusStop> all_stops_;
    std::unordered_map<std::string_view, BusStop*> stop_name_to_stop_;

    std::deque<BusRoute> all_routes_;
    std::unordered_map<std::string_view, BusRoute*> route_name_to_route_;

    std::unordered_map<BusStop*, std::unordered_set<BusRoute*>> stop_name_to_route_set_;

    std::unordered_map<BusStopPtrPair, int, BusStopPtrHash, BusStopPtrPairEqualKey> all_distances_;

    void ProcessOneRequest(const InputParser::Request& req);

    void AddNewBusStop(const InputParser::Request& new_stop);

    void AddNewBusRoute(const InputParser::Request& new_route);

    void ProcessDistanceInRequest(const InputParser::Request& new_stop);

    [[nodiscard]] const std::pair<BusStop*, std::unordered_set<BusRoute*>> SearchBusStop(const std::string& stop_name) const;

    [[nodiscard]] const BusRoute& SearchBusRoute(const std::string& route_name);

    int CalculateTrueRouteLengthForRoute(BusRoute* route);

};

} // end of namespace: transport_catalogue
