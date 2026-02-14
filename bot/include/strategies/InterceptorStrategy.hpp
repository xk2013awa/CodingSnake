#pragma once

#include "CodingSnake.hpp"

#include <string>

namespace bot {

// 拦截者：优先追击场上最长蛇，尝试拦截其未来路径
std::string decideInterceptor(const GameState& state);

}  // namespace bot
