#ifndef INVERTED_INDEX_LOGGER_HPP
#define INVERTED_INDEX_LOGGER_HPP

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fstream>
#include <mutex>
#include <optional>
#include <string>

#define LOG_DEBUG(format, ...)                                                                                         \
    inverted_index::Logger::getInstance().log(inverted_index::LogLevel::DEBUG, __FILE__, __LINE__, format,             \
                                              ##__VA_ARGS__)

#define LOG_INFO(format, ...)                                                                                          \
    inverted_index::Logger::getInstance().log(inverted_index::LogLevel::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...)                                                                                       \
    inverted_index::Logger::getInstance().log(inverted_index::LogLevel::WARNING, __FILE__, __LINE__, format,           \
                                              ##__VA_ARGS__)

#define LOG_ERROR(format, ...)                                                                                         \
    inverted_index::Logger::getInstance().log(inverted_index::LogLevel::ERROR, __FILE__, __LINE__, format,             \
                                              ##__VA_ARGS__)

namespace inverted_index {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
  public:
    static Logger &getInstance();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void setLogLevel(LogLevel level);

    void enableFileLogging(const std::string &filename);

    void disableFileLogging();

    template <typename... Args>
    void log(LogLevel level, const char *file, int line, const std::string &format_str, Args &&...args) {
        if (level < current_level_) {
            return;
        }

        std::string message = fmt::format(format_str, std::forward<Args>(args)...);
        logMessage(level, file, line, message);
    }

  private:
    Logger();
    ~Logger();

    void logMessage(LogLevel level, const char *file, int line, const std::string &message);

    std::string levelToString(LogLevel level) const;

    LogLevel current_level_;
    std::mutex mtx_;

    std::optional<std::ofstream> file_stream_;
};

} // namespace inverted_index

#endif // INVERTED_INDEX_LOGGER_HPP
