cmake_minimum_required(VERSION 3.12)
project(PortfolioCoreDLL CXX)

set(CMAKE_CXX_STANDARD 17)

# Статическая линковка рантайма (/MT) для независимости DLL на VPS
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(PortfolioCoreDLL SHARED 
    src/DllMain.cpp
)

# Задаем имя скомпилированного файла PortfolioCore.dll
set_target_properties(PortfolioCoreDLL PROPERTIES 
    PREFIX ""
    SUFFIX ".dll"
    OUTPUT_NAME "PortfolioCore"
)