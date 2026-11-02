// ==========================================
// Файл: IndicatorProvider.cpp (Реализация)
// ==========================================
#include "indicators/IndicatorProvider.h"
#include <numeric>
#include <algorithm>
#include <cmath>

IndicatorProvider::IndicatorProvider() {
    // Резервируем память, чтобы не было переаллокаций
    close_history.reserve(1000);
    median_history.reserve(1000);
}

void IndicatorProvider::OnBarClose(const IndicatorSnapshot& snapshot) {
    // 1. Сохраняем данные нового бара в историю
    close_history.push_back(snapshot.bar.close);
    // Медианная цена (High + Low) / 2 нужна для расчета АО Вильямса
    double median_price = (snapshot.bar.high + snapshot.bar.low) / 2.0;
    median_history.push_back(median_price);

    // 2. Проверяем, достаточно ли данных для расчета (минимум 55 баров для EMA55)
    if (close_history.size() < 55) {
        return; // Ждем накопления истории
    }

    // 3. Расчет ОЛ (SMA3 - SMA34) на основе медианных цен
    double sma3 = CalculateSMA(3, median_history);
    double sma34 = CalculateSMA(34, median_history);
    current_ol = sma3 - sma34;

    // 4. Расчет СП (EMA21 - EMA55) на основе цен закрытия
    double ema21 = CalculateEMA(21, close_history);
    double ema55 = CalculateEMA(55, close_history);
    current_sp = ema21 - ema55;

    // 5. Расчет ЛБ (Линия Баланса) - по спецификации это (EMA34 - SMA34)
    double ema34 = CalculateEMA(34, close_history);
    current_lb = ema34 - sma34;

    // TODO: Здесь будет логика расчета WPR (в следующих уроках)
}

// Простая скользящая средняя (SMA)
double IndicatorProvider::CalculateSMA(int period, const std::vector<double>& data) {
    if (data.size() < period) return 0.0;
    
    double sum = 0.0;
    // Берем последние 'period' значений
    for (int i = data.size() - period; i < data.size(); ++i) {
        sum += data[i];
    }
    return sum / period;
}

// Экспоненциальная скользящая средняя (EMA)
double IndicatorProvider::CalculateEMA(int period, const std::vector<double>& data) {
    if (data.size() < period) return 0.0;

    // Коэффициент сглаживания
    double multiplier = 2.0 / (period + 1.0);
    
    // В качестве начального значения EMA берем SMA за первый период
    double ema = CalculateSMA(period, std::vector<double>(data.begin(), data.begin() + period));
    
    // Идем по остальным данным и считаем EMA
    for (size_t i = period; i < data.size(); ++i) {
        ema = (data[i] - ema) * multiplier + ema;
    }
    return ema;
}
