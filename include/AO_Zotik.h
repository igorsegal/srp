#ifndef AO_ZOTIK_H
#define AO_ZOTIK_H

#include <vector>
#include "DataManager.h"

struct AO_ZotikResult {
    std::vector<double> histogram;
    std::vector<double> signal;
    std::vector<double> line2;
    std::vector<double> line3;
};

class AO_Zotik {
public:
    static AO_ZotikResult calculate(const std::vector<Bar>& bars, int fastPeriod = 5, int slowPeriod = 35, int signalPeriod = 5);
};

#endif

