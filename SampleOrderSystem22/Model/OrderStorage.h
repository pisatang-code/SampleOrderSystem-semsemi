#pragma once
#include "IDataStorage.h"
#include "Order.h"
#include <string>

class OrderStorage : public IDataStorage<Order> {
public:
    explicit OrderStorage(const std::string& filePath = "db/orders.json");
    void create(const Order& item) override;
    std::vector<Order> readAll() const override;
    std::optional<Order> readById(const std::string& id) const override;
    bool update(const Order& item) override;
    bool remove(const std::string& id) override;

private:
    std::string m_filePath;
    void save(const std::vector<Order>& items) const;
};
