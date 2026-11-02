#pragma once
#include "core/IModule.h"
#include "core/EventLogger.h"
#include "core/Types.h"
#include <vector>

class TRStateMachine : public IModule {
public:
    enum State {
        WAIT_CROSS,
        SEARCH_EXTREMUM,
        CHECK_4_BARS,
        TR_CONFIRMED
    };

    TRStateMachine();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;

    bool IsTRConfirmed() const { return current_state == TR_CONFIRMED; }
    bool IsSearchingUp() const { return is_searching_up; }
    double GetTRLevel() const { return tr_price_level; }
    TRTargets GetTargets() const { return current_targets; }

private:
    State current_state;
    double prev_ol;
    double prev_sp;
    bool is_initialized;

    bool is_searching_up;      
    double wave_start_price;   
    double tr_price_level;     
    int bars_since_extremum;   
    double current_extremum_val; 

    TRTargets current_targets;

    void CalculatePotential(const IndicatorSnapshot& snapshot);
};
