#pragma once
#include "core/Types.h"
#include <string>
#include <vector>

class BarReader {
public:
    BarReader();
    // Загружает файл и возвращает true, если успешно
    bool LoadFile(const std::string& filepath);
    
    // Возвращает массив прочитанных баров
    const std::vector<BarSnapshot>& GetBars() const { return bars; }
    std::string GetSymbol() const { return symbol; }
    int GetTimeframeSeconds() const { return period_seconds; }

private:
    std::vector<BarSnapshot> bars;
    std::string symbol;
    int period_seconds;
};
