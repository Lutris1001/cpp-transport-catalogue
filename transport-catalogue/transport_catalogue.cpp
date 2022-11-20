#include <cassert>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <vector>
#include <iterator>

#include "transport_catalogue.h"
#include "geo.h"
#include "domain.h"
#include "graph.h"
#include "router.h"

namespace transport_catalogue {

using namespace domain;
using namespace std::literals;

void TransportCatalogue::AddStop(const std::string& name, Coordinates map_point) {

    assert(!name.empty()); // check name not empty

    Stop stop(name, map_point, static_cast<int>(all_stops_.size()));

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

void TransportCatalogue::AddRoute(const std::string &name, const std::vector <std::string> &stops, bool is_round) {

    assert(!name.empty()); // check number of args

    Route route(name, static_cast<int>(all_routes_.size()));

    route.is_roundtrip = is_round;

    all_routes_.push_back(std::move(route));
    Route* route_ptr = &(all_routes_.back());

    route_name_to_route_[std::string_view(route_ptr->name)] = route_ptr;

    for (const auto& i : stops) {
        route_ptr->stops.push_back(stop_name_to_stop_.at(std::string_view(i)));
    }

    for (const auto& i : route_ptr->stops) {
        stop_name_to_route_set_[i].insert(route_ptr);
    }

    all_route_parameters_.push_back(RouteAdditionalParameters(route_ptr));
    route_name_to_additional_parameters_[std::string_view(route_ptr->name)] = &all_route_parameters_.back();

}

const transport_catalogue::StopSearchResponse TransportCatalogue::SearchStop(const std::string& stop_name) const {

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

const transport_catalogue::RouteSearchResponse TransportCatalogue::SearchRoute(const std::string& route_name) {

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

const OptimalPathSearchResponse TransportCatalogue::SearchOptimalPath(const std::string& from, const std::string& to) const {

    static const OptimalPathSearchResponse dummy{{},
                                    0,
                                    false};

    if (!stop_name_to_stop_.count(from) ||
        !stop_name_to_stop_.count(to)) {
        return dummy;
    }

    graph::VertexId id_1 = stop_name_to_stop_.at(from)->id;
    graph::VertexId id_2 = stop_name_to_stop_.at(to)->id;

    auto path = graph_router_->BuildRoute(id_1,id_2);

    if (!path.has_value()) {
        return dummy;
    }

    const auto& edges = path.value().edges;

    std::vector<OptimalPathItem> items;

    double total_time = 0.0;

    for (auto edge_id : edges) {

        const auto& edge = graph_->GetEdge(edge_id);
        const PathInfo& info = edge_id_to_path_info_.at(edge_id);

        items.emplace_back(OptimalPathItem{
                "Wait"sv,
                info.from->name,
                0,
                bus_wait_time_
        });

        items.emplace_back(OptimalPathItem{
                "Bus"sv,
                info.route_ptr->name,
                info.span,
                edge.weight - bus_wait_time_
        });

        total_time += edge.weight;
    }


    return {std::move(items),
            total_time,
            true};
}

int TransportCatalogue::GetDistance(const std::string& stop_name_from, const std::string& stop_name_to) const {
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

        for (auto i = 0 ; i < static_cast<int>(params_ref.route_ptr->stops.size() - 1) ; ++i) {

            params_ref.true_route_length += GetDistance(params_ref.route_ptr->stops[i]->name,
                                                        params_ref.route_ptr->stops[i + 1]->name);

        }
        return params_ref.true_route_length;
    }

    return 0;
}

std::map<std::string, const Route*> TransportCatalogue::GetAllRoutesPtr() const {

    if (all_routes_.empty()) {
        return {};
    }

    std::map<std::string, const Route*> result;
    for (const auto& i : all_routes_) {
        result[i.name] = const_cast<Route*>(&i);
    }
    return result;
}

bool TransportCatalogue::IsStopExist(const std::string_view& name) const {
    return stop_name_to_stop_.count(name) != 0;
}

const Route* TransportCatalogue::GetRoutePtr(const std::string_view& route_name) const {
    return const_cast<Route*>(route_name_to_route_.at(route_name));
}

void TransportCatalogue::SetBusWaitTime(int time) {
    bus_wait_time_ = time;
}

void TransportCatalogue::SetBusVelocity(double velocity) {
    bus_velocity_ = velocity;
}

void TransportCatalogue::FillGraph(graph::DirectedWeightedGraph<double>* graph_ptr) {

    graph_ = graph_ptr;

    for (const auto& route : all_routes_) {
        route.is_roundtrip ? AddRoundRouteEdge(route) :
                AddNonRoundRouteEdge(route);
    }

}

void TransportCatalogue::AddRoundRouteEdge(const Route& route) {

    for (auto from_it = route.stops.begin(); from_it != route.stops.end(); ++from_it) {

        for (auto to_it = from_it + 1; to_it != route.stops.end(); ++to_it) {

            AddOneRouteEdge(route, from_it, to_it);
        }
    }
}

void TransportCatalogue::AddNonRoundRouteEdge(const Route& route) {

    auto middle_it = route.stops.begin() + (route.stops.size() / 2) == route.stops.end() ? route.stops.end() :
            route.stops.begin() + (route.stops.size() / 2) + 1 ;


    for (auto from_it = route.stops.begin(); from_it != middle_it; ++from_it) {
        for (auto to_it = route.stops.begin(); to_it != middle_it; ++to_it) {
            AddOneRouteEdge(route, from_it, to_it);
        }
    }

    for (auto from_it= middle_it - 1; from_it != route.stops.end(); ++from_it) {
        for (auto to_it = middle_it; to_it != route.stops.end(); ++to_it) {
            AddOneRouteEdge(route, from_it, to_it);
        }
    }
}

    size_t TransportCatalogue::GetStopsCount() {
        return all_stops_.size();
    }

    void TransportCatalogue::SetRouter(graph::Router<double>* router_ptr) {
        graph_router_ = router_ptr;
    }

} // end of namespace: transport_catalogue