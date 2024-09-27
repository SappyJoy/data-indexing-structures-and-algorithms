#include "log/Logger.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iostream>

namespace inverted_index {

Logger::Logger() : current_level_(LogLevel::INFO), file_stream_(std::nullopt) {}

Logger::~Logger() {
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->close();
    }
}

Logger &Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mtx_);
    current_level_ = level;
}

void Logger::enableFileLogging(const std::string &filename) {
    std::lock_guard<std::mutex> lock(mtx_);

    std::filesystem::path filepath(filename);
    if (filepath.has_parent_path()) {
        std::error_code ec;
        if (!std::filesystem::exists(filepath.parent_path())) {
            if (!std::filesystem::create_directories(filepath.parent_path(), ec)) {
                throw std::runtime_error("Failed to create directories for log file: " +
                                         filepath.parent_path().string());
            }
        }
    }

    if (!file_stream_ || !file_stream_->is_open()) {
        file_stream_.emplace(filename, std::ios::app);
        if (!file_stream_->is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }
}

void Logger::disableFileLogging() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (file_stream_ && file_stream_->is_open()) {
        file_stream_->close();
        file_stream_ = std::nullopt;
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

void Logger::logMessage(LogLevel level, const char *file, int line, const std::string &message) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms_part = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm buf;
#ifdef _WIN32
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif

    std::filesystem::path file_path(file);
    std::string filename = file_path.filename().string();

    const std::string RESET_COLOR = "\033[0m";
    const std::string DEBUG_COLOR = "\033[36m";   // Cyan
    const std::string INFO_COLOR = "\033[32m";    // Green
    const std::string WARNING_COLOR = "\033[33m"; // Yellow
    const std::string ERROR_COLOR = "\033[31m";   // Red

    std::string color;
    switch (level) {
    case LogLevel::DEBUG:
        color = DEBUG_COLOR;
        break;
    case LogLevel::INFO:
        color = INFO_COLOR;
        break;
    case LogLevel::WARNING:
        color = WARNING_COLOR;
        break;
    case LogLevel::ERROR:
        color = ERROR_COLOR;
        break;
    default:
        color = RESET_COLOR;
    }

    std::string log_message = fmt::format("{}[{:%Y-%m-%d %H:%M:%S}.{:03}] [{:<7}] [{}:{}] {}{}", color, buf,
                                          ms_part.count(), levelToString(level), filename, line, message, RESET_COLOR);

    log_message += "\n";

    std::lock_guard<std::mutex> lock(mtx_);

    std::cout << log_message;

    if (file_stream_ && file_stream_->is_open()) {
        (*file_stream_) << log_message;
    }
}

} // namespace inverted_index
