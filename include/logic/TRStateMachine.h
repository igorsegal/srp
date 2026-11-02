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
    
    // Переменные для отслеживания пересечения
    double prev_ol;
    double prev_sp;
    bool is_initialized;

    // Переменные для отслеживания экстремума (ТР)
    bool is_searching_up;      // Ищем ТР вверх или вниз
    double tr_price_level;     // Ценовой уровень экстремума (ТР)
    int bars_since_extremum;   // Счетчик баров после экстремума
    double current_extremum_val; // Значение экстремума на графике цены
};
