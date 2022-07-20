#pragma once

#include <cmath>

namespace transport_catalogue {

constexpr static const double MAX_DELTA_ERROR = 1e-6;
constexpr static const int MEAN_EARTH_RADIUS_METERS = 6371000;

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return std::abs(lat - other.lat) <= MAX_DELTA_ERROR && std::abs(lng - other.lng) <= MAX_DELTA_ERROR;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * MEAN_EARTH_RADIUS_METERS;
}

} // end of namespace: transport_catalogue
