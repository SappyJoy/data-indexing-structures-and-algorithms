#include "kdtree/BoundingBox.hpp"
#include <stdexcept>

namespace kdtree {

BoundingBox::BoundingBox(const Point &lower_bound, const Point &upper_bound)
    : lower_bound_(lower_bound), upper_bound_(upper_bound) {
    if (lower_bound_.dimension() != upper_bound_.dimension()) {
        throw std::invalid_argument("Lower and upper bounds must have the same dimension");
    }
}

bool BoundingBox::contains(const Point &point) const {
    if (point.dimension() != lower_bound_.dimension()) {
        return false;
    }
    for (size_t i = 0; i < point.dimension(); ++i) {
        if (point.get_coordinate(i) < lower_bound_.get_coordinate(i) ||
            point.get_coordinate(i) > upper_bound_.get_coordinate(i)) {
            return false;
        }
    }
    return true;
}

bool BoundingBox::intersects(const BoundingBox &other) const {
    if (lower_bound_.dimension() != other.lower_bound_.dimension()) {
        return false;
    }
    // WARN: Не уверен, но это возможно не верная реализация, т.к. lower_bound не во всех координатах может быть меньше
    // upper_bound. И вообще проверки lower_bound < upper_bound нет в конструкторе, потому что не понятно какая она
    // должна быть
    for (size_t i = 0; i < lower_bound_.dimension(); ++i) {
        if (lower_bound_.get_coordinate(i) > other.upper_bound_.get_coordinate(i) ||
            upper_bound_.get_coordinate(i) < other.lower_bound_.get_coordinate(i)) {
            return false;
        }
    }
    return true;
}

const Point &BoundingBox::get_lower_bound() const { return lower_bound_; }

const Point &BoundingBox::get_upper_bound() const { return upper_bound_; }

} // namespace kdtree
