// ==========================================
// Файл: Types.h (Обновленная версия)
// ==========================================
#pragma once
#include <string>
#include <chrono>
#include <vector>

// Структура для хранения одного бара
struct BarSnapshot {
    std::string symbol;
    int timeframe;
    std::chrono::system_clock::time_point close_time;
    double open, high, low, close;
    int tick_volume;
};

// Структура для хранения значений индикаторов
struct IndicatorSnapshot {
    BarSnapshot bar;
    // Значения ZOTIC
    double ol_value; // Основная линия (SMA3 - SMA34)
    double sp_value; // Сигнальная полоса (EMA21 - EMA55)
    double lb_value; // Линия баланса (EMA34 - SMA34)
    // Значения WPR (пока не рассчитываем, оставляем для будущего)
    double wpr_fast; // WPR 21
    double wpr_slow; // WPR 252
};

// Вспомогательная структура для хранения истории цен для расчета MA
struct PriceHistory {
    std::vector<double> close_prices;
    std::vector<double> median_prices; // (high+low)/2 для АО
};
