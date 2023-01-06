#pragma once

#include <memory>
#include <deque>

#include "graph.h"
#include "router.h"
#include "ranges.h"
#include "domain.h"

#include <optional>

namespace transport_catalogue {
    using namespace domain;
    using namespace graph;

    struct PathInfo {
        const Route* route_ptr;
        const Stop* from;
        int span;
    };

    struct RouterSettings {
        double bus_wait_time_ = 0.0;
        double bus_velocity_ = 0.0;
    };

class TransportRouter {
public:

    using DistanceMap = std::unordered_map<StopPtrPair, int, StopPtrHash, StopPtrPairEqualKey>;

    TransportRouter(RouterSettings&& settings, const DistanceMap& distances, const std::deque<Route>& routes, size_t vertex_count)
        : settings_(std::move(settings)), distances_(distances), routes_(routes), graph_(vertex_count)
    {
        AutoFillGraph(vertex_count);
    }

    TransportRouter(RouterSettings&& settings, const DistanceMap& distances,
                    const std::deque<Route>& routes, graph::DirectedWeightedGraph<double>&& graph,
                    std::unordered_map<graph::EdgeId, transport_catalogue::PathInfo>&& all_info)
            : settings_(settings),
            distances_(distances),
            routes_(routes),
            graph_(std::move(graph)),
            edge_id_to_path_info_(std::move(all_info))
    {
    }

    std::optional<Router<double>::RouteInfo> BuildRoute(int from, int to);

    Edge<double> GetEdge(int edge_id) const {
        return graph_.GetEdge(EdgeId(edge_id));
    }

    const PathInfo& GetInfo(int edge_id) const {
        return edge_id_to_path_info_.at(edge_id);
    }

    double GetBusWaitTme() const {
        return settings_.bus_wait_time_;
    }

    const graph::DirectedWeightedGraph<double>& GetGraph() {
        return graph_;
    }

    const std::unordered_map<graph::EdgeId, PathInfo>& GetAllPathInfo() {
        return edge_id_to_path_info_;
    }

    void SetAllPathInfo(std::unordered_map<graph::EdgeId, PathInfo>&& all_info) {
        edge_id_to_path_info_ = std::move(all_info);
    }

private:
    RouterSettings settings_;
    const DistanceMap& distances_;
    const std::deque<Route>& routes_;
    graph::DirectedWeightedGraph<double> graph_;

    std::unique_ptr<graph::Router<double>> router_ = nullptr;

    std::unordered_map<graph::EdgeId, PathInfo> edge_id_to_path_info_;

    void AutoFillGraph(size_t vertex_count);

    void AddRoundEdge(const Route& route);

    void AddNonRoundEdge(const Route& route);

    auto AddEdge(int from, int to, double time) {
        return graph_.AddEdge({VertexId(from), VertexId(to), time});
    }

    double GetDistance(Stop* stop_ptr_from, Stop* stop_ptr_to) const;

    void AddInfo(int edge_id, PathInfo info) {
        edge_id_to_path_info_[EdgeId(edge_id)] = std::move(info);
    }

    template <typename It>
    double CalculateRealDistance(It from, It to) const {
        double result = 0.0;

        if (std::distance(from, to) > 0) {
            for (auto i = from; i != to; ++i) {
                result += std::abs(GetDistance(*i, *(i + 1)));
            }
        } else {
            for (auto i = from; i != to; --i) {
                result += std::abs(GetDistance(*i, *(i - 1)));
            }
        }

        return result;
    }

    template <typename It>
    void CalcAndAddEdge(const Route& route, It from, It to) {

        double real_dist = CalculateRealDistance(from, to);

        // ((meters / 1000) / (velocity km/h)) * 60 minutes_in_hour) + wait_time_in_minutes ==>
        // Coefficient from km/h to meters/minute = 0.06;
        static const double MULTIPLY_COEF = 0.06;

        double time = MULTIPLY_COEF * (real_dist / settings_.bus_velocity_) + settings_.bus_wait_time_;

        int span = std::abs(std::distance(from, to));

        auto edge_id = AddEdge((*from)->id,
                               (*to)->id,
                               time
        );

        AddInfo(edge_id, PathInfo{&route,
                                  *from,
                                  span}
        );
    }
};

} // end of namespace: transport_catalogue