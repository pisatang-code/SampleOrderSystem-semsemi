#include "Sample.h"
#include "JsonUtil.h"
#include <sstream>

std::string Sample::toJsonObject() const {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":\""               << JsonUtil::escapeJson(id)   << "\","
       << "\"name\":\""             << JsonUtil::escapeJson(name) << "\","
       << "\"avgProductionTime\":"  << avgProductionTime           << ","
       << "\"yieldRate\":"          << yieldRate                   << ","
       << "\"stock\":"              << stock
       << "}";
    return ss.str();
}

Sample Sample::fromJsonObject(const std::string& json) {
    Sample s;
    s.id   = JsonUtil::getValue(json, "id");
    s.name = JsonUtil::getValue(json, "name");
    auto apt = JsonUtil::getValue(json, "avgProductionTime");
    if (!apt.empty())  s.avgProductionTime = std::stoi(apt);
    auto yr = JsonUtil::getValue(json, "yieldRate");
    if (!yr.empty())   s.yieldRate = std::stod(yr);
    auto st = JsonUtil::getValue(json, "stock");
    if (!st.empty())   s.stock = std::stoi(st);
    return s;
}
