#include "PerformanceAnalyzer.h"
#include <cmath>

BacktestResult PerformanceAnalyzer::evaluate(const std::vector<Bar>& bars, const std::vector<TradeSignal>& signals) {
    BacktestResult res = {0, 0, 0, 0.0, 0.0};
    if (signals.empty() || bars.empty()) return res;

    res.totalTrades = static_cast<int>(signals.size());
    double cumulativePips = 0.0;

    for (const auto& sig : signals) {
        bool tradeClosed = false;
        for (size_t i = sig.barIndex + 1; i < bars.size(); ++i) {
            if (sig.isBuy) {
                if (bars[i].low <= sig.stopLoss) {
                    res.losingTrades++;
                    cumulativePips -= (sig.entryPrice - sig.stopLoss) / 0.0001;
                    tradeClosed = true;
                    break;
                } else if (bars[i].high >= sig.takeProfit) {
                    res.winningTrades++;
                    cumulativePips += (sig.takeProfit - sig.entryPrice) / 0.0001;
                    tradeClosed = true;
                    break;
                }
            } else {
                if (bars[i].high >= sig.stopLoss) {
                    res.losingTrades++;
                    cumulativePips -= (sig.stopLoss - sig.entryPrice) / 0.0001;
                    tradeClosed = true;
                    break;
                } else if (bars[i].low <= sig.takeProfit) {
                    res.winningTrades++;
                    cumulativePips += (sig.entryPrice - sig.takeProfit) / 0.0001;
                    tradeClosed = true;
                    break;
                }
            }
        }
        if (!tradeClosed) {
            size_t lastIdx = bars.size() - 1;
            double diff = sig.isBuy ? (bars[lastIdx].close - sig.entryPrice) : (sig.entryPrice - bars[lastIdx].close);
            cumulativePips += diff / 0.0001;
            if (diff > 0) res.winningTrades++; else res.losingTrades++;
        }
    }

    res.winRate = res.totalTrades > 0 ? (static_cast<double>(res.winningTrades) / res.totalTrades) * 100.0 : 0.0;
    res.totalProfitPips = cumulativePips;

    return res;
}
