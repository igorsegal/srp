#ifndef AO_ZOTIK_H
#define AO_ZOTIK_H

#include <vector>
#include "DataManager.h"

struct AO_Result {
    std::vector<double> histogram;
};

class AO_Zotik {
public:
    static AO_Result calculate(const std::vector<Bar>& bars, int fastPeriod = 5, int slowPeriod = 34);
};

#endif
