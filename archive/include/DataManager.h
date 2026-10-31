#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <vector>
#include <string>
#include <cstdint>

struct Bar {
    int64_t time;
    double open;
    double high;
    double low;
    double close;
    int64_t tick_volume;
    int spread;
    int64_t real_volume;
};

class DataManager {
private:
    std::vector<Bar> bars;
    std::string symbol;
    int periodSeconds;

public:
    bool loadData(const std::string& filepath);
    const std::vector<Bar>& getBars() const { return bars; }
    std::string getSymbol() const { return symbol; }
    int getPeriod() const { return periodSeconds; }
};

#endif
