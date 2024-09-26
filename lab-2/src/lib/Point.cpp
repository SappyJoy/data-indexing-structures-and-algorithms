#include "kdtree/Point.hpp"
#include <cmath>
#include <stdexcept>

namespace kdtree {

Point::Point(const std::vector<float> &coordinates) : coordinates_(coordinates) {}

float Point::operator[](size_t index) const {
    if (index >= coordinates_.size()) {
        throw std::out_of_range("Index out of range in Point::operator[] const");
    }
    return coordinates_[index];
}

float &Point::operator[](size_t index) {
    if (index >= coordinates_.size()) {
        throw std::out_of_range("Index out of range in Point::operator[]");
    }
    return coordinates_[index];
}

size_t Point::dimension() const { return coordinates_.size(); }

const float *Point::data() const { return coordinates_.data(); }

} // namespace kdtree
