#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <map>
#include <unordered_set>
#include <optional>

#include "geo.h"
#include "domain.h"

#include "graph.h"
#include "router.h"

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

struct OptimalPathItem {
    std::string_view type;
    std::string_view name;
    int span_count = 0;
    double time = 0.0;
};

struct OptimalPathSearchResponse {
    std::vector<OptimalPathItem> items;
    double total_time;
    bool is_found;

};

struct PathInfo {
    const Route* route_ptr;
    const Stop* from;
    int span;
};

class TransportCatalogue {

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
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};

public:

    void AddStop(const std::string& name, Coordinates map_point);

    void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_round);

    void AddDistance(const std::string& stop_name_from, const std::string& stop_name_to, int distance);

    int GetDistance(const std::string& stop_name_from, const std::string& stop_name_to) const;

    std::map<std::string, const Route*> GetAllRoutesPtr() const;

    bool IsStopExist(const std::string_view& name) const;

    const Route* GetRoutePtr(const std::string_view& route_name) const;

    int CalculateTrueRouteLength(const std::string& name);

    [[nodiscard]] const StopSearchResponse SearchStop(const std::string& stop_name) const;

    [[nodiscard]] const RouteSearchResponse SearchRoute(const std::string& route_name);

    void SetBusWaitTime(int time);
    void SetBusVelocity(double velocity);

    [[nodiscard]] const OptimalPathSearchResponse SearchOptimalPath(const std::string& from, const std::string& to) const;

    void FillGraph(graph::DirectedWeightedGraph<double>* graph_ptr);

    size_t GetStopsCount();

    void SetRouter(graph::Router<double>* router_ptr);

private:

    std::deque<Stop> all_stops_;
    std::unordered_map<std::string_view, Stop*> stop_name_to_stop_;

    std::deque<Route> all_routes_;
    std::unordered_map<std::string_view, Route*> route_name_to_route_;

    std::unordered_map<Stop*, std::unordered_set<Route*>> stop_name_to_route_set_;

    std::unordered_map<StopPtrPair, int, StopPtrHash, StopPtrPairEqualKey> all_distances_;

    std::deque<RouteAdditionalParameters> all_route_parameters_;
    std::unordered_map<std::string_view, RouteAdditionalParameters*> route_name_to_additional_parameters_;

    graph::DirectedWeightedGraph<double>* graph_ = nullptr;
    graph::Router<double>* graph_router_ = nullptr;

    std::unordered_map<graph::EdgeId, PathInfo> edge_id_to_path_info_;

    double bus_wait_time_ = 0.0;
    double bus_velocity_ = 0.0;

    void AddRoundRouteEdge(const Route& route);

    void AddNonRoundRouteEdge(const Route& route);

    template <typename It>
    double CalculateRealDistance(It from, It to) const {
        double result = 0.0;

        if (std::distance(from, to) > 0) {
            for (auto i = from; i != to; ++i) {
                result += std::abs(GetDistance((*i)->name, (*(i + 1))->name));
            }
        } else {
            for (auto i = from; i != to; --i) {
                result += std::abs(GetDistance((*i)->name, (*(i - 1))->name));
            }
        }

        return result;
    }

    template <typename It>
    void AddOneRouteEdge(const Route& route, It from, It to) {

        double real_dist = CalculateRealDistance(from, to);

        double time = (real_dist * 3) / (50 * bus_velocity_)  + bus_wait_time_;

        int span = std::abs(std::distance(from, to));

        auto edge_id = graph_->AddEdge({
                                               graph::VertexId((*from)->id),
                                               graph::VertexId((*to)->id),
                                               time
                                       });

        edge_id_to_path_info_[edge_id] = PathInfo{&route,
                                                  &(*(*from)),
                                                  span
        };
    }

};

} // end of namespace: transport_catalogue