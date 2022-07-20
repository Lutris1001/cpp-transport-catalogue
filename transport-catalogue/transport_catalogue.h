#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <unordered_set>

//#include "input_reader.h"
#include "geo.h"


namespace transport_catalogue {

class TransportCatalogue {

struct Stop {

    Stop(const std::string& name, double x, double y)
        : name(name), map_point(Coordinates{x, y})
    {
        assert(double(-90.0) <= x && x <= double(90.0));
        assert(double(-180.0) <= y && y <= double(180.0));
    }

    bool operator==(const Stop& other);

    std::string name;
    Coordinates map_point;
};

struct Route {

    Route(const std::string& name)
        : name(name)
    {
        assert(!name.empty());
    }

    std::vector<Stop*> stops;
    std::string name;

};

struct RouteAdditionalParameters {

    Route* route_ptr = nullptr;

    // These parameters will be calculated only if needed

    double geo_route_length = 0;
    int true_route_length = 0;
    std::size_t unique_stops = 0;
    std::size_t route_size = 0;

    RouteAdditionalParameters(Route* ptr);

    double CalculateGeoRouteLength();
    std::size_t CalculateUniqueStops();
    std::size_t CalculateRouteSize();

};

struct StopPtrHash {
    size_t operator()(const std::pair<Stop*, Stop*>& p) const {
        auto hash1 = std::hash<const void *>{}(p.first);
        auto hash2 = std::hash<const void *>{}(p.second);

        if (hash1 != hash2) {
            return hash1 ^ hash2;
        }

        return hash1;
    }
};

    using StopPtrPair = std::pair<Stop*, Stop*>;

class StopPtrPairEqualKey { // EqualTo class for all_distances_
public:
    constexpr bool operator()(const StopPtrPair &lhs, const StopPtrPair &rhs) const
    {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

};

public:

    using RouteParameters = std::tuple<std::string_view, double, int, std::size_t, std::size_t, bool>;
    using StopParameters = std::tuple<std::string_view, std::vector<std::string_view>, bool>;

    void AddStop(std::string& name, double x, double y);

    void AddRoute(const std::string& name, std::vector<std::string>& stops);

    void AddDistance(const std::string& name, std::string& another_name, int distance);

    int GetDistance(const std::string& name, std::string& another_name);

    int CalculateTrueRouteLength(const std::string& name);

    [[nodiscard]] const StopParameters SearchStop(const std::string& stop_name) const;

    [[nodiscard]] const RouteParameters SearchRoute(const std::string& route_name);

private:

    std::deque<Stop> all_stops_;
    std::unordered_map<std::string_view, Stop*> stop_name_to_stop_;

    std::deque<Route> all_routes_;
    std::unordered_map<std::string_view, Route*> route_name_to_route_;

    std::unordered_map<Stop*, std::unordered_set<Route*>> stop_name_to_route_set_;

    std::unordered_map<StopPtrPair, int, StopPtrHash, StopPtrPairEqualKey> all_distances_;

    std::deque<RouteAdditionalParameters> all_route_parameters_;
    std::unordered_map<std::string_view, RouteAdditionalParameters*> route_name_to_additional_parameters_;

};

} // end of namespace: transport_catalogue
