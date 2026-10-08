cmake_minimum_required(VERSION 3.15)
project(SRP_TradingSystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(srp_backtester src/main.cpp)
target_include_directories(srp_backtester PRIVATE include)

