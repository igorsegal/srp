#include "logic/TRStateMachine.h"
TRStateMachine::TRStateMachine() : current_state(WAIT_CROSS), bars_since_extremum(0) {}
void TRStateMachine::OnBarClose(const IndicatorSnapshot& snapshot) {
    switch (current_state) {
        case WAIT_CROSS:
            // TODO: Логика ожидания пересечения ОЛ и СП
            break;
        case SEARCH_EXTREMUM:
            // TODO: Логика поиска бугорка
            break;
        case CHECK_4_BARS:
            // TODO: Логика проверки 4-х баров и отмены/подтверждения ТР
            break;
        case TR_CONFIRMED:
            // TODO: Логика ожидания пробоя (Отработка vs Отмена)
            break;
        default:
            break;
    }
}
