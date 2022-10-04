#include "json_reader.h"

#include <iostream>
#include <string>
#include <vector>
#include "json.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "map_renderer.h"

using namespace json;
using namespace transport_catalogue;
using namespace std::literals;

JsonReader::JsonReader(TransportCatalogue* ptr) 
    : catalogue_ptr_(ptr)
{
}

std::istream& JsonReader::ReadJSON(std::istream& input) {
    all_objects_ = Load(input);
    return input;
}
    
void JsonReader::FillCatalogue() {

    AddAllStops();
    AddAllDistances();
    AddAllRoutes();

}
    
void JsonReader::ProcessRequests() {
    
    const auto& stat_requests = all_objects_.GetRoot().AsDict().at("stat_requests"s).AsArray();
    
    for (const auto& i : stat_requests) {
        const auto& request = i.AsDict();

        if (request.at("type"s).AsString() == "Stop"s) {
            ProcessStopRequest(request);
        }
        
        if (request.at("type"s).AsString() == "Bus"s) {
            ProcessRouteRequest(request);
        }

        if (request.at("type"s).AsString() == "Map"s) {
            ProcessMapRequest(request);
        }
    }
}

std::ostream& JsonReader::PrintResponses(std::ostream& output) {

    Document responses_doc(responses_);
    Print(responses_doc, output);
    return output;

}

const json::Document& JsonReader::GetJSONDocument() const {
    return all_objects_;
}

void JsonReader::AddOneStop(const Dict& request) {
    catalogue_ptr_->AddStop(request.at("name"s).AsString(),
                            Coordinates{request.at("latitude"s).AsDouble(),
                            request.at("longitude"s).AsDouble()});
}

void JsonReader::AddAllStops() {

    const auto& base_requests = all_objects_.GetRoot().AsDict().at("base_requests"s).AsArray();

    for (const auto& i : base_requests) {
        const auto& request = i.AsDict();
        if (request.at("type"s).AsString() == "Stop"s) {
            AddOneStop(request);
        }
    }
}

void JsonReader::AddOneDistance(const Dict& request) {

    const auto& distances = request.at("road_distances"s).AsDict();
    for (const auto& name_to_distance : distances) {

        catalogue_ptr_->AddDistance(request.at("name"s).AsString(),
                                    name_to_distance.first,
                                    name_to_distance.second.AsInt());

    }
}

void JsonReader::AddAllDistances() {

    const auto& base_requests = all_objects_.GetRoot().AsDict().at("base_requests"s).AsArray();

    for (const auto& i : base_requests) {
        const auto& request = i.AsDict();
        if (request.at("type"s).AsString() == "Stop"s) {
            AddOneDistance(request);
        }
    }
}

void JsonReader::AddOneRoute(const Dict& request) {

    // Check if route is not empty
    if (request.at("stops"s).AsArray().empty()) {
        return;
    }

    // Check if all stops in route exists
    for (const auto& j : request.at("stops"s).AsArray()) {
        if (!catalogue_ptr_->IsStopExist(std::string_view(j.AsString()))) {
            return;
        }
    }

    std::vector<std::string> stop_names;

    if (request.at("is_roundtrip"s).AsBool()) {
        for (const auto& j : request.at("stops"s).AsArray()) {
            stop_names.push_back(j.AsString());
        }
        catalogue_ptr_->AddRoute(request.at("name"s).AsString(), stop_names, true);
    } else {
        for (int j = 0; j < static_cast<int>(request.at("stops"s).AsArray().size()); ++j) {
            stop_names.push_back(request.at("stops"s).AsArray()[j].AsString());
        }
        for (int j = static_cast<int>(request.at("stops"s).AsArray().size()) -2; j >= 0 ; --j) {
            stop_names.push_back(request.at("stops"s).AsArray()[j].AsString());
        }
        catalogue_ptr_->AddRoute(request.at("name"s).AsString(), stop_names, false);
    }

}

void JsonReader::AddAllRoutes() {

    const auto& base_requests = all_objects_.GetRoot().AsDict().at("base_requests"s).AsArray();

    for (const auto& i : base_requests) {

        const auto& request = i.AsDict();

        if (request.at("type"s).AsString() == "Bus"s) {
            AddOneRoute(request);
        }
    }
}

void JsonReader::ProcessStopRequest(const Dict& request) {

    auto response = catalogue_ptr_->SearchStop(request.at("name"s).AsString());

    if (response.is_found) {

        Array routes;

        for (const auto& i : response.route_names_at_stop) {
            routes.push_back(Node(static_cast<std::string>(i)));
        }

        //responses_.push_back(Dict{{"buses"s, routes},
                                  //{"request_id"s, Node(request.at("id"s))}});

        responses_.push_back(Builder{}
                            .StartDict()
                            .Key("buses"s).Value(routes)
                            .Key("request_id"s).Value(Node(request.at("id"s)).GetValue())
                            .EndDict().Build());
        return;
    }

//    responses_.push_back(Dict{{"request_id"s, Node(request.at("id"s))},
//                              {"error_message"s, Node("not found"s)}});

    responses_.push_back(Builder{}
                    .StartDict()
                    .Key("request_id"s).Value(Node(request.at("id"s)).GetValue())
                    .Key("error_message"s).Value(Node("not found"s).GetValue())
                    .EndDict().Build());
}

void JsonReader::ProcessRouteRequest(const Dict& request) {

    auto response = catalogue_ptr_->SearchRoute(request.at("name"s).AsString());

    if (response.is_found) {

//        responses_.push_back(Dict{{"curvature"s, Node(response.true_route_length/response.geo_route_length)},
//                                  {"request_id"s, Node(request.at("id"s))},
//                                  {"route_length"s, Node(double(response.true_route_length))},
//                                  {"stop_count"s, Node(static_cast<int>(response.route_size))},
//                                  {"unique_stop_count"s, Node(static_cast<int>(response.unique_stops))}});

        responses_.push_back(Builder{}
                        .StartDict()
                        .Key("curvature"s).Value(Node(response.true_route_length/response.geo_route_length).GetValue())
                        .Key("request_id"s).Value(Node(request.at("id"s)).GetValue())
                        .Key("route_length"s).Value(Node(double(response.true_route_length)).GetValue())
                        .Key("stop_count"s).Value(Node(static_cast<int>(response.route_size)).GetValue())
                        .Key("unique_stop_count"s).Value(Node(static_cast<int>(response.unique_stops)).GetValue())
                        .EndDict().Build());


        return;
    }

//    responses_.push_back(Dict{{"request_id"s, Node(request.at("id"s))},
//                              {"error_message"s, Node("not found"s)}});

    responses_.push_back(Builder{}
                    .StartDict()
                    .Key("request_id"s).Value(Node(request.at("id"s)).GetValue())
                    .Key("error_message"s).Value(Node("not found"s).GetValue())
                    .EndDict().Build());

}

void JsonReader::ProcessMapRequest(const Dict& request) {

    using namespace renderer;

    MapRenderer renderer(catalogue_ptr_);

    std::stringstream output;

    renderer.ReadSettings(this);

    renderer.GetCompleteMap(output);

    std::string map_as_string = output.str();

//    responses_.push_back(Dict{{"map"s, Node(map_as_string)},
//                              {"request_id"s, Node(request.at("id"s))}});

    responses_.push_back(Builder{}
                                 .StartDict()
                                 .Key("map"s).Value(Node(map_as_string).GetValue())
                                 .Key("request_id"s).Value(Node(request.at("id"s)).GetValue())
                                 .EndDict().Build());

}