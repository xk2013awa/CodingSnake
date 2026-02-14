#include "CodingSnake.hpp"
#include "strategies/InterceptorStrategy.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

std::string getEnvOrDefault(const char* key, const std::string& fallback) {
    const char* value = std::getenv(key);
    return value ? std::string(value) : fallback;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string endpoint = getEnvOrDefault("CS_ENDPOINT", "http://127.0.0.1:18080");

    // 支持命令行覆盖：
    // ./interceptor_bot <uid> <paste> [name]
    const std::string uid = argc > 1 ? argv[1] : "interceptor";
    const std::string paste = argc > 2 ? argv[2] : "paste_here";
    const std::string name = argc > 3 ? argv[3] : "interceptor";

    try {
        CodingSnake game(endpoint);
        game.login(uid, paste);
        game.join(name, "#FF0000");
        game.run(bot::decideInterceptor);
    } catch (const SnakeException& e) {
        std::cerr << "拦截者启动失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
