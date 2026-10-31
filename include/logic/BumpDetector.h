#pragma once
#include "core/IModule.h"
class BumpDetector : public IModule {
public:
    void OnBarClose(const IndicatorSnapshot& snapshot) override;
};
