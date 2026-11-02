#include "logic/TradeEngine.h"
#include "core/EventLogger.h"
#include <iostream>

TradeEngine::TradeEngine() 
    : current_position(NO_POSITION), entry_price(0), stop_loss(0), take_profit(0), tr_sm(nullptr), trend(nullptr) {}

void TradeEngine::SetSignals(TRStateMachine* tr_sm_ptr, TrendAnalyzer* trend_ptr) {
    tr_sm = tr_sm_ptr;
    trend = trend_ptr;
}

void TradeEngine::OnBarClose(const IndicatorSnapshot& snapshot) {
    if (!tr_sm || !trend) return;

    double current_price = snapshot.bar.close;

    if (current_position == LONG) {
        if (current_price >= take_profit) {
            EventLogger::Log("TRADE", "СДЕЛКА ЗАКРЫТА В ПЛЮС (TP). Профит!", snapshot.bar.close_time);
            current_position = NO_POSITION;
        } else if (current_price <= stop_loss) {
            EventLogger::Log("TRADE", "СДЕЛКА ЗАКРЫТА В МИНУС (SL). Лось.", snapshot.bar.close_time);
            current_position = NO_POSITION;
        }
    } 
    else if (current_position == SHORT) {
        if (current_price <= take_profit) {
            EventLogger::Log("TRADE", "СДЕЛКА ЗАКРЫТА В ПЛЮС (TP). Профит!", snapshot.bar.close_time);
            current_position = NO_POSITION;
        } else if (current_price >= stop_loss) {
            EventLogger::Log("TRADE", "СДЕЛКА ЗАКРЫТА В МИНУС (SL). Лось.", snapshot.bar.close_time);
            current_position = NO_POSITION;
        }
    }

    if (current_position == NO_POSITION) {
        if (tr_sm->IsTRConfirmed() && tr_sm->IsSearchingUp() && trend->IsUptrend()) {
            current_position = LONG;
            entry_price = current_price;
            stop_loss = tr_sm->GetTRLevel(); 
            take_profit = tr_sm->GetTargets().target_161; 
            EventLogger::Log("TRADE", "ОТКРЫТА СДЕЛКА НА ПОКУПКУ (LONG). Вход: " + std::to_string(entry_price), snapshot.bar.close_time);
        }
        else if (tr_sm->IsTRConfirmed() && !tr_sm->IsSearchingUp() && !trend->IsUptrend()) {
            current_position = SHORT;
            entry_price = current_price;
            stop_loss = tr_sm->GetTRLevel(); 
            take_profit = tr_sm->GetTargets().target_161; 
            EventLogger::Log("TRADE", "ОТКРЫТА СДЕЛКА НА ПРОДАЖУ (SHORT). Вход: " + std::to_string(entry_price), snapshot.bar.close_time);
        }
    }
}
