#pragma once
#include <vector>
#include <algorithm>
#include "Bar.h"

struct WPR_Result {
    std::vector<double> wpr;
};

class WPR_VSmark {
public:
    static WPR_Result calculate(const std::vector<Bar>& bars, int period = 14) {
        WPR_Result res;
        size_t n = bars.size();
        res.wpr.resize(n, -50.0);
        if (n == 0) return res;

        for (size_t i = 0; i < n; ++i) {
            if ((int)i < period - 1) {
                res.wpr[i] = -50.0;
                continue;
            }
            double highestHigh = -1e9;
            double lowestLow = 1e9;
            for (int k = 0; k < period; ++k) {
                double h = bars[i - k].high;
                double l = bars[i - k].low;
                if (h > highestHigh) highestHigh = h;
                if (l < lowestLow) lowestLow = l;
            }
            double denom = highestHigh - lowestLow;
            if (denom == 0.0) {
                res.wpr[i] = 0.0;
            } else {
                res.wpr[i] = -100.0 * (highestHigh - bars[i].close) / denom;
            }
        }
        return res;
    }
};