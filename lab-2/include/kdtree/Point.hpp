#ifndef POINT_HPP
#define POINT_HPP

#include <vector>

namespace kdtree {

class Point {
  public:
    Point() = delete;
    explicit Point(const std::vector<float> &coordinates);

    // Accessors
    float operator[](std::size_t index) const;
    float &operator[](std::size_t index);
    std::size_t dimension() const;

    // Conversion to raw data pointer (for FAISS)
    const float *data() const;

  private:
    std::vector<float> coordinates_;
};

} // namespace kdtree

#endif
