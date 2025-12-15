
#pragma once

#include <fstream>
#include <iostream>
#include <mutex>
#include <syslog.h>
#include <sstream>


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
    void log(Args&&... args) 
    {
        std::lock_guard<std::mutex> lock(logMutex);

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);

        char timestamp[64];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);

        std::ostringstream oss;
        oss << "[" << timestamp << "] ";
        (oss << ... << args);
        oss << std::endl;

        auto& out = logFile.is_open() ? logFile : std::cout;
        out << oss.str(); 
        out.flush();

        syslog(LOG_INFO, "%s", oss.str().c_str());}
};
