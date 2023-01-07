#pragma once

#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "transport_catalogue.h"

#include "transport_catalogue.pb.h"

#include "json.h"
#include "json_reader.h"
#include "json_builder.h"
#include "map_renderer.h"

using namespace std::literals;
using namespace json;

namespace serial_database {

    // ----------- Serialize functions --------------------------------

    static inline proto_catalogue::Stop SerializeStop(const Stop& stop);

    static inline proto_catalogue::Distance SerializeDistance(const std::string& from, const std::string& to, int dist);

    static inline proto_catalogue::Route SerializeRoute(const Route& route);

    static inline proto_svg::Color SerializeColor(const Node& json_color);

    static inline proto_renderer::RenderSetting SerialRenderSetting(const Dict& json_settings);

    static inline proto_catalogue::TransportCatalogue SerializeCatalogueData(
            const transport_catalogue::TransportCatalogue& catalogue);

    static inline proto_router::RoutingSetting SerialRoutingSetting(const Dict& json_settings);

    static inline proto_router::Router SerialRouter(const Dict& json_settings);

    void FillCatalogue(const proto_catalogue::TransportCatalogue& data,
                       transport_catalogue::TransportCatalogue& catalogue);

    bool MakeBase(std::istream& input);

    // ----------- Deserialize functions --------------------------------

    proto_catalogue::TransportCatalogue DeserializeFile(std::istream& file);

    renderer::Settings DeserializeRenderSettings(const proto_catalogue::TransportCatalogue& data);

    //  creates router in catalogue with alternative constructor
    void DeserializeRouter(transport_catalogue::TransportCatalogue& catalogue,
                           const proto_router::Router& proto_router);

    bool ProcessRequests(std::istream& input, std::ostream& output);

} // end namespace serial_database