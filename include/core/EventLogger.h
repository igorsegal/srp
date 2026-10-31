#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
class EventLogger {
public:
    static void Log(const std::string& event_type, const std::string& message, std::chrono::system_clock::time_point known_at) {
        auto time_t = std::chrono::system_clock::to_time_t(known_at);
        std::cout << "[" << std::ctime(&time_t) << "] EVENT: " << event_type << " | " << message << std::endl;
    }
};
