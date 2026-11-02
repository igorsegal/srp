#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include "core/Types.h"
#include "core/IModule.h"
#include "indicators/IndicatorProvider.h"
#include "logic/BumpDetector.h"
#include "logic/TRStateMachine.h"
#include "logic/ABCPatternDetector.h" // <-- Новый модуль

int main() {
    std::cout << "=== SRP Architecture Simulator v1.0 ===" << std::endl;

    auto indicator_provider = std::make_unique<IndicatorProvider>();
    auto tr_state_machine = std::make_unique<TRStateMachine>();
    auto abc_detector = std::make_unique<ABCPatternDetector>(); // <-- Создаем детектор

    std::cout << "Generating fake market data..." << std::endl;
    
    for (int i = 0; i < 300; ++i) { // Увеличили до 300 баров для наглядности
        BarSnapshot bar;
        bar.close_time = std::chrono::system_clock::now() + std::chrono::hours(i);
        // Немного изменим формулу волны, чтобы было больше пересечений
        bar.close = 100.0 + std::sin(i * 0.05) * 15.0 + std::sin(i * 0.1) * 5.0; 
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

        // Передаем данные в обе наши системы
        tr_state_machine->OnBarClose(snapshot);
        abc_detector->OnBarClose(snapshot); // <-- Проверяем паттерн ABC
    }

    std::cout << "=== Simulation Finished ===" << std::endl;
    return 0;
}
