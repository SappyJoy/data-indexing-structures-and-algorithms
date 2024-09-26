#ifndef FASHION_MNIST_HPP
#define FASHION_MNIST_HPP

#include "kdtree/Point.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

inline std::vector<kdtree::Point> LoadFashionMNIST(const std::string &filepath) {
    std::vector<kdtree::Point> points;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Assuming the CSV format: label,pixel1,pixel2,...,pixel784
        std::stringstream ss(line);
        std::string item;
        std::vector<float> pixels;

        // Skip the label
        if (!std::getline(ss, item, ',')) {
            continue; // Empty or malformed line
        }

        // Read pixel values
        while (std::getline(ss, item, ',')) {
            try {
                float pixel = std::stof(item) / 255.0f; // Normalize pixel values
                pixels.push_back(pixel);
            } catch (const std::invalid_argument &) {
                // Handle non-float values if necessary
                pixels.push_back(0.0f);
            }
        }

        // Create a kdtree::Point if the correct number of pixels is read
        if (pixels.size() == 784) { // 28x28 images flattened
            points.emplace_back(pixels);
        }
    }

    file.close();
    return points;
}

#endif // FASHION_MNIST_HPP
