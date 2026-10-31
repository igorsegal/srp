\# Спецификация Интерфейсов Модулей (Module API Contracts) Версия 1.0

\*Проект: SRP / АО Зотик. Назначение: Жесткая фиксация входных и выходных данных для каждого модуля конвейера. Защита от спагетти-кода (смешивания логики).\*



\## 1. Общие структуры данных (Global Data Structures)

Эти структуры передаются между модулями как единые пакеты данных.



\### `BarSnapshot` (Снимок закрытого бара)

Передается от `ClosedBarFeed` во все остальные модули. 

\*   `symbol` (string)

\*   `timeframe` (int)

\*   `close\_time` (datetime) — время закрытия бара (это и есть `known\_at`)

\*   `open`, `high`, `low`, `close` (double)

\*   `tick\_volume` (int)



\### `IndicatorSnapshot` (Снимок индикаторов на баре)

Передается от `IndicatorProvider`.

\*   `bar` (`BarSnapshot`)

\*   `ol\_value` (double) — Основная линия AO

\*   `sp\_value` (double) — Сигнальная полоса

\*   `lb\_value` (double) — Линия Баланса

\*   `wpr\_fast` (double) — Значение WPR (период 21)

\*   `wpr\_slow` (double) — Значение WPR (период 252)



\### `Point` (Временная точка)

\*   `time` (datetime)

\*   `price` (double)



\---



\## 2. Спецификация модулей (Module I/O)



\### Модуль: `IndicatorProvider` (Провайдер индикаторов)

\*   \*\*Задача:\*\* Считать ОЛ, СП, ЛБ и WPR на основе формул из исходников.

\*   \*\*Вход (Input):\*\* `BarSnapshot`

\*   \*\*Выход (Output):\*\* `IndicatorSnapshot`

\*   \*\*Запрет:\*\* Не ищет бугорки, не принимает торговых решений.



\### Модуль: `BumpDetector` (Детектор бугорков ОЛ)

\*   \*\*Задача:\*\* Находить локальные экстремумы ОЛ и синхронизировать их с ценовыми экстремумами.

\*   \*\*Вход (Input):\*\* Поток `IndicatorSnapshot` (нужны `ol\_value` и цены из `bar`).

\*   \*\*Выход (Output):\*\* Объект `Bump`

&#x20;   \*   `bump\_id` (int)

&#x20;   \*   `direction` (enum: UP / DOWN)

&#x20;   \*   `price\_extremum\_time` (datetime) — когда был ценовой пик

&#x20;   \*   `bump\_peak\_time` (datetime) — когда ОЛ сформировала вершину (лаг \~2 бара)

&#x20;   \*   `ol\_peak\_value` (double)

&#x20;   \*   `known\_at` (datetime) — момент, когда ОЛ развернулась и бугорок подтвержден.

\*   \*\*Событие в EventLog:\*\* `BUMP\_CONFIRMED`



\### Модуль: `TRStateMachine` (Движок Точки Развития)

\*   \*\*Задача:\*\* Управлять жизненным циклом ТР (поиск, проверка 4-мя барами, отработка, отмена).

\*   \*\*Вход (Input):\*\* 

&#x20;   \*   `IndicatorSnapshot` (для проверки пересечений ОЛ/СП)

&#x20;   \*   Объект `Bump`

\*   \*\*Выход (Output):\*\* Объект `TR\_Object`

&#x20;   \*   `tr\_id` (int)

&#x20;   \*   `direction` (enum: BUY / SELL)

&#x20;   \*   `extreme\_time` (datetime) — время ценового пика

&#x20;   \*   `target\_level` (double) — уровень ТР

&#x20;   \*   `base\_level` (double) — основание ТР

&#x20;   \*   `potential` (enum: HIGH / LOW / DISPUTED)

&#x20;   \*   `status` (enum: CONFIRMED / WORKED\_OUT / CANCELLED)

&#x20;   \*   `known\_at` (datetime)

\*   \*\*События в EventLog:\*\* `TR\_CONFIRMED`, `TR\_LEVEL\_BROKEN`, `TR\_BASE\_BROKEN`



\### Модуль: `A\_Wave\_Strength\_Evaluator` (Оценка силы А-волны)

\*   \*\*Задача:\*\* Понять, является ли текущая волна А сильной или слабой.

\*   \*\*Вход (Input):\*\* 

&#x20;   \*   `TR\_Object` (только что отработанная)

&#x20;   \*   История предыдущих `Bump` и предыдущих `StartPoint` (Точек Старта)

\*   \*\*Выход (Output):\*\* Объект `WaveStrength`

&#x20;   \*   `tr\_id` (int)

&#x20;   \*   `strength` (enum: STRONG / WEAK / UNRESOLVED)

&#x20;   \*   `reason` (string) — например, "Перебит предыдущий бугорок, но не пробита ТС".

&#x20;   \*   `known\_at` (datetime)



\### Модуль: `Hypothesis\_Tracker` (Трекер Гипотез)

\*   \*\*Задача:\*\* Принимать решение о смене волновой модели при пробитии Точки Старта (ТС).

\*   \*\*Вход (Input):\*\* 

&#x20;   \*   Событие пробоя `StartPoint` (ТС)

&#x20;   \*   Объект `WaveStrength` (оценка силы А-волны)

\*   \*\*Выход (Output):\*\* Объект `Hypothesis`

&#x20;   \*   `hypothesis\_id` (int)

&#x20;   \*   `type` (enum: H1\_DEEP\_B / H2\_NEW\_A)

&#x20;   \*   `known\_at` (datetime)

\*   \*\*Событие в EventLog:\*\* `HYPOTHESIS\_FORMED`



\### Модуль: `WPR\_Validator` (Валидатор WPR)

\*   \*\*Задача:\*\* Проверять выход быстрой линии WPR из Экстремальной Зоны (ЭЗ).

\*   \*\*Вход (Input):\*\* `IndicatorSnapshot`

\*   \*\*Выход (Output):\*\* Объект `WPR\_Signal`

&#x20;   \*   `direction` (enum: BUY / SELL)

&#x20;   \*   `is\_valid` (bool)

&#x20;   \*   `known\_at` (datetime)

\*   \*\*Событие в EventLog:\*\* `WPR\_EXIT\_EZ`



\### Модуль: `SystemEntry\_Validator` (Финальный валидатор ТСВ)

\*   \*\*Задача:\*\* Собрать все сигналы вместе и выдать разрешение на сделку (11 пунктов ТСВ).

\*   \*\*Вход (Input):\*\* 

&#x20;   \*   `TR\_Object` (готовность к отработке)

&#x20;   \*   `Hypothesis` (текущая гипотеза H1 или H2)

&#x20;   \*   `WPR\_Signal` (выход из ЭЗ)

&#x20;   \*   Данные старшего ТФ (наличие волны С)

&#x20;   \*   Флаг `NewsFilter` (отсутствие новостей)

\*   \*\*Выход (Output):\*\* Объект `TradeSignal`

&#x20;   \*   `signal\_id` (int)

&#x20;   \*   `direction` (enum: BUY / SELL)

&#x20;   \*   `entry\_price` (double)

&#x20;   \*   `stop\_loss` (double) — за ТР или РК

&#x20;   \*   `take\_profit` (double) — 150% / 156.18%

&#x20;   \*   `is\_approved` (bool) — true, если совпали все жесткие условия

&#x20;   \*   `rejection\_reason` (string) — если false, то почему (например, "Нет волны С на H1")

&#x20;   \*   `known\_at` (datetime)

\*   \*\*Событие в EventLog:\*\* `TRADE\_SIGNAL\_GENERATED`



\---



\## 3. Принципы взаимодействия

1\.  \*\*Однонаправленный поток:\*\* Данные текут строго от `IndicatorProvider` к `TradeSignal`. Никаких обратных циклов (модуль принятия решений не может изменить показания индикатора).

2\.  \*\*Отсутствие дублирования:\*\* `TRStateMachine` не считает `High/Low` сам. Он берет их из `BarSnapshot`.

3\.  \*\*Строгий known\_at:\*\* Любой Output содержит время `known\_at`, когда результат был вычислен. Если сигнал сформирован на закрытии бара, `known\_at` равен времени закрытия этого бара.

