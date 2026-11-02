#include "logic/TRStateMachine.h"
#include <iostream>
#include <cmath>

TRStateMachine::TRStateMachine() 
    : current_state(WAIT_CROSS), prev_ol(0), prev_sp(0), is_initialized(false),
      is_searching_up(false), tr_price_level(0), bars_since_extremum(0), current_extremum_val(0) {}

void TRStateMachine::OnBarClose(const IndicatorSnapshot& snapshot) {
    // Если это первый бар, просто запоминаем значения и выходим
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
            // УСЛОВИЕ 1: Ищем пересечение ОЛ и СП
            // Если ОЛ пересекла СП снизу вверх (бычий разворот)
            if (prev_ol <= prev_sp && current_ol > current_sp) {
                current_state = SEARCH_EXTREMUM;
                is_searching_up = true; // Ищем ТР вверх (разворот тренда вверх)
                current_extremum_val = low; // Начинаем отслеживать минимум
                EventLogger::Log("TR_SM", "Пересечение ОЛ/СП вверх. Ищем ТР вверх.", snapshot.bar.close_time);
            }
            // Если ОЛ пересекла СП сверху вниз (медвежий разворот)
            else if (prev_ol >= prev_sp && current_ol < current_sp) {
                current_state = SEARCH_EXTREMUM;
                is_searching_up = false; // Ищем ТР вниз
                current_extremum_val = high; // Начинаем отслеживать максимум
                EventLogger::Log("TR_SM", "Пересечение ОЛ/СП вниз. Ищем ТР вниз.", snapshot.bar.close_time);
            }
            break;

        case SEARCH_EXTREMUM:
            // УСЛОВИЕ 2: Ищем бугорок на ОЛ и экстремум на цене.
            // В упрощенном виде: если ищем ТР вверх, отслеживаем минимум цены.
            if (is_searching_up) {
                if (low < current_extremum_val) {
                    current_extremum_val = low; // Обновляем минимум
                } else {
                    // Цена начала расти (появился экстремум). Фиксируем ТР.
                    tr_price_level = current_extremum_val;
                    bars_since_extremum = 0;
                    current_state = CHECK_4_BARS;
                    EventLogger::Log("TR_SM", "Найден экстремум вверх. Ждем 4 бара.", snapshot.bar.close_time);
                }
            } else {
                if (high > current_extremum_val) {
                    current_extremum_val = high; // Обновляем максимум
                } else {
                    // Цена начала падать (появился экстремум). Фиксируем ТР.
                    tr_price_level = current_extremum_val;
                    bars_since_extremum = 0;
                    current_state = CHECK_4_BARS;
                    EventLogger::Log("TR_SM", "Найден экстремум вниз. Ждем 4 бара.", snapshot.bar.close_time);
                }
            }
            break;

        case CHECK_4_BARS:
            // УСЛОВИЕ 3: Проверка временем
            bars_since_extremum++;
            
            // Если цена пробивает наш экстремум раньше 4-х баров - ТР сломана, все заново
            if ((is_searching_up && close < tr_price_level) || (!is_searching_up && close > tr_price_level)) {
                current_state = WAIT_CROSS;
                EventLogger::Log("TR_SM", "Экстремум пробит раньше 4-х баров. Сброс.", snapshot.bar.close_time);
            } 
            // Если прошло 4 бара, и экстремум не пробит - ТР сформирована!
            else if (bars_since_extremum >= 4) {
                current_state = TR_CONFIRMED;
                EventLogger::Log("TR_SM", "Прошло 4 бара. ТР сформирована и ждет отработки!", snapshot.bar.close_time);
            }
            break;

        case TR_CONFIRMED:
            // УСЛОВИЕ 4: Отработка ТР (пробой 5-м баром)
            if (is_searching_up && close > tr_price_level) {
                 EventLogger::Log("TR_SM", "ТР вверх ОТРАБОТАНА! (Пробой 5-м баром)", snapshot.bar.close_time);
                 current_state = WAIT_CROSS; // Ищем новое пересечение
            } else if (!is_searching_up && close < tr_price_level) {
                 EventLogger::Log("TR_SM", "ТР вниз ОТРАБОТАНА! (Пробой 5-м баром)", snapshot.bar.close_time);
                 current_state = WAIT_CROSS; // Ищем новое пересечение
            }
            // Если цена пошла обратно и пробила основание (сломала ТР)
            if ((is_searching_up && low < tr_price_level) || (!is_searching_up && high > tr_price_level)) {
                 // В реальной системе здесь проверяется отмена ТР, но для MVP сбрасываем
                 current_state = WAIT_CROSS;
            }
            break;
    }

    // В конце метода сохраняем текущие значения для следующего бара
    prev_ol = current_ol;
    prev_sp = current_sp;
}
