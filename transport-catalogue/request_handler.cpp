#include "request_handler.h"

#include <unordered_set>
#include <optional>
#include "transport_catalogue.h"
#include "map_renderer.h"


RequestHandler::RequestHandler(TransportCatalogue* catalogue_ptr, renderer::MapRenderer* renderer_ptr)
    : catalogue_(catalogue_ptr), renderer_(renderer_ptr)
{
}

std::optional<RouteSearchResponse> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    auto response = catalogue_->SearchRoute(std::string(bus_name));
    if (response.is_found) {
        return response;
    }
    return {};
}

const std::unordered_set<const Route*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    auto responce = catalogue_->SearchStop(std::string(stop_name));
    std::unordered_set<const Route*> result;
    for (auto i : responce.route_names_at_stop) {
        result.insert(catalogue_->GetRoutePtr(i));
    }
    return result;
}

const svg::Document& RequestHandler::RenderMap() const {
    return renderer_->GetMapDocumentRef();
}