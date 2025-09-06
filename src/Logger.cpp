#include "Logger.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <mutex>



Logger &logger() {
    static Logger logger("log.txt");
    return logger;
}

// Logger constructor
Logger::Logger(const std::string &filename) {
    // Check if file size exceeds 60MB (using file size check via fstream)
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        const std::streampos file_size = file.tellg();  // Get file size
        file.close();

        if (file_size > MAX_LOG_SIZE) {  // 60MB limit
            // Remove the file if it's larger than the limit, later it will recreate the log file
            std::remove(filename.c_str());
        }
    }

    // Open log file in append mode
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

// Logger destructor
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

// Log message function
void Logger::message(const std::string &msg, const LogLevel level) {
    if (!logFile.is_open()) {
        std::cerr << "Log file is not open." << std::endl;
        return;
    }

    std::string levelStr;
    switch (level) {
        case INFO:
            levelStr = "INFO";
        break;
        case WARNING:
            levelStr = "WARNING";
        break;
        case ERR0R:
            levelStr = "ERROR";
        break;
    }

    const std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeStr = std::ctime(&time);
    timeStr.pop_back(); // Remove the last newline character


    logFile << " [" << timeStr << "]" << "[" << levelStr << "] " << msg << std::endl;
}
