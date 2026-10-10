$ErrorActionPreference = "Stop"

Write-Host "Creating directory structure..."
$scriptDir =$PSScriptRoot
Set-Location $scriptDir

if (!(Test-Path "include")) { New-Item -ItemType Directory -Path "include" }
if (!(Test-Path "src")) { New-Item -ItemType Directory -Path "src" }

Write-Host "Writing Bar.h..."
@"
#pragma once
#include <cstdint>

struct Bar {
    int64_t time;
    double open;
    double high;
    double low;
    double close;
    int64_t volume;
};
"@ | Out-File -Encoding utf8 "include\Bar.h"

Write-Host "Writing AO_Zotik.h..."
@"
#pragma once
#include <vector>
#include "Bar.h"

struct AO_Result {
    std::vector<double> histogram;
};

class AO_Zotik {
public:
    static AO_Result calculate(const std::vector<Bar>& bars, int fastPeriod = 5, int slowPeriod = 34) {
        AO_Result res;
        size_t n = bars.size();
        res.histogram.resize(n, 0.0);
        if (n == 0) return res;

        std::vector<double> median(n);
        for (size_t i = 0; i < n; ++i) {
            median[i] = (bars[i].high + bars[i].low) / 2.0;
        }

        auto sma = [](const std::vector<double>& data, int period, size_t idx) {
            if (idx + 1 < (size_t)period) return 0.0;
            double sum = 0.0;
            for (int k = 0; k < period; ++k) {
                sum += data[idx - k];
            }
            return sum / period;
        };

        for (size_t i = 0; i < n; ++i) {
            double fast = sma(median, fastPeriod, i);
            double slow = sma(median, slowPeriod, i);
            res.histogram[i] = fast - slow;
        }
        return res;
    }
};
"@ | Out-File -Encoding utf8 "include\AO_Zotik.h"

Write-Host "Writing WPR_VSmark.h..."
@"
#pragma once
#include <vector>
#include <algorithm>
#include "Bar.h"

struct WPR_Result {
    std::vector<double> wpr;
};

class WPR_VSmark {
public:
    static WPR_Result calculate(const std::vector<Bar>& bars, int period = 14) {
        WPR_Result res;
        size_t n = bars.size();
        res.wpr.resize(n, -50.0);
        if (n == 0) return res;

        for (size_t i = 0; i < n; ++i) {
            if ((int)i < period - 1) {
                res.wpr[i] = -50.0;
                continue;
            }
            double highestHigh = -1e9;
            double lowestLow = 1e9;
            for (int k = 0; k < period; ++k) {
                double h = bars[i - k].high;
                double l = bars[i - k].low;
                if (h > highestHigh) highestHigh = h;
                if (l < lowestLow) lowestLow = l;
            }
            double denom = highestHigh - lowestLow;
            if (denom == 0.0) {
                res.wpr[i] = 0.0;
            } else {
                res.wpr[i] = -100.0 * (highestHigh - bars[i].close) / denom;
            }
        }
        return res;
    }
};
"@ | Out-File -Encoding utf8 "include\WPR_VSmark.h"

Write-Host "Writing WaveABC.h..."
@"
#pragma once
#include <vector>
#include "Bar.h"

struct ABC_Pattern {
    size_t indexA;
    size_t indexB;
    size_t indexC;
    double priceA;
    double priceB;
    double priceC;
    bool isBullish;
};

class WaveABC {
public:
    static std::vector<ABC_Pattern> detect(const std::vector<Bar>& bars, double minSwingPips, double pipSize) {
        std::vector<ABC_Pattern> patterns;
        if (bars.size() < 10) return patterns;

        double swingVal = minSwingPips * pipSize;
        enum Trend { DIR_UP, DIR_DOWN };
        Trend currentDir = (bars[1].close > bars[0].close) ? DIR_UP : DIR_DOWN;
        double extremePrice = currentDir == DIR_UP ? bars[0].low : bars[0].high;
        size_t extremeIdx = 0;

        struct Pivot {
            size_t idx;
            double price;
            bool isHigh;
        };
        std::vector<Pivot> pivots;

        for (size_t i = 1; i < bars.size(); ++i) {
            if (currentDir == DIR_UP) {
                if (bars[i].high > extremePrice) {
                    extremePrice = bars[i].high;
                    extremeIdx = i;
                } else if (extremePrice - bars[i].low >= swingVal) {
                    pivots.push_back({extremeIdx, extremePrice, true});
                    currentDir = DIR_DOWN;
                    extremePrice = bars[i].low;
                    extremeIdx = i;
                }
            } else {
                if (bars[i].low < extremePrice) {
                    extremePrice = bars[i].low;
                    extremeIdx = i;
                } else if (bars[i].high - extremePrice >= swingVal) {
                    pivots.push_back({extremeIdx, extremePrice, false});
                    currentDir = DIR_UP;
                    extremePrice = bars[i].high;
                    extremeIdx = i;
                }
            }
        }

        if (pivots.size() >= 3) {
            for (size_t i = 2; i < pivots.size(); ++i) {
                Pivot pA = pivots[i - 2];
                Pivot pB = pivots[i - 1];
                Pivot pC = pivots[i];

                if (pA.isHigh && !pB.isHigh && pC.isHigh) {
                    patterns.push_back({pA.idx, pB.idx, pC.idx, pA.price, pB.price, pC.price, false});
                } else if (!pA.isHigh && pB.isHigh && !pC.isHigh) {
                    patterns.push_back({pA.idx, pB.idx, pC.idx, pA.price, pB.price, pC.price, true});
                }
            }
        }
        return patterns;
    }
};
"@ | Out-File -Encoding utf8 "include\WaveABC.h"

Write-Host "Writing RiskManager.h..."
@"
#pragma once
#include <cmath>

class RiskManagerModule {
public:
    static double calculateRiskPct(double balance, double startDeposit, double baseRiskPct) {
        double balanceRatio = balance / startDeposit;
        if (balanceRatio < 1.0) balanceRatio = 1.0;
        return baseRiskPct / std::sqrt(balanceRatio);
    }

    static double calculateLotSize(double balance, double riskPct, double riskPips, double minLot, double maxLot) {
        double riskUSD = balance * riskPct;
        if (riskPips < 10.0) riskPips = 10.0;
        double lotSize = riskUSD / (riskPips * 10.0);
        if (lotSize < minLot) lotSize = minLot;
        if (lotSize > maxLot) lotSize = maxLot;
        return lotSize;
    }

    static bool checkMarginShield(double marginLevelPct) {
        if (marginLevelPct <= 0.0) return true;
        if (marginLevelPct < 500.0) {
            return false;
        }
        return true;
    }
};
"@ | Out-File -Encoding utf8 "include\RiskManager.h"

Write-Host "Writing DllMain.cpp..."
@"
#include <windows.h>
#include "Bar.h"
#include "RiskManager.h"
#include "AO_Zotik.h"
#include "WPR_VSmark.h"
#include "WaveABC.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) double GetTargetLotSize(double balance, double startDeposit, double baseRiskPct, double riskPips) {
        double currentRisk = RiskManagerModule::calculateRiskPct(balance, startDeposit, baseRiskPct);
        double lot = RiskManagerModule::calculateLotSize(balance, currentRisk, riskPips, 0.01, 50.0);
        return lot;
    }

    __declspec(dllexport) bool CheckMarginShieldLevel(double marginLevelPct) {
        return RiskManagerModule::checkMarginShield(marginLevelPct);
    }
}
"@ | Out-File -Encoding utf8 "src\DllMain.cpp"

Write-Host "Writing CMakeLists.txt..."
@"
cmake_minimum_required(VERSION 3.12)
project(PortfolioCoreDLL CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

include_directories(`$({CMAKE_CURRENT_SOURCE_DIR}/include))

set(PROJECT_FILES
    src/DllMain.cpp
    include/Bar.h
    include/AO_Zotik.h
    include/WPR_VSmark.h
    include/WaveABC.h
    include/RiskManager.h
)

add_library(PortfolioCoreDLL SHARED `$PROJECT_FILES)

set_target_properties(PortfolioCoreDLL PROPERTIES 
    PREFIX ""
    SUFFIX ".dll"
    OUTPUT_NAME "PortfolioCore"
)
"@ | Out-File -Encoding utf8 "CMakeLists.txt"

Write-Host "All files created successfully!"