#include "BacktestEngine.h"
#include "WaveABC.h"
#include "AO_Zotik.h"
#include "WPR_VSmark.h"
#include "VolumeVUK.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <numeric>
struct SimulatedTrade {
    size_t barIndex;
    bool isWin;
    double pips;
    double balanceAfter;
};
std::vector<TradeSignal> BacktestEngine::run(const std::vector<Bar>& m5Bars, const std::vector<Bar>& h1Bars) {
    std::vector<TradeSignal> signals;
    if (m5Bars.empty() || h1Bars.empty()) return signals;
    const double h1MinSwing = 40.0;
    const double m5MinSwing = 35.0;
    auto h1AO = AO_Zotik::calculate(h1Bars, 5, 34);
    auto h1Patterns = WaveABC::detect(h1Bars, h1MinSwing, 0.0001);
    auto m5Patterns = WaveABC::detect(m5Bars, m5MinSwing, 0.0001);
    auto m5AO = AO_Zotik::calculate(m5Bars, 5, 34);
    auto m5WPR = WPR_VSmark::calculate(m5Bars, 14);
    auto getH1Index = [&](int64_t m5Time) {
        size_t bestIdx = 0;
        for (size_t i = 0; i < h1Bars.size(); ++i) {
            if (h1Bars[i].time <= m5Time) {
                bestIdx = i;
            } else {
                break;
            }
        }
        return bestIdx;
    };
    struct RawSignal {
        size_t idx;
        double price;
        bool isBullish;
        double tp;
        double sl;
    };
    std::vector<RawSignal> rawSignals;
    for (const auto& pat : m5Patterns) {
        size_t idx = pat.indexC;
        if (idx >= m5Bars.size()) continue;
        size_t h1Idx = getH1Index(m5Bars[idx].time);
        double h1Val = (h1Idx < h1Bars.size()) ? h1AO.histogram[h1Idx] : 0.0;
        bool h1BullishTrend = (h1Val > 0.0);
        bool h1BearishTrend = (h1Val < 0.0);
        if (pat.isBullish) {
            if (!h1BullishTrend) continue;
            if (m5AO.histogram[idx] <= 0.0) continue;
            if (m5WPR.wpr[idx] <= -80.0) continue;
            rawSignals.push_back({idx, m5Bars[idx].close, true, pat.priceC + 0.0070, pat.priceC - 0.0035});
        } else {
            if (!h1BearishTrend) continue;
            if (m5AO.histogram[idx] >= 0.0) continue;
            if (m5WPR.wpr[idx] >= -20.0) continue;
            rawSignals.push_back({idx, m5Bars[idx].close, false, pat.priceC - 0.0070, pat.priceC + 0.0035});
        }
    }
    // --- Аутентичная симуляция результатов по свечам с учетом MSP-логики ---
    double deposit = 1000.0;
    double balance = deposit;
    double peakBalance = deposit;
    double maxDrawdownUSD = 0.0;
    double maxDrawdownPct = 0.0;
    std::vector<SimulatedTrade> simTrades;
    std::vector<double> tradeReturns;
    double grossProfit = 0.0;
    double grossLoss = 0.0;
    int wins = 0;
    int losses = 0;
    for (size_t i = 0; i < rawSignals.size(); ++i) {
        const auto& sig = rawSignals[i];
        bool isWin = false;
        double tradePips = 0.0;
        size_t checkLimit = std::min(sig.idx + 200, m5Bars.size());
        for (size_t b = sig.idx + 1; b < checkLimit; ++b) {
            if (sig.isBullish) {
                if (m5Bars[b].low <= sig.sl) {
                    isWin = false;
                    tradePips = (sig.sl - sig.price) * 10000.0; // отрицательные пипсы
                    break;
                }
                if (m5Bars[b].high >= sig.tp) {
                    isWin = true;
                    tradePips = (sig.tp - sig.price) * 10000.0; // положительные пипсы
                    break;
                }
            } else {
                if (m5Bars[b].high >= sig.sl) {
                    isWin = false;
                    tradePips = (sig.price - sig.sl) * 10000.0;
                    break;
                }
                if (m5Bars[b].low <= sig.tp) {
                    isWin = true;
                    tradePips = (sig.price - sig.tp) * 10000.0;
                    break;
                }
            }
        }
        // Если сделка не дошла ни до TP ни до SL за лимит, закрываем по текущей цене фиксации волны
        if (tradePips == 0.0) {
            double finalPrice = m5Bars[std::min(sig.idx + 50, m5Bars.size() - 1)].close;
            tradePips = sig.isBullish ? (finalPrice - sig.price) * 10000.0 : (sig.price - finalPrice) * 10000.0;
            isWin = (tradePips > 0);
        }
        double riskAmountUSD = balance * 0.03; // 3% риска от депозита
        double riskPips = 35.0; // Базовый риск в пипсах по волне
        double lotSize = riskAmountUSD / (riskPips * 10.0);
        if (lotSize < 0.01) lotSize = 0.01;
        double tradePnlUSD = 0.0;
        if (isWin) {
            tradePnlUSD = lotSize * std::abs(tradePips) * 10.0;
            grossProfit += tradePnlUSD;
            wins++;
        } else {
            tradePnlUSD = -riskAmountUSD;
            grossLoss += std::abs(tradePnlUSD);
            losses++;
        }
        double oldBalance = balance;
        balance += tradePnlUSD;
        if (balance < 0) balance = 0;
        if (balance > peakBalance) {
            peakBalance = balance;
        }
        double currentDDUSD = peakBalance - balance;
        double currentDDPct = (peakBalance > 0) ? (currentDDUSD / peakBalance) * 100.0 : 0.0;
        if (currentDDUSD > maxDrawdownUSD) maxDrawdownUSD = currentDDUSD;
        if (currentDDPct > maxDrawdownPct) maxDrawdownPct = currentDDPct;
        tradeReturns.push_back((balance - oldBalance) / oldBalance);
        simTrades.push_back({sig.idx, isWin, tradePips, balance});
        signals.push_back({sig.idx, sig.price, sig.isBullish, sig.sl, sig.tp});
    }
    int totalTrades = wins + losses;
    double winRate = (totalTrades > 0) ? (static_cast<double>(wins) / totalTrades) * 100.0 : 0.0;
    double profitFactor = (grossLoss > 0.0) ? (grossProfit / grossLoss) : (grossProfit > 0 ? 999.0 : 0.0);
    double netProfitUSD = balance - deposit;
    double netProfitPct = (netProfitUSD / deposit) * 100.0;
    double avgReturn = 0.0;
    if (!tradeReturns.empty()) {
        avgReturn = std::accumulate(tradeReturns.begin(), tradeReturns.end(), 0.0) / tradeReturns.size();
    }
    double variance = 0.0;
    for (double r : tradeReturns) {
        variance += (r - avgReturn) * (r - avgReturn);
    }
    double stdDev = (tradeReturns.size() > 1) ? std::sqrt(variance / (tradeReturns.size() - 1)) : 1.0;
    double sharpeRatio = (stdDev > 0.0) ? (avgReturn / stdDev) * std::sqrt(252.0) : 0.0;
    // --- Единый профессиональный отчёт копитрейдера ---
    std::cout << "\n=================================================================\n";
    std::cout << "        MASTERFOREX-V (MSP) PROFESSIONAL COPY-TRADER REPORT       \n";
    std::cout << "=================================================================\n";
    std::cout << " Initial Deposit     : $" << std::fixed << std::setprecision(2) << deposit << "\n";
    std::cout << " Ending Balance      : $" << balance << " (" << (netProfitUSD >= 0 ? "+" : "") << netProfitPct << "%)\n";
    std::cout << " Net Profit          : $" << netProfitUSD << "\n";
    std::cout << "-----------------------------------------------------------------\n";
    std::cout << " Total Trades        : " << totalTrades << "\n";
    std::cout << " Winning Trades      : " << wins << " (" << std::setprecision(2) << winRate << "%)\n";
    std::cout << " Losing Trades       : " << losses << " (" << (100.0 - winRate) << "%)\n";
    std::cout << " Gross Profit        : $" << grossProfit << "\n";
    std::cout << " Gross Loss          : $" << grossLoss << "\n";
    std::cout << " Profit Factor       : " << profitFactor << "\n";
    std::cout << "-----------------------------------------------------------------\n";
    std::cout << " Max Drawdown ($)    : $" << maxDrawdownUSD << " (-" << std::setprecision(2) << maxDrawdownPct << "%)\n";
    std::cout << " Sharpe Ratio (Ann.) : " << std::setprecision(2) << sharpeRatio << "\n";
    std::cout << " Risk per Trade      : 3.0% of Balance (Dynamic Lot Compounding)\n";
    std::cout << "=================================================================\n";
    // --- ASCII График баланса ---
    std::cout << "\n[Equity Balance Curve Chart ($)]\n";
    if (!simTrades.empty()) {
        int steps = std::min(12, static_cast<int>(simTrades.size()));
        for (int s = 0; s <= steps; ++s) {
            size_t tIdx = (s * (simTrades.size() - 1)) / steps;
            double bVal = simTrades[tIdx].balanceAfter;
            int barLen = static_cast<int>((bVal - deposit) / (std::max(1.0, balance - deposit + 200.0)) * 25.0);
            if (barLen < 0) barLen = 0;
            if (barLen > 25) barLen = 25;
            std::cout << " T" << std::setw(2) << tIdx << " | $" << std::setw(7) << std::fixed << std::setprecision(0) << bVal 
                      << " " << std::string(barLen, '*') << "\n";
        }
    }
    std::cout << "=================================================================\n\n";
    return signals;
}