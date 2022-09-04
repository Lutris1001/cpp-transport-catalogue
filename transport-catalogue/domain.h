#pragma once

#include <cassert>
#include "geo.h"

using namespace geo;

namespace domain {

struct Stop {

    Stop(const std::string& name, Coordinates geo_map_point)
        : name(name), map_point(geo_map_point)
    {
        assert(double(-90.0) <= geo_map_point.lat && geo_map_point.lat <= double(90.0));
        assert(double(-180.0) <= geo_map_point.lng && geo_map_point.lng <= double(180.0));
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
    bool is_roundtrip;

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