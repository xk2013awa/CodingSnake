#include "CodingSnake.hpp"
#include "common/BotConfigLoader.hpp"
#include "strategies/GluttonStrategy.hpp"
#include "strategies/InterceptorStrategy.hpp"
#include "strategies/ParasiteStrategy.hpp"
#include "strategies/PatrollerStrategy.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

struct BotConfig {
    std::string role;
    std::string color;
    std::string uid;
    std::string paste;
    std::string name;
    std::string (*decide)(const GameState&);
};

void runOneBot(const std::string& endpoint, const BotConfig& config) {
    try {
        CodingSnake game(endpoint);
        game.login(config.uid, config.paste);
        game.join(config.name, config.color);
        game.run(config.decide);
    } catch (const SnakeException& e) {
        std::cerr << "[" << config.role << "] 运行异常: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[" << config.role << "] 标准异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[" << config.role << "] 未知异常" << std::endl;
    }
}

}  // namespace

int main() {
    // 优先读取配置文件；如不存在则回退到环境变量
    const std::unordered_map<std::string, std::string> config =
        bot::loadSimpleConfig("config/bots.conf");

    // 默认回环地址：bot 与 server 在同机运行
    const std::string endpoint = bot::getConfigValue(
        config,
        "endpoint",
        "CS_ENDPOINT",
        "http://127.0.0.1:18080"
    );

    // 参数优先级：配置文件 > 环境变量 > 默认值
    std::vector<BotConfig> bots = {
        {
            "interceptor",
            "#FF0000",
            bot::getConfigValue(config, "interceptor.uid", "CS_INTERCEPTOR_UID", "interceptor"),
            bot::getConfigValue(config, "interceptor.paste", "CS_INTERCEPTOR_PASTE", "paste_here"),
            bot::getConfigValue(config, "interceptor.name", "CS_INTERCEPTOR_NAME", "interceptor"),
            bot::decideInterceptor,
        },
        {
            "glutton",
            "#FFFF00",
            bot::getConfigValue(config, "glutton.uid", "CS_GLUTTON_UID", "glutton"),
            bot::getConfigValue(config, "glutton.paste", "CS_GLUTTON_PASTE", "paste_here"),
            bot::getConfigValue(config, "glutton.name", "CS_GLUTTON_NAME", "glutton"),
            bot::decideGlutton,
        },
        {
            "patroller",
            "#0000FF",
            bot::getConfigValue(config, "patroller.uid", "CS_PATROLLER_UID", "patroller"),
            bot::getConfigValue(config, "patroller.paste", "CS_PATROLLER_PASTE", "paste_here"),
            bot::getConfigValue(config, "patroller.name", "CS_PATROLLER_NAME", "patroller"),
            bot::decidePatroller,
        },
        {
            "parasite",
            "#800080",
            bot::getConfigValue(config, "parasite.uid", "CS_PARASITE_UID", "parasite"),
            bot::getConfigValue(config, "parasite.paste", "CS_PARASITE_PASTE", "paste_here"),
            bot::getConfigValue(config, "parasite.name", "CS_PARASITE_NAME", "parasite"),
            bot::decideParasite,
        },
    };

    std::cout << "启动 4 个 Bot，目标服务器: " << endpoint << std::endl;

    // 每个 Bot 在独立线程中运行，互不阻塞
    std::vector<std::thread> workers;
    workers.reserve(bots.size());

    for (const auto& config : bots) {
        workers.emplace_back([endpoint, config]() {
            runOneBot(endpoint, config);
        });
    }

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    return 0;
}
