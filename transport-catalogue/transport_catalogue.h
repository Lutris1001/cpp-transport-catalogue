#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <map>
#include <unordered_set>
#include <optional>

//#include "input_reader.h"
#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

using namespace geo;
using namespace domain;

struct StopSearchResponse {

    std::string_view name;
    std::vector<std::string_view> route_names_at_stop;
    bool is_found;

};

struct RouteSearchResponse {

    std::string_view name;
    double geo_route_length;
    int true_route_length;
    std::size_t unique_stops;
    std::size_t route_size;
    bool is_found;

};

class TransportCatalogue {

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

    void AddStop(const std::string& name, Coordinates map_point);

    void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_round);

    void AddDistance(const std::string& stop_name_from, const std::string& stop_name_to, int distance);

    int GetDistance(const std::string& stop_name_from, const std::string& stop_name_to);

    std::map<std::string, const Route*> GetAllRoutesPtr() const;

    bool IsStopExist(const std::string_view& name) const;

    const Route* GetRoutePtr(const std::string_view& route_name) const;

    int CalculateTrueRouteLength(const std::string& name);

    [[nodiscard]] const StopSearchResponse SearchStop(const std::string& stop_name) const;

    [[nodiscard]] const RouteSearchResponse SearchRoute(const std::string& route_name);

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