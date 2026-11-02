#include "logic/TrendAnalyzer.h"
#include "core/EventLogger.h"
#include <cmath>

TrendAnalyzer::TrendAnalyzer() : ma3_value(0), slope_angle(0) {
    close_history.reserve(1000);
}

void TrendAnalyzer::OnBarClose(const IndicatorSnapshot& snapshot) {
    close_history.push_back(snapshot.bar.close);

    // 1. Считаем MA3 (Тяжелую среднюю) на 200 баров
    if (close_history.size() >= 200) {
        ma3_value = CalculateSMA(200, close_history);
    }

    // 2. Считаем угол наклона (Накось) за последние 10 баров
    if (close_history.size() > 10) {
        double price_10_bars_ago = close_history[close_history.size() - 10];
        double current_price = close_history.back();
        
        // Простая производная: разница в цене деленная на количество баров
        slope_angle = (current_price - price_10_bars_ago) / 10.0;

        // Выводим лог только при сильных изменениях, чтобы не спамить консоль
        if (std::abs(slope_angle) > 0.05) {
            std::string direction = slope_angle > 0 ? "ВВЕРХ (Бычий)" : "ВНИЗ (Медвежий)";
            std::string msg = "Вектор Накоси: " + direction + " (Угол: " + std::to_string(slope_angle) + ")";
            EventLogger::Log("NAKOSI", msg, snapshot.bar.close_time);
        }
    }
}

double TrendAnalyzer::CalculateSMA(int period, const std::vector<double>& data) {
    if (data.size() < period) return 0.0;
    double sum = 0.0;
    for (int i = data.size() - period; i < data.size(); ++i) {
        sum += data[i];
    }
    return sum / period;
}
