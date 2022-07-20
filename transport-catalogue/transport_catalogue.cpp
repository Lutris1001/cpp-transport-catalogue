
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

void TransportCatalogue::AddStop(std::string& name, double x, double y) {

        assert(!name.empty()); // check name not empty

        Stop stop(name, x, y);

        all_stops_.push_back(std::move(stop));
        Stop* stop_ptr = &(all_stops_.back());

        stop_name_to_stop_[std::string_view(stop_ptr->name)] = stop_ptr;

        stop_name_to_route_set_[stop_ptr];

}

void TransportCatalogue::AddDistance(const std::string& name, std::string& another_name, int distance) {

    assert(!name.empty());

    StopPtrPair  tmp = {stop_name_to_stop_[std::string_view(name)],
                stop_name_to_stop_[std::string_view(another_name)]};

    all_distances_[tmp] = distance;

}

void TransportCatalogue::AddRoute(const std::string &name, std::vector <std::string> &stops) {

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

using StopParameters = std::tuple<std::string_view, std::vector<std::string_view>, bool>;

const StopParameters TransportCatalogue::SearchStop(const std::string& stop_name) const {

    if (stop_name_to_stop_.count(std::string_view(stop_name)) != 0) {

        auto stop_ptr = stop_name_to_stop_.at(std::string_view(stop_name));

        std::vector<std::string_view> tmp;

        for (auto i : stop_name_to_route_set_.at(stop_ptr)) {
            tmp.push_back(std::string_view(i->name));
        }
        std::sort(tmp.begin(), tmp.end());
        StopParameters result{std::string_view(stop_ptr->name), std::move(tmp), true};

        return result;

    } else {

        StopParameters dummy_stop{std::string_view(stop_name), std::vector<std::string_view>(), false};
        return dummy_stop;

    }

}

using RouteParameters = std::tuple<std::string_view, double, int, std::size_t, std::size_t, bool>;

const RouteParameters TransportCatalogue::SearchRoute(const std::string& route_name) {

    if (route_name_to_route_.count(std::string_view(route_name)) != 0) {

        Route& tmp = *route_name_to_route_.at(std::string_view(route_name));

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
            CalculateTrueRouteLength(tmp.name);
        }

        RouteParameters result{std::string_view(tmp.name), params.geo_route_length,
                               params.true_route_length, params.route_size, params.unique_stops, true};
        return result;

    } else {

        RouteParameters dummy_route{std::string_view(route_name), 0, 0, 0, 0, false};
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

    std::vector<std::string_view> tmp;

    for (auto i : route_ptr->stops) {
        auto str = std::string_view(i->name);
        if (std::count(tmp.begin(), tmp.end(), str) == 0) {
            tmp.push_back(str);
        }
    }

    unique_stops = tmp.size();

    return unique_stops;
}

bool TransportCatalogue::Stop::operator==(const Stop& other) {
    return name == other.name;
}


std::size_t TransportCatalogue::RouteAdditionalParameters::CalculateRouteSize() {
    route_size = route_ptr->stops.size();
    return route_ptr->stops.size();
}

int TransportCatalogue::GetDistance(const std::string& name, std::string& another_name) {
    auto tmp = StopPtrPair{stop_name_to_stop_.at(std::string_view(name)),
                           stop_name_to_stop_.at(std::string_view(another_name))};

    if (all_distances_.count(tmp) != 0) {
        return all_distances_.at(tmp);
    } else {
        auto tmp2 = StopPtrPair{stop_name_to_stop_.at(std::string_view(another_name)),
                                stop_name_to_stop_.at(std::string_view(name))};

        return all_distances_.at(tmp2);
    }

}

int TransportCatalogue::CalculateTrueRouteLength(const std::string& name) {

    if (route_name_to_additional_parameters_.count(std::string_view(name)) != 0) {
        RouteAdditionalParameters& tmp = *route_name_to_additional_parameters_.at(std::string_view(name));

        for (auto i = 0 ; i < tmp.route_ptr->stops.size() - 1 ; ++i) {

            tmp.true_route_length += GetDistance(tmp.route_ptr->stops[i]->name,
                                                  tmp.route_ptr->stops[i + 1]->name);

        }
        return tmp.true_route_length;
    }

    return 0;
}

    TransportCatalogue::RouteAdditionalParameters::RouteAdditionalParameters(Route * ptr)
        : route_ptr(ptr)
    {
    }

} // end of namespace: transport_catalogue
