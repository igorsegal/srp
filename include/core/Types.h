#pragma once
#include <string>
#include <chrono>
struct BarSnapshot {
    std::string symbol;
    int timeframe;
    std::chrono::system_clock::time_point close_time;
    double open, high, low, close;
    int tick_volume;
};
struct IndicatorSnapshot {
    BarSnapshot bar;
    double ol_value; // Основная линия
    double sp_value; // Сигнальная полоса
    double lb_value; // Линия баланса
    double wpr_fast; // WPR 21
    double wpr_slow; // WPR 252
};
