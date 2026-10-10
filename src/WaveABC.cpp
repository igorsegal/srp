#include "WaveABC.h"
#include <cmath>
#include <algorithm>

std::vector<ABC_Pattern> WaveABC::detect(const std::vector<Bar>& bars, double minSwingPips, double pointSize) {
    std::vector<ABC_Pattern> patterns;
    if (bars.size() < 50) return patterns;

    double minDelta = minSwingPips * pointSize;
    struct SwingPoint {
        size_t index;
        double price;
        bool isHigh;
    };

    std::vector<SwingPoint> swings;
    int depth = 10;

    for (size_t i = depth; i < bars.size() - depth; ++i) {
        bool isHigh = true;
        bool isLow = true;

        for (int j = 1; j <= depth; ++j) {
            if (bars[i].high <= bars[i - j].high || bars[i].high <= bars[i + j].high) isHigh = false;
            if (bars[i].low >= bars[i - j].low || bars[i].low >= bars[i + j].low) isLow = false;
        }

        if (isHigh) {
            if (swings.empty() || swings.back().isHigh != true) {
                swings.push_back({i, bars[i].high, true});
            } else if (bars[i].high > swings.back().price) {
                swings.back() = {i, bars[i].high, true};
            }
        } else if (isLow) {
            if (swings.empty() || swings.back().isHigh != false) {
                swings.push_back({i, bars[i].low, false});
            } else if (bars[i].low < swings.back().price) {
                swings.back() = {i, bars[i].low, false};
            }
        }
    }

    if (swings.size() >= 3) {
        for (size_t k = 0; k < swings.size() - 2; ++k) {
            SwingPoint pA = swings[k];
            SwingPoint pB = swings[k + 1];
            SwingPoint pC = swings[k + 2];

            if (pA.isHigh != pB.isHigh && pB.isHigh != pC.isHigh) {
                double abSpan = std::abs(pB.price - pA.price) / pointSize;
                double bcSpan = std::abs(pC.price - pB.price) / pointSize;

                if (abSpan >= minSwingPips && bcSpan >= minSwingPips) {
                    ABC_Pattern pat;
                    pat.indexA = pA.index;
                    pat.indexB = pB.index;
                    pat.indexC = pC.index;
                    pat.priceA = pA.price;
                    pat.priceB = pB.price;
                    pat.priceC = pC.price;
                    pat.abSpanPips = abSpan;
                    pat.bcSpanPips = bcSpan;
                    pat.isBullish = !pA.isHigh;

                    patterns.push_back(pat);
                }
            }
        }
    }
    return patterns;
}
