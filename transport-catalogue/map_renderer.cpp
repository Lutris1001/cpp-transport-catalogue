#include "map_renderer.h"

#include <string>
#include <vector>

#include "json.h"
#include "json_reader.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

using namespace json;
using namespace std::literals;

namespace renderer {

MapRenderer::MapRenderer(transport_catalogue::TransportCatalogue* ptr)
    : catalogue_(ptr)
{
}

void MapRenderer::CreateSphereProjector() {

    std::vector<Coordinates> coords;

    for (const auto& name_to_stop_ptr : all_stops_in_routes_) {
            coords.push_back(name_to_stop_ptr.second->map_point);
    }

    projector_ = SphereProjector(coords.begin(),
                              coords.end(),
                              settings_.width,
                              settings_.height,
                              settings_.padding);

}

void MapRenderer::RenderRoutes() {

    assert(projector_);

    int color_counter = 0;

    for (const auto& name_to_route_ptr : all_routes_) {

        if (name_to_route_ptr.second->stops.empty()) {
            continue;
        }

        svg::Polyline route_map_line;

        route_map_line.SetFillColor(svg::NoneColor).
                        SetStrokeWidth(settings_.line_width).
                        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                        SetStrokeColor(settings_.color_palette[color_counter]);

        ++color_counter;
        if (color_counter == static_cast<int>(settings_.color_palette.size())) {
            color_counter = 0;
        }

        for (const auto& stop_ptr : name_to_route_ptr.second->stops) {
            route_map_line.AddPoint(projector_.value()(stop_ptr->map_point));
        }

        picture_.Add(route_map_line);

    }
}

void MapRenderer::RenderRoutesNames() {

    assert(projector_);

    int color_counter = 0;

    for (const auto& name_to_route_ptr : all_routes_) {

        if (name_to_route_ptr.second->stops.empty()) {
            continue;
        }

        auto first_stop = projector_.value()(name_to_route_ptr.second->stops.at(0)->map_point);

        picture_.Add(RouteUnderlinerPrintTemplate(settings_)
                .SetPosition(first_stop)
                .SetData(name_to_route_ptr.first));

        picture_.Add(RoutePrintTemplate(settings_)
                .SetPosition(first_stop)
                .SetFillColor(settings_.color_palette[color_counter])
                .SetData(name_to_route_ptr.first));

        if (!name_to_route_ptr.second->is_roundtrip) {

            auto end_stop = projector_.value()(name_to_route_ptr.second->stops.at(static_cast<int>(name_to_route_ptr.second->stops.size())/2)->map_point);

            if (!(first_stop == end_stop)) {

                picture_.Add(RouteUnderlinerPrintTemplate(settings_)
                        .SetPosition(end_stop)
                        .SetData(name_to_route_ptr.first));

                picture_.Add(RoutePrintTemplate(settings_)
                        .SetPosition(end_stop)
                        .SetFillColor(settings_.color_palette[color_counter])
                        .SetData(name_to_route_ptr.first));
            }

        }

        ++color_counter;
        if (color_counter == static_cast<int>(settings_.color_palette.size())) {
            color_counter = 0;
        }
    }
}

void MapRenderer::RenderStopsCircles() {

    assert(projector_);

    for (const auto& i : all_stops_in_routes_) {
        svg::Circle stop_circle;
        stop_circle.SetCenter(projector_.value()(i.second->map_point))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white"s);
        picture_.Add(stop_circle);
    }

}

void MapRenderer::RenderStopsNames() {

    assert(projector_);

    for (const auto& i : all_stops_in_routes_) {

        picture_.Add(StopUnderlinerPrintTemplate(settings_)
                             .SetPosition(projector_.value()(i.second->map_point))
                             .SetData(i.first));

        picture_.Add(StopPrintTemplate(settings_)
                             .SetPosition(projector_.value()(i.second->map_point))
                             .SetFillColor("black"s)
                             .SetData(i.first));
    }

}

std::ostream& MapRenderer::RenderRouteMap(std::ostream& output) {
    picture_.Render(output);
    return output;
}


svg::Color MapRenderer::ParseColor(const json::Node& node) {

    if (node.IsString()) {
        return {node.AsString()};
    }

    if (node.IsArray()) {

        if (static_cast<int>(node.AsArray().size()) == 4) {
            return {svg::Rgba(node.AsArray()[0].AsInt(),
                                        node.AsArray()[1].AsInt(),
                                        node.AsArray()[2].AsInt(),
                                        node.AsArray()[3].AsDouble())};
        } else if (static_cast<int>(node.AsArray().size()) == 3) {
            return {svg::Rgb(node.AsArray()[0].AsInt(),
                            node.AsArray()[1].AsInt(),
                            node.AsArray()[2].AsInt())};
        }
    }
    return svg::NoneColor;
}

void MapRenderer::Fill() {

    all_routes_ = catalogue_->GetAllRoutesPtr();

    for (const auto &name_to_route_ptr: all_routes_) {

        if (name_to_route_ptr.second->stops.empty()) {
            continue;
        }

        for (const auto i : name_to_route_ptr.second->stops) {
            all_stops_in_routes_[i->name] = const_cast<Stop*>(i);
        }
    }
}

void MapRenderer::ReadSettings(JsonReader* ptr) {
    using namespace std::literals;

    const auto& doc = ptr->GetJSONDocument();

    const auto& render_settings_ = doc.GetRoot().AsDict().at("render_settings"s).AsDict();

    settings_.width = render_settings_.at("width"s).AsDouble();
    settings_.height = render_settings_.at("height"s).AsDouble();

    settings_.padding = render_settings_.at("padding"s).AsDouble();
    assert(settings_.padding < std::min(settings_.width, settings_.height)/2);
    assert(settings_.padding > 0);

    settings_.line_width = render_settings_.at("line_width"s).AsDouble();
    settings_.stop_radius = render_settings_.at("stop_radius"s).AsDouble();

    settings_.bus_label_font_size = render_settings_.at("bus_label_font_size"s).AsInt();

    for (const auto &i: render_settings_.at("bus_label_offset"s).AsArray()) {
        settings_.bus_label_offset.push_back(i.AsDouble());
    }

    settings_.stop_label_font_size = render_settings_.at("stop_label_font_size"s).AsInt();

    for (const auto &i: render_settings_.at("stop_label_offset"s).AsArray()) {
        settings_.stop_label_offset.push_back(i.AsDouble());
    }

    settings_.underlayer_color = ParseColor(render_settings_.at("underlayer_color"s));
    settings_.underlayer_width = render_settings_.at("underlayer_width"s).AsDouble();

    for (const auto &i: render_settings_.at("color_palette"s).AsArray()) {
        settings_.color_palette.push_back(ParseColor(i));
    }
}

std::ostream& MapRenderer::GetCompleteMap(std::ostream& output) {

    Fill();
    CreateSphereProjector();
    RenderRoutes();
    RenderRoutesNames();
    RenderStopsCircles();
    RenderStopsNames();

    RenderRouteMap(output);

    return output;
}

const svg::Document& MapRenderer::GetMapDocumentRef() const {
    return picture_;
}

} // end namespace renderer