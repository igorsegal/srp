#pragma once
#include "core/IModule.h"
#include "core/Types.h"

class ABCPatternDetector : public IModule {
public:
    enum ABCState {
        WAIT_A,
        WAIT_B,
        WAIT_C
    };

    ABCPatternDetector();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;

private:
    ABCState current_state;
    bool is_bullish; 
    double prev_ol;
    double prev_sp;
};
