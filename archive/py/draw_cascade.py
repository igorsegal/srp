import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
# Загружаем чистые данные сделок
df = pd.read_csv("pure_two_pair_trades.csv")
df['DateTime'] = pd.to_datetime(df['Timestamp'], unit='s')
df['YearMonth'] = df['DateTime'].dt.to_period('M')
# Считаем помесячный PnL и процент доходности
monthly_pnl = df.groupby('YearMonth')['PnL_USD'].sum()
monthly_df = pd.DataFrame({'PnL': monthly_pnl})
monthly_df.index = monthly_df.index.to_timestamp()
monthly_df['StartBalance'] = 1000.0 + monthly_df['PnL'].cumsum().shift(1).fillna(0.0)
monthly_df['ReturnPct'] = (monthly_df['PnL'] / monthly_df['StartBalance']) * 100.0
# Логика накопительного каскада с накоплением (Waterfall)
bottoms = []
heights = []
colors = []
curr_cum = 0.0
for val in monthly_df['ReturnPct']:
    if val >= 0:
        bottoms.append(curr_cum)
        heights.append(val)
        colors.append('#00BFFF') # Голубой для плюса
        curr_cum += val
    else:
        curr_cum += val
        bottoms.append(curr_cum)
        heights.append(-val)
        colors.append('#FF8C00') # Оранжевый для минуса
# Построение графика
fig, ax = plt.subplots(figsize=(14, 7))
ax.bar(range(len(monthly_df)), heights, bottom=bottoms, color=colors, width=0.8)
ax.axhline(0, color='grey', linewidth=0.8, linestyle='--')
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
plt.title("Каскадная диаграмма с накоплением (Waterfall Chart) — GBPUSD + USDJPY", fontsize=14, fontweight='bold', pad=15)
plt.xlabel("Месяц", fontsize=11)
plt.ylabel("Накопительный эффект / Доходность (%)", fontsize=11)
plt.tight_layout()
plt.savefig("waterfall_cascade.png", dpi=300)
print("Каскадная диаграмма с накоплением успешно сохранена в waterfall_cascade.png!")
plt.show()