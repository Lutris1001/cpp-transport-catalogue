#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "json.h"
#include "transport_catalogue.h"
#include "geo.h"

using namespace domain;

class JsonReader {
public:

explicit JsonReader(transport_catalogue::TransportCatalogue* ptr);

std::istream& ReadJSON(std::istream& input);

const json::Document& GetJSONDocument() const;
    
void FillCatalogue();
    
void ProcessRequests();

std::ostream& PrintResponses(std::ostream& output);
    
private:
    json::Document all_objects_ = json::Document(json::Node());
    transport_catalogue::TransportCatalogue* catalogue_ptr_;
    json::Array responses_;
};







