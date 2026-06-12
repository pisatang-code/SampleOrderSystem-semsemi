#pragma once
#include <string>
#include <vector>

namespace JsonUtil {
    std::string escapeJson(const std::string& s);
    std::string getValue(const std::string& obj, const std::string& key);
    std::vector<std::string> parseObjects(const std::string& content);
    std::string readFile(const std::string& path);
    void writeFile(const std::string& path, const std::string& content);
}
