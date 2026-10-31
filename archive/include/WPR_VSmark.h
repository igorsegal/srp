#ifndef WPR_VSMARK_H
#define WPR_VSMARK_H

#include <vector>
#include "DataManager.h"

struct WPR_Result {
    std::vector<double> wpr;
};

class WPR_VSmark {
public:
    static WPR_Result calculate(const std::vector<Bar>& bars, int period = 14);
};

#endif
