#pragma once

#include <cassert>
#include "geo.h"

using namespace geo;

namespace domain {

struct Stop {

    explicit Stop(const std::string& name, Coordinates geo_map_point, int stop_id)
        : name(name), map_point(geo_map_point), id(stop_id)
    {
        assert(id >= 0);
        assert(double(-90.0) <= geo_map_point.lat && geo_map_point.lat <= double(90.0));
        assert(double(-180.0) <= geo_map_point.lng && geo_map_point.lng <= double(180.0));
    }

    bool operator==(const Stop& other);

    std::string name;
    Coordinates map_point;
    int id;
};

using StopPtrPair = std::pair<Stop*, Stop*>;

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

class StopPtrPairEqualKey { // EqualTo class for all_distances_
public:
    constexpr bool operator()(const StopPtrPair &lhs, const StopPtrPair &rhs) const
    {
        return *(lhs.first) == *(rhs.first) && *(lhs.second) == *(rhs.second);
    }
};

struct Route {

    explicit Route(const std::string& name, int route_id)
        : name(name), id(route_id)
    {
        assert(route_id >= 0);
        assert(!name.empty());
    }

    std::vector<Stop*> stops;
    std::string name;
    bool is_roundtrip;
    int id;
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

} // end namespace domain