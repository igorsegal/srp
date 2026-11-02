#pragma once
#include "core/IModule.h"
#include "core/Types.h"
#include <vector>

class TrendAnalyzer : public IModule {
public:
    TrendAnalyzer();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;

    // Геттеры для получения данных о наклоне
    double GetMA3() const { return ma3_value; }
    double GetSlopeAngle() const { return slope_angle; }
    bool IsUptrend() const { return slope_angle > 0; }

private:
    std::vector<double> close_history;
    double ma3_value;
    double slope_angle;

    double CalculateSMA(int period, const std::vector<double>& data);
};
