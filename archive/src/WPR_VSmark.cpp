#include "WPR_VSmark.h"
#include <algorithm>

WPR_Result WPR_VSmark::calculate(const std::vector<Bar>& bars, int period) {
    WPR_Result res;
    res.wpr.resize(bars.size(), -50.0);
    if (bars.size() < static_cast<size_t>(period)) return res;

    for (size_t i = period - 1; i < bars.size(); ++i) {
        double highestHigh = bars[i].high;
        double lowestLow = bars[i].low;

        for (int j = 0; j < period; ++j) {
            highestHigh = std::max(highestHigh, bars[i - j].high);
            lowestLow = std::min(lowestLow, bars[i - j].low);
        }

        if (highestHigh - lowestLow == 0.0) {
            res.wpr[i] = 0.0;
        } else {
            res.wpr[i] = -100.0 * (highestHigh - bars[i].close) / (highestHigh - lowestLow);
        }
    }
    return res;
}
