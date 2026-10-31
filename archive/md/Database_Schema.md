\# Спецификация Контрактов Данных и Схемы БД (Версия 1.0)

\*Проект: SRP / АО Зотик. Назначение: Фундамент для Event-Driven бэктестера и торгового советника. Защита от Look-ahead bias.\*



\## 1. Общие принципы (The Zero Contract)

1\. \*\*Иммутабельность (Неизменяемость):\*\* Если событие записано в журнал, его нельзя изменить или удалить. Если гипотеза изменилась (например, Слабая А превратилась в Новую А), мы не переписываем старую запись, а создаем \*\*новую версию\*\* события.

2\. \*\*Разделение времени:\*\* Везде, где это возможно, используются два поля:

&#x20;  \*   `event\_time` — время бара (или тика), к которому относится событие (например, бар, на котором сформировался экстремум).

&#x20;  \*   `known\_at` — время, когда у алгоритма появились все данные для подтверждения (например, через 4 закрытых бара или через 2 бара лага бугорка).

3\. \*\*Единый формат цен:\*\* Все цены хранятся в виде чисел с плавающей точкой (Double). Все времена — в формате Unix Timestamp.



\---



\## 2. Справочник рыночных данных (Market Data Tables)

Эти таблицы содержат «сырые» данные и значения индикаторов.



\### Таблица `01\_Bars` (Барные данные)

\*   `id` (INT, Primary Key)

\*   `symbol` (VARCHAR)

\*   `timeframe` (INT, например 5, 15, 60)

\*   `event\_time` (TIMESTAMP — время открытия бара)

\*   `close\_time` (TIMESTAMP — время закрытия бара)

\*   `open`, `high`, `low`, `close` (DOUBLE)

\*   `tick\_volume` (INT)



\### Таблица `02\_Indicators` (Показания индикаторов)

\*   `bar\_id` (INT, Foreign Key -> `01\_Bars.id`)

\*   `ol` (DOUBLE — Основная линия AO)

\*   `sp` (DOUBLE — Сигнальная полоса)

\*   `lb` (DOUBLE — Линия Баланса)

\*   `wpr\_fast` (DOUBLE — WPR 21)

\*   `wpr\_slow` (DOUBLE — WPR 252)

\*   `mas\_level\_1` ... `mas\_level\_n` (DOUBLE — Уровни наклона)



\---



\## 3. Реестр структурных объектов (Structure Registries)

Таблицы для хранения объектов, которые имеют свой жизненный цикл.



\### Таблица `10\_Bumps` (Бугорки ОЛ)

\*   `bump\_id` (INT, Primary Key)

\*   `symbol`, `timeframe`

\*   `price\_extremum\_time` (TIMESTAMP — бар ценового High/Low)

\*   `bump\_peak\_time` (TIMESTAMP — бар вершины бугорка ОЛ)

\*   `ol\_value` (DOUBLE — значение ОЛ на пике)

\*   `known\_at` (TIMESTAMP — момент, когда ОЛ развернулась и подтвердила бугорок)

\*   `is\_noise` (BOOLEAN — является ли бугорком по правилам асимметрии)



\### Таблица `20\_DevelopmentPoints` (ТР — Точки Развития)

\*   `tr\_id` (INT, Primary Key)

\*   `symbol`, `timeframe`, `direction` (BUY/SELL)

\*   `extreme\_time` (TIMESTAMP — время ценового экстремума)

\*   `base\_level` (DOUBLE — цена основания ТР)

\*   `target\_level` (DOUBLE — ценовой уровень ТР)

\*   `status` (VARCHAR — `SEARCHING`, `CHECKING\_4\_BARS`, `CONFIRMED`, `WORKED\_OUT`, `CANCELLED`)

\*   `known\_at` (TIMESTAMP — момент готовности, т.е. 4-й бар)

\*   `potential` (VARCHAR — `HIGH`, `LOW`, `DISPUTED`)



\### Таблица `30\_StartPoints` (ТС — Точки Старта)

\*   `start\_point\_id` (INT, Primary Key)

\*   `tr\_id` (INT, Foreign Key -> `20\_DevelopmentPoints.tr\_id`)

\*   `price\_level` (DOUBLE — уровень основания А-волны)

\*   `status` (VARCHAR — `ACTIVE`, `BROKEN`)

\*   `strength\_of\_A` (VARCHAR — `STRONG`, `WEAK`, `UNRESOLVED` — оценка силы волны А на момент `known\_at`)



\### Таблица `40\_Waves` (Волновая разметка)

\*   `wave\_id` (INT, Primary Key)

\*   `wave\_type` (VARCHAR — `A`, `B`, `C`)

\*   `tr\_id` (INT, Foreign Key -> `20\_DevelopmentPoints.tr\_id`)

\*   `status` (VARCHAR — `FORMING`, `CONFIRMED`, `INVALIDATED`)

\*   `known\_at` (TIMESTAMP)



\### Таблица `50\_Nakos\_Axes` (Наклонные оси)

\*   `nakos\_id` (INT, Primary Key)

\*   `point\_X\_time`, `point\_X\_price` (TIMESTAMP, DOUBLE)

\*   `point\_Y\_time`, `point\_Y\_price` (TIMESTAMP, DOUBLE)

\*   `status` (VARCHAR — `CANDIDATE`, `ACTIVE`, `BROKEN`)

\*   `known\_at` (TIMESTAMP — момент построения линии)



\### Таблица `60\_RubberSquares` (Резиновые квадраты)

\*   `rk\_id` (INT, Primary Key)

\*   `start\_time`, `end\_time` (TIMESTAMP)

\*   `area` (DOUBLE — Площадь D\*T)

\*   `status` (VARCHAR — `ACTIVE`, `DESTROYED`)

\*   `known\_at` (TIMESTAMP)



\---



\## 4. Журнал Событий (Event Store / Audit Log)

Это главная таблица, в которой фиксируются \*все\* изменения. Каждый модуль (ТР, ВУК, Накось) пишет сюда свои сигналы.



\### Таблица `90\_Event\_Log`

\*   `event\_id` (INT, Primary Key)

\*   `event\_time` (TIMESTAMP — время, когда это произошло на рынке)

\*   `known\_at` (TIMESTAMP — время, когда алгоритм осознал событие)

\*   `event\_type` (VARCHAR) — например:

&#x20;   \*   `CROSS\_OL\_SP` (Пересечение ОЛ и СП)

&#x20;   \*   `BUMP\_CONFIRMED` (Бугорок подтвержден)

&#x20;   \*   `TR\_CONFIRMED` (ТР готова к работе)

&#x20;   \*   `TR\_LEVEL\_BROKEN` (Пробитие уровня ТР — отработка)

&#x20;   \*   `TR\_BASE\_BROKEN` (Пробитие основания ТР — отмена)

&#x20;   \*   `START\_POINT\_BROKEN` (Пробой Точки Старта)

&#x20;   \*   `WPR\_EXIT\_EZ` (Выход из экстремальной зоны)

&#x20;   \*   `RK\_DESTROYED` (Разрушение Резинового Квадрата)

\*   `object\_id` (INT — ID объекта, к которому относится событие, например, `tr\_id`)

\*   `price` (DOUBLE — цена, на которой произошло событие)

\*   `payload` (TEXT/JSON — любые дополнительные данные, например, значения индикаторов на этот момент)



\---



\## 5. Контракты принятия решений (Decision Contracts)

Эти таблицы хранят результаты работы «Комбайна» (Слой 6 Архитектуры).



\### Таблица `70\_Hypotheses` (Гипотезы)

\*   `hypothesis\_id` (INT, Primary Key)

\*   `start\_point\_id` (INT, Foreign Key -> `30\_StartPoints`)

\*   `hypothesis\_type` (VARCHAR — `H1\_DEEP\_B`, `H2\_NEW\_A`)

\*   `confidence` (VARCHAR — `HIGH`, `MEDIUM`, `LOW`)

\*   `created\_at` (TIMESTAMP)



\### Таблица `80\_Trade\_Signals` (Сигналы на вход)

\*   `signal\_id` (INT, Primary Key)

\*   `bar\_time` (TIMESTAMP — бар, на котором принято решение)

\*   `direction` (VARCHAR — `BUY`, `SELL`)

\*   `entry\_price`, `sl`, `tp` (DOUBLE)

\*   `reasoning` (TEXT — строка, объясняющая, какие 11 условий ТСВ совпали)

\*   `status` (VARCHAR — `PENDING`, `EXECUTED`, `REJECTED`)



\---



\## 6. Как это работает при бэктесте (Replay Engine)

Когда мы будем тестировать систему на истории, мы запустим \*\*Replay Engine\*\*:

1\. Он читает `01\_Bars` бар за баром.

2\. На каждом баре он обновляет индикаторы и пишет в `90\_Event\_Log` только те события, чей `known\_at` наступил на этом баре.

3\. Если на баре возникает сигнал (совпали 11 условий), создается запись в `80\_Trade\_Signals`.

4\. Все данные из `90\_Event\_Log` выгружаются в CSV для анализа: мы сможем посмотреть, сколько раз система ошибалась, и на каком именно шаге (например, слишком ранний `TR\_CONFIRMED`).

