#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <set>
#include <map>
#include <fstream>
#include "DataManager.h"
#include "WaveABC.h"
#include "AO_Zotik.h"
#include "WPR_VSmark.h"
namespace fs = std::filesystem;
struct TradeSignal {
    int64_t time;
    size_t barIndex;
    std::string symbol;
    double entryPrice;
    bool isBullish;
    double sl;
    double tp;
    double riskPips;
    bool passEmaTrend;
    int hourUTC;
};
struct ActiveTrade {
    std::string symbol;
    std::string baseCurrency;
    std::string quoteCurrency;
    int64_t entryTime;
    int64_t exitTime;
    double lotSize;
    double marginUsed;
    double riskUSD;
    double pnlUSD;
    bool isWin;
};
struct ExportTradeRecord {
    int64_t exitTime;
    std::string symbol;
    std::string outcome;
    double lotSize;
    double pnlUSD;
    double balanceAfter;
};
struct InstrumentData {
    std::string symbol;
    std::vector<Bar> m5Bars;
    std::vector<Bar> h1Bars;
    bool isJpy;
};
std::vector<double> calculateEMA(const std::vector<Bar>& bars, int period) {
    std::vector<double> ema(bars.size(), 0.0);
    if (bars.empty()) return ema;
    double multiplier = 2.0 / (period + 1.0);
    ema[0] = bars[0].close;
    for (size_t i = 1; i < bars.size(); ++i) {
        ema[i] = (bars[i].close * multiplier) + (ema[i - 1] * (1.0 - multiplier));
    }
    return ema;
}
void runAndExportPureTwoPairTrades(const std::vector<TradeSignal>& allSignals,
                                   const std::vector<InstrumentData>& portfolioData,
                                   const std::set<std::string>& allowedSymbols,
                                   double startDeposit,
                                   double riskPct,
                                   double leverage,
                                   const std::string& csvPath) {
    double deposit = startDeposit;
    double balance = deposit;
    double peakEquity = deposit;
    double maxDrawdownUSD = 0.0;
    double maxDrawdownPct = 0.0;
    std::vector<ActiveTrade> openTrades;
    std::vector<ExportTradeRecord> exportRecords;
    const double spreadPips = 1.5;
    const double slippagePips = 0.5;
    const double commissionPerLot = 6.0;
    const double minLot = 0.01;
    const double maxLot = 50.0;
    const int64_t startTime2020 = 1577836800; // 2020-01-01
    int wins = 0;
    int losses = 0;
    double grossProfit = 0.0;
    double grossLoss = 0.0;
    for (const auto& sig : allSignals) {
        if (sig.time < startTime2020) continue;
        if (!sig.passEmaTrend) continue;
        if (sig.hourUTC < 7 || sig.hourUTC > 20) continue;
        if (allowedSymbols.find(sig.symbol) == allowedSymbols.end()) continue;
        auto it = openTrades.begin();
        while (it != openTrades.end()) {
            if (it->exitTime <= sig.time) {
                balance += it->pnlUSD;
                if (balance < 0) balance = 0;
                exportRecords.push_back({it->exitTime, it->symbol, it->pnlUSD >= 0 ? "WIN" : "LOSS", it->lotSize, it->pnlUSD, balance});
                it = openTrades.erase(it);
            } else {
                ++it;
            }
        }
        std::string sigBase = sig.symbol.substr(0, 3);
        std::string sigQuote = sig.symbol.substr(3, 3);
        double totalUsedMargin = 0.0;
        for (const auto& tr : openTrades) totalUsedMargin += tr.marginUsed;
        double marginLevel = (totalUsedMargin > 0.0) ? (balance / totalUsedMargin) * 100.0 : 99999.0;
        if (marginLevel <= 500.0 && !openTrades.empty()) continue;
        double riskUSD = balance * riskPct;
        double riskPips = 35.0;
        if (riskPips < 10.0) riskPips = 10.0;
        double lotSize = riskUSD / (riskPips * 10.0);
        if (lotSize < minLot) lotSize = minLot;
        if (lotSize > maxLot) lotSize = maxLot;
        double contractSize = 100000.0;
        double marginRequired = (lotSize * contractSize) / leverage;
        if (balance < marginRequired) continue;
        auto instIt = std::find_if(portfolioData.begin(), portfolioData.end(), [&](const InstrumentData& id) {
            return id.symbol == sig.symbol;
        });
        if (instIt == portfolioData.end()) continue;
        const auto& bars = instIt->m5Bars;
        bool isJpy = instIt->isJpy;
        bool isWin = false;
        double tradePips = 0.0;
        size_t checkLimit = std::min(sig.barIndex + 200, bars.size());
        int64_t exitTime = sig.time + (200 * 300);
        double pipMultiplier = isJpy ? 100.0 : 10000.0;
        for (size_t b = sig.barIndex + 1; b < checkLimit; ++b) {
            if (sig.isBullish) {
                if (bars[b].low <= sig.sl) {
                    isWin = false;
                    tradePips = (sig.sl - sig.entryPrice) * pipMultiplier;
                    exitTime = bars[b].time;
                    break;
                }
                if (bars[b].high >= sig.tp) {
                    isWin = true;
                    tradePips = (sig.tp - sig.entryPrice) * pipMultiplier;
                    exitTime = bars[b].time;
                    break;
                }
            } else {
                if (bars[b].high >= sig.sl) {
                    isWin = false;
                    tradePips = (sig.entryPrice - sig.sl) * pipMultiplier;
                    exitTime = bars[b].time;
                    break;
                }
                if (bars[b].low <= sig.tp) {
                    isWin = true;
                    tradePips = (sig.entryPrice - sig.tp) * pipMultiplier;
                    exitTime = bars[b].time;
                    break;
                }
            }
        }
        if (tradePips == 0.0) {
            double finalPrice = bars[std::min(sig.barIndex + 50, bars.size() - 1)].close;
            tradePips = sig.isBullish ? (finalPrice - sig.entryPrice) * pipMultiplier : (sig.entryPrice - finalPrice) * pipMultiplier;
            isWin = (tradePips > 0);
        }
        double totalFrictionPips = spreadPips + (slippagePips * 2.0);
        if (isWin) {
            tradePips -= totalFrictionPips;
            if (tradePips < 0) isWin = false;
        } else {
            tradePips += totalFrictionPips;
        }
        double tradeCommissionUSD = lotSize * commissionPerLot;
        double pnlUSD = 0.0;
        if (isWin) {
            pnlUSD = (lotSize * std::abs(tradePips) * 10.0) - tradeCommissionUSD;
            grossProfit += (lotSize * std::abs(tradePips) * 10.0);
            wins++;
        } else {
            pnlUSD = -(riskUSD + tradeCommissionUSD);
            grossLoss += std::abs(pnlUSD);
            losses++;
        }
        openTrades.push_back({sig.symbol, sigBase, sigQuote, sig.time, exitTime, lotSize, marginRequired, riskUSD, pnlUSD, isWin});
        double currentEq = balance;
        for (const auto& tr : openTrades) currentEq += tr.pnlUSD;
        if (currentEq > peakEquity) peakEquity = currentEq;
        double ddUSD = peakEquity - currentEq;
        double ddPct = (peakEquity > 0) ? (ddUSD / peakEquity) * 100.0 : 0.0;
        if (ddUSD > maxDrawdownUSD) maxDrawdownUSD = ddUSD;
        if (ddPct > maxDrawdownPct) maxDrawdownPct = ddPct;
    }
    for (const auto& tr : openTrades) {
        balance += tr.pnlUSD;
        if (balance < 0) balance = 0;
        exportRecords.push_back({tr.exitTime, tr.symbol, tr.pnlUSD >= 0 ? "WIN" : "LOSS", tr.lotSize, tr.pnlUSD, balance});
    }
    std::sort(exportRecords.begin(), exportRecords.end(), [](const ExportTradeRecord& a, const ExportTradeRecord& b) {
        return a.exitTime < b.exitTime;
    });
    std::ofstream outFile(csvPath);
    outFile << "Timestamp,Symbol,Outcome,LotSize,PnL_USD,BalanceAfter\n";
    for (const auto& rec : exportRecords) {
        outFile << rec.exitTime << "," << rec.symbol << "," << rec.outcome << "," << rec.lotSize << "," << rec.pnlUSD << "," << rec.balanceAfter << "\n";
    }
    outFile.close();
    int totalTrades = wins + losses;
    double winRate = (totalTrades > 0) ? (static_cast<double>(wins) / totalTrades) * 100.0 : 0.0;
    double netProfitUSD = balance - deposit;
    double netProfitPct = (netProfitUSD / deposit) * 100.0;
    std::cout << "\n=========================================================\n";
    std::cout << " PURE 2-PAIR SIMULATION (GBPUSD & USDJPY) [Risk 7%, Lev 300] \n";
    std::cout << "=========================================================\n";
    std::cout << "Starting Deposit : $" << std::fixed << std::setprecision(2) << deposit << "\n";
    std::cout << "Ending Balance   : $" << balance << " (" << netProfitPct << "%)\n";
    std::cout << "Total Trades     : " << totalTrades << " (Wins: " << wins << ", Losses: " << losses << ")\n";
    std::cout << "Win Rate         : " << std::setprecision(1) << winRate << "%\n";
    std::cout << "Max Drawdown     : $" << std::setprecision(0) << maxDrawdownUSD << " (" << std::setprecision(1) << maxDrawdownPct << "%)\n";
    std::cout << "Exported Trades  : " << csvPath << "\n";
    std::cout << "=========================================================\n";
}
int main() {
    std::string rawDataDir = "D:\\AHexaTrader\\1DataFiles\\raw";
    if (!fs::exists(rawDataDir)) {
        std::cerr << "Data directory not found: " << rawDataDir << std::endl;
        return 1;
    }
    std::vector<std::string> targetPairs = {"GBPUSD", "USDJPY"};
    std::vector<InstrumentData> portfolioData;
    for (const auto& pairName : targetPairs) {
        fs::path pairDir = fs::path(rawDataDir) / pairName;
        if (fs::exists(pairDir)) {
            std::string m5Path = (pairDir / (pairName + "_M5.bin")).string();
            std::string h1Path = (pairDir / (pairName + "_H1.bin")).string();
            if (fs::exists(m5Path) && fs::exists(h1Path)) {
                DataManager dm5, dh1;
                if (dm5.loadData(m5Path) && dh1.loadData(h1Path)) {
                    bool isJpy = (pairName.find("JPY") != std::string::npos);
                    portfolioData.push_back({pairName, dm5.getBars(), dh1.getBars(), isJpy});
                }
            }
        }
    }
    std::vector<TradeSignal> allSignals;
    auto processInstrument = [&](const InstrumentData& inst, double minSwing) {
        if (inst.m5Bars.empty() || inst.h1Bars.empty()) return;
        auto h1AO = AO_Zotik::calculate(inst.h1Bars, 5, 34);
        auto h1EMA = calculateEMA(inst.h1Bars, 50);
        auto m5AO = AO_Zotik::calculate(inst.m5Bars, 5, 34);
        auto m5WPR = WPR_VSmark::calculate(inst.m5Bars, 14);
        auto m5Patterns = WaveABC::detect(inst.m5Bars, minSwing, 0.0001);
        auto getH1Index = [&](int64_t m5Time) {
            size_t bestIdx = 0;
            for (size_t i = 0; i < inst.h1Bars.size(); ++i) {
                if (inst.h1Bars[i].time <= m5Time) bestIdx = i;
                else break;
            }
            return bestIdx;
        };
        double fixedSlOffset = inst.isJpy ? 0.35 : 0.0035;
        double fixedTpOffset = inst.isJpy ? 0.70 : 0.0070;
        for (const auto& pat : m5Patterns) {
            size_t idx = pat.indexC;
            if (idx >= inst.m5Bars.size()) continue;
            int64_t t = inst.m5Bars[idx].time;
            int hourUTC = static_cast<int>((t / 3600) % 24);
            if (hourUTC < 0) hourUTC += 24;
            size_t h1Idx = getH1Index(t);
            double h1Val = (h1Idx < inst.h1Bars.size()) ? h1AO.histogram[h1Idx] : 0.0;
            double h1Close = (h1Idx < inst.h1Bars.size()) ? inst.h1Bars[h1Idx].close : 0.0;
            double h1EmaVal = (h1Idx < h1EMA.size()) ? h1EMA[h1Idx] : h1Close;
            bool h1BullishAO = (h1Val > 0.0);
            bool h1BearishAO = (h1Val < 0.0);
            bool h1TrendBullish = (h1Close > h1EmaVal);
            bool h1TrendBearish = (h1Close < h1EmaVal);
            if (pat.isBullish) {
                if (!h1BullishAO || m5AO.histogram[idx] <= 0.0 || m5WPR.wpr[idx] <= -80.0) continue;
                allSignals.push_back({t, idx, inst.symbol, inst.m5Bars[idx].close, true, pat.priceC - fixedSlOffset, pat.priceC + fixedTpOffset, 35.0, h1TrendBullish, hourUTC});
            } else {
                if (!h1BearishAO || m5AO.histogram[idx] >= 0.0 || m5WPR.wpr[idx] >= -20.0) continue;
                allSignals.push_back({t, idx, inst.symbol, inst.m5Bars[idx].close, false, pat.priceC + fixedSlOffset, pat.priceC - fixedTpOffset, 35.0, h1TrendBullish, hourUTC});
            }
        }
    };
    for (const auto& inst : portfolioData) {
        processInstrument(inst, 25.0);
    }
    std::sort(allSignals.begin(), allSignals.end(), [](const TradeSignal& a, const TradeSignal& b) { return a.time < b.time; });
    std::set<std::string> twoPairs = {"GBPUSD", "USDJPY"};
    std::string csvOut = "D:\\AHexaTrader\\2026.10.08 SRP\\pure_two_pair_trades.csv";
    runAndExportPureTwoPairTrades(allSignals, portfolioData, twoPairs, 1000.0, 0.07, 300.0, csvOut);
    return 0;
}