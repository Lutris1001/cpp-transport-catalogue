
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

#include "transport_catalogue.h"

#include "transport_catalogue.pb.h"

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;
using namespace json;

namespace serial_database {

    proto_serialize::Stop SerializeStop(const Stop& stop) {

        proto_serialize::Stop proto_stop;
        proto_stop.set_name(stop.name);
        proto_stop.set_latitude(stop.map_point.lat);
        proto_stop.set_longitude(stop.map_point.lng);

        return std::move(proto_stop);
    }

    proto_serialize::Distance SerializeDistance(const std::string& from, const std::string& to, int dist) {

        proto_serialize::Distance proto_distance;
        proto_distance.set_stop_name_from(from);
        proto_distance.set_stop_name_to(to);
        proto_distance.set_distance(dist);

        return std::move(proto_distance);
    }

    proto_serialize::Route SerializeRoute(const Route& route) {

        proto_serialize::Route proto_route;
        proto_route.set_name(route.name);
        proto_route.set_is_roundtrip(route.is_roundtrip);

        if (!route.is_roundtrip) {

            for (auto i = route.stops.begin(); i < route.stops.begin() + route.stops.size() / 2 + 1; ++i) {
                proto_route.add_stop_name((*i)->name);
            }

            return std::move(proto_route);
        }

        for (const auto& stop : route.stops) {
            proto_route.add_stop_name(stop->name);
        }

        return std::move(proto_route);
    }

    proto_serialize::Color SerializeColor(const Node& json_color) {
        proto_serialize::Color color;

        if (json_color.IsString()) {
            color.set_is_string(true);
            color.set_is_transparent(false);
            color.set_string_color(json_color.AsString());
            return color;
        }

        if (json_color.AsArray().size() == 4) {
            color.set_is_transparent(true);
            color.set_is_string(false);
            color.set_r(json_color.AsArray().at(0).AsInt());
            color.set_g(json_color.AsArray().at(1).AsInt());
            color.set_b(json_color.AsArray().at(2).AsInt());
            color.set_a(json_color.AsArray().at(3).AsDouble());
            return color;
        }

        color.set_is_string(false);
        color.set_is_transparent(false);
        color.set_r(json_color.AsArray().at(0).AsInt());
        color.set_g(json_color.AsArray().at(1).AsInt());
        color.set_b(json_color.AsArray().at(2).AsInt());
        return color;
    }

    proto_serialize::RoutingSetting SerialRoutingSetting(const transport_catalogue::RouterSettings& settings) {
        proto_serialize::RoutingSetting serial_settings;
        serial_settings.set_bus_velocity_(settings.bus_velocity_);
        serial_settings.set_bus_wait_time_(settings.bus_wait_time_);
        return serial_settings;
    }

    proto_serialize::RenderSetting SerialRenderSetting(const Dict& json_settings) {

        proto_serialize::RenderSetting settings;

        settings.set_width(json_settings.at("width"s).AsDouble());
        settings.set_height(json_settings.at("height"s).AsDouble());

        settings.set_padding(json_settings.at("padding"s).AsDouble());

        settings.set_line_width(json_settings.at("line_width"s).AsDouble());
        settings.set_stop_radius(json_settings.at("stop_radius"s).AsDouble());

        settings.set_bus_label_font_size(json_settings.at("bus_label_font_size"s).AsInt());

        for (const auto& i : json_settings.at("bus_label_offset"s).AsArray()) {
            settings.add_bus_label_offset(i.AsDouble());
        }

        settings.set_stop_label_font_size(json_settings.at("stop_label_font_size"s).AsInt());

        for (const auto& i : json_settings.at("stop_label_offset"s).AsArray()) {
            settings.add_stop_label_offset(i.AsDouble());
        }

        *settings.mutable_underlayer_color_r() = SerializeColor(
                json_settings.at("underlayer_color"s));
        settings.set_underlayer_width(json_settings.at("underlayer_width"s).AsDouble());

        for (const auto& i : json_settings.at("color_palette"s).AsArray()) {
            *settings.add_color_palette() = SerializeColor(i);
        }

        return settings;
    }

    proto_serialize::TransportCatalogue SerializeCatalogueData(const transport_catalogue::TransportCatalogue& catalogue) {

        proto_serialize::TransportCatalogue full_data;

        const auto all_stops_ptr = catalogue.GetConstStopsPtr();

        for (const auto &stop: *all_stops_ptr) {
            *full_data.add_all_stops() = SerializeStop(stop);
        }

        const auto all_distances_ptr = catalogue.GetConstDistancesPtr();

        for (const auto &[pair, dist]: *all_distances_ptr) {
            *full_data.add_all_distances() = SerializeDistance(pair.first->name, pair.second->name, dist);
        }

        const auto all_routes_ptr = catalogue.GetConstRoutePtr();

        for (const auto &route: *all_routes_ptr) {
            *full_data.add_all_routes() = SerializeRoute(route);
        }

        return full_data;
    }

    transport_catalogue::RouterSettings RouterSettingsFromJSON(const Dict& json_settings) {
        transport_catalogue::RouterSettings router_settings;

        router_settings.bus_velocity_ = json_settings.at("bus_velocity"s).AsDouble();
        router_settings.bus_wait_time_ = json_settings.at("bus_wait_time"s).AsDouble();

        return router_settings;
    }

    proto_serialize::Router SerialRouter(transport_catalogue::TransportCatalogue& catalogue,
                                                       proto_serialize::RoutingSetting&& settings) {

        proto_serialize::Router router;

        auto& ref = catalogue.GetRouter();

        *router.mutable_routing_settings() = std::move(settings);

        assert(ref.get() != nullptr);

        for (const auto& e : ref.get()->GetGraph().GetEdgesRef()) {
            proto_serialize::Edge edge;
            edge.set_vertex_id_from(e.from);
            edge.set_vertex_id_to(e.to);
            edge.set_weight(e.weight);
            *router.add_edges() = edge;
        }

        for (const auto& e : ref.get()->GetGraph().GetIncidenceListsRef()) {
            proto_serialize::IncidenceList list;

            for (const auto& e_2 : e) {
                list.add_edge_id(e_2);
            }

            *router.add_incidence_lists() = list;
        }

        for (const auto& [key, value] : ref.get()->GetAllPathInfo()) {
            proto_serialize::PathInfo info;
            info.set_edge_id(key);
            info.set_route_name(value.route_ptr->name);
            info.set_stop_name(value.from->name);
            info.set_span(value.span);

            *router.add_all_info() = std::move(info);
        }

        return router;
    }

    bool MakeBase(std::istream& input) {

        // Initialization part :
        transport_catalogue::TransportCatalogue catalogue;

        JsonReader reader(&catalogue);

        reader.ReadJSON(input);

        reader.FillCatalogue();

        // Filling proto object :
        auto doc = reader.GetJSONDocument();

        auto file_name = doc.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();

        auto routing_settings = RouterSettingsFromJSON(
                doc.GetRoot().AsDict().at("routing_settings"s).AsDict());

        catalogue.CreateRouter(routing_settings);

        auto data = SerializeCatalogueData(catalogue);

        auto render_settings = SerialRenderSetting(
                doc.GetRoot().AsDict().at("render_settings"s).AsDict());

        auto serial_routing_settings = SerialRoutingSetting(routing_settings);

        auto serial_router = SerialRouter(catalogue, std::move(serial_routing_settings));

        *data.mutable_router() = std::move(serial_router);

        *data.mutable_render_settings() = std::move(render_settings);


        // Write to file part :
        try {
            std::ofstream out_file(file_name, std::ios::binary);
            if (!out_file) {
                throw std::runtime_error("Out_file_open_error"s);
            }

            // structure of serial file:
//            TransportCatalogue:
//                  RenderSetting;
//                  Router router;
//                  repeated Stop;
//                  repeated Distance;
//                  repeated Route;
//            }

            // only one write sys calling;
            data.SerializePartialToOstream(&out_file);

            return true;
        }
        catch (const std::runtime_error& err) {
            std::cerr << err.what() << std::endl;
        }

        return false;
    }

    void FillCatalogue(const proto_serialize::TransportCatalogue& data, transport_catalogue::TransportCatalogue& catalogue) {

        for (auto i = 0; i < data.all_stops_size(); ++i) {
            catalogue.AddStop(data.all_stops(i).name(),
                              {data.all_stops(i).latitude(), data.all_stops(i).longitude()});

        }

        for (auto i = 0; i < data.all_distances_size(); ++i) {
            catalogue.SetDistance(
                    data.all_distances(i).stop_name_from(),
                    data.all_distances(i).stop_name_to(),
                    data.all_distances(i).distance()
            );
        }

        for (auto i = 0; i < data.all_routes_size(); ++i) {
            std::vector<std::string> stops_str;

            for (const auto& j : data.all_routes(i).stop_name()) {
                stops_str.push_back(j);
            }

            if (!data.all_routes(i).is_roundtrip()) {
                for (auto j = data.all_routes(i).stop_name_size() - 2 ; j >= 0; --j) {
                    stops_str.push_back(data.all_routes(i).stop_name(j));
                }
            }

            catalogue.AddRoute(data.all_routes(i).name(),
                               stops_str,
                               data.all_routes(i).is_roundtrip());
        }
    }

    proto_serialize::TransportCatalogue DeserializeFile(std::istream& file) {

        proto_serialize::TransportCatalogue data;
        data.ParseFromIstream(&file);

        return data;
    }

    svg::Color DeserializeColor(const proto_serialize::Color& proto_color) {

        if (proto_color.is_string()) {
            return {proto_color.string_color()};
        }

        if (proto_color.is_transparent()) {
            return {svg::Rgba{uint8_t(proto_color.r()),
                                 uint8_t(proto_color.g()),
                                 uint8_t(proto_color.b()),
                                 double(proto_color.a())}};
        }

        return {svg::Rgb{uint8_t(proto_color.r()),
                                    uint8_t(proto_color.g()),
                                    uint8_t(proto_color.b())}};
    }

    renderer::Settings DeserializeRenderSettings(const proto_serialize::TransportCatalogue& data) {

        renderer::Settings settings;

        settings.height = data.render_settings().height();
        settings.width = data.render_settings().width();
        settings.padding = data.render_settings().padding();
        settings.line_width = data.render_settings().line_width();
        settings.stop_radius = data.render_settings().stop_radius();
        settings.bus_label_font_size = data.render_settings().bus_label_font_size();

        settings.bus_label_offset.clear();
        for (auto i : data.render_settings().bus_label_offset()) {
            settings.bus_label_offset.push_back(i);
        }

        settings.stop_label_font_size = data.render_settings().stop_label_font_size();

        settings.stop_label_offset.clear();
        for (auto i : data.render_settings().stop_label_offset()) {
            settings.stop_label_offset.emplace_back(i);
        }

        settings.underlayer_color = (DeserializeColor(data.render_settings().underlayer_color_r()));

        settings.underlayer_width = data.render_settings().underlayer_width();

        settings.color_palette.clear();
        for (const auto& i : data.render_settings().color_palette()) {
            settings.color_palette.push_back(DeserializeColor(i));
        }

        return settings;
    }

    void DeserializeRouter(transport_catalogue::TransportCatalogue& catalogue,
                           const proto_serialize::Router& proto_router) {

        graph::DirectedWeightedGraph<double> graph;

        std::vector<graph::Edge<double>> edges(proto_router.edges_size());

        for (auto i = 0; i < proto_router.edges_size(); ++i) {
            edges[i].from = proto_router.edges(i).vertex_id_from();
            edges[i].to = proto_router.edges(i).vertex_id_to();
            edges[i].weight = proto_router.edges(i).weight();
        }

        graph.SetEdges(std::move(edges));

        std::vector<std::vector<graph::EdgeId>> incidence_lists(proto_router.incidence_lists_size());

        for (auto l = 0; l < proto_router.incidence_lists_size(); ++l) {
            std::vector<graph::EdgeId> line(proto_router.incidence_lists(l).edge_id_size());

            for (auto i = 0; i < line.size(); ++i) {
                line[i] = proto_router.incidence_lists(l).edge_id(i);
            }

            incidence_lists[l] = std::move(line);
        }

        graph.SetIncidenceLists(std::move(incidence_lists));

        transport_catalogue::RouterSettings router_settings{proto_router.routing_settings().bus_wait_time_(),
                                                            proto_router.routing_settings().bus_velocity_()
                                                            };

//        ----------------------

        std::unordered_map<graph::EdgeId, transport_catalogue::PathInfo> all_info;

        for (const auto& item : proto_router.all_info()) {
            all_info[item.edge_id()] = transport_catalogue::PathInfo{
                catalogue.GetRoutePtr(item.route_name()),
                catalogue.GetStopPtr(item.stop_name()),
                int(item.span())};
        }

        catalogue.CreateRouterFromProto(std::move(router_settings), std::move(graph), std::move(all_info));

    }

    bool ProcessRequests(std::istream& input, std::ostream& output) {

        transport_catalogue::TransportCatalogue catalogue;
        JsonReader reader(&catalogue);
        reader.ReadJSON(input);

        auto doc = reader.GetJSONDocument();

        auto file_name = doc.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();

        try {

            std::ifstream in_file(file_name, std::ios::binary);
            if (!in_file) {
                throw std::runtime_error("Input_file_open_error"s);
            }

            auto proto_catalogue = DeserializeFile(in_file);

            FillCatalogue(proto_catalogue, catalogue);

            reader.SetRenderSettings(DeserializeRenderSettings(proto_catalogue));

            DeserializeRouter(catalogue, proto_catalogue.router());

            reader.ProcessRequests();
            reader.PrintResponses(output);

        }
        catch (const std::runtime_error& err) {
            std::cerr << err.what() << std::endl;
        }

        return false;
    }

} // end namespace serial_database