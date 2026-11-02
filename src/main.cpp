#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include "core/Types.h"
#include "core/IModule.h"
#include "indicators/IndicatorProvider.h"
#include "logic/BumpDetector.h"
#include "logic/TRStateMachine.h"
#include "logic/ABCPatternDetector.h"
#include "logic/TrendAnalyzer.h" // <-- Подключаем новый модуль

int main() {
    std::cout << "=== SRP Architecture Simulator v1.0 ===" << std::endl;

    auto indicator_provider = std::make_unique<IndicatorProvider>();
    auto tr_state_machine = std::make_unique<TRStateMachine>();
    auto abc_detector = std::make_unique<ABCPatternDetector>();
    auto trend_analyzer = std::make_unique<TrendAnalyzer>(); // <-- Создаем анализатор тренда

    std::cout << "Generating fake market data..." << std::endl;
    
    for (int i = 0; i < 500; ++i) { // Увеличили до 500 баров для расчета MA3 (нужно минимум 200)
        BarSnapshot bar;
        bar.close_time = std::chrono::system_clock::now() + std::chrono::hours(i);
        
        // Создаем искусственный тренд: сначала вверх, потом вниз
        double trend = (i < 250) ? (i * 0.1) : ((500 - i) * 0.1); 
        bar.close = 100.0 + trend + std::sin(i * 0.1) * 2.0; 
        
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

        // Отправляем данные во все модули
        tr_state_machine->OnBarClose(snapshot);
        abc_detector->OnBarClose(snapshot);
        trend_analyzer->OnBarClose(snapshot); // <-- Анализируем тренд
    }

    std::cout << "=== Simulation Finished ===" << std::endl;
    return 0;
}
