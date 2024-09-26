#ifndef POINT_HPP
#define POINT_HPP

#include <vector>

namespace kdtree {

class Point {
  public:
    Point() = delete;
    explicit Point(const std::vector<float> &coordinates);

    float operator[](std::size_t index) const;
    float &operator[](std::size_t index);
    std::size_t dimension() const;

    // Conversion to raw data pointer (for FAISS)
    const float *data() const;

    const std::vector<float> &coordinates() const { return coordinates_; }

    std::vector<float> &coordinates() { return coordinates_; }

    std::vector<float>::const_iterator begin() const { return coordinates_.begin(); }
    std::vector<float>::const_iterator end() const { return coordinates_.end(); }

  private:
    std::vector<float> coordinates_;
};

} // namespace kdtree

#endif
