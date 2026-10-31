#ifndef RISK_MANAGER_H
#define RISK_MANAGER_H

#include <iostream>
#include <string>
#include "WaveABC.h"

// Структура торгового ордера для интеграции с RiskManager и HexaTrade
struct TradeOrder {
    long long ticket;
    double entryPrice;
    double volume;
    bool isCounterTrend;     // Признак торговли в коррекции / против основного тренда[cite: 8]
    bool isPartialClosed;    // Флаг выполнения частичной фиксации[cite: 8]
    double firstTargetPrice; // Уровень первой цели для частичного закрытия[cite: 8]
    bool isTargetReached;    // Флаг достижения основного тейк-профита
};

class RiskManager {
public:
    RiskManager() {}

    // 1. Контроль "обезличенных" ордеров
    // Если волна C завершилась (пробит ТР2 или перебит НП), а цель не взята — принудительно закрываем ордер[cite: 8]
    void CheckDeclassifiedOrders(TradeOrder& order, const WaveABCData& waveData) {
        if (waveData.cStatus == WaveCStatus::COMPLETED && !order.isTargetReached) {
            CloseOrderMarket(order, "Declassified: Wave C finished without hitting TP");[cite: 8]
        }
    }

    // 2. Сопровождение сделок в коррекции (против тренда / в волне B)[cite: 8]
    // Короткие цели, закрытие >= 50% объема, перевод остатка в безубыток[cite: 8]
    void ManageCounterTrendPosition(TradeOrder& order, double currentPrice) {
        if (order.isCounterTrend && !order.isPartialClosed) {
            if (currentPrice >= order.firstTargetPrice) {
                ClosePartialVolume(order, 0.5); // Закрываем 50% объема[cite: 8]
                MoveToBreakEven(order);         // Перевод в безубыток[cite: 8]
                order.isPartialClosed = true;
            }
        }
    }

private:
    // Принудительное закрытие ордера по рынку (исключение пересиживания убытков)[cite: 8]
    void CloseOrderMarket(TradeOrder& order, const std::string& reason) {
        std::cout << "[RiskManager] Принудительное закрытие ордера #" << order.ticket 
                  << " по рынку. Причина: " << reason << std::endl;
        // Здесь вызывается системная функция отправки ордера на закрытие
        order.volume = 0.0;
    }

    // Частичное закрытие объема (50% и более)[cite: 8]
    void ClosePartialVolume(TradeOrder& order, double fraction) {
        double closedPart = order.volume * fraction;
        std::cout << "[RiskManager] Частичное закрытие " << (fraction * 100) << "% объема "
                  << "для ордера #" << order.ticket << " (закрыто: " << closedPart << ")" << std::endl;
        order.volume -= closedPart;
    }

    // Перевод позиции в безубыток (BE)[cite: 8]
    void MoveToBreakEven(TradeOrder& order) {
        std::cout << "[RiskManager] Перевод оставшейся части ордера #" << order.ticket 
                  << " в безубыток (стоп в точке входа)." << std::endl;
    }
};

#endif // RISK_MANAGER_H