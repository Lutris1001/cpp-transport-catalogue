#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <map>
#include <unordered_set>
#include <optional>
#include <memory>

#include "geo.h"
#include "domain.h"

#include "transport_router.h"

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

class TransportCatalogue {

public:

    void AddStop(const std::string& name, Coordinates map_point);

    void AddRoute(const std::string& name, const std::vector<std::string>& stops, bool is_round);

    void SetDistance(const std::string& stop_name_from, const std::string& stop_name_to, int distance);

    int GetDistance(const std::string& stop_name_from, const std::string& stop_name_to) const;

    std::map<std::string, const Route*> GetAllRoutesPtr() const;

    bool IsStopExist(const std::string_view& name) const;

    const Route* GetRoutePtr(const std::string_view& route_name) const;

    const Stop* GetStopPtr(const std::string_view& stop_name) const;

    int CalculateTrueRouteLength(const std::string& name);

    [[nodiscard]] const StopSearchResponse SearchStop(const std::string& stop_name) const;

    [[nodiscard]] const RouteSearchResponse SearchRoute(const std::string& route_name);

    [[nodiscard]] const OptimalPathSearchResponse SearchOptimalPath(const std::string& from, const std::string& to);

    bool RouterExist() const;
    void CreateRouter(RouterSettings settings);

    const std::deque<Stop>* GetConstStopsPtr() const {
        return &all_stops_;
    }
    const std::deque<Route>* GetConstRoutePtr() const {
        return &all_routes_;
    }

    const std::unordered_map<StopPtrPair, int, StopPtrHash, StopPtrPairEqualKey>* GetConstDistancesPtr() const {
        return &all_distances_;
    }

    std::unique_ptr<TransportRouter>& GetRouter();

    void CreateRouterFromProto(RouterSettings&& settings, graph::DirectedWeightedGraph<double>&& graph,
                               std::unordered_map<graph::EdgeId, transport_catalogue::PathInfo>&& all_info);

private:

    std::deque<Stop> all_stops_;
    std::unordered_map<std::string_view, Stop*> stop_name_to_stop_;
    std::unordered_map<int, Stop*> stop_id_to_stop_;

    std::deque<Route> all_routes_;
    std::unordered_map<std::string_view, Route*> route_name_to_route_;

    std::unordered_map<Stop*, std::unordered_set<Route*>> stop_name_to_route_set_;

    std::unordered_map<StopPtrPair, int, StopPtrHash, StopPtrPairEqualKey> all_distances_;

    std::deque<RouteAdditionalParameters> all_route_parameters_;
    std::unordered_map<std::string_view, RouteAdditionalParameters*> route_name_to_additional_parameters_;

    std::unique_ptr<TransportRouter> router_ = nullptr;

};

} // end of namespace: transport_catalogue