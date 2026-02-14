# Bot 工程说明

本目录提供 1 个统一入口程序 `bot_main`，运行后会同时拉起 4 个 Bot：

- interceptor（拦截者）
- glutton（暴食者）
- patroller（巡逻兵）
- parasite（寄生虫）

默认连接地址为回环：`http://127.0.0.1:18080`（可通过环境变量 `CS_ENDPOINT` 覆盖）。

推荐使用配置文件：`config/bots.conf`（优先级高于环境变量）。

## 目录结构

```text
bot/
├── CMakeLists.txt
├── CodingSnake.hpp
├── include/
│   ├── common/
│   │   └── DirectionUtils.hpp
│   └── strategies/
│       ├── GluttonStrategy.hpp
│       ├── InterceptorStrategy.hpp
│       ├── ParasiteStrategy.hpp
│       └── PatrollerStrategy.hpp
└── src/
	├── common/
	│   └── DirectionUtils.cpp
	└── main.cpp
	├── bots/
	│   ├── glutton/main.cpp      # 预留（当前不参与构建）
	│   ├── interceptor/main.cpp  # 预留（当前不参与构建）
	│   ├── parasite/main.cpp     # 预留（当前不参与构建）
	│   └── patroller/main.cpp    # 预留（当前不参与构建）
	└── strategies/
		├── GluttonStrategy.cpp
		├── InterceptorStrategy.cpp
		├── ParasiteStrategy.cpp
		└── PatrollerStrategy.cpp
```

## 构建

```bash
cd /home/seve/codingsnake/bot
cmake -S . -B build
cmake --build build -j
```

## 运行示例

```bash
# 从示例生成配置
cp config/bots.conf.example config/bots.conf

# 修改 config/bots.conf 后启动
./build/bot_main
```

也可以继续使用环境变量：

```bash
# 配置 4 个账号参数（示例）
export CS_INTERCEPTOR_UID="10001"
export CS_INTERCEPTOR_PASTE="paste_a"
export CS_GLUTTON_UID="10002"
export CS_GLUTTON_PASTE="paste_b"
export CS_PATROLLER_UID="10003"
export CS_PATROLLER_PASTE="paste_c"
export CS_PARASITE_UID="10004"
export CS_PARASITE_PASTE="paste_d"

# 启动后会并发运行 4 个 Bot
./build/bot_main
```

优先级说明：`config/bots.conf` > 环境变量 > 代码默认值。