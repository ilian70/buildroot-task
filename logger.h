
#pragma once

#include <fstream>
#include <mutex>


class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
public:
    bool Open(const std::string& filename) 
    {
        logFile.open(filename, std::ios::app | std::ios::out);
        return logFile.is_open();
    }
    
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    template<typename... Args>
    void log(Args&&... args) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto tm = *std::localtime(&time_t);
            
            char timestamp[64];
            std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
            
            logFile << "[" << timestamp << "] ";
            (logFile << ... << args);
            logFile << std::endl;
        }
    }
};
