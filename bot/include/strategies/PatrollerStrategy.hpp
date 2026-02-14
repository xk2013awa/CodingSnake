#pragma once

#include "CodingSnake.hpp"

#include <string>

namespace bot {

// 巡逻兵：维持在矩形轨迹巡逻，受阻时临时避障并回归
std::string decidePatroller(const GameState& state);

}  // namespace bot
