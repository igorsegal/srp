#pragma once
#include "core/IModule.h"
#include "core/EventLogger.h"
#include <vector>

class TRStateMachine : public IModule {
public:
    enum State {
        WAIT_CROSS,       // Условие 1: Ждем пересечения ОЛ и СП
        SEARCH_EXTREMUM,  // Условие 2: Ищем бугорок на ОЛ и экстремум на цене
        CHECK_4_BARS,     // Условие 3: Проверка временем (4 бара)
        TR_CONFIRMED      // Условие 4: ТР пробита (отработка)
    };

    TRStateMachine();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;

private:
    State current_state;
    
    // Для отслеживания пересечения
    double prev_ol;
    double prev_sp;
    bool is_initialized;

    // Данные о текущей ТР
    bool is_searching_up;      
    double wave_start_price;   // Цена, от которой началось движение (старт волны)
    double tr_price_level;     // Ценовой уровень экстремума (ТР)
    int bars_since_extremum;   
    double current_extremum_val; 

    // Потенциал ТР
    TRTargets current_targets;

    void CalculatePotential(const IndicatorSnapshot& snapshot);
};
