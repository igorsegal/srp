#include "DataManager.h"
#include <iostream>
#include <fstream>

DataManager::DataManager() {}
DataManager::~DataManager() {}

bool DataManager::loadData(const std::string& filepath) {
    std::cout << "[DataManager] Loading XFBAR binary data from: " << filepath << std::endl;
    
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "[DataManager] Error: Could not open file " << filepath << std::endl;
        return false;
    }

    // 1. Чтение заголовка файла
    char magic[9] = {0};
    file.read(magic, 8);
    if (std::string(magic) != "XFBAR001") {
        std::cout << "[DataManager] Error: Invalid magic number: " << magic << std::endl;
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char*>(&version_), sizeof(version_));
    
    int32_t record_size = 0;
    file.read(reinterpret_cast<char*>(&record_size), sizeof(record_size));
    if (record_size != 60) {
        std::cout << "[DataManager] Error: Unexpected record size: " << record_size << " (expected 60)" << std::endl;
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char*>(&period_seconds_), sizeof(period_seconds_));
    file.read(reinterpret_cast<char*>(&digits_), sizeof(digits_));
    file.read(reinterpret_cast<char*>(&point_), sizeof(point_));
    file.read(reinterpret_cast<char*>(&bar_count_), sizeof(bar_count_));
    file.read(reinterpret_cast<char*>(&first_time_), sizeof(first_time_));
    file.read(reinterpret_cast<char*>(&last_time_), sizeof(last_time_));

    int32_t symbol_len = 0;
    file.read(reinterpret_cast<char*>(&symbol_len), sizeof(symbol_len));

    if (symbol_len > 0 && symbol_len < 256) {
        std::vector<char> symbol_buf(symbol_len + 1, 0);
        file.read(symbol_buf.data(), symbol_len);
        symbol_ = std::string(symbol_buf.data());
    } else {
        symbol_ = "UNKNOWN";
    }

    std::cout << "[DataManager] Symbol: " << symbol_ << ", Period: " << period_seconds_ << "s, Bars: " << bar_count_ << std::endl;

    // 2. Чтение массива свечей
    bars_.resize(bar_count_);
    std::streamsize bytes_to_read = static_cast<std::streamsize>(bar_count_ * sizeof(Bar));

    if (file.read(reinterpret_cast<char*>(bars_.data()), bytes_to_read)) {
        std::cout << "[DataManager] Successfully loaded " << bars_.size() << " bars." << std::endl;
    } else {
        std::cout << "[DataManager] Error reading bar data from file." << std::endl;
        file.close();
        return false;
    }

    file.close();
    return true;
}

const std::vector<Bar>& DataManager::getBars() const {
    return bars_;
}

