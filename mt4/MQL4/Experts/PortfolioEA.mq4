//+------------------------------------------------------------------+
//|                                                 PortfolioEA.mq4  |
//|                                      Copyright 2026, AHexaTrader |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2026"
#property link      "https://www.mql5.com"
#property version   "1.00"
#property strict

// Импорт нашей C++ DLL из папки MQL4\Libraries\
#import "PortfolioCore.dll"
  double GetTargetLotSize(double balance, double startDeposit, double baseRiskPct, double riskPips);
  bool   CheckMarginShieldLevel(double marginLevelPct);
#import

// Входные параметры советника
input double InpStartDeposit = 2000.0;   // Стартовый депозит для формулы обратного корня
input double InpBaseRiskPct  = 0.10;   // Базовый риск (10%)
input double InpRiskPips     = 35.0;   // Стоп-лосс в пунктах
input double InpMinMarginPct = 500.0;  // Порог маржинального файрвола (%)

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
{
   Print("PortfolioEA initialized successfully. C++ DLL Core linked.");
   return(INIT_SUCCEEDED);
}

//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
   Print("PortfolioEA stopped.");
}

//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
{
   // Проверка нового бара (M5)
   static datetime lastBarTime = 0;
   datetime currentBarTime = iTime(_Symbol, PERIOD_M5, 0);
   if (currentBarTime == lastBarTime) return;
   lastBarTime = currentBarTime;

   // 1. Проверяем Firewall Risk Manager (уровень маржи)
   double currentMargin = AccountMargin() > 0 ? (AccountFreeMargin() / AccountMargin()) * 100.0 : 9999.0; // упрощенный расчет или AccountInfoDouble(ACCOUNT_MARGIN_LEVEL)
   // В MT4 лучше использовать встроенную функцию AccountInfoDouble(ACCOUNT_MARGIN_LEVEL):
   double marginLevel = AccountInfoDouble(ACCOUNT_MARGIN_LEVEL);
   if (marginLevel > 0.0 && !CheckMarginShieldLevel(marginLevel)) {
      Print("FIREWALL ALERT: Margin level is below threshold! New trades blocked.");
      return;
   }

   // 2. Расчет динамического лота по формуле обратного корня через C++ DLL
   double currentBal = AccountBalance();
   double recommendedLot = GetTargetLotSize(currentBal, InpStartDeposit, InpBaseRiskPct, InpRiskPips);

   // Здесь далее пойдет диспетчеризация ордеров по парам: GBPUSD, USDJPY, USDCAD, USDCHF
}
//+------------------------------------------------------------------+