#include "common/BotConfigLoader.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>

namespace bot {

namespace {

std::string trim(const std::string& input) {
    std::size_t left = 0;
    while (left < input.size() && std::isspace(static_cast<unsigned char>(input[left]))) {
        ++left;
    }

    std::size_t right = input.size();
    while (right > left && std::isspace(static_cast<unsigned char>(input[right - 1]))) {
        --right;
    }

    return input.substr(left, right - left);
}

}  // namespace

std::unordered_map<std::string, std::string> loadSimpleConfig(const std::string& filePath) {
    std::unordered_map<std::string, std::string> result;

    std::ifstream in(filePath);
    if (!in.is_open()) {
        return result;
    }

    std::string line;
    while (std::getline(in, line)) {
        const std::string stripped = trim(line);
        if (stripped.empty() || stripped[0] == '#') {
            continue;
        }

        const auto pos = stripped.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        const std::string key = trim(stripped.substr(0, pos));
        const std::string value = trim(stripped.substr(pos + 1));
        if (!key.empty()) {
            result[key] = value;
        }
    }

    return result;
}

std::string getConfigValue(
    const std::unordered_map<std::string, std::string>& config,
    const std::string& key,
    const char* envKey,
    const std::string& fallback
) {
    const auto it = config.find(key);
    if (it != config.end() && !it->second.empty()) {
        return it->second;
    }

    const char* value = std::getenv(envKey);
    if (value != nullptr && value[0] != '\0') {
        return std::string(value);
    }

    return fallback;
}

}  // namespace bot
