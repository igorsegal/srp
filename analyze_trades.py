import pandas as pd
import numpy as np
# Загружаем журнал сделок из готового файла
df = pd.read_csv("portfolio_4pair_lot10_trades.csv")
total_trades = len(df)
wins = df[df['PnL_USD'] >= 0]
losses = df[df['PnL_USD'] < 0]
win_count = len(wins)
loss_count = len(losses)
win_rate = (win_count / total_trades) * 100 if total_trades > 0 else 0
gross_profit = wins['PnL_USD'].sum()
gross_loss = abs(losses['PnL_USD'].sum())
net_profit = gross_profit - gross_loss
profit_factor = gross_profit / gross_loss if gross_loss > 0 else 0
max_win = wins['PnL_USD'].max() if win_count > 0 else 0
max_loss = losses['PnL_USD'].min() if loss_count > 0 else 0
# Абсолютно безопасный расчет серий побед/убытков через reset_index
df['IsWin'] = df['PnL_USD'] >= 0
df['StreakGroup'] = (df['IsWin'] != df['IsWin'].shift()).cumsum()
streak_df = df.groupby(['StreakGroup', 'IsWin']).size().reset_index(name='StreakLen')
max_win_streak = streak_df[streak_df['IsWin'] == True]['StreakLen'].max() if not streak_df[streak_df['IsWin'] == True].empty else 0
max_loss_streak = streak_df[streak_df['IsWin'] == False]['StreakLen'].max() if not streak_df[streak_df['IsWin'] == False].empty else 0
# Расчет годового коэффициента Шарпа по дневной доходности
df['DateTime'] = pd.to_datetime(df['Timestamp'], unit='s')
df['Date'] = df['DateTime'].dt.date
daily_pnl = df.groupby('Date')['PnL_USD'].sum()
daily_balance = 1000.0 + daily_pnl.cumsum()
daily_return = daily_pnl / daily_balance.shift(1).fillna(1000.0)
mean_ret = daily_return.mean()
std_ret = daily_return.std()
sharpe_ratio = (mean_ret / std_ret) * np.sqrt(252) if std_ret > 0 and not np.isnan(std_ret) else 0
# Формирование текстового отчета
report = f"""===================================================================
 ПОЛНАЯ СТАТИСТИКА ПОРТФЕЛЯ (4 пары, Макс. лот 10.0, Риск 4%)
===================================================================
 Всего сделок          : {total_trades}
 Прибыльных сделок     : {win_count} ({win_rate:.1f}%)
 Убыточных сделок      : {loss_count} ({100-win_rate:.1f}%)
 Валовая прибыль       : ${gross_profit:,.2f}
 Валовый убыток        : ${gross_loss:,.2f}
 Чистая прибыль        : ${net_profit:,.2f}
 Профит-фактор         : {profit_factor:.2f}
 Самая крупная победа  : ${max_win:,.2f}
 Самый крупный убыток  : ${max_loss:,.2f}
 Макс. серия побед     : {int(max_win_streak)} подряд
 Макс. серия убытков   : {int(max_loss_streak)} подряд
 Коэффициент Шарпа (Годовой) : {sharpe_ratio:.2f}
===================================================================
"""
print(report)
# Сохранение в единый файл отчета
output_file = "comprehensive_statistics_report.txt"
with open(output_file, "w", encoding="utf-8") as f:
    f.write(report)
print(f"Полный статистический отчет успешно сохранен в файл: {output_file}")