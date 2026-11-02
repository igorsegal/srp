#pragma once
#include "core/Types.h"

class IModule {
public:
    virtual ~IModule() = default;
    virtual void OnBarClose(const IndicatorSnapshot& snapshot) = 0;
};
