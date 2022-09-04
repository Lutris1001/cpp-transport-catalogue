#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <sstream>

#include "json.h"
#include "json_reader.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

using namespace json;
using namespace std::literals;

struct Settings {
    double width = 0;
    double height = 0;

    double padding = 0;

    double line_width = 0;
    double stop_radius = 0;

    int bus_label_font_size = 0;
    std::vector<double> bus_label_offset;

    int stop_label_font_size = 0;
    std::vector<double> stop_label_offset;

    svg::Color underlayer_color = svg::NoneColor;
    double underlayer_width = 0;

    std::vector<svg::Color> color_palette;
};

class RoutePrintTemplate : public svg::Text {
public:
    explicit RoutePrintTemplate(const Settings& settings)
    {
        SetFontFamily("Verdana"s);
        SetFontSize(settings.bus_label_font_size);
        SetFontWeight("bold"s);
        SetOffset({settings.bus_label_offset.at(0), settings.bus_label_offset.at(1)});
    }
};

class RouteUnderlinerPrintTemplate : public RoutePrintTemplate {
public:
    explicit RouteUnderlinerPrintTemplate(const Settings& settings)
        : RoutePrintTemplate(settings)
    {
        SetFillColor(settings.underlayer_color);
        SetStrokeColor(settings.underlayer_color);
        SetStrokeWidth(settings.underlayer_width);
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    }
};

class StopPrintTemplate : public svg::Text {
public:
    explicit StopPrintTemplate(const Settings& settings)
    {
        SetFontFamily("Verdana"s);
        SetFontSize(settings.stop_label_font_size);
        SetOffset({settings.stop_label_offset.at(0), settings.stop_label_offset.at(1)});
    }
};

class StopUnderlinerPrintTemplate : public StopPrintTemplate {
public:
    explicit StopUnderlinerPrintTemplate(const Settings& settings)
            : StopPrintTemplate(settings)
    {
        SetFillColor(settings.underlayer_color);
        SetStrokeColor(settings.underlayer_color);
        SetStrokeWidth(settings.underlayer_width);
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    }
};

class MapRenderer {
    
public:

    void ReadSettings(JsonReader* ptr);
    
    explicit MapRenderer(transport_catalogue::TransportCatalogue* ptr);
    
    void ProcessRoutes();

    void ProcessRoutesNames();

    void ProcessStopsCircles();

    void ProcessStopsNames();

    void CreateSphereProjector();

    void Fill();

    std::ostream& GetCompliteMap(std::ostream& output);
    
    std::ostream& RenderRouteMap(std::ostream& output);
    
private:

    Settings settings_;
    transport_catalogue::TransportCatalogue* catalogue_;
    svg::Document picture_;
    std::optional<SphereProjector> projector_ = {};

    std::map<std::string, const Route*> all_routes_;
    std::map<std::string, const Stop*> all_stops_in_routes_;

    [[nodiscard]] static svg::Color ParseColor(const json::Node& node);

};