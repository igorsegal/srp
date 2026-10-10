#pragma once
#include <cmath>

class RiskManagerModule {
public:
    // Дискретный расчет риска по формуле обратного корня
    static double calculateRiskPct(double balance, double startDeposit, double baseRiskPct) {
        double balanceRatio = balance / startDeposit;
        if (balanceRatio < 1.0) balanceRatio = 1.0;
        return baseRiskPct / std::sqrt(balanceRatio);
    }

    // Расчет размера лота
    static double calculateLotSize(double balance, double riskPct, double riskPips, double minLot, double maxLot) {
        double riskUSD = balance * riskPct;
        if (riskPips < 10.0) riskPips = 10.0;
        double lotSize = riskUSD / (riskPips * 10.0);
        if (lotSize < minLot) lotSize = minLot;
        if (lotSize > maxLot) lotSize = maxLot;
        return lotSize;
    }

    // Firewall Risk Manager: проверка уровня маржи портфеля (щит от маржин-колла < 500%)
    static bool checkMarginShield(double marginLevelPct) {
        // Если маржа не передана (например, в тестере нет позиций) или выше 500% — торговля разрешена
        if (marginLevelPct <= 0.0) return true; 
        if (marginLevelPct < 500.0) {
            return false; // Файрвол срабатывает и блокирует новые входы
        }
        return true;
    }
};