#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "map_renderer.h"

using namespace domain;
using namespace renderer;

class JsonReader {
public:

explicit JsonReader(transport_catalogue::TransportCatalogue* ptr);

std::istream& ReadJSON(std::istream& input);

const json::Document& GetJSONDocument() const;
    
void FillCatalogue();
    
void ProcessRequests();

std::ostream& PrintResponses(std::ostream& output);

void SetRenderSettings(Settings&& settings);
    
private:
    json::Document all_objects_ = json::Document(json::Node());
    transport_catalogue::TransportCatalogue* catalogue_ptr_ = nullptr;
    json::Array responses_;
    std::optional<MapRenderer> renderer_ = std::nullopt;

    void AddOneStop(const json::Dict& request);
    void AddAllStops();

    void AddOneDistance(const json::Dict& request);
    void AddAllDistances();

    void AddOneRoute(const json::Dict& request);
    void AddAllRoutes();

    void ProcessStopRequest(const json::Dict& request);
    void ProcessRouteRequest(const json::Dict& request);
    void ProcessMapRequest(const json::Dict& request);

    void ProcessOptimalPathRequest(const json::Dict& request);
    void SetRoutingSettings(const json::Dict& routing_settings) const;
};







