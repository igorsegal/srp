#include "DataManager.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
bool DataManager::loadData(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[DataManager] Error: Could not open file " << filepath << std::endl;
        return false;
    }
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    // Кандидаты размеров записей и заголовков для любых брокерских форматов (MT4/MT5/Custom)
    struct FormatCandidate {
        size_t headerSize;
        size_t recordSize;
        bool timeIs64Bit;
    };
    FormatCandidate candidates[] = {
        {6,  60, false}, // Стандартный SRP M5/H1 (6 байт заголовок, 60 байт запись, 4-байт время)
        {6,  60, true},  // 6 байт заголовок, 60 байт запись, 8-байт время
        {148, 44, false}, // MT4 HST v400 (148 байт заголовок, 44 байта запись)
        {148, 60, false}, // MT4 HST v401 (148 байт заголовок, 60 байт запись)
        {0,  60, false},
        {0,  44, false},
        {48, 60, false},
        {32, 60, false},
        {6,  56, false},
        {6,  48, false},
        {6,  40, false}
    };
    size_t bestHeader = 0;
    size_t bestRecord = 0;
    bool bestTime64 = false;
    size_t bestCount = 0;
    bool found = false;
    for (const auto& cand : candidates) {
        if (fileSize > cand.headerSize && (fileSize - cand.headerSize) % cand.recordSize == 0) {
            size_t count = (fileSize - cand.headerSize) / cand.recordSize;
            if (count > 0) {
                // Проверим первый бар на валидность таймстампа (между 2000 и 2040 годами)
                file.seekg(cand.headerSize, std::ios::beg);
                int64_t tVal = 0;
                if (cand.timeIs64Bit) {
                    file.read(reinterpret_cast<char*>(&tVal), 8);
                } else {
                    int32_t t32 = 0;
                    file.read(reinterpret_cast<char*>(&t32), 4);
                    tVal = static_cast<int64_t>(t32);
                }
                // 946684800 = 2000-01-01, 2208988800 = 2040-01-01 (или если это относительный таймстамп > 0)
                if ((tVal >= 946684800LL && tVal <= 2208988800LL) || (tVal > 0 && tVal < 2000000000LL)) {
                    bestHeader = cand.headerSize;
                    bestRecord = cand.recordSize;
                    bestTime64 = cand.timeIs64Bit;
                    bestCount = count;
                    found = true;
                    break;
                }
            }
        }
    }
    if (!found) {
        // Fallback: берем первый попавшийся делитель без строгой проверки времени
        for (const auto& cand : candidates) {
            if (fileSize > cand.headerSize && (fileSize - cand.headerSize) % cand.recordSize == 0) {
                bestHeader = cand.headerSize;
                bestRecord = cand.recordSize;
                bestTime64 = cand.timeIs64Bit;
                bestCount = (fileSize - cand.headerSize) / cand.recordSize;
                found = true;
                break;
            }
        }
    }
    if (!found || bestCount == 0) {
        std::cerr << "[DataManager] Error: Unsupported binary format for " << filepath << " (Size: " << fileSize << ")" << std::endl;
        return false;
    }
    bars.resize(bestCount);
    symbol = "EURUSD";
    periodSeconds = (filepath.find("H1") != std::string::npos) ? 3600 : 300;
    file.seekg(bestHeader, std::ios::beg);
    for (size_t i = 0; i < bestCount; ++i) {
        size_t recordStartPos = file.tellg();
        if (bestTime64) {
            file.read(reinterpret_cast<char*>(&bars[i].time), 8);
        } else {
            int32_t t32 = 0;
            file.read(reinterpret_cast<char*>(&t32), 4);
            bars[i].time = static_cast<int64_t>(t32);
            // Пропуск возможных байтов выравнивания или spare внутри записи
            if (bestRecord == 60 && bestHeader == 6) {
                char pad[4];
                file.read(pad, 4);
            } else if (bestRecord == 44) {
                // MT4 44-байтовая запись: time(4), open(8), low(8), high(8), close(8), volume(8)
                // Или разный порядок. Считаем OHLC и volume стандартно:
            }
        }
        // Чтение OHLC в зависимости от размера записи
        if (bestRecord == 60) {
            if (!bestTime64 && bestHeader == 6) {
                // Уже прочитали time(4) и pad(4), читаем OHLC (32), tick_vol(8), spread(4), real_vol(8)
                file.read(reinterpret_cast<char*>(&bars[i].open), 8);
                file.read(reinterpret_cast<char*>(&bars[i].high), 8);
                file.read(reinterpret_cast<char*>(&bars[i].low), 8);
                file.read(reinterpret_cast<char*>(&bars[i].close), 8);
                file.read(reinterpret_cast<char*>(&bars[i].tick_volume), 8);
                file.read(reinterpret_cast<char*>(&bars[i].spread), 4);
                file.read(reinterpret_cast<char*>(&bars[i].real_volume), 8);
            } else {
                file.read(reinterpret_cast<char*>(&bars[i].open), 8);
                file.read(reinterpret_cast<char*>(&bars[i].high), 8);
                file.read(reinterpret_cast<char*>(&bars[i].low), 8);
                file.read(reinterpret_cast<char*>(&bars[i].close), 8);
                file.read(reinterpret_cast<char*>(&bars[i].tick_volume), 8);
                file.read(reinterpret_cast<char*>(&bars[i].spread), 4);
                file.read(reinterpret_cast<char*>(&bars[i].real_volume), 8);
            }
        } else if (bestRecord == 44) {
            // MT4 формат: open, low, high, close (или open, high, low, close)
            // Обычно в MT4: open(8), low(8), high(8), close(8), volume(8)
            file.read(reinterpret_cast<char*>(&bars[i].open), 8);
            file.read(reinterpret_cast<char*>(&bars[i].low), 8);
            file.read(reinterpret_cast<char*>(&bars[i].high), 8);
            file.read(reinterpret_cast<char*>(&bars[i].close), 8);
            double vol = 0;
            file.read(reinterpret_cast<char*>(&vol), 8);
            bars[i].tick_volume = static_cast<int64_t>(vol);
            bars[i].spread = 0;
            bars[i].real_volume = 0;
        } else {
            // Общий fallback для других размеров
            file.read(reinterpret_cast<char*>(&bars[i].open), 8);
            file.read(reinterpret_cast<char*>(&bars[i].high), 8);
            file.read(reinterpret_cast<char*>(&bars[i].low), 8);
            file.read(reinterpret_cast<char*>(&bars[i].close), 8);
            file.read(reinterpret_cast<char*>(&bars[i].tick_volume), 8);
            bars[i].spread = 0;
            bars[i].real_volume = 0;
        }
        // Переход к следующей записи точно по recordSize
        file.seekg(recordStartPos + bestRecord, std::ios::beg);
    }
    std::cout << "[DataManager] Loaded " << bars.size() << " bars from " << filepath 
              << " (Hdr: " << bestHeader << ", Rec: " << bestRecord << ")" << std::endl;
    return true;
}