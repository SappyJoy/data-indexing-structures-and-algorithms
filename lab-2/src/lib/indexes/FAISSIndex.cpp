// src/lib/indexes/FAISSIndex.cpp

#include "kdtree/indexes/FAISSIndex.hpp"
#include <faiss/IndexFlat.h>
#include <faiss/index_io.h>
#include <stdexcept>

namespace kdtree {

// Constructor: Initializes the FAISS index
FAISSIndex::FAISSIndex() : faiss_index_(nullptr), dimension_(0) {}

// Destructor: FAISS indices handle their own memory
FAISSIndex::~FAISSIndex() = default;

// Build the FAISS index with the provided points
void FAISSIndex::build(const std::vector<Point> &points) {
    if (points.empty()) {
        throw std::invalid_argument("Point set is empty.");
    }

    // Assuming Point stores coordinates as float
    // Ensure all points have the same dimension
    dimension_ = points.front().dimension();
    for (const auto &point : points) {
        if (point.dimension() != dimension_) {
            throw std::invalid_argument("All points must have the same dimension.");
        }
    }

    // Initialize the FAISS index
    faiss_index_ = std::make_unique<faiss::IndexFlatL2>(dimension_);

    // Prepare data in FAISS's expected format (row-major)
    size_t num_points = points.size();
    std::vector<float> data(num_points * dimension_);
    for (size_t i = 0; i < num_points; ++i) {
        for (size_t d = 0; d < dimension_; ++d) {
            data[i * dimension_ + d] = static_cast<float>(points[i][d]);
        }
    }

    // Add points to the FAISS index
    faiss_index_->add(num_points, data.data());

    // Store points for retrieval
    points_ = points;
}

// Insert a single point into the FAISS index
void FAISSIndex::insert(const Point &point) {
    if (!faiss_index_) {
        throw std::runtime_error("FAISS index is not initialized. Call build() first.");
    }
    if (point.dimension() != dimension_) {
        throw std::invalid_argument("Point dimensionality does not match FAISS index.");
    }

    // Prepare data in FAISS's expected format
    std::vector<float> data(dimension_);
    for (size_t d = 0; d < dimension_; ++d) {
        data[d] = static_cast<float>(point[d]);
    }

    // Add the point to the FAISS index
    faiss_index_->add(1, data.data());

    // Store the point for retrieval
    points_.emplace_back(point);
}

// Find k nearest neighbors using FAISS
std::vector<Point> FAISSIndex::nearest_neighbors(const Point &query, size_t k) const {
    if (!faiss_index_) {
        throw std::runtime_error("FAISS index is not initialized. Call build() first.");
    }
    if (query.dimension() != dimension_) {
        throw std::invalid_argument("Query point dimensionality does not match FAISS index.");
    }
    if (k == 0) {
        return {};
    }

    // Ensure k does not exceed the number of points in the index
    k = std::min(k, static_cast<size_t>(faiss_index_->ntotal));

    // Prepare query in FAISS's expected format
    std::vector<float> query_data(dimension_);
    for (size_t d = 0; d < dimension_; ++d) {
        query_data[d] = static_cast<float>(query[d]);
    }

    // Allocate space for FAISS results
    std::vector<float> distances(k);
    std::vector<faiss::idx_t> labels(k);

    // Perform the search
    faiss_index_->search(1, query_data.data(), k, distances.data(), labels.data());

    // Retrieve the corresponding points
    std::vector<Point> neighbors;
    neighbors.reserve(k);
    for (size_t i = 0; i < k; ++i) {
        faiss::idx_t label = labels[i];
        if (label >= 0 && static_cast<size_t>(label) < points_.size()) {
            neighbors.emplace_back(points_[label]);
        } else {
            throw std::out_of_range("FAISS returned an invalid label.");
        }
    }

    return neighbors;
}

// Range search using FAISS (not directly supported in basic indices)
std::vector<Point> FAISSIndex::range_search(const Point &query, double radius) const {
    if (!faiss_index_) {
        throw std::runtime_error("FAISS index is not initialized. Call build() first.");
    }
    if (query.dimension() != dimension_) {
        throw std::invalid_argument("Query point dimensionality does not match FAISS index.");
    }

    // FAISS does not support range search directly.
    // Implement a workaround by performing k-NN search with k set to total points
    // and filtering results by radius. Note: This is inefficient for large datasets.

    size_t total_points = faiss_index_->ntotal;
    if (total_points == 0) {
        return {};
    }

    // Perform k-NN search with k = total_points
    std::vector<float> query_data(dimension_);
    for (size_t d = 0; d < dimension_; ++d) {
        query_data[d] = static_cast<float>(query[d]);
    }

    std::vector<float> distances(total_points);
    std::vector<faiss::idx_t> labels(total_points);

    faiss_index_->search(1, query_data.data(), total_points, distances.data(), labels.data());

    // Collect points within the specified radius
    std::vector<Point> results;
    results.reserve(total_points); // Reserve maximum possible size

    double radius_sq = radius * radius; // Compare squared distances

    for (size_t i = 0; i < total_points; ++i) {
        double dist_sq = static_cast<double>(distances[i]);
        if (dist_sq <= radius_sq) {
            faiss::idx_t label = labels[i];
            if (label >= 0 && static_cast<size_t>(label) < points_.size()) {
                results.emplace_back(points_[label]);
            }
        }
    }

    return results;
}

} // namespace kdtree
