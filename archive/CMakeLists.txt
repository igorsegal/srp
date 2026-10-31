cmake_minimum_required(VERSION 3.15)
project(srp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(srp
    src/main.cpp
    src/DataManager.cpp
    src/AO_Zotik.cpp
    src/WPR_VSmark.cpp
    src/WaveABC.cpp
    src/VolumeVUK.cpp
    src/BacktestEngine.cpp
    src/PerformanceAnalyzer.cpp
)
