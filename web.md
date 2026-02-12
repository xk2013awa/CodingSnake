

# 贪吃蛇多人对战网络 API 设计

## 1. 概述

本 API 为一套极简 HTTP REST 接口，用于多名玩家通过脚本控制贪吃蛇进行在线对战。

* 通信协议：HTTP / JSON
* 接口风格：REST
* 返回格式统一为 `{ code, msg, data }`
* API 版本：v1.0.0

### 1.1 基础信息

**Base URL**: `http://your-server-address:port`

**请求头要求**:
```
Content-Type: application/json
```

**认证方式**:
- 登录后获取 `key`（账号级别凭证）
- 加入游戏后获取 `token`（游戏会话凭证）
- 在请求体中携带对应凭证即可，无需复杂的 Header 认证

---

## 2. 核心流程

1. 玩家脚本向服务器注册账号并获取身份凭证
2. 玩家加入当前对局
3. 周期性获取地图状态
4. 计算并提交移动方向
5. 服务器按回合推进游戏逻辑
6. 重复步骤 3~5

---

## 3. 游戏规则

### 3.1 地图与坐标系

* 地图大小固定，为 `width × height` 的矩形区域
* 坐标系：左上角为 `(0, 0)`，向右为 x 轴正方向，向下为 y 轴正方向
* 有效坐标范围：`0 <= x < width`, `0 <= y < height`

### 3.2 玩家与蛇

* 玩家初始长度为 **3**，初始位置随机，保证头部周围一定范围内安全，初始时有一定回合无敌状态，不会被淘汰也不会淘汰其他玩家
* 蛇由头部 `head` 和若干身体块 `blocks` 组成
* `blocks[0]` 即为头部位置

### 3.3 移动规则

* 每回合玩家可选择一个移动方向：`UP` / `DOWN` / `LEFT` / `RIGHT`
* 若本回合未发送指令，则沿用上一回合方向
* 不允许反向移动（例如当前向右移动时，不能直接选择向左）

### 3.4 淘汰条件

玩家发生以下情况将被淘汰，需要重新加入：
* 撞墙（头部移出地图边界）
* 撞到自身身体
* 撞到其他玩家身体

### 3.5 食物与成长

* 每回合按一定密度生成食物 `food`
* 头部进入食物位置时，长度 +1，食物消失
* 蛇的长度会影响得分

### 3.6 回合机制

* 单回合时长为 `round_time` 毫秒（默认 1000ms）
* 服务器按固定时间推进回合，所有玩家同步更新

---

## 4. 速率限制

为保证服务器稳定运行和游戏公平性，API 实施以下速率限制：

### 4.1 全局限制

| 端点                      | 限制                   | 说明                     |
| ----------------------- | -------------------- | ---------------------- |
| `/api/game/login`       | 10 次 / 小时 / IP      |                  |
| `/api/game/join`        | 5 次 / 分钟 / key      |                  |
| `/api/game/move`        | 每回合 1 次 / token     | 每回合只能提交一次移动指令          |
| `/api/game/map`         | 无限制                  | **无需token，公开访问**，建议按回合轮询       |
| `/api/game/map/delta`   | 无限制                  | **推荐**，无需token，获取增量地图，流量节省80%+    |
| `/api/status`           | 60 次 / 分钟 / IP      | 服务器状态查询                |
| `/api/metrics`          | 无限制                  | 性能监控指标导出               |

### 4.2 触发限制时的响应

```json
{
  "code": 429,
  "msg": "too many requests, please retry after 10 seconds",
  "data": {
    "retry_after": 10
  }
}
```

### 4.3 最佳实践

1. **移动指令**：每回合只发送一次，建议在获取地图状态后立即计算并发送
2. **地图轮询**：按 `round_time` 间隔轮询，不要过于频繁
3. **错误重试**：遇到 429 错误时，按 `retry_after` 等待后重试

---

## 5. 通用响应格式

### 5.1 成功响应

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

### 5.2 通用异常响应

| code | 含义                   |
| ---- | -------------------- |
| 0    | 成功                   |
| 400  | 请求参数错误               |
| 401  | 未认证 / key 或 token 无效 |
| 403  | 无权限操作                |
| 404  | 资源不存在                |
| 409  | 状态冲突（重复加入、非法操作等）     |
| 429  | 请求过于频繁               |
| 500  | 服务器内部错误              |
| 503  | 服务器维护中               |

**异常响应示例：**

```json
{
  "code": 401,
  "msg": "invalid token",
  "data": null
}
```

---

## 6. API 端点说明

---

### 6.1 获取服务器状态

**GET** `/api/status`

**请求参数**: 无

**说明**: 获取服务器运行状态和游戏基础配置，无需认证。

#### 成功响应

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "status": "running",
    "player_count": 10,
    "map_size": {
      "width": 50,
      "height": 50
    },
    "round_time": 1000
  }
}
```

#### 可能异常

| code | msg                |
| ---- | ------------------ |
| 503  | server maintenance |

---

### 6.2 玩家注册与加入

#### 6.2.1 登录（获取身份 key）

**POST** `/api/game/login`

**说明**: 通过洛谷 UID 和剪贴板后缀验证身份，获取账号级别的 key。

**请求体**

```json
{
  "uid": "123456",
  "paste": "luogu剪贴板后缀"
}
```

**参数说明**:
- `uid` (string, 必需): 洛谷用户 ID
- `paste` (string, 必需): 洛谷剪贴板后缀，用于身份验证

**成功响应**

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "key": "玩家唯一身份令牌"
  }
}
```

**可能异常**

| code | msg                   |
| ---- | --------------------- |
| 400  | invalid uid or paste  |
| 403  | authentication failed |
| 500  | internal error        |

---

#### 6.2.2 加入游戏

**POST** `/api/game/join`

**说明**: 使用 key 加入当前游戏对局，获取游戏会话 token。

**请求体**

```json
{
  "key": "玩家唯一身份令牌",
  "name": "玩家自定义名称",
  "color": "#FF0000"
}
```

**参数说明**:
- `key` (string, 必需): 登录后获得的身份令牌
- `name` (string, 必需): 玩家显示名称，长度 1-20 字符
- `color` (string, 可选): 蛇的颜色，十六进制格式，默认随机分配

**成功响应**

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "token": "游戏唯一标识符",
    "id": "玩家在本局中的唯一ID",
    "initial_direction": "right",
    "map_state": {}
  }
}
```

**字段说明**:
- `token`: 游戏会话令牌，用于后续API调用
- `id`: 玩家在本局中的唯一ID
- `initial_direction`: 玩家蛇的初始移动方向（`UP` / `DOWN` / `LEFT` / `RIGHT`）
- `map_state`: 完整的初始地图状态




**可能异常**

| code | msg                    |
| ---- | ---------------------- |
| 401  | invalid key            |
| 409  | player already in game |
| 503  | server maintenance     |

---

### 6.3 地图状态

#### 6.3.1 地图结构说明

```json
{
  "players": [
    {
      "name": "nalong",
      "id": "114514",
      "color": "#FF0000",
      "head": { "x": 10, "y": 15 },
      "blocks": [
        { "x": 10, "y": 15 },
        { "x": 10, "y": 16 },
        { "x": 10, "y": 17 }
      ],
      "length": 3,
      "invincible_rounds": 2 //剩余无敌回合数
    }
  ],
  "foods": [
    { "x": 5, "y": 5 },
    { "x": 20, "y": 25 }
  ],
  "round": 42,
  "timestamp": 1706342400000,
  "next_round_timestamp": 1706342401000
}
```

**字段说明**:
- `players`: 所有在线玩家列表
  - `name`: 玩家名称
  - `id`: 玩家 ID
  - `color`: 蛇的颜色
  - `head`: 蛇头坐标
  - `blocks`: 蛇身所有块的坐标数组，`blocks[0]` 为头部
  - `length`: 蛇的当前长度
- `foods`: 当前地图上所有食物的坐标
- `round`: 当前回合数
- `timestamp`: 服务器时间戳（毫秒）
- `next_round_timestamp`: 下一回合开始的时间戳（毫秒），客户端可据此计算下一回合到来的时间

---

#### 6.3.2 获取地图状态（完整）

**GET** `/api/game/map`

**说明**: 获取当前游戏地图的完整状态。每次都返回所有玩家和食物的完整信息。**无需token验证，公开访问。**

**请求参数**: 无

**示例**: `GET /api/game/map`

**响应**

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "map_state": {
      "players": [...],
      "foods": [...],
      "round": 42,
      "timestamp": 1706342400000,
      "next_round_timestamp": 1706342401000
    }
  }
}
```

**可能异常**

| code | msg                |
| ---- | ------------------ |
| 500  | internal error     |

---

#### 6.3.3 获取地图状态（增量）**【推荐】**

**GET** `/api/game/map/delta`

**说明**: 获取当前回合相对于上一回合的增量变化。

**请求参数**: 无

**示例**: `GET /api/game/map/delta`

**增量数据结构说明**:

增量数据只包含变化的部分，客户端需要自行维护完整状态：

1. **所有玩家的简化信息** - 每回合都变化，但只发送关键字段（不含完整blocks数组）
2. **新加入的玩家** - 包含完整信息，客户端需要添加到本地玩家列表
3. **死亡的玩家ID** - 客户端需要从本地玩家列表中移除
4. **新增的食物** - 客户端需要添加到本地食物列表
5. **移除的食物** - 客户端需要从本地食物列表中移除

**响应格式**

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "delta_state": {
      "round": 42,
      "timestamp": 1706342400000,
      "next_round_timestamp": 1706342401000,
      
      "players": [
        {
          "id": "player1",
          "head": { "x": 10, "y": 15 },
          "direction": "UP",
          "length": 5,
          "invincible_rounds": 0
        }
      ],
      
      "joined_players": [
        {
          "id": "player2",
          "name": "新玩家",
          "color": "#FF0000",
          "head": { "x": 20, "y": 20 },
          "blocks": [
            { "x": 20, "y": 20 },
            { "x": 20, "y": 21 },
            { "x": 20, "y": 22 }
          ],
          "length": 3,
          "invincible_rounds": 5
        }
      ],
      
      "died_players": ["player3"],
      
      "added_foods": [
        { "x": 5, "y": 5 },
        { "x": 8, "y": 12 }
      ],
      
      "removed_foods": [
        { "x": 10, "y": 10 }
      ]
    }
  }
}
```

**字段说明**:

- `round`: 当前回合数
- `timestamp`: 服务器时间戳（毫秒）
- `next_round_timestamp`: 下一回合开始的时间戳（毫秒）
- `players`: **所有在线玩家**的简化信息数组
  - `id`: 玩家ID
  - `head`: 蛇头当前坐标
  - `direction`: 当前移动方向 (`UP` / `down` / `left` / `right`)
  - `length`: 蛇的当前长度
  - `invincible_rounds`: 剩余无敌回合数
- `joined_players`: 本回合**新加入**的玩家完整信息数组
  - 包含完整的玩家信息（name, color, blocks等）
  - 客户端需要将这些玩家添加到本地状态
- `died_players`: 本回合**死亡**的玩家ID数组
  - 客户端需要从本地玩家列表中移除这些玩家
- `added_foods`: 本回合**新增**的食物坐标数组
  - 客户端需要将这些食物添加到本地食物列表
- `removed_foods`: 本回合**移除**的食物坐标数组
  - 客户端需要从本地食物列表中移除这些食物

**客户端状态维护逻辑**:

```python
# 伪代码示例
def update_with_delta(local_state, delta):
    # 1. 更新回合数和时间戳
    local_state.round = delta.round
    local_state.timestamp = delta.timestamp
    
    # 2. 处理死亡玩家
    for player_id in delta.died_players:
        local_state.players.remove(player_id)
    
    # 3. 添加新加入的玩家
    for player in delta.joined_players:
        local_state.players[player.id] = player
    
    # 4. 更新所有玩家的简化信息
    for player_update in delta.players:
        if player_update.id in local_state.players:
            local_state.players[player_update.id].head = player_update.head
            local_state.players[player_update.id].direction = player_update.direction
            local_state.players[player_update.id].length = player_update.length
            local_state.players[player_update.id].invincible_rounds = player_update.invincible_rounds
    
    # 5. 移除食物
    for food_pos in delta.removed_foods:
        local_state.foods.remove(food_pos)
    
    # 6. 添加食物
    for food_pos in delta.added_foods:
        local_state.foods.add(food_pos)
```

**使用建议**:

1. **首次拉取**：使用 `/api/game/map` 获取完整地图状态
2. **后续轮询**：使用 `/api/game/map/delta` 获取增量更新
3. **定期刷新**：每隔一定回合（如50回合）重新获取完整地图，避免累积误差
4. **异常恢复**：如果增量更新出现问题，重新获取完整地图

**可能异常**

| code | msg                |
| ---- | ------------------ |
| 500  | internal error     |

---

### 6.4 移动指令

**POST** `/api/game/move`

**说明**: 提交本回合的移动方向指令。每回合只能提交一次。

**请求体**

```json
{
  "token": "游戏唯一标识符",
  "direction": "up"
}
```

**参数说明**:
- `token` (string, 必需): 加入游戏后获得的会话令牌
- `direction` (string, 必需): 移动方向，可选值：`up` / `down` / `left` / `right`

**成功响应**

```json
{
  "code": 0,
  "msg": "success"
}
```

**可能异常**

| code | msg                               |
| ---- | --------------------------------- |
| 400  | invalid direction                 |
| 401  | invalid token                     |
| 409  | move not allowed in current round |
| 429  | too many requests                 |
| 404  | player not in game                |

**注意**: 玩家被淘汰后会返回 404 错误，需要重新调用 `/api/game/join` 加入游戏。

---

### 6.5 排行榜

**GET** `/api/leaderboard`

**说明**: 获取排行榜数据，支持分页与时间范围过滤。

**请求参数** (Query):
- `type` (string, 可选): 排行榜类型，`kills` / `max_length`，默认 `kills`
- `limit` (int, 可选): 每页数量，默认 50，受服务端 `max_entries` 限制
- `offset` (int, 可选): 偏移量，默认 0
- `start_time` (int64, 可选): 起始时间戳（毫秒）
- `end_time` (int64, 可选): 结束时间戳（毫秒）

**成功响应**

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "type": "kills",
    "limit": 50,
    "offset": 0,
    "start_time": 0,
    "end_time": 0,
    "refresh_interval_rounds": 5,
    "cache_ttl_seconds": 5,
    "entries": [
      {
        "uid": "123456",
        "name": "player1",
        "season_id": "all_time",
        "now_length": 6,
        "max_length": 8,
        "kills": 3,
        "deaths": 1,
        "games_played": 5,
        "total_food": 25,
        "last_round": 180,
        "timestamp": 1706342400000,
        "rank": 1
      }
    ]
  }
}
```

**可能异常**

| code | msg                |
| ---- | ------------------ |
| 400  | invalid parameter  |
| 500  | internal error     |

---

### 6.6 性能监控

**GET** `/api/metrics`

**说明**: 获取服务器性能指标，支持 JSON 和 Prometheus 格式。

**请求参数** (Query):
- `format` (string, 可选): `json` / `prometheus`，默认 `json`

**示例**:
- `GET /api/metrics`
- `GET /api/metrics?format=prometheus`

**JSON 响应**

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "metrics": {
      "enabled": true,
      "timestamp_ms": 1706342400000,
      "config": {
        "window_seconds": 60,
        "sample_rate": 0.2,
        "max_samples": 2000
      },
      "qps": {
        "overall": 12.5,
        "per_endpoint": {
          "status": 1.2,
          "map": 6.3
        }
      },
      "requests_total": 10240,
      "requests_total_per_endpoint": {
        "status": 120,
        "map": 640
      },
      "latency_ms": {
        "overall": {
          "p95": 8.2,
          "p99": 15.6,
          "sample_count": 500
        },
        "per_endpoint": {
          "map": {
            "p95": 6.9,
            "p99": 12.3,
            "sample_count": 200
          }
        }
      },
      "round_ms": {
        "last": 7.4,
        "p95": 9.1,
        "p99": 12.8,
        "sample_count": 120
      },
      "locks": {
        "GameManager.state": {
          "count": 1200,
          "avg_ms": 0.12,
          "max_ms": 2.3,
          "last_ms": 0.05
        }
      },
      "gauges": {
        "moves_current_size": 4,
        "moves_pending_size": 8
      },
      "memory": {
        "rss_bytes": 73400320
      }
    }
  }
}
```

**Prometheus 响应**

```text
# HELP snake_qps Overall QPS in the configured window
# TYPE snake_qps gauge
snake_qps 12.5
# HELP snake_requests_total Total HTTP requests
# TYPE snake_requests_total counter
snake_requests_total 10240
```

**可能异常**

| code | msg              |
| ---- | ---------------- |
| 503  | metrics disabled |

---

## 7. 使用示例
