#pragma once
#include <vector>
#include <optional>
#include <string>

template<typename T>
class IDataStorage {
public:
    virtual ~IDataStorage() = default;
    virtual void create(const T& item) = 0;
    virtual std::vector<T> readAll() const = 0;
    virtual std::optional<T> readById(const std::string& id) const = 0;
    virtual bool update(const T& item) = 0;
    virtual bool remove(const std::string& id) = 0;
};
