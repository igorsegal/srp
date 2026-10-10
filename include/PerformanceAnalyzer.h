#ifndef PERFORMANCE_ANALYZER_H
#define PERFORMANCE_ANALYZER_H

#include <vector>
#include "DataManager.h"
#include "BacktestEngine.h"

struct BacktestResult {
    int totalTrades;
    int winningTrades;
    int losingTrades;
    double winRate;
    double totalProfitPips;
};

class PerformanceAnalyzer {
public:
    static BacktestResult evaluate(const std::vector<Bar>& bars, const std::vector<TradeSignal>& signals);
};

#endif
