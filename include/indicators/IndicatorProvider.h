#pragma once
#include "core/IModule.h"
class IndicatorProvider : public IModule {
public:
    void OnBarClose(const IndicatorSnapshot& snapshot) override;
};
