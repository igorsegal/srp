cmake_minimum_required(VERSION 3.10)
project(SRP_Trading_System)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Указываем, где лежат заголовочные файлы (.h)
include_directories(include)

# Собираем исполняемый файл из всех наших исходников
add_executable(srp_simulator
    src/main.cpp
    src/indicators/IndicatorProvider.cpp
    src/logic/TRStateMachine.cpp
    src/logic/ABCPatternDetector.cpp
    src/logic/TrendAnalyzer.cpp
    src/logic/TradeEngine.cpp
)
