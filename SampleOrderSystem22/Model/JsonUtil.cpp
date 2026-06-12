#include "JsonUtil.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

namespace JsonUtil {

std::string escapeJson(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (unsigned char c : s) {
        if      (c == '"')  result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else                result += static_cast<char>(c);
    }
    return result;
}

std::string getValue(const std::string& obj, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t pos = obj.find(searchKey);
    if (pos == std::string::npos) return "";
    pos += searchKey.size();

    while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t')) ++pos;
    if (pos >= obj.size()) return "";

    if (obj[pos] == '"') {
        ++pos;
        std::string value;
        while (pos < obj.size() && obj[pos] != '"') {
            if (obj[pos] == '\\' && pos + 1 < obj.size()) {
                ++pos;
                switch (obj[pos]) {
                case '"':  value += '"';  break;
                case '\\': value += '\\'; break;
                case 'n':  value += '\n'; break;
                case 'r':  value += '\r'; break;
                default:   value += obj[pos]; break;
                }
            } else {
                value += obj[pos];
            }
            ++pos;
        }
        return value;
    }

    // number or bare value
    std::string value;
    while (pos < obj.size() && obj[pos] != ',' && obj[pos] != '}' &&
           obj[pos] != ' '  && obj[pos] != '\n') {
        value += obj[pos++];
    }
    return value;
}

std::vector<std::string> parseObjects(const std::string& content) {
    std::vector<std::string> objects;
    size_t pos = 0;

    while (pos < content.size() && content[pos] != '[') ++pos;
    if (pos >= content.size()) return objects;
    ++pos;

    while (pos < content.size()) {
        while (pos < content.size() &&
               (content[pos] == ' ' || content[pos] == '\t' ||
                content[pos] == '\n' || content[pos] == '\r' ||
                content[pos] == ',')) {
            ++pos;
        }
        if (pos >= content.size() || content[pos] == ']') break;

        if (content[pos] == '{') {
            size_t start = pos;
            int depth = 0;
            bool inStr = false;
            bool escaped = false;
            while (pos < content.size()) {
                char c = content[pos];
                if (inStr) {
                    if      (escaped)   escaped = false;
                    else if (c == '\\') escaped = true;
                    else if (c == '"')  inStr = false;
                } else {
                    if      (c == '{') ++depth;
                    else if (c == '}') {
                        --depth;
                        if (depth == 0) {
                            objects.push_back(content.substr(start, pos - start + 1));
                            ++pos;
                            break;
                        }
                    } else if (c == '"') inStr = true;
                }
                ++pos;
            }
        } else {
            ++pos;
        }
    }
    return objects;
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "[]";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void writeFile(const std::string& path, const std::string& content) {
    std::filesystem::path p(path);
    if (p.has_parent_path() && !p.parent_path().empty()) {
        std::filesystem::create_directories(p.parent_path());
    }
    std::ofstream file(path);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path);
    file << content;
}

} // namespace JsonUtil
