#include "SampleStorage.h"
#include "JsonUtil.h"
#include <algorithm>

SampleStorage::SampleStorage(const std::string& filePath) : m_filePath(filePath) {}

void SampleStorage::create(const Sample& item) {
    auto items = readAll();
    items.push_back(item);
    save(items);
}

std::vector<Sample> SampleStorage::readAll() const {
    auto objects = JsonUtil::parseObjects(JsonUtil::readFile(m_filePath));
    std::vector<Sample> result;
    result.reserve(objects.size());
    for (const auto& obj : objects)
        result.push_back(Sample::fromJsonObject(obj));
    return result;
}

std::optional<Sample> SampleStorage::readById(const std::string& id) const {
    for (const auto& item : readAll())
        if (item.id == id) return item;
    return std::nullopt;
}

bool SampleStorage::update(const Sample& item) {
    auto items = readAll();
    for (auto& existing : items) {
        if (existing.id == item.id) {
            existing = item;
            save(items);
            return true;
        }
    }
    return false;
}

bool SampleStorage::remove(const std::string& id) {
    auto items = readAll();
    auto before = items.size();
    items.erase(std::remove_if(items.begin(), items.end(),
        [&id](const Sample& s) { return s.id == id; }), items.end());
    if (items.size() == before) return false;
    save(items);
    return true;
}

void SampleStorage::save(const std::vector<Sample>& items) const {
    JsonUtil::writeFile(m_filePath, JsonUtil::serializeArray(items));
}
