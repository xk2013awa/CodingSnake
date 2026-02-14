#pragma once

#include "CodingSnake.hpp"

#include <string>
#include <vector>

namespace bot {

// 四个基础方向，便于策略复用
const std::vector<std::string>& allDirections();

// 根据方向计算下一步坐标
Point nextPoint(const Point& from, const std::string& direction);

// 判断某个方向是否安全（不越界且不撞障碍）
bool isSafeDirection(const GameState& state, const Point& head, const std::string& direction);

// 在候选方向中选一个最接近目标点的方向（可选是否要求安全）
std::string chooseDirectionToward(
    const GameState& state,
    const Point& from,
    const Point& target,
    bool safeOnly
);

}  // namespace bot
