cmake_minimum_required(VERSION 3.10)
project(SRP_Trading_System)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

# Добавили src/core/BarReader.cpp
add_executable(srp_simulator
    main.cpp
    src/core/BarReader.cpp
    src/indicators/IndicatorProvider.cpp
    src/logic/TRStateMachine.cpp
    src/logic/ABCPatternDetector.cpp
    src/logic/TrendAnalyzer.cpp
    src/logic/TradeEngine.cpp
)
