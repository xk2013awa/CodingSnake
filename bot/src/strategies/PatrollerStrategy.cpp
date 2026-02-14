#include "strategies/PatrollerStrategy.hpp"

#include "common/DirectionUtils.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

namespace bot {

namespace {

struct PatrolState {
    bool inited = false;
    int minX = 10;
    int maxX = 30;
    int minY = 10;
    int maxY = 30;
    int index = 0;
};

// 基于 id 做稳定哈希，让每条蛇形成不同巡逻矩形
std::uint64_t hashId(const std::string& id) {
    std::uint64_t h = 1469598103934665603ull;
    for (unsigned char c : id) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

std::vector<Point> rectanglePath(const PatrolState& ps) {
    return {
        {ps.minX, ps.minY},
        {ps.maxX, ps.minY},
        {ps.maxX, ps.maxY},
        {ps.minX, ps.maxY},
    };
}

std::string bfsFirstStep(const GameState& state, const Point& start, const Point& target) {
    if (start.x == target.x && start.y == target.y) {
        return "right";
    }

    const int width = state.getMapWidth();
    const int height = state.getMapHeight();
    if (width <= 0 || height <= 0) {
        return "right";
    }

    auto id = [width](int x, int y) {
        return y * width + x;
    };

    std::vector<int> prev(width * height, -1);
    std::queue<Point> q;
    q.push(start);
    prev[id(start.x, start.y)] = id(start.x, start.y);

    while (!q.empty()) {
        const Point cur = q.front();
        q.pop();

        for (const auto& dir : allDirections()) {
            Point nxt = nextPoint(cur, dir);
            if (!state.isValidPos(nxt.x, nxt.y)) {
                continue;
            }

            // 目标点允许被当作可进入节点，避免永远不可达
            if ((nxt.x != target.x || nxt.y != target.y) && state.hasObstacle(nxt.x, nxt.y)) {
                continue;
            }

            const int nid = id(nxt.x, nxt.y);
            if (prev[nid] != -1) {
                continue;
            }

            prev[nid] = id(cur.x, cur.y);
            if (nxt.x == target.x && nxt.y == target.y) {
                // 回溯找到从 start 出发的第一步
                int curId = nid;
                int parent = prev[curId];
                while (parent != id(start.x, start.y)) {
                    curId = parent;
                    parent = prev[curId];
                    if (parent == -1) {
                        return "right";
                    }
                }

                const int stepX = curId % width;
                const int stepY = curId / width;
                if (stepX == start.x && stepY == start.y - 1) {
                    return "up";
                }
                if (stepX == start.x && stepY == start.y + 1) {
                    return "down";
                }
                if (stepX == start.x - 1 && stepY == start.y) {
                    return "left";
                }
                if (stepX == start.x + 1 && stepY == start.y) {
                    return "right";
                }
                return "right";
            }

            q.push(nxt);
        }
    }

    return "right";
}

void initPatrolIfNeeded(PatrolState& ps, const GameState& state, const Snake& me) {
    if (ps.inited) {
        return;
    }

    const int width = std::max(20, state.getMapWidth());
    const int height = std::max(20, state.getMapHeight());
    const std::uint64_t h = hashId(me.id);

    // 以 4 象限为基础，给出一个确定性矩形
    const int quadX = static_cast<int>((h >> 0) & 1ull);
    const int quadY = static_cast<int>((h >> 1) & 1ull);

    const int halfW = width / 2;
    const int halfH = height / 2;
    const int x0 = quadX == 0 ? 0 : halfW;
    const int x1 = quadX == 0 ? halfW - 1 : width - 1;
    const int y0 = quadY == 0 ? 0 : halfH;
    const int y1 = quadY == 0 ? halfH - 1 : height - 1;

    const int margin = 4;
    ps.minX = std::clamp(x0 + margin, 0, width - 1);
    ps.maxX = std::clamp(x1 - margin, 0, width - 1);
    ps.minY = std::clamp(y0 + margin, 0, height - 1);
    ps.maxY = std::clamp(y1 - margin, 0, height - 1);

    if (ps.minX >= ps.maxX) {
        ps.minX = std::max(0, x0);
        ps.maxX = std::min(width - 1, x1);
    }
    if (ps.minY >= ps.maxY) {
        ps.minY = std::max(0, y0);
        ps.maxY = std::min(height - 1, y1);
    }

    ps.index = 0;
    ps.inited = true;
}

}  // namespace

std::string decidePatroller(const GameState& state) {
    const Snake me = state.getMySnake();
    static PatrolState patrol;
    initPatrolIfNeeded(patrol, state, me);

    auto path = rectanglePath(patrol);
    Point target = path[patrol.index];

    // 到达当前顶点后切换到下一个顶点
    if (me.head.x == target.x && me.head.y == target.y) {
        patrol.index = (patrol.index + 1) % static_cast<int>(path.size());
        target = path[patrol.index];
    }

    // 通过 BFS 计算更平滑的最短路回归
    std::string dir = bfsFirstStep(state, me.head, target);
    if (!isSafeDirection(state, me.head, dir)) {
        // 最短路第一步不可用时，回退为局部贪心
        dir = chooseDirectionToward(state, me.head, target, true);
    }

    if (isSafeDirection(state, me.head, dir)) {
        return dir;
    }

    // 被阻挡时临时避障：找任意安全方向，下一回合会继续尝试回到轨迹
    for (const auto& candidate : allDirections()) {
        if (isSafeDirection(state, me.head, candidate)) {
            return candidate;
        }
    }

    return "right";
}

}  // namespace bot
