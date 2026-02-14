#include "strategies/ParasiteStrategy.hpp"

#include "common/DirectionUtils.hpp"

#include <array>
#include <limits>
#include <string>
#include <vector>

namespace bot {

namespace {

Snake chooseHost(const GameState& state, const Snake& me) {
    const auto others = state.getOtherPlayers();
    if (others.empty()) {
        return me;
    }

    // 近似“排行榜第一”：选长度最长者
    Snake host = others.front();
    for (const auto& s : others) {
        if (s.length > host.length) {
            host = s;
        }
    }
    return host;
}

Point inferMoveVector(const Snake& snake) {
    if (snake.blocks.size() < 2) {
        return {0, 0};
    }
    return {
        snake.blocks[0].x - snake.blocks[1].x,
        snake.blocks[0].y - snake.blocks[1].y,
    };
}

}  // namespace

std::string decideParasite(const GameState& state) {
    const Snake me = state.getMySnake();
    const Snake host = chooseHost(state, me);

    // 记录上一回合选择的偏移，减少“左右抖动”
    static std::string lastHostId;
    static Point preferredOffset = {1, 0};

    if (lastHostId != host.id) {
        preferredOffset = {1, 0};
        lastHostId = host.id;
    }

    // 预测宿主下一步头部位置，提前站位
    const Point moveVec = inferMoveVector(host);
    Point predictedHead = host.head;
    predictedHead.x += moveVec.x;
    predictedHead.y += moveVec.y;
    if (!state.isValidPos(predictedHead.x, predictedHead.y)) {
        predictedHead = host.head;
    }

    // 候选侧位：保持在宿主预测头部周围 1 格
    const std::array<Point, 4> offsets = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    std::vector<Point> sideTargets;
    sideTargets.reserve(offsets.size());

    // 把“上一回合偏好侧位”放在最前，优先尝试稳定伴随
    sideTargets.push_back({predictedHead.x + preferredOffset.x, predictedHead.y + preferredOffset.y});
    for (const auto& offset : offsets) {
        if (offset.x == preferredOffset.x && offset.y == preferredOffset.y) {
            continue;
        }
        sideTargets.push_back({predictedHead.x + offset.x, predictedHead.y + offset.y});
    }

    Point best = predictedHead;
    int bestDist = std::numeric_limits<int>::max();
    Point bestOffset = preferredOffset;

    for (const auto& p : sideTargets) {
        const Point offset = {p.x - predictedHead.x, p.y - predictedHead.y};
        if (!state.isValidPos(p.x, p.y) || state.hasObstacle(p.x, p.y)) {
            continue;
        }
        const int d = me.head.distance(p);
        if (d < bestDist) {
            bestDist = d;
            best = p;
            bestOffset = offset;
        }
    }

    // 没有合适侧位时，退化为跟随宿主预测头部附近
    if (bestDist == std::numeric_limits<int>::max()) {
        best = predictedHead;
    } else {
        preferredOffset = bestOffset;
    }

    std::string dir = chooseDirectionToward(state, me.head, best, true);
    if (isSafeDirection(state, me.head, dir)) {
        return dir;
    }

    for (const auto& candidate : allDirections()) {
        if (isSafeDirection(state, me.head, candidate)) {
            return candidate;
        }
    }

    return "right";
}

}  // namespace bot
