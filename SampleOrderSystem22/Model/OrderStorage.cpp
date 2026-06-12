#include "OrderStorage.h"
#include "JsonUtil.h"
#include <algorithm>
#include <sstream>

OrderStorage::OrderStorage(const std::string& filePath) : m_filePath(filePath) {}

void OrderStorage::create(const Order& item) {
    auto items = readAll();
    items.push_back(item);
    save(items);
}

std::vector<Order> OrderStorage::readAll() const {
    auto objects = JsonUtil::parseObjects(JsonUtil::readFile(m_filePath));
    std::vector<Order> result;
    result.reserve(objects.size());
    for (const auto& obj : objects)
        result.push_back(Order::fromJsonObject(obj));
    return result;
}

std::optional<Order> OrderStorage::readById(const std::string& id) const {
    for (const auto& item : readAll())
        if (item.orderNumber == id) return item;
    return std::nullopt;
}

bool OrderStorage::update(const Order& item) {
    auto items = readAll();
    for (auto& existing : items) {
        if (existing.orderNumber == item.orderNumber) {
            existing = item;
            save(items);
            return true;
        }
    }
    return false;
}

bool OrderStorage::remove(const std::string& id) {
    auto items = readAll();
    auto before = items.size();
    items.erase(std::remove_if(items.begin(), items.end(),
        [&id](const Order& o) { return o.orderNumber == id; }), items.end());
    if (items.size() == before) return false;
    save(items);
    return true;
}

void OrderStorage::save(const std::vector<Order>& items) const {
    std::ostringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < items.size(); ++i) {
        ss << "  " << items[i].toJsonObject();
        if (i + 1 < items.size()) ss << ",";
        ss << "\n";
    }
    ss << "]";
    JsonUtil::writeFile(m_filePath, ss.str());
}
