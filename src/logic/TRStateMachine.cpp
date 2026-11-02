#include "logic/TRStateMachine.h"
#include <iostream>
#include <cmath>

TRStateMachine::TRStateMachine() 
    : current_state(WAIT_CROSS), prev_ol(0), prev_sp(0), is_initialized(false),
      is_searching_up(false), wave_start_price(0), tr_price_level(0), 
      bars_since_extremum(0), current_extremum_val(0) {}

void TRStateMachine::OnBarClose(const IndicatorSnapshot& snapshot) {
    if (!is_initialized) {
        prev_ol = snapshot.ol_value;
        prev_sp = snapshot.sp_value;
        is_initialized = true;
        return;
    }

    double current_ol = snapshot.ol_value;
    double current_sp = snapshot.sp_value;
    double high = snapshot.bar.high;
    double low = snapshot.bar.low;
    double close = snapshot.bar.close;

    switch (current_state) {
        case WAIT_CROSS:
            if (prev_ol <= prev_sp && current_ol > current_sp) {
                current_state = SEARCH_EXTREMUM;
                is_searching_up = true; 
                wave_start_price = close; 
                current_extremum_val = low; 
                EventLogger::Log("TR_SM", "Пересечение ОЛ/СП вверх. Ищем ТР вверх.", snapshot.bar.close_time);
            }
            else if (prev_ol >= prev_sp && current_ol < current_sp) {
                current_state = SEARCH_EXTREMUM;
                is_searching_up = false; 
                wave_start_price = close; 
                current_extremum_val = high; 
                EventLogger::Log("TR_SM", "Пересечение ОЛ/СП вниз. Ищем ТР вниз.", snapshot.bar.close_time);
            }
            break;

        case SEARCH_EXTREMUM:
            if (is_searching_up) {
                if (low < current_extremum_val) {
                    current_extremum_val = low; 
                } else {
                    tr_price_level = current_extremum_val;
                    bars_since_extremum = 0;
                    current_state = CHECK_4_BARS;
                    EventLogger::Log("TR_SM", "Найден экстремум вверх. Ждем 4 бара.", snapshot.bar.close_time);
                }
            } else {
                if (high > current_extremum_val) {
                    current_extremum_val = high; 
                } else {
                    tr_price_level = current_extremum_val;
                    bars_since_extremum = 0;
                    current_state = CHECK_4_BARS;
                    EventLogger::Log("TR_SM", "Найден экстремум вниз. Ждем 4 бара.", snapshot.bar.close_time);
                }
            }
            break;

        case CHECK_4_BARS:
            bars_since_extremum++;
            if ((is_searching_up && close < tr_price_level) || (!is_searching_up && close > tr_price_level)) {
                current_state = WAIT_CROSS;
                EventLogger::Log("TR_SM", "Экстремум пробит раньше 4-х баров. Сброс.", snapshot.bar.close_time);
            } 
            else if (bars_since_extremum >= 4) {
                current_state = TR_CONFIRMED;
                CalculatePotential(snapshot);
                EventLogger::Log("TR_SM", "Прошло 4 бара. ТР сформирована! Ожидаем отработку.", snapshot.bar.close_time);
            }
            break;

        case TR_CONFIRMED:
            if (is_searching_up && close > tr_price_level) {
                 EventLogger::Log("TR_SM", "ТР вверх ОТРАБОТАНА! (Пробой 5-м баром)", snapshot.bar.close_time);
                 current_state = WAIT_CROSS; 
            } else if (!is_searching_up && close < tr_price_level) {
                 EventLogger::Log("TR_SM", "ТР вниз ОТРАБОТАНА! (Пробой 5-м баром)", snapshot.bar.close_time);
                 current_state = WAIT_CROSS; 
            }
            break;
    }

    prev_ol = current_ol;
    prev_sp = current_sp;
}

void TRStateMachine::CalculatePotential(const IndicatorSnapshot& snapshot) {
    current_targets.is_valid = true;
    double wave_length = std::abs(tr_price_level - wave_start_price);
    current_targets.level_100 = wave_length;
    double direction = is_searching_up ? 1.0 : -1.0;

    current_targets.target_123 = tr_price_level + (wave_length * 1.23 * direction);
    current_targets.target_138 = tr_price_level + (wave_length * 1.38 * direction);
    current_targets.target_161 = tr_price_level + (wave_length * 1.618 * direction);
    current_targets.target_176 = tr_price_level + (wave_length * 1.764 * direction);

    if (std::abs(snapshot.ol_value) < 0.005) {
        current_targets.is_high_potential = false;
        EventLogger::Log("FIBO", "Потенциал НИЗКИЙ (ТР возле нуля).", snapshot.bar.close_time);
    } else {
        current_targets.is_high_potential = true;
        EventLogger::Log("FIBO", "Потенциал ВЫСОКИЙ (сильный импульс).", snapshot.bar.close_time);
    }

    std::string targets_msg = "Цели: 123%=" + std::to_string(current_targets.target_123) + 
                              " | 161%=" + std::to_string(current_targets.target_161) +
                              " | 176%=" + std::to_string(current_targets.target_176);
    EventLogger::Log("FIBO", targets_msg, snapshot.bar.close_time);
}
