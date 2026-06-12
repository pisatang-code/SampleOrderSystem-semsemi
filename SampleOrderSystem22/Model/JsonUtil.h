#pragma once
#include <string>
#include <vector>

namespace JsonUtil {
    std::string escapeJson(const std::string& s);
    std::string getValue(const std::string& obj, const std::string& key);
    std::vector<std::string> parseObjects(const std::string& content);
    std::string readFile(const std::string& path);
    void writeFile(const std::string& path, const std::string& content);

    // 직렬화 가능한 타입 T의 벡터를 JSON 배열 문자열로 변환
    // T는 toJsonObject() const 메서드를 가져야 한다
    template<typename T>
    std::string serializeArray(const std::vector<T>& items) {
        std::string result = "[\n";
        for (size_t i = 0; i < items.size(); ++i) {
            result += "  " + items[i].toJsonObject();
            if (i + 1 < items.size()) result += ",";
            result += "\n";
        }
        result += "]";
        return result;
    }
}
