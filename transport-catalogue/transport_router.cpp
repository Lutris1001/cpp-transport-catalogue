
#include <optional>

#include "domain.h"
#include "transport_router.h"

namespace transport_catalogue {

    std::optional<Router<double>::RouteInfo> TransportRouter::BuildRoute(int from, int to) {
        if (router_ == nullptr) {
            router_ = std::make_unique<Router<double>>(Router<double>{graph_});
        }

        return router_->BuildRoute(VertexId(from), VertexId(to));
    }

    void TransportRouter::AutoFillGraph(size_t vertex_count) {

        for (const auto& route : routes_) {
            route.is_roundtrip ? AddRoundEdge(route) :
            AddNonRoundEdge(route);
        }

    }

    double TransportRouter::GetDistance(Stop* stop_ptr_from, Stop* stop_ptr_to) const {
        StopPtrPair ptr_pair(stop_ptr_from, stop_ptr_to);
        StopPtrPair reverse_ptr_pair(stop_ptr_to, stop_ptr_from);

        return distances_.count(ptr_pair) ? distances_.at(ptr_pair) : distances_.at(reverse_ptr_pair);
    }

    void TransportRouter::AddRoundEdge(const Route& route) {

        for (auto from_it = route.stops.begin(); from_it != route.stops.end(); ++from_it) {

            for (auto to_it = from_it + 1; to_it != route.stops.end(); ++to_it) {

                CalcAndAddEdge(route, from_it, to_it);
            }
        }
    }

    void TransportRouter::AddNonRoundEdge(const Route& route) {

        auto middle_it = route.stops.begin() + (route.stops.size() / 2) == route.stops.end() ? route.stops.end() :
                         route.stops.begin() + (route.stops.size() / 2) + 1 ;


        for (auto from_it = route.stops.begin(); from_it != middle_it; ++from_it) {
            for (auto to_it = route.stops.begin(); to_it != middle_it; ++to_it) {
                CalcAndAddEdge(route, from_it, to_it);
            }
        }

        for (auto from_it= middle_it - 1; from_it != route.stops.end(); ++from_it) {
            for (auto to_it = middle_it; to_it != route.stops.end(); ++to_it) {
                CalcAndAddEdge(route, from_it, to_it);
            }
        }
    }

}



