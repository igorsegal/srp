import pandas as pd
import matplotlib.pyplot as plt
# Загружаем данные сделок по фунту и иене
df = pd.read_csv("pure_two_pair_trades.csv")
df['DateTime'] = pd.to_datetime(df['Timestamp'], unit='s')
# Универсальная группировка по месяцам
df['YearMonth'] = df['DateTime'].dt.to_period('M')
monthly_pnl = df.groupby('YearMonth')['PnL_USD'].sum()
monthly_df = pd.DataFrame({'PnL': monthly_pnl})
monthly_df.index = monthly_df.index.to_timestamp()
monthly_df['StartBalance'] = 1000.0 + monthly_df['PnL'].cumsum().shift(1).fillna(0.0)
monthly_df['ReturnPct'] = (monthly_df['PnL'] / monthly_df['StartBalance']) * 100.0
# Цветовая гамма: голубой для прибыли, оранжевый для убытка
colors = ['#00BFFF' if x >= 0 else '#FF8C00' for x in monthly_df['ReturnPct']]
# Построение диаграммы без цифровых подписей на столбцах
fig, ax = plt.subplots(figsize=(12, 6))
ax.bar(range(len(monthly_df)), monthly_df['ReturnPct'], color=colors, width=0.8)
# Настройка внешнего вида
ax.axhline(0, color='grey', linewidth=0.8, linestyle='--')
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
plt.title("Помесячная доходность (GBPUSD + USDJPY)", fontsize=14, fontweight='bold')
plt.xlabel("Месяц", fontsize=11)
plt.ylabel("Доходность (%)", fontsize=11)
plt.tight_layout()
plt.savefig("clean_cascade.png", dpi=300)
print("График успешно создан и сохранен как clean_cascade.png!")
plt.show()