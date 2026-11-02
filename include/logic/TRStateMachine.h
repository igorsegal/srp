    // Геттеры для Торгового Движка
    bool IsTRConfirmed() const { return current_state == TR_CONFIRMED; }
    bool IsSearchingUp() const { return is_searching_up; }
    double GetTRLevel() const { return tr_price_level; }
    TRTargets GetTargets() const { return current_targets; }
