#include "AO_Zotik.h"

AO_ZotikResult AO_Zotik::calculate(const std::vector<Bar>& bars, int fastPeriod, int slowPeriod, int signalPeriod) {
    size_t n = bars.size();
    AO_ZotikResult res;
    res.histogram.assign(n, 0.0);
    res.signal.assign(n, 0.0);
    res.line2.assign(n, 0.0);
    res.line3.assign(n, 0.0);

    if (n < static_cast<size_t>(slowPeriod)) {
        return res;
    }

    std::vector<double> medianPrices(n);
    for (size_t i = 0; i < n; ++i) {
        medianPrices[i] = (bars[i].high + bars[i].low) / 2.0;
    }

    auto calcSMA = [&](const std::vector<double>& src, int period, size_t index) -> double {
        if (index + 1 < static_cast<size_t>(period)) return 0.0;
        double sum = 0.0;
        for (int i = 0; i < period; ++i) {
            sum += src[index - i];
        }
        return sum / period;
    };

    // 1. Расчет основного AO (гистограмма)
    for (size_t i = slowPeriod - 1; i < n; ++i) {
        double smaFast = calcSMA(medianPrices, fastPeriod, i);
        double smaSlow = calcSMA(medianPrices, slowPeriod, i);
        res.histogram[i] = smaFast - smaSlow;
    }

    // 2. Расчет сигнальной линии (SMA по гистограмме)
    for (size_t i = slowPeriod - 1 + signalPeriod - 1; i < n; ++i) {
        res.signal[i] = calcSMA(res.histogram, signalPeriod, i);
    }

    // 3. Расчет дополнительных буферов Zotik (line2 и line3)
    for (size_t i = slowPeriod; i < n; ++i) {
        res.line2[i] = res.histogram[i] * 1.05; // Модифицирующий коэффициент контура 2
        res.line3[i] = res.histogram[i] * 0.95; // Модифицирующий коэффициент контура 3
    }

    return res;
}

