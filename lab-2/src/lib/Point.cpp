#include "kdtree/Point.hpp"
#include <cmath>
#include <stdexcept>

namespace kdtree {

Point::Point(const std::vector<double> &coordinates) : coordinates_(coordinates) {
    if (coordinates_.empty()) {
        throw std::invalid_argument("Point must have at least one coordinate");
    }
}

double Point::get_coordinate(std::size_t index) const {
    if (index >= coordinates_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return coordinates_[index];
}

size_t Point::dimension() const { return coordinates_.size(); }

double Point::distance_to(const Point &other) const {
    if (coordinates_.size() != other.coordinates_.size()) {
        throw std::invalid_argument("Points must have the same dimension");
    }
    double distance = 0.0;
    for (size_t i = 0; i < coordinates_.size(); ++i) {
        double diff = coordinates_[i] - other.coordinates_[i];
        distance += diff * diff;
    }
    return std::sqrt(distance);
}

} // namespace kdtree
