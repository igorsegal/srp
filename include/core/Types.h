#pragma once
#include <string>
#include <chrono>
#include <vector>

struct BarSnapshot {
    std::string symbol;
    int timeframe;
    std::chrono::system_clock::time_point close_time;
    double open, high, low, close;
    int tick_volume;
};

struct IndicatorSnapshot {
    BarSnapshot bar;
    double ol_value; 
    double sp_value; 
    double lb_value; 
    double wpr_fast; 
    double wpr_slow; 
};

struct PriceHistory {
    std::vector<double> close_prices;
    std::vector<double> median_prices; 
};

struct TRTargets {
    bool is_valid = false;
    bool is_high_potential = false; 
    double level_100 = 0.0; 
    double target_123 = 0.0;
    double target_138 = 0.0;
    double target_161 = 0.0;
    double target_176 = 0.0;
};
