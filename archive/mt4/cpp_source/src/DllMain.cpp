#include <windows.h>
#include "Bar.h"
#include "RiskManager.h"
#include "AO_Zotik.h"
#include "WPR_VSmark.h"
#include "WaveABC.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

extern "C" {
    // Добавлен __stdcall для корректной работы стека в MQL4
    __declspec(dllexport) double __stdcall GetTargetLotSize(double balance, double startDeposit, double baseRiskPct, double riskPips) {
        double risk = RiskManagerModule::calculateRiskPct(balance, startDeposit, baseRiskPct);
        return RiskManagerModule::calculateLotSize(balance, risk, riskPips, 0.01, 50.0);
    }

    // Добавлен __stdcall для файрвола маржи
    __declspec(dllexport) bool __stdcall CheckMarginShieldLevel(double marginLevelPct) {
        return RiskManagerModule::checkMarginShield(marginLevelPct);
    }
}