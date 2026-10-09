#ifndef WPR_VSMARK_H
#define WPR_VSMARK_H

#include <vector>
#include "DataManager.h"

struct WPR_VSmarkResult {
    std::vector<double> wpr;
    std::vector<double> signal;
    std::vector<double> line2;
    std::vector<double> line3;
};

class WPR_VSmark {
public:
    static WPR_VSmarkResult calculate(const std::vector<Bar>& bars, int period = 14, int signalPeriod = 5);
};

#endif

