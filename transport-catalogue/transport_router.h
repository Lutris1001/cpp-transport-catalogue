#pragma once

#include <memory>

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

class TransportRouter {
public:

    explicit TransportRouter(size_t vertex_count)
        : graph_(vertex_count)
    {
    }

    auto AddEdge(int from, int to, double time) {
        return graph_.AddEdge({VertexId(from), VertexId(to), time});
    }

    auto GetEdge(int edge_id) {
        return graph_.GetEdge(EdgeId(edge_id));
    }

    const PathInfo& GetInfo(int edge_id) const {
        return edge_id_to_path_info_.at(edge_id);
    }

    auto BuildRoute(int from, int to) {
        if (router_ == nullptr) {
            router_ = std::make_unique<Router<double>>(Router<double>{graph_});
        }

        return router_->BuildRoute(VertexId(from), VertexId(to));
    }

    void AddInfo(int edge_id, PathInfo info) {
        edge_id_to_path_info_[EdgeId(edge_id)] = std::move(info);
    }

private:
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_ = nullptr;

    std::unordered_map<graph::EdgeId, PathInfo> edge_id_to_path_info_;


};

} // end of namespace: transport_catalogue