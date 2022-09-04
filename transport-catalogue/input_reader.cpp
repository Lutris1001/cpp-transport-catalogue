#include "input_reader.h"

#include <iostream>
#include <string_view>
#include <vector>

using namespace std::literals;

namespace transport_catalogue {

InputParser::InputParser(TransportCatalogue *catalogue_ptr)
    : catalogue_ptr_(catalogue_ptr)
    {
    }

// All parsing functions writen with idea, that could be any ' ' space symbols in any position in input line

static void ParseDistances(InputParser::Request& req, std::string_view& distance_line) {

    distance_line.remove_prefix(std::min(distance_line.find_first_not_of(" ,"), distance_line.size()));

    int64_t pos = 0;

    const int64_t pos_end_ = distance_line.npos;

    while (!(distance_line.empty())) {
        int64_t space = distance_line.find(',');
        if (space == pos_end_) {

            int64_t last_digit = distance_line.find('m');

            req.req_body.push_back(std::string(distance_line.substr(0, last_digit)));

            distance_line.remove_prefix(std::min(distance_line.find_first_not_of(' ', last_digit + 1) + 2,
                                                 distance_line.size()));
            distance_line.remove_prefix(std::min(distance_line.find_first_not_of(' '), distance_line.size()));

            std::string_view two = (distance_line.substr(pos));
            two.remove_suffix(two.size() - two.find_last_not_of(' ') - 1);

            req.req_body.push_back(std::string(two));
        } else {

            std::string_view tmp = distance_line.substr(0, space);

            int64_t last_digit = tmp.find('m');

            req.req_body.push_back(std::string(tmp.substr(0, last_digit)));

            tmp.remove_prefix(distance_line.find_first_not_of(' ',
                                                              (distance_line.find_first_not_of(' ',
                                                              last_digit + 1) + 2)));
            tmp.remove_suffix(tmp.size() - tmp.find_last_not_of(' ') - 1);

            req.req_body.push_back(std::string(tmp));
        }
        distance_line.remove_prefix(std::min(distance_line.find_first_not_of(" ,", space),
                                             distance_line.size()));
    }

}

static void ParseDouble(InputParser::Request& req, std::string_view& digit_line) {

    digit_line.remove_prefix(std::min(digit_line.find_first_not_of(" "), digit_line.size()));

    int64_t pos = 0;

    const int64_t pos_end_ = digit_line.npos;

    for (int i = 0; i < 2; ++i) {
        int64_t space = digit_line.find_first_of(" ,");
        req.req_body.push_back(space == pos_end_ ? std::string(digit_line.substr(pos)) : std::string(digit_line.substr(pos, space)));
        digit_line.remove_prefix(std::min(digit_line.find_first_not_of(" ,", space), digit_line.size()));
    }

}

static void ParseRoute(InputParser::Request& req, std::string_view& route_line) {

    route_line.remove_prefix(std::min(route_line.find_first_not_of(" "), route_line.size()));

    int64_t pos = 0;

    const int64_t pos_end_ = route_line.npos;

    while (!(route_line.empty())) {
        int64_t space = route_line.find_first_of(">-");
        if (space == pos_end_) {
            std::string_view two = (route_line.substr(pos));
            two.remove_suffix(two.size() - two.find_last_not_of(' ') - 1);

            req.req_body.push_back(std::string(two));
        } else {
            std::string_view one = (route_line.substr(pos, space));
            one.remove_suffix(one.size() - one.find_last_not_of(' ') - 1);

            req.req_body.push_back(std::string(one));
        }
        route_line.remove_prefix(std::min(route_line.find_first_not_of(" >-", space), route_line.size()));
    }

}

static InputParser::RequestType ParseType(std::string_view& line) {

    std::string_view tmp;

    line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));

    auto pos = line.find(' ');

    tmp = line.substr(0, pos);

    line.remove_prefix(pos);

    if (tmp == "Stop") {
        return InputParser::RequestType::ADD_STOP;
    }
    return InputParser::RequestType::ADD_ROUTE;
}

static std::string ParseName(std::string_view& line) {

    std::string result;

    line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));

    auto pos = line.find(':');

    result = std::string(line.substr(0, pos));

    line.remove_prefix(pos + 1);

    return result;
}


InputParser::Request InputParser::ParseLine(std::string_view line) {

    Request req;

    req.req_type = ParseType(line); // Cuts Type prefix of string_view;

    req.req_body.push_back(ParseName(line)); // Cuts Name prefix of string_view;

    if (req.req_type == InputParser::RequestType::ADD_STOP) {

        ParseDouble(req, line); // Cuts Geo x, y position of string_view;
        ParseDistances(req, line); // Cuts all distances of string_view to empty;

        return req;
    }

    // If type is ADD_ROUTE

    if (line.find('-') != line.npos) {
        req.route_type = InputParser::RouteType::STRAIGHT;
    } else if (line.find('>') != line.npos) {
        req.route_type = InputParser::RouteType::CIRCLE;
    }

    ParseRoute(req, line);

    return req;
}

std::vector<InputParser::Request> InputParser::ParseFromStream(std::istream& input) {
    std::string count;
    std::getline(input, count);
    std::vector<InputParser::Request> result;
    result.resize(std::stoi(count));
    for (int i = 0; i < std::stoi(count); ++i ) {
        std::string tmp;
        std::getline(input, tmp);
        result[i] = std::move(ParseLine(std::string_view (tmp)));
    }
    return result;
}

void InputParser::ProcessRequests(const std::vector<Request>& requests) {

    // Adding stops to catalogue
    for (auto i : requests) {
        if (i.req_type == InputParser::RequestType::ADD_STOP) {
            catalogue_ptr_->AddStop(i.req_body[0],
                                    Coordinates{std::stod(i.req_body[1]),
                                                std::stod(i.req_body[2])});
        }
    }

    // Adding distances between stops to catalogue
    for (auto i : requests) {
        if (i.req_type == InputParser::RequestType::ADD_STOP) {
            for (int j = 3; j < i.req_body.size() - 1; j += 2) {
                catalogue_ptr_->AddDistance(i.req_body[0],
                                            i.req_body[j + 1],
                                            std::stoi(i.req_body[j]));
            }
        }
    }

    // Adding routes to catalogue
    for (auto i : requests) {
        if (i.req_type == InputParser::RequestType::ADD_ROUTE) {

            std::vector<std::string> tmp;

            if (i.route_type == InputParser::RouteType::STRAIGHT) {
                for (auto j = 1; j < i.req_body.size(); ++j) {
                    tmp.push_back(i.req_body[j]);
                }
                for (auto j = i.req_body.size() - 2 ; j >= 1 ; --j) {
                    tmp.push_back(i.req_body[j]);
                }
                catalogue_ptr_->AddRoute(i.req_body[0], tmp, false);
            } else {
                for (auto j = 1; j < i.req_body.size(); ++j) {
                    tmp.push_back(i.req_body[j]);
                }
                catalogue_ptr_->AddRoute(i.req_body[0], tmp, true);
            }
        }
    }

}

} // end of namespace: transport_catalogue
