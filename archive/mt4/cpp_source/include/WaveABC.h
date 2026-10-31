#pragma once
#include <vector>
#include "Bar.h"
struct ABC_Pattern {
    size_t indexA;
    size_t indexB;
    size_t indexC;
    double priceA;
    double priceB;
    double priceC;
    bool isBullish;
};
class WaveABC {
public:
    static std::vector<ABC_Pattern> detect(const std::vector<Bar>& bars, double minSwingPips, double pipSize) {
        std::vector<ABC_Pattern> patterns;
        if (bars.size() < 10) return patterns;
        double swingVal = minSwingPips * pipSize;
        bool isUp = bars[1].close > bars[0].close;
        double extremePrice = isUp ? bars[0].low : bars[0].high;
        size_t extremeIdx = 0;
        struct Pivot { size_t idx; double price; bool isHigh; };
        std::vector<Pivot> pivots;
        for (size_t i = 1; i < bars.size(); ++i) {
            if (isUp) {
                if (bars[i].high > extremePrice) { extremePrice = bars[i].high; extremeIdx = i; }
                else if (extremePrice - bars[i].low >= swingVal) {
                    pivots.push_back({extremeIdx, extremePrice, true});
                    isUp = false; extremePrice = bars[i].low; extremeIdx = i;
                }
            } else {
                if (bars[i].low < extremePrice) { extremePrice = bars[i].low; extremeIdx = i; }
                else if (bars[i].high - extremePrice >= swingVal) {
                    pivots.push_back({extremeIdx, extremePrice, false});
                    isUp = true; extremePrice = bars[i].high; extremeIdx = i;
                }
            }
        }
        if (pivots.size() >= 3) {
            for (size_t i = 2; i < pivots.size(); ++i) {
                Pivot pA = pivots[i - 2], pB = pivots[i - 1], pC = pivots[i];
                if (pA.isHigh && !pB.isHigh && pC.isHigh) patterns.push_back({pA.idx, pB.idx, pC.idx, pA.price, pB.price, pC.price, false});
                else if (!pA.isHigh && pB.isHigh && !pC.isHigh) patterns.push_back({pA.idx, pB.idx, pC.idx, pA.price, pB.price, pC.price, true});
            }
        }
        return patterns;
    }
};
