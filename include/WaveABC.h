#ifndef WAVE_ABC_H
#define WAVE_ABC_H

#include <vector>
#include "DataManager.h"

struct ABC_Pattern {
    size_t indexA;
    size_t indexB;
    size_t indexC;
    double priceA;
    double priceB;
    double priceC;
    double abSpanPips;
    double bcSpanPips;
    bool isBullish;
};

class WaveABC {
public:
    static std::vector<ABC_Pattern> detect(const std::vector<Bar>& bars, double minSwingPips = 35.0, double pointSize = 0.0001);
};

#endif
