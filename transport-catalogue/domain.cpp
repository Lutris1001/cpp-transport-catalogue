#include "domain.h"
#include "transport_catalogue.h"

namespace domain {

double RouteAdditionalParameters::CalculateGeoRouteLength() {

    if (route_ptr->stops.empty()) {
        return 0;
    }

    for (auto i = 0 ; i < route_ptr->stops.size() - 1 ; ++i) {
        geo_route_length += ComputeDistance(route_ptr->stops[i]->map_point, route_ptr->stops[i + 1]->map_point);
    }

    return geo_route_length;
}

std::size_t RouteAdditionalParameters::CalculateUniqueStops() {

    std::vector<std::string_view> stops_names;

    for (auto i : route_ptr->stops) {
        auto str = std::string_view(i->name);
        if (std::count(stops_names.begin(), stops_names.end(), str) == 0) {
            stops_names.push_back(str);
        }
    }

    unique_stops = stops_names.size();

    return unique_stops;
}

bool Stop::operator==(const Stop& other) {
    return name == other.name;
}

std::size_t RouteAdditionalParameters::CalculateRouteSize() {
    route_size = route_ptr->stops.size();
    return route_ptr->stops.size();
}

RouteAdditionalParameters::RouteAdditionalParameters(Route * ptr)
    : route_ptr(ptr)
{
}

} // end namespace domain