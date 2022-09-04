
#include "transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <set>

namespace transport_catalogue {

StatParser::StatParser(TransportCatalogue* ptr)
    : catalogue_ptr_(ptr)
    {
    }

std::vector<StatParser::Query> StatParser::ReadFromStream(std::istream &input) {
    std::string count;
    std::getline(input, count);
    std::vector<StatParser::Query> result;
    result.resize(std::stoi(count));
    for (int i = 0; i < std::stoi(count); ++i ) {
        std::string tmp;
        std::getline(input, tmp);
        result[i] =  std::move(ParseLine(std::string_view(tmp)));
    }
    return result;
}

void StatParser::ProcessOneQuery(StatParser::Query query) {
    if (query.query_type == QueryType::SEARCH_ROUTE) {
        PrintRoute(query.query_body[0]);
    } else {
        PrintStop(query.query_body[0]);
    }
}

std::istream& StatParser::ProcessQueries(std::istream &input) {
    auto queries = ReadFromStream(input);

    for (auto i : queries) {
        ProcessOneQuery(i);
    }

    return input;
}

StatParser::Query StatParser::ParseLine(std::string_view line) {

    line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));
    auto pos_end = line.find(' ');
    std::string query_type = std::string(line.substr(0, pos_end));

    Query query;

    if (query_type == "Stop") {
        query.query_type = QueryType::SEARCH_STOP;
    } else {
        query.query_type = QueryType::SEARCH_ROUTE;
    }

    line.remove_prefix(std::min(line.find_first_not_of(' ', pos_end), line.size()));

    auto pos = std::min(line.find_last_not_of(' '), line.size());

    std::string name = std::string(line.substr(0, pos + 1));

    query.query_body.push_back(std::string(name));

    return query;
}

void StatParser::PrintRoute(const std::string& name) const {

    // Output example:
    // Bus 751: not found
    // Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature

    const auto result = catalogue_ptr_->SearchRoute(name);

    if (!result.is_found) {
        std::cout << "Bus " << std::string(result.name) << ": not found" << std::endl;
        return;
    } else {
        std::cout << "Bus " << std::string(result.name) << ": "
        << result.route_size  << " stops on route, " <<
        result.unique_stops << " unique stops, " << std::setprecision(6) << result.true_route_length <<
        " route length, "<< double(result.true_route_length / result.geo_route_length) << " curvature"<< std::endl;
    }

}


void StatParser::PrintStop(const std::string& name) const {

    // Output example:
    //  Stop Samara: not found
    //  Stop Prazhskaya: no buses
    //  Stop Biryulyovo Zapadnoye: buses 256 828

    const auto result = catalogue_ptr_->SearchStop(name);

    if (result.is_found && !result.route_names_at_stop.empty()) {
        std::cout << "Stop " << std::string(result.name) << ": buses";

        for (auto i : result.route_names_at_stop) {
            std::cout << ' ' << std::string(i);
        }
        std::cout << std::endl;

    }
    if (result.is_found && result.route_names_at_stop.empty()) {

        std::cout << "Stop " << std::string(result.name) <<  ": no buses" << std::endl;
        return;

    }
    if (!result.is_found) {
        std::cout << "Stop " << std::string(result.name) << ": not found" << std::endl;
        return;
    }

}

} // end of namespace: transport_catalogue
