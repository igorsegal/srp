#ifndef BACKTEST_ENGINE_H
#define BACKTEST_ENGINE_H

#include <vector>
#include "DataManager.h"

struct TradeSignal {
    size_t barIndex;
    double entryPrice;
    bool isBuy;
    double stopLoss;
    double takeProfit;
};

class BacktestEngine {
public:
    static std::vector<TradeSignal> run(const std::vector<Bar>& m5Bars, const std::vector<Bar>& h1Bars);
};

#endif
