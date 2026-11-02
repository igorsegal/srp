#include "logic/ABCPatternDetector.h"
#include "core/EventLogger.h"

ABCPatternDetector::ABCPatternDetector()
    : current_state(WAIT_A), is_bullish(true), prev_ol(0), prev_sp(0) {}

void ABCPatternDetector::OnBarClose(const IndicatorSnapshot& snapshot) {
    double ol = snapshot.ol_value;
    double sp = snapshot.sp_value;

    switch (current_state) {
        case WAIT_A:
            if (prev_ol <= prev_sp && ol > sp) {
                current_state = WAIT_B;
                is_bullish = true;
                EventLogger::Log("ABC", "Начало волны А (вверх). Ждем волну В.", snapshot.bar.close_time);
            }
            else if (prev_ol >= prev_sp && ol < sp) {
                current_state = WAIT_B;
                is_bullish = false;
                EventLogger::Log("ABC", "Начало волны А (вниз). Ждем волну В.", snapshot.bar.close_time);
            }
            break;

        case WAIT_B:
            if (is_bullish && ol < sp) {
                current_state = WAIT_C;
                EventLogger::Log("ABC", "Волна В подтверждена (заход за СП вниз). Ждем волну С.", snapshot.bar.close_time);
            }
            else if (!is_bullish && ol > sp) {
                current_state = WAIT_C;
                EventLogger::Log("ABC", "Волна В подтверждена (заход за СП вверх). Ждем волну С.", snapshot.bar.close_time);
            }
            break;

        case WAIT_C:
            if (is_bullish) {
                if (prev_ol >= prev_sp && ol < sp) {
                    EventLogger::Log("ABC", "Волна С ЗАВЕРШЕНА (новая ТР вниз). Паттерн ABC готов!", snapshot.bar.close_time);
                    current_state = WAIT_A;
                }
            } else {
                if (prev_ol <= prev_sp && ol > sp) {
                    EventLogger::Log("ABC", "Волна С ЗАВЕРШЕНА (новая ТР вверх). Паттерн ABC готов!", snapshot.bar.close_time);
                    current_state = WAIT_A;
                }
            }
            break;
    }
    prev_ol = ol;
    prev_sp = sp;
}
