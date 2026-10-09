#include "WPR_VSmark.h"
#include <algorithm>

WPR_VSmarkResult WPR_VSmark::calculate(const std::vector<Bar>& bars, int period, int signalPeriod) {
    size_t n = bars.size();
    WPR_VSmarkResult res;
    res.wpr.assign(n, 0.0);
    res.signal.assign(n, 0.0);
    res.line2.assign(n, 0.0);
    res.line3.assign(n, 0.0);

    if (n < static_cast<size_t>(period)) return res;

    for (size_t i = period - 1; i < n; ++i) {
        double highestHigh = bars[i - period + 1].high;
        double lowestLow = bars[i - period + 1].low;
        for (size_t j = 1; j < static_cast<size_t>(period); ++j) {
            highestHigh = std::max(highestHigh, bars[i - period + 1 + j].high);
            lowestLow = std::min(lowestLow, bars[i - period + 1 + j].low);
        }
        double range = highestHigh - lowestLow;
        if (range == 0.0) {
            res.wpr[i] = 0.0;
        } else {
            res.wpr[i] = (highestHigh - bars[i].close) / range * -100.0;
        }
    }

    // Сигнальная линия (Buf 1)
    for (size_t i = period - 1 + signalPeriod - 1; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < signalPeriod; ++k) {
            sum += res.wpr[i - k];
        }
        res.signal[i] = sum / signalPeriod;
    }

    // Дополнительные контуры line2 (Buf 2) и line3 (Buf 3)
    for (size_t i = period; i < n; ++i) {
        res.line2[i] = res.wpr[i] * 0.9 + res.signal[i] * 0.1;
        res.line3[i] = res.wpr[i] * 0.8 + res.signal[i] * 0.2;
    }

    return res;
}

