#pragma once
#include "../Model/SampleStorage.h"
#include "../Model/OrderStorage.h"
#include <memory>
#include <string>
#include <vector>

class OrderController {
public:
    OrderController(std::shared_ptr<SampleStorage> sampleStorage,
                    std::shared_ptr<OrderStorage>  orderStorage);

    std::string         receiveOrder(const std::string& sampleId,
                                     const std::string& customerName,
                                     int quantity);
    void                approveOrder(const std::string& orderNumber);
    void                rejectOrder(const std::string& orderNumber);
    std::vector<Order>  getReservedOrders() const;

private:
    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;

    std::string generateOrderNumber() const;
};
