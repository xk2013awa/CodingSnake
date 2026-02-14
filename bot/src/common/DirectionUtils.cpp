#include "common/DirectionUtils.hpp"

#include <limits>

namespace bot {

const std::vector<std::string>& allDirections() {
    static const std::vector<std::string> directions = {"up", "down", "left", "right"};
    return directions;
}

Point nextPoint(const Point& from, const std::string& direction) {
    Point next = from;
    if (direction == "up") {
        --next.y;
    } else if (direction == "down") {
        ++next.y;
    } else if (direction == "left") {
        --next.x;
    } else if (direction == "right") {
        ++next.x;
    }
    return next;
}

bool isSafeDirection(const GameState& state, const Point& head, const std::string& direction) {
    const Point next = nextPoint(head, direction);
    return state.isValidPos(next.x, next.y) && !state.hasObstacle(next.x, next.y);
}

std::string chooseDirectionToward(
    const GameState& state,
    const Point& from,
    const Point& target,
    bool safeOnly
) {
    std::string bestDir = "right";
    int bestDist = std::numeric_limits<int>::max();

    for (const auto& dir : allDirections()) {
        const Point next = nextPoint(from, dir);
        if (!state.isValidPos(next.x, next.y)) {
            continue;
        }
        if (safeOnly && state.hasObstacle(next.x, next.y)) {
            continue;
        }

        const int dist = next.distance(target);
        if (dist < bestDist) {
            bestDist = dist;
            bestDir = dir;
        }
    }

    return bestDir;
}

}  // namespace bot
