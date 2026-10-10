import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Загружаем журнал сделок для лимита 10.0 лотов
df = pd.read_csv("portfolio_4pair_lot10_trades.csv")
df["DateTime"] = pd.to_datetime(df["Timestamp"], unit="s")
df["YearMonth"] = df["DateTime"].dt.to_period("M")

# Считаем помесячный PnL и доходность
monthly_pnl = df.groupby("YearMonth")["PnL_USD"].sum()
monthly_df = pd.DataFrame({"PnL": monthly_pnl})
monthly_df.index = monthly_df.index.to_timestamp()
monthly_df["StartBalance"] = (
    1000.0 + monthly_df["PnL"].cumsum().shift(1).fillna(0.0)
)
monthly_df["ReturnPct"] = (
    monthly_df["PnL"] / monthly_df["StartBalance"]
) * 100.0

# Логика накопительного каскада (Waterfall)
bottoms = []
heights = []
colors = []
curr_cum = 0.0

for val in monthly_df["ReturnPct"]:
  if val >= 0:
    bottoms.append(curr_cum)
    heights.append(val)
    colors.append("#00BFFF")  # Голубой для плюса
    curr_cum += val
  else:
    curr_cum += val
    bottoms.append(curr_cum)
    heights.append(-val)
    colors.append("#FF8C00")  # Оранжевый для минуса

# Построение каскадной диаграммы
fig, ax = plt.subplots(figsize=(14, 7))
ax.bar(range(len(monthly_df)), heights, bottom=bottoms, color=colors, width=0.8)

ax.axhline(0, color="grey", linewidth=0.8, linestyle="--")
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)

plt.title(
    "Каскадная диаграмма с накоплением (4 пары, Макс. лот 10.0, Риск 4%)",
    fontsize=13,
    fontweight="bold",
    pad=15,
)
plt.xlabel("Месяц", fontsize=11)
plt.ylabel("Накопительный эффект / Доходность (%)", fontsize=11)

plt.tight_layout()
plt.savefig("waterfall_4pair_lot10.png", dpi=300)
print(
    "Каскадная диаграмма успешно сохранена в файл waterfall_4pair_lot10.png!"
)
plt.show()