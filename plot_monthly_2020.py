import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
csv_path = r"D:\AHexaTrader\2026.10.08 SRP\monthly_trades_2020.csv"
df = pd.read_csv(csv_path)
df['DateTime'] = pd.to_datetime(df['Timestamp'], unit='s')
df.set_index('DateTime', inplace=True)
# Группируем PnL по месяцам
monthly = df.resample('ME')['PnL_USD'].sum()
starting_balance = 10000.0
monthly_df = pd.DataFrame({'PnL': monthly})
monthly_df['Cumulative_PnL'] = monthly_df['PnL'].cumsum()
monthly_df['Total_Balance'] = starting_balance + monthly_df['Cumulative_PnL']
# Считаем совокупную (кумулятивную) доходность в процентах от начального депозита
monthly_df['Cumulative_Return_Pct'] = ((monthly_df['Total_Balance'] - starting_balance) / starting_balance) * 100.0
report_path = r"D:\AHexaTrader\2026.10.08 SRP\monthly_returns_report_2020.csv"
monthly_df.to_csv(report_path)
print(f"[Export] Cumulative report saved to: {report_path}")
# Построение плавного совокупного графика
plt.figure(figsize=(14, 6), dpi=300)
plt.plot(monthly_df.index, monthly_df['Cumulative_Return_Pct'], color='#1f77b4', linewidth=2.0, label='Cumulative Return (%)')
plt.axhline(0, color='black', linewidth=0.8, linestyle='--')
plt.title("Masterforex-V Portfolio (2020+): Cumulative Total Return (%) [Risk 2.0% + HTF EMA 50 Filter]", fontsize=14, fontweight='bold', pad=15)
plt.xlabel("Year - Month", fontsize=11, labelpad=10)
plt.ylabel("Cumulative Growth (%)", fontsize=11, labelpad=10)
plt.gca().xaxis.set_major_locator(mdates.YearLocator())
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y'))
plt.grid(True, linestyle=':', alpha=0.5)
plt.legend(loc='upper left')
plt.tight_layout()
chart_path = r"D:\AHexaTrader\2026.10.08 SRP\monthly_returns_2020_cumulative_chart.png"
plt.savefig(chart_path)
print(f"[Plot] Cumulative chart saved to: {chart_path}")
print("---------------------------------------------------------")
print(f" Final Cumulative Return : +{monthly_df['Cumulative_Return_Pct'].iloc[-1]:.2f}%")
print(f" Peak Cumulative Return  : +{monthly_df['Cumulative_Return_Pct'].max():.2f}%")
print("---------------------------------------------------------")