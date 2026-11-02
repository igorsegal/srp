#pragma once
#include "core/IModule.h"
#include "core/Types.h"

class ABCPatternDetector : public IModule {
public:
    enum ABCState {
        WAIT_A, // Ищем начало волны А (пересечение ОЛ и СП)
        WAIT_B, // Ждем коррекции (заход за СП)
        WAIT_C  // Ждем завершения волны С (новая ТР)
    };

    ABCPatternDetector();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;

private:
    ABCState current_state;
    bool is_bullish; // Направление текущего паттерна (вверх или вниз)
    double prev_ol;
    double prev_sp;
};
