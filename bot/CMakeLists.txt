cmake_minimum_required(VERSION 3.16)
project(CodingSnakeBots LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 公共头文件目录：
# - include/            项目内部头文件
# - .                   直接包含 CodingSnake.hpp
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(bot_core
    src/common/BotConfigLoader.cpp
    src/common/DirectionUtils.cpp
    src/strategies/InterceptorStrategy.cpp
    src/strategies/GluttonStrategy.cpp
    src/strategies/PatrollerStrategy.cpp
    src/strategies/ParasiteStrategy.cpp
)

target_include_directories(bot_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(bot_main src/main.cpp)
target_link_libraries(bot_main PRIVATE bot_core)
