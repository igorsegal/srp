#include <iostream>
#include "DataManager.h"
#include "AO_Zotik.h"
#include "WPR_VSmark.h"
#include "WaveABC.h"

int main() {
    std::cout << "=== srp Backtester Startup === " << std::endl;
    DataManager manager;
    
    std::string filepath = "D:\\AHexaTrader\\1DataFiles\\raw\\EURUSD\\EURUSD_M5.bin";
    if (manager.loadData(filepath)) {
        auto abcPatterns = WaveABC::detect(manager.getBars(), 35.0, 0.0001);
        std::cout << "[Main] WaveABC strict structural analysis complete. Total valid patterns: " << abcPatterns.size() << std::endl;
        
        if (!abcPatterns.empty()) {
            auto lastPat = abcPatterns.back();
            std::cout << "[Main] Last Valid ABC Pattern (Type: " << (lastPat.isBullish ? "Bullish" : "Bearish") << "):" << std::endl;
            std::cout << "  Point A Index: " << lastPat.indexA << " Price: " << lastPat.priceA << std::endl;
            std::cout << "  Point B Index: " << lastPat.indexB << " Price: " << lastPat.priceB << " (AB span: " << lastPat.abSpanPips << " pips)" << std::endl;
            std::cout << "  Point C Index: " << lastPat.indexC << " Price: " << lastPat.priceC << " (BC span: " << lastPat.bcSpanPips << " pips)" << std::endl;
        }
    } else {
        std::cout << "[Main] Failed to load data." << std::endl;
    }

    return 0;
}

