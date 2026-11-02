// ==========================================
// Файл: IndicatorProvider.h
// ==========================================
#pragma once
#include "core/IModule.h"
#include "core/Types.h"

class IndicatorProvider : public IModule {
public:
    IndicatorProvider();
    void OnBarClose(const IndicatorSnapshot& snapshot) override;

    // Геттеры для получения текущих значений
    double GetOL() const { return current_ol; }
    double GetSP() const { return current_sp; }
    double GetLB() const { return current_lb; }

private:
    // История для расчета скользящих средних
    std::vector<double> close_history;
    std::vector<double> median_history;

    double current_ol = 0.0;
    double current_sp = 0.0;
    double current_lb = 0.0;

    // Функции расчета
    double CalculateSMA(int period, const std::vector<double>& data);
    double CalculateEMA(int period, const std::vector<double>& data);
};
