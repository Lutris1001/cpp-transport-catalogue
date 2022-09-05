#pragma once

#include <optional>
#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace transport_catalogue;
using namespace renderer;

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(TransportCatalogue* catologue_ptr, MapRenderer* renderer_ptr);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<RouteSearchResponse> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<const Route*> GetBusesByStop(const std::string_view& stop_name) const;

    const svg::Document& RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    TransportCatalogue* catalogue_;
    renderer::MapRenderer* renderer_;
};
