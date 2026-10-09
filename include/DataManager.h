#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <vector>
#include <cstdint>

#pragma pack(push, 1)
struct Bar {
    int64_t time;
    double open;
    double high;
    double low;
    double close;
    int64_t tick_volume;
    int32_t spread;
    int64_t real_volume;
};
#pragma pack(pop)

class DataManager {
public:
    DataManager();
    ~DataManager();
    bool loadData(const std::string& filepath);
    const std::vector<Bar>& getBars() const;

    std::string getSymbol() const { return symbol_; }
    int32_t getPeriodSeconds() const { return period_seconds_; }
    int64_t getBarCount() const { return bar_count_; }

private:
    std::string symbol_;
    int32_t version_ = 0;
    int32_t period_seconds_ = 0;
    int32_t digits_ = 0;
    double point_ = 0.0;
    int64_t bar_count_ = 0;
    int64_t first_time_ = 0;
    int64_t last_time_ = 0;
    
    std::vector<Bar> bars_;
};

#endif

