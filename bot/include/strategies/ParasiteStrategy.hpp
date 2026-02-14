#pragma once

#include "CodingSnake.hpp"

#include <string>

namespace bot {

// 寄生虫：贴近排行榜第一（这里近似为最长蛇）并保持侧向伴随
std::string decideParasite(const GameState& state);

}  // namespace bot
