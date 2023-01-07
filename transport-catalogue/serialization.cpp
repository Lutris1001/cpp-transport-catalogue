
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

    proto_catalogue::Stop SerializeStop(const Stop& stop) {

        proto_catalogue::Stop proto_stop;
        proto_stop.set_name(stop.name);
        proto_stop.set_latitude(stop.map_point.lat);
        proto_stop.set_longitude(stop.map_point.lng);

        return std::move(proto_stop);
    }

    proto_catalogue::Distance SerializeDistance(const std::string& from, const std::string& to, int dist) {

        proto_catalogue::Distance proto_distance;
        proto_distance.set_stop_name_from(from);
        proto_distance.set_stop_name_to(to);
        proto_distance.set_distance(dist);

        return std::move(proto_distance);
    }

    proto_catalogue::Route SerializeRoute(const Route& route) {

        proto_catalogue::Route proto_route;
        proto_route.set_name(route.name);
        proto_route.set_is_roundtrip(route.is_roundtrip);

        if (!route.is_roundtrip) {

            for (auto index = 0; index < route.stops.size() / 2 + 1; ++index) {
                proto_route.add_stop_name(route.stops.at(index)->name);
            }

            return std::move(proto_route);
        }

        for (const auto& stop : route.stops) {
            proto_route.add_stop_name(stop->name);
        }

        return std::move(proto_route);
    }

    proto_svg::Color SerializeColor(const Node& json_color) {
        proto_svg::Color color;

        if (json_color.IsString()) {
            proto_svg::StringColor string_color;
            string_color.set_string_color(json_color.AsString());
            *color.mutable_string_color() = std::move(string_color);
            return color;
        }

        if (json_color.AsArray().size() == 4) {
            proto_svg::RGBA rgba;
            rgba.set_r(json_color.AsArray().at(0).AsInt());
            rgba.set_g(json_color.AsArray().at(1).AsInt());
            rgba.set_b(json_color.AsArray().at(2).AsInt());
            rgba.set_a(json_color.AsArray().at(3).AsDouble());
            *color.mutable_rgba() = std::move(rgba);
            return color;
        }
        if (json_color.AsArray().size() == 3) {
            proto_svg::RGB rgb;
            rgb.set_r(json_color.AsArray().at(0).AsInt());
            rgb.set_g(json_color.AsArray().at(1).AsInt());
            rgb.set_b(json_color.AsArray().at(2).AsInt());
            *color.mutable_rgb() = std::move(rgb);
            return color;
        }

        return color;
    }

    proto_router::RoutingSetting SerialRoutingSetting(const transport_catalogue::RouterSettings& settings) {
        proto_router::RoutingSetting serial_settings;
        serial_settings.set_bus_velocity_(settings.bus_velocity_);
        serial_settings.set_bus_wait_time_(settings.bus_wait_time_);
        return serial_settings;
    }

    proto_renderer::RenderSetting SerialRenderSetting(const Dict& json_settings) {

        proto_renderer::RenderSetting settings;

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

    proto_catalogue::TransportCatalogue SerializeCatalogueData(const transport_catalogue::TransportCatalogue& catalogue) {

        proto_catalogue::TransportCatalogue full_data;

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

    proto_router::Router SerialRouter(transport_catalogue::TransportCatalogue& catalogue,
                                                       proto_router::RoutingSetting&& settings) {

        proto_router::Router router;

        auto& ref = catalogue.GetRouter();

        if (ref.get() == nullptr) {
            throw std::runtime_error("Unable to serialize router"s);
        }

        *router.mutable_routing_settings() = std::move(settings);

        for (const auto& edge : ref.get()->GetGraph().GetEdgesRef()) {
            proto_graph::Edge proto_edge;
            proto_edge.set_vertex_id_from(edge.from);
            proto_edge.set_vertex_id_to(edge.to);
            proto_edge.set_weight(edge.weight);
            *router.add_edges() = proto_edge;
        }

        for (const auto& line : ref.get()->GetGraph().GetIncidenceListsRef()) {
            proto_graph::IncidenceList list;

            for (const auto& edge_id : line) {
                list.add_edge_id(edge_id);
            }

            *router.add_incidence_lists() = list;
        }

        for (const auto& [edge_id, path_info] : ref.get()->GetAllPathInfo()) {
            proto_router::PathInfo info;
            info.set_edge_id(edge_id);
            info.set_route_name(path_info.route_ptr->name);
            info.set_stop_name(path_info.from->name);
            info.set_span(path_info.span);

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
        std::ofstream out_file(file_name, std::ios::binary);
        if (!out_file) {
            std::cout << "Out_file_open_error"s << std::endl;
            return false;
        }

        // only one write sys calling;
        data.SerializePartialToOstream(&out_file);

        return true;

    }

    void FillCatalogue(const proto_catalogue::TransportCatalogue& data, transport_catalogue::TransportCatalogue& catalogue) {

        for (auto index = 0; index < data.all_stops_size(); ++index) {
            catalogue.AddStop(data.all_stops(index).name(),
                              {data.all_stops(index).latitude(), data.all_stops(index).longitude()});

        }

        for (auto index = 0; index < data.all_distances_size(); ++index) {
            catalogue.SetDistance(
                    data.all_distances(index).stop_name_from(),
                    data.all_distances(index).stop_name_to(),
                    data.all_distances(index).distance()
            );
        }

        for (auto route_index = 0; route_index < data.all_routes_size(); ++route_index) {
            std::vector<std::string> stops_str;

            for (const auto& stop_name : data.all_routes(route_index).stop_name()) {
                stops_str.push_back(stop_name);
            }

            if (!data.all_routes(route_index).is_roundtrip()) {
                auto stop_index = std::max(0, int(data.all_routes(route_index).stop_name_size()) - 2);
                for ( ; stop_index >= 0; --stop_index) {
                    stops_str.push_back(data.all_routes(route_index).stop_name(stop_index));
                }
            }

            catalogue.AddRoute(data.all_routes(route_index).name(),
                               stops_str,
                               data.all_routes(route_index).is_roundtrip());
        }
    }

    proto_catalogue::TransportCatalogue DeserializeFile(std::istream& file) {

        proto_catalogue::TransportCatalogue data;
        data.ParseFromIstream(&file);

        return data;
    }

    svg::Color DeserializeColor(const proto_svg::Color& proto_color) {

        if (proto_color.has_string_color() && !proto_color.has_rgb() && !proto_color.has_rgba()) {
            return {proto_color.string_color().string_color()};
        }

        if (!proto_color.has_string_color() && !proto_color.has_rgb() && proto_color.has_rgba()) {
            return {svg::Rgba{uint8_t(proto_color.rgba().r()),
                                 uint8_t(proto_color.rgba().g()),
                                 uint8_t(proto_color.rgba().b()),
                                 double(proto_color.rgba().a())}};
        }

        if (!proto_color.has_string_color() && proto_color.has_rgb() && !proto_color.has_rgba()) {
            return {svg::Rgb{uint8_t(proto_color.rgb().r()),
                             uint8_t(proto_color.rgb().g()),
                             uint8_t(proto_color.rgb().b())}};

        }

        return svg::NoneColor;
    }

    renderer::Settings DeserializeRenderSettings(const proto_catalogue::TransportCatalogue& data) {

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
                           const proto_router::Router& proto_router) {

        graph::DirectedWeightedGraph<double> graph;

        std::vector<graph::Edge<double>> edges(proto_router.edges_size());

        for (auto index = 0; index < proto_router.edges_size(); ++index) {
            edges[index].from = proto_router.edges(index).vertex_id_from();
            edges[index].to = proto_router.edges(index).vertex_id_to();
            edges[index].weight = proto_router.edges(index).weight();
        }

        graph.SetEdges(std::move(edges));

        std::vector<std::vector<graph::EdgeId>> incidence_lists(proto_router.incidence_lists_size());

        for (auto line_index = 0; line_index < proto_router.incidence_lists_size(); ++line_index) {
            std::vector<graph::EdgeId> line(proto_router.incidence_lists(line_index).edge_id_size());

            for (auto edge_id_index = 0; edge_id_index < line.size(); ++edge_id_index) {
                line[edge_id_index] = proto_router.incidence_lists(line_index).edge_id(edge_id_index);
            }

            incidence_lists[line_index] = std::move(line);
        }

        graph.SetIncidenceLists(std::move(incidence_lists));

        transport_catalogue::RouterSettings router_settings{proto_router.routing_settings().bus_wait_time_(),
                                                            proto_router.routing_settings().bus_velocity_()
                                                            };

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

        std::ifstream in_file(file_name, std::ios::binary);
        if (!in_file) {
            std::cout << "Input_file_open_error"s << std::endl;
            return false;
        }

        auto proto_catalogue = DeserializeFile(in_file);

        FillCatalogue(proto_catalogue, catalogue);

        reader.SetRenderSettings(DeserializeRenderSettings(proto_catalogue));

        DeserializeRouter(catalogue, proto_catalogue.router());

        reader.ProcessRequests();
        reader.PrintResponses(output);

        return true;
    }

} // end namespace serial_database