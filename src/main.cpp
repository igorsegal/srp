#include <iostream>
#include <vector>
#include <memory>
#include "core/Types.h"
#include "core/IModule.h"
#include "core/BarReader.h"
#include "indicators/IndicatorProvider.h"
#include "logic/TRStateMachine.h"
#include "logic/ABCPatternDetector.h"
#include "logic/TrendAnalyzer.h"
#include "logic/TradeEngine.h"

int main() {
    std::cout << "=== SRP Full Trading System (Real Data Mode) ===" << std::endl;

    // 1. Загружаем реальные данные из твоего файла
    BarReader reader;
    std::string filepath = "D:/AHexaTrader/1DataFiles/raw/GBPJPY/GBPJPY_M5.bin"; 
    
    if (!reader.LoadFile(filepath)) {
        std::cerr << "Не удалось запустить симуляцию. Проверьте путь к файлу." << std::endl;
        return 1;
    }

    auto indicator_provider = std::make_unique<IndicatorProvider>();
    auto tr_state_machine = std::make_unique<TRStateMachine>();
    auto abc_detector = std::make_unique<ABCPatternDetector>();
    auto trend_analyzer = std::make_unique<TrendAnalyzer>();
    auto trade_engine = std::make_unique<TradeEngine>();

    // Связываем торговый движок с анализаторами
    trade_engine->SetSignals(tr_state_machine.get(), trend_analyzer.get());

    std::cout << "Запуск бэктеста на реальных данных..." << std::endl;
    
    // 2. Проходим по всем загруженным барам
    const auto& bars = reader.GetBars();
    for (const auto& bar : bars) {
        IndicatorSnapshot snapshot;
        snapshot.bar = bar;
        
        // Считаем индикаторы
        indicator_provider->OnBarClose(snapshot);
        
        snapshot.ol_value = indicator_provider->GetOL();
        snapshot.sp_value = indicator_provider->GetSP();
        snapshot.lb_value = indicator_provider->GetLB();

        // Передаем в анализаторы и движок
        tr_state_machine->OnBarClose(snapshot);
        abc_detector->OnBarClose(snapshot);
        trend_analyzer->OnBarClose(snapshot);
        trade_engine->OnBarClose(snapshot);
    }

    std::cout << "=== Бэктест завершен ===" << std::endl;
    return 0;
}
