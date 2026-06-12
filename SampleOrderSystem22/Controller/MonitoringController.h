#pragma once
#include "../Model/SampleStorage.h"
#include "../Model/OrderStorage.h"
#include <memory>
#include <vector>
#include <string>

class MonitoringController {
public:
    enum class StockStatus { YUYU, BUJOK, GOGAL };

    struct OrderCounts {
        int reserved  = 0;
        int producing = 0;
        int confirmed = 0;
        int released  = 0;
    };

    struct SampleStockInfo {
        Sample      sample;
        StockStatus status = StockStatus::YUYU;
        int         pendingQuantity = 0;
    };

    MonitoringController(std::shared_ptr<SampleStorage> sampleStorage,
                         std::shared_ptr<OrderStorage>  orderStorage);

    OrderCounts                  getOrderCountByStatus() const;
    std::vector<Order>           getOrdersByStatus(OrderStatus status) const;
    std::vector<SampleStockInfo> getStockStatuses() const;
    std::vector<Order>           getConfirmedOrders() const;
    void                         shipOrder(const std::string& orderNumber);

private:
    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;
};
