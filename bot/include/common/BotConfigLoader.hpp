#pragma once

#include <string>
#include <unordered_map>

namespace bot {

// 读取 key=value 配置文件（支持 # 注释与空行）
// 解析失败时返回空映射，不抛异常。
std::unordered_map<std::string, std::string> loadSimpleConfig(const std::string& filePath);

// 读取配置项，优先级：配置文件 > 环境变量 > 默认值
std::string getConfigValue(
    const std::unordered_map<std::string, std::string>& config,
    const std::string& key,
    const char* envKey,
    const std::string& fallback
);

}  // namespace bot
