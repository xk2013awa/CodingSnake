#pragma once

#include "CodingSnake.hpp"

#include <string>

namespace bot {

// 暴食者：优先抢夺“我能先到、且别人也想吃”的食物
std::string decideGlutton(const GameState& state);

}  // namespace bot
