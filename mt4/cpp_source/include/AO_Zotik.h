#pragma once
#include <vector>
#include "Bar.h"

struct AO_Result {
    std::vector<double> histogram;
};

class AO_Zotik {
public:
    static AO_Result calculate(const std::vector<Bar>& bars, int fastPeriod = 5, int slowPeriod = 34) {
        AO_Result res;
        size_t n = bars.size();
        res.histogram.resize(n, 0.0);
        if (n == 0) return res;

        std::vector<double> median(n);
        for (size_t i = 0; i < n; ++i) {
            median[i] = (bars[i].high + bars[i].low) / 2.0;
        }

        auto sma = [](const std::vector<double>& data, int period, size_t idx) {
            if (idx + 1 < (size_t)period) return 0.0;
            double sum = 0.0;
            for (int k = 0; k < period; ++k) {
                sum += data[idx - k];
            }
            return sum / period;
        };

        for (size_t i = 0; i < n; ++i) {
            double fast = sma(median, fastPeriod, i);
            double slow = sma(median, slowPeriod, i);
            res.histogram[i] = fast - slow;
        }
        return res;
    }
};