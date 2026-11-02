#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include "core/Types.h"
#include "core/IModule.h"
#include "indicators/IndicatorProvider.h"
#include "logic/BumpDetector.h"
#include "logic/TRStateMachine.h"

int main() {
    std::cout << "=== SRP Architecture Simulator v1.0 ===" << std::endl;

    // Создаем наши модули
    auto indicator_provider = std::make_unique<IndicatorProvider>();
    auto tr_state_machine = std::make_unique<TRStateMachine>();

    // Генерируем искусственную цену (синусоида для имитации тренда и откатов)
    std::cout << "Generating fake market data..." << std::endl;
    
    for (int i = 0; i < 200; ++i) {
        // Формируем бар (искусственная цена)
        BarSnapshot bar;
        bar.close_time = std::chrono::system_clock::now() + std::chrono::hours(i);
        bar.close = 100.0 + std::sin(i * 0.1) * 10.0; // Цена колеблется от 90 до 110
        bar.high = bar.close + 1.0;
        bar.low = bar.close - 1.0;
        bar.open = bar.close - 0.5;
        bar.tick_volume = 100;

        // Создаем снапшот для текущего бара
        IndicatorSnapshot snapshot;
        snapshot.bar = bar;
        
        // ВАЖНО: Передаем бар в IndicatorProvider, чтобы он посчитал ОЛ, СП и ЛБ
        indicator_provider->OnBarClose(snapshot);
        
        // Забираем посчитанные значения из провайдера
        snapshot.ol_value = indicator_provider->GetOL();
        snapshot.sp_value = indicator_provider->GetSP();
        snapshot.lb_value = indicator_provider->GetLB();

        // Передаем снапшот с УЖЕ ПОСЧИТАННЫМИ индикаторами в Машину Состояний
        tr_state_machine->OnBarClose(snapshot);
    }

    std::cout << "=== Simulation Finished ===" << std::endl;
    return 0;
}
