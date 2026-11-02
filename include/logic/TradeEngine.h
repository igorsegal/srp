#pragma once
#include "core/IModule.h"
#include "core/Types.h"
#include "logic/TRStateMachine.h"
#include "logic/TrendAnalyzer.h"

class TradeEngine : public IModule {
public:
    enum PositionState {
        NO_POSITION,
        LONG,
        SHORT
    };

    TradeEngine();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;
    void SetSignals(TRStateMachine* tr_sm, TrendAnalyzer* trend_analyzer);

private:
    PositionState current_position;
    double entry_price;
    double stop_loss;
    double take_profit;

    TRStateMachine* tr_sm;
    TrendAnalyzer* trend;
};
