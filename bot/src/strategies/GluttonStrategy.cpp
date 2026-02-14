#include "strategies/GluttonStrategy.hpp"

#include "common/DirectionUtils.hpp"

#include <limits>

namespace bot {

namespace {

Point inferMoveVector(const Snake& snake) {
    if (snake.blocks.size() < 2) {
        return {0, 0};
    }
    return {
        snake.blocks[0].x - snake.blocks[1].x,
        snake.blocks[0].y - snake.blocks[1].y,
    };
}

int estimateOneStepDist(const GameState& state, const Snake& snake, const Point& target) {
    const int nowDist = snake.head.distance(target);
    const Point vec = inferMoveVector(snake);
    Point next = snake.head;
    next.x += vec.x;
    next.y += vec.y;

    // 只做“一步前瞻”：若推断出的下一步非法，则回退为当前距离
    if (!state.isValidPos(next.x, next.y) || state.hasObstacle(next.x, next.y)) {
        return nowDist;
    }

    const int nextDist = next.distance(target);
    return std::min(nowDist, nextDist);
}

}  // namespace

std::string decideGlutton(const GameState& state) {
    const Snake me = state.getMySnake();
    const auto foods = state.getFoods();
    const auto players = state.getAllPlayers();

    if (foods.empty()) {
        return "right";
    }

    // 1) 为每个食物打分：
    //    - 我比别人先到（D_me < D_other_min）加分
    //    - 别人也离得近（说明是热点）再加分
    // 2) 找不到可抢食物时，退化为最近食物
    Point bestFood = foods.front();
    int bestScore = std::numeric_limits<int>::min();
    int bestMyDist = std::numeric_limits<int>::max();

    for (const auto& food : foods) {
        const int myDist = me.head.distance(food);
        int otherMinDist = std::numeric_limits<int>::max();

        for (const auto& player : players) {
            if (player.id == me.id) {
                continue;
            }
            const int d = estimateOneStepDist(state, player, food);
            if (d < otherMinDist) {
                otherMinDist = d;
            }
        }

        // 无其他玩家时，让“冲突距离”退化为较大值
        if (otherMinDist == std::numeric_limits<int>::max()) {
            otherMinDist = 200;
        }

        int score = -myDist;
        if (myDist < otherMinDist) {
            score += 100;
        }
        // otherMinDist 越小越可能形成冲突，适当加权
        score += (100 - otherMinDist);

        // 当前一步就可能撞墙/撞人时，给一个惩罚，避免完全无效冲刺
        std::string toward = chooseDirectionToward(state, me.head, food, false);
        if (!isSafeDirection(state, me.head, toward)) {
            score -= 25;
        }

        if (score > bestScore || (score == bestScore && myDist < bestMyDist)) {
            bestScore = score;
            bestMyDist = myDist;
            bestFood = food;
        }
    }

    // 按 TODO 描述，暴食者几乎不做避障，激进吃食
    return chooseDirectionToward(state, me.head, bestFood, false);
}

}  // namespace bot
