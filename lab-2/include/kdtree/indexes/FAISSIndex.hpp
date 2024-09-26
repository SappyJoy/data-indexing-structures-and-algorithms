#ifndef FAISS_INDEX_HPP
#define FAISS_INDEX_HPP

#include "Index.hpp"
#include "kdtree/Point.hpp"
#include <faiss/IndexFlat.h> // Adjust based on the FAISS index you choose
#include <memory>

namespace kdtree {

class FAISSIndex : public Index {
  public:
    FAISSIndex();
    ~FAISSIndex() override;

    // Build the FAISS index with a set of points
    void build(const std::vector<Point> &points) override;

    // Insert a single point into the FAISS index
    void insert(const Point &point) override;

    // Find k nearest neighbors using FAISS
    std::vector<Point> nearest_neighbors(const Point &query, size_t k) const override;

    // Range search is not directly supported by FAISS's basic indices
    std::vector<Point> range_search(const Point &query, double radius) const override;

  private:
    std::unique_ptr<faiss::IndexFlatL2> faiss_index_;
    size_t dimension_;
    std::vector<Point> points_; // Store points to retrieve by label
};

} // namespace kdtree

#endif
