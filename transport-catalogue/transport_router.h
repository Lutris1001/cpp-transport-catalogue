#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "ranges.h"
#include "domain.h"

#include <optional>

namespace transport_catalogue {
    using namespace domain;
    using namespace graph;

    template <typename Weight>
class TransportRouter {
public:

    explicit TransportRouter(size_t vertex_count)
        : graph_(vertex_count), router_(graph_)
    {
    }

    auto AddEdge(const Edge<double>& edge) {
        return graph_.AddEdge(edge);
    }

    auto GetEdge(EdgeId edge_id) const {
        return graph_.GetEdge(edge_id);
    }

    auto BuildRoute(VertexId from, VertexId to) const {
        return router_.BuildRoute(from, to);
    }

    size_t GetVertexCount() const {
        return graph_.GetVertexCount();
    }

    size_t GetEdgeCount() const {
        return graph_.GetEdgeCount();
    }

private:
    graph::DirectedWeightedGraph<Weight> graph_;
    graph::Router<Weight> router_;
};

} // end of namespace: transport_catalogue