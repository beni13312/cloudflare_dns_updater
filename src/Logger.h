#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <locale>

#define MAX_LOG_SIZE 60000000 // 60MB

enum LogLevel {
    INFO,
    WARNING,
    ERR0R
};

class Logger {
public:
    ~Logger();
    explicit Logger(const std::string &filename);
    void message(const std::string &msg, LogLevel level);

private:
    std::ofstream logFile;
};

Logger &logger();
#endif // LOGGER_H
