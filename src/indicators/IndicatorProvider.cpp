#include "indicators/IndicatorProvider.h"
#include <numeric>
#include <algorithm>
#include <cmath>

IndicatorProvider::IndicatorProvider() {
    close_history.reserve(1000);
    median_history.reserve(1000);
}

void IndicatorProvider::OnBarClose(const IndicatorSnapshot& snapshot) {
    close_history.push_back(snapshot.bar.close);
    double median_price = (snapshot.bar.high + snapshot.bar.low) / 2.0;
    median_history.push_back(median_price);

    if (close_history.size() < 55) {
        return; 
    }

    double sma3 = CalculateSMA(3, median_history);
    double sma34 = CalculateSMA(34, median_history);
    current_ol = sma3 - sma34;

    double ema21 = CalculateEMA(21, close_history);
    double ema55 = CalculateEMA(55, close_history);
    current_sp = ema21 - ema55;

    double ema34 = CalculateEMA(34, close_history);
    current_lb = ema34 - sma34;
}

double IndicatorProvider::CalculateSMA(int period, const std::vector<double>& data) {
    if (data.size() < period) return 0.0;
    double sum = 0.0;
    for (int i = data.size() - period; i < data.size(); ++i) {
        sum += data[i];
    }
    return sum / period;
}

double IndicatorProvider::CalculateEMA(int period, const std::vector<double>& data) {
    if (data.size() < period) return 0.0;
    double multiplier = 2.0 / (period + 1.0);
    double ema = CalculateSMA(period, std::vector<double>(data.begin(), data.begin() + period));
    for (size_t i = period; i < data.size(); ++i) {
        ema = (data[i] - ema) * multiplier + ema;
    }
    return ema;
}
