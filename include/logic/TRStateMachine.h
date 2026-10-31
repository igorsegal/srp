#pragma once
#include "core/IModule.h"
#include "core/EventLogger.h"
class TRStateMachine : public IModule {
public:
    enum State {
        WAIT_CROSS,
        SEARCH_EXTREMUM,
        CHECK_4_BARS,
        TR_CONFIRMED,
        TR_BASE_BROKEN_STATE,
        A_WAVE_VALIDATION,
        HYPOTHESIS_TRACKER,
        ABC_TRACKER
    };
    TRStateMachine();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;
private:
    State current_state;
    int bars_since_extremum;
    // TODO: Добавить поля для хранения уровней ТР, экстремумов, retry_count и т.д.
};
