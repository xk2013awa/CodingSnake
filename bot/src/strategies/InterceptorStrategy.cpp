#include "strategies/InterceptorStrategy.hpp"

#include "common/DirectionUtils.hpp"

#include <limits>

namespace bot {

namespace {

// 通过头和第二节身体推断“上一回合移动方向”
Point inferMoveVector(const Snake& snake) {
    if (snake.blocks.size() < 2) {
        return {1, 0};
    }

    const Point& head = snake.blocks[0];
    const Point& neck = snake.blocks[1];
    return {head.x - neck.x, head.y - neck.y};
}

}  // namespace

std::string decideInterceptor(const GameState& state) {
    const Snake me = state.getMySnake();
    const auto others = state.getOtherPlayers();

    if (others.empty()) {
        return "right";
    }

    // 1) 锁定长度最长的目标
    Snake target = others.front();
    for (const auto& s : others) {
        if (s.length > target.length) {
            target = s;
        }
    }

    // 2) 根据对手方向预判其 4 步后位置
    const Point moveVec = inferMoveVector(target);
    Point predicted = target.head;
    predicted.x += moveVec.x * 4;
    predicted.y += moveVec.y * 4;

    // 3) 优先选安全且更接近预测点的方向
    std::string bestSafe = "right";
    int bestSafeDist = std::numeric_limits<int>::max();
    for (const auto& dir : allDirections()) {
        if (!isSafeDirection(state, me.head, dir)) {
            continue;
        }
        const Point next = nextPoint(me.head, dir);
        const int dist = next.distance(predicted);
        if (dist < bestSafeDist) {
            bestSafeDist = dist;
            bestSafe = dir;
        }
    }
    if (bestSafeDist != std::numeric_limits<int>::max()) {
        return bestSafe;
    }

    // 4) 如果没有安全方向，按“最接近目标”做激进行为
    return chooseDirectionToward(state, me.head, predicted, false);
}

}  // namespace bot
