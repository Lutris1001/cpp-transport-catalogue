
#include "input_reader.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <cassert>
#include <string_view>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <tuple>

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& name, Coordinates map_point) {

        assert(!name.empty()); // check name not empty

        Stop stop(name, map_point);

        all_stops_.push_back(std::move(stop));
        Stop* stop_ptr = &(all_stops_.back());

        stop_name_to_stop_[std::string_view(stop_ptr->name)] = stop_ptr;

        stop_name_to_route_set_[stop_ptr];

}

void TransportCatalogue::AddDistance(const std::string& stop_name_from, const std::string& stop_name_to, int distance) {

    assert(!stop_name_from.empty());
    assert(!stop_name_to.empty());
    assert(distance >= 0);

    StopPtrPair  key = {stop_name_to_stop_[std::string_view(stop_name_from)],
                stop_name_to_stop_[std::string_view(stop_name_to)]};

    all_distances_[key] = distance;

}

void TransportCatalogue::AddRoute(const std::string &name, const std::vector <std::string> &stops) {

    assert(!name.empty()); // check number of args

    Route route(name);

    all_routes_.push_back(std::move(route));
    Route* route_ptr = &(all_routes_.back());

    route_name_to_route_[std::string_view(route_ptr->name)] = route_ptr;

    for (auto i : stops) {
        route_ptr->stops.push_back(stop_name_to_stop_.at(std::string_view(i)));
    }

    for (auto i : route_ptr->stops) {
        stop_name_to_route_set_[i].insert(route_ptr);
    }

    all_route_parameters_.push_back(RouteAdditionalParameters(route_ptr));
    route_name_to_additional_parameters_[std::string_view(route_ptr->name)] = &all_route_parameters_.back();

}

const TransportCatalogue::StopSearchResponse TransportCatalogue::SearchStop(const std::string& stop_name) const {

    if (stop_name_to_stop_.count(std::string_view(stop_name)) != 0) {

        auto stop_ptr = stop_name_to_stop_.at(std::string_view(stop_name));

        std::vector<std::string_view> routes_names;

        for (auto i : stop_name_to_route_set_.at(stop_ptr)) {
            routes_names.push_back(std::string_view(i->name));
        }
        std::sort(routes_names.begin(), routes_names.end());
        StopSearchResponse result{std::string_view(stop_ptr->name), std::move(routes_names), true};

        return result;

    } else {

        StopSearchResponse dummy_stop{std::string_view(stop_name), std::vector<std::string_view>(), false};
        return dummy_stop;

    }

}


const TransportCatalogue::RouteSearchResponse TransportCatalogue::SearchRoute(const std::string& route_name) {

    if (route_name_to_route_.count(std::string_view(route_name)) != 0) {

        Route& found_route = *route_name_to_route_.at(std::string_view(route_name));

        RouteAdditionalParameters& params = *route_name_to_additional_parameters_.at(std::string_view(route_name));

        if (params.geo_route_length == 0) {
            params.CalculateGeoRouteLength();
        }
        if (params.route_size == 0) {
            params.CalculateRouteSize();
        }
        if (params.unique_stops == 0) {
            params.CalculateUniqueStops();
        }
        if (params.true_route_length == 0) {
            CalculateTrueRouteLength(found_route.name);
        }

        RouteSearchResponse result{std::string_view(found_route.name), params.geo_route_length,
                               params.true_route_length, params.unique_stops,
                               params.route_size, true};
        return result;

    } else {

        RouteSearchResponse dummy_route{std::string_view(route_name), 0, 0, 0, 0, false};
        return dummy_route;
    }
}

double TransportCatalogue::RouteAdditionalParameters::CalculateGeoRouteLength() {

    for (auto i = 0 ; i < route_ptr->stops.size() - 1 ; ++i) {
        geo_route_length += ComputeDistance(route_ptr->stops[i]->map_point, route_ptr->stops[i + 1]->map_point);
    }

    return geo_route_length;
}

std::size_t TransportCatalogue::RouteAdditionalParameters::CalculateUniqueStops() {

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

bool TransportCatalogue::Stop::operator==(const Stop& other) {
    return name == other.name;
}


std::size_t TransportCatalogue::RouteAdditionalParameters::CalculateRouteSize() {
    route_size = route_ptr->stops.size();
    return route_ptr->stops.size();
}

int TransportCatalogue::GetDistance(const std::string& stop_name_from, const std::string& stop_name_to) {
    auto key = StopPtrPair{stop_name_to_stop_.at(std::string_view(stop_name_from)),
                           stop_name_to_stop_.at(std::string_view(stop_name_to))};

    if (all_distances_.count(key) != 0) {
        return all_distances_.at(key);
    } else {
        auto reversed_key = StopPtrPair{stop_name_to_stop_.at(std::string_view(stop_name_to)),
                                stop_name_to_stop_.at(std::string_view(stop_name_from))};

        return all_distances_.at(reversed_key);
    }

}

int TransportCatalogue::CalculateTrueRouteLength(const std::string& name) {

    if (route_name_to_additional_parameters_.count(std::string_view(name)) != 0) {
        RouteAdditionalParameters& params_ref = *route_name_to_additional_parameters_.at(std::string_view(name));

        for (auto i = 0 ; i < params_ref.route_ptr->stops.size() - 1 ; ++i) {

            params_ref.true_route_length += GetDistance(params_ref.route_ptr->stops[i]->name,
                                                        params_ref.route_ptr->stops[i + 1]->name);

        }
        return params_ref.true_route_length;
    }

    return 0;
}

    TransportCatalogue::RouteAdditionalParameters::RouteAdditionalParameters(Route * ptr)
        : route_ptr(ptr)
    {
    }

} // end of namespace: transport_catalogue
