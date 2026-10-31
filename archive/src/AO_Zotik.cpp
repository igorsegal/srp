#include "AO_Zotik.h"
#include <numeric>

AO_Result AO_Zotik::calculate(const std::vector<Bar>& bars, int fastPeriod, int slowPeriod) {
    AO_Result res;
    res.histogram.resize(bars.size(), 0.0);
    if (bars.size() < static_cast<size_t>(slowPeriod)) return res;

    std::vector<double> medianPrices(bars.size());
    for (size_t i = 0; i < bars.size(); ++i) {
        medianPrices[i] = (bars[i].high + bars[i].low) / 2.0;
    }

    for (size_t i = slowPeriod - 1; i < bars.size(); ++i) {
        double fastSum = 0.0;
        for (int j = 0; j < fastPeriod; ++j) {
            fastSum += medianPrices[i - j];
        }
        double fastSMA = fastSum / fastPeriod;

        double slowSum = 0.0;
        for (int j = 0; j < slowPeriod; ++j) {
            slowSum += medianPrices[i - j];
        }
        double slowSMA = slowSum / slowPeriod;

        res.histogram[i] = fastSMA - slowSMA;
    }
    return res;
}
