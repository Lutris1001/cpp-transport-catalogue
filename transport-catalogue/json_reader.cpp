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

JsonReader::JsonReader(TransportCatalogue* ptr) 
    : catalogue_ptr_(ptr)
{
}

std::istream& JsonReader::ReadJSON(std::istream& input) {
    all_objects_ = Load(input);
    return input;
}
    
void JsonReader::FillCatalogue() {
    using namespace std::literals;
    
    const auto& base_requests = all_objects_.GetRoot().AsMap().at("base_requests"); // Array;
    
    for (const auto& i : base_requests.AsArray()) {
        const auto& request = i.AsMap();
        if (request.at("type"s) == "Stop"s) {
            catalogue_ptr_->AddStop(request.at("name"s).AsString(),
                                    Coordinates{request.at("latitude"s).AsDouble(),
                                                request.at("longitude"s).AsDouble()});
        }
    }

    for (const auto& i : base_requests.AsArray()) {
        const auto& request = i.AsMap();
        if (request.at("type"s) == "Stop"s) {
            const auto& distances = request.at("road_distances"s).AsMap();
            for (const auto& name_to_distance : distances) {
                catalogue_ptr_->AddDistance(request.at("name"s).AsString(),
                                            name_to_distance.first,
                                            name_to_distance.second.AsInt());
            }
        }
    }

    for (const auto& i : base_requests.AsArray()) {

        const auto& request = i.AsMap();
        if (request.at("type"s) == "Bus"s) {

            if (request.at("stops"s).AsArray().empty()) {
                continue;
            }

            bool skip = false;
            for (const auto& j : request.at("stops"s).AsArray()) {
                if (!catalogue_ptr_->IsStopExist(std::string_view(j.AsString()))) {
                    skip = true;
                    break;
                }
            }

            if (skip) {
                continue;
            }

            std::vector<std::string> stop_names;

            if (request.at("is_roundtrip"s).AsBool()) {
                for (const auto& j : request.at("stops"s).AsArray()) {
                    stop_names.push_back(j.AsString());
                }
                catalogue_ptr_->AddRoute(request.at("name"s).AsString(), std::move(stop_names), true);
            } else {
                for (int j = 0; j < static_cast<int>(request.at("stops"s).AsArray().size()); ++j) {
                    stop_names.push_back(request.at("stops"s).AsArray()[j].AsString());
                }
                for (int j = static_cast<int>(request.at("stops"s).AsArray().size()) -2; j >= 0 ; --j) {
                    stop_names.push_back(request.at("stops"s).AsArray()[j].AsString());
                }
                catalogue_ptr_->AddRoute(request.at("name"s).AsString(), std::move(stop_names), false);
            }
        }
    }
}
    
void JsonReader::ProcessRequests() {
    using namespace std::literals;
    
    const auto& stat_requests = all_objects_.GetRoot().AsMap().at("stat_requests"s); // Array;
    
    for (const auto& i : stat_requests.AsArray()) {
        const auto& request = i.AsMap();
        if (request.at("type"s) == "Stop"s) {
            auto response = catalogue_ptr_->SearchStop(request.at("name"s).AsString());
            
            if (response.is_found) {
                
                Array routes;
                
                for (const auto& j : response.route_names_at_stop) {
                    routes.push_back(Node(static_cast<std::string>(j)));
                }
                
                responses_.push_back(Dict{{"buses"s, routes},
                                          {"request_id"s, Node(request.at("id"s))}});
                
            } else {

                responses_.push_back(Dict{{"request_id"s, Node(request.at("id"s))},
                                          {"error_message"s, Node("not found"s)}});
                
            }
        }
        
        if (request.at("type"s) == "Bus"s) {
            auto response = catalogue_ptr_->SearchRoute(request.at("name"s).AsString());
            
            if (response.is_found) {
                
                responses_.push_back(Dict{{"curvature"s, Node(response.true_route_length/response.geo_route_length)},
                                          {"request_id"s, Node(request.at("id"s))},
                                          {"route_length"s, Node(double(response.true_route_length))},
                                          {"stop_count"s, Node(static_cast<int>(response.route_size))},
                                          {"unique_stop_count"s, Node(static_cast<int>(response.unique_stops))}});

            } else {
                
                responses_.push_back(Dict{{"request_id"s, Node(request.at("id"s))},
                                          {"error_message"s, Node("not found"s)}});

            }
        }

        if (request.at("type"s) == "Map"s) {

            MapRenderer renderer(catalogue_ptr_);

            std::stringstream output;

            renderer.ReadSettings(this);

            renderer.GetCompliteMap(output);

            std::string map_as_string = output.str();

            responses_.push_back(Dict{{"map"s, Node(map_as_string)},
                                      {"request_id"s, Node(request.at("id"s))}});

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
