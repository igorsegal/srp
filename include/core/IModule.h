#pragma once
#include "Types.h"
class IModule {
public:
    virtual ~IModule() = default;
    // Вызывается движком строго на закрытии бара (known_at)
    virtual void OnBarClose(const IndicatorSnapshot& snapshot) = 0;
};
