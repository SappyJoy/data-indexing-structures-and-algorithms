#ifndef FAISS_INDEX_HPP
#define FAISS_INDEX_HPP

#include "Index.hpp"
#include <faiss/IndexFlat.h>
#include <vector>

namespace index {

class FAISSIndex : public Index {
public:
    FAISSIndex(size_t dimension);

    void build(const std::vector<Point>& points) override;
    void insert(const Point& point) override;
    std::vector<Point> nearest_neighbors(const Point& query, size_t k) const override;
    std::vector<Point> range_search(const Point& query, double radius) const override;

private:
    size_t dimension_;
    faiss::IndexFlatL2 index_;
};

}  // namespace index

#endif
