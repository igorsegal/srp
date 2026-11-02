#include "core/BarReader.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <chrono>

// Структуры для точного чтения бинарных данных
#pragma pack(push, 1)
struct BinaryHeader {
    char magic[8];
    int32_t version;
    int32_t record_size;
    int32_t period_seconds;
    int32_t digits;
    double point;
    int64_t bar_count;
    int64_t first_time;
    int64_t last_time;
    int32_t symbol_len;
};

struct BinaryBar {
    int64_t time;
    double open;
    double high;
    double low;
    double close;
    int64_t tick_volume;
    int32_t spread;
    int64_t real_volume;
};
#pragma pack(pop)

BarReader::BarReader() : period_seconds(0) {}

bool BarReader::LoadFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ОШИБКА: Не удалось открыть файл " << filepath << std::endl;
        return false;
    }

    // 1. Читаем заголовок
    BinaryHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(BinaryHeader));
    
    if (std::string(header.magic, 8) != "XFBAR001") {
        std::cerr << "ОШИБКА: Неверный формат файла (magic)!" << std::endl;
        return false;
    }

    period_seconds = header.period_seconds;

    // 2. Читаем имя символа (оно идет сразу после фиксированного заголовка)
    char* symbol_buf = new char[header.symbol_len + 1];
    file.read(symbol_buf, header.symbol_len);
    symbol_buf[header.symbol_len] = '\0';
    symbol = std::string(symbol_buf);
    delete[] symbol_buf;

    std::cout << "Успешно загружен: " << symbol << " | Баров: " << header.bar_count << std::endl;

    // 3. Читаем сами свечи
    bars.reserve(header.bar_count);
    for (int64_t i = 0; i < header.bar_count; ++i) {
        BinaryBar raw_bar;
        file.read(reinterpret_cast<char*>(&raw_bar), sizeof(BinaryBar));
        
        BarSnapshot bar;
        // Конвертируем Unix время в chrono
        bar.close_time = std::chrono::system_clock::from_time_t(raw_bar.time);
        bar.open = raw_bar.open;
        bar.high = raw_bar.high;
        bar.low = raw_bar.low;
        bar.close = raw_bar.close;
        bar.tick_volume = static_cast<int>(raw_bar.tick_volume);
        bar.symbol = symbol;
        bar.timeframe = period_seconds;
        
        bars.push_back(bar);
    }

    file.close();
    return true;
}
