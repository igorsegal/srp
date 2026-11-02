#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include "core/Types.h"
#include "core/IModule.h"
#include "indicators/IndicatorProvider.h"
#include "logic/TRStateMachine.h"
#include "logic/ABCPatternDetector.h"
#include "logic/TrendAnalyzer.h"
#include "logic/TradeEngine.h"

int main() {
    std::cout << "=== SRP Full Trading System Simulator ===" << std::endl;

    auto indicator_provider = std::make_unique<IndicatorProvider>();
    auto tr_state_machine = std::make_unique<TRStateMachine>();
    auto abc_detector = std::make_unique<ABCPatternDetector>();
    auto trend_analyzer = std::make_unique<TrendAnalyzer>();
    auto trade_engine = std::make_unique<TradeEngine>();

    trade_engine->SetSignals(tr_state_machine.get(), trend_analyzer.get());

    std::cout << "Generating fake market data and running backtest..." << std::endl;
    
    for (int i = 0; i < 600; ++i) { 
        BarSnapshot bar;
        bar.close_time = std::chrono::system_clock::now() + std::chrono::hours(i);
        
        double trend = (i < 300) ? (i * 0.05) : ((600 - i) * 0.05); 
        bar.close = 100.0 + trend + std::sin(i * 0.2) * 3.0; 
        
        bar.high = bar.close + 1.0;
        bar.low = bar.close - 1.0;
        bar.open = bar.close - 0.5;
        bar.tick_volume = 100;

        IndicatorSnapshot snapshot;
        snapshot.bar = bar;
        
        indicator_provider->OnBarClose(snapshot);
        
        snapshot.ol_value = indicator_provider->GetOL();
        snapshot.sp_value = indicator_provider->GetSP();
        snapshot.lb_value = indicator_provider->GetLB();

        tr_state_machine->OnBarClose(snapshot);
        abc_detector->OnBarClose(snapshot);
        trend_analyzer->OnBarClose(snapshot);
        trade_engine->OnBarClose(snapshot);
    }

    std::cout << "=== Backtest Finished ===" << std::endl;
    return 0;
}
