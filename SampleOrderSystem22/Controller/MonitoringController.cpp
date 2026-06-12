#include "MonitoringController.h"
#include <stdexcept>
#include <algorithm>

MonitoringController::MonitoringController(std::shared_ptr<SampleStorage> sampleStorage,
                                           std::shared_ptr<OrderStorage>  orderStorage)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage) {}

MonitoringController::OrderCounts MonitoringController::getOrderCountByStatus() const {
    OrderCounts counts;
    for (const auto& o : m_orderStorage->readAll()) {
        switch (o.status) {
        case OrderStatus::RESERVED:  ++counts.reserved;  break;
        case OrderStatus::PRODUCING: ++counts.producing; break;
        case OrderStatus::CONFIRMED: ++counts.confirmed; break;
        case OrderStatus::RELEASE:   ++counts.released;  break;
        case OrderStatus::REJECTED:  break;  // 제외
        }
    }
    return counts;
}

std::vector<Order> MonitoringController::getOrdersByStatus(OrderStatus status) const {
    if (status == OrderStatus::REJECTED) return {};

    std::vector<Order> result;
    for (const auto& o : m_orderStorage->readAll())
        if (o.status == status) result.push_back(o);
    return result;
}

std::vector<MonitoringController::SampleStockInfo>
MonitoringController::getStockStatuses() const {
    std::vector<SampleStockInfo> result;

    for (const auto& sample : m_sampleStorage->readAll()) {
        int pending = 0;
        for (const auto& o : m_orderStorage->readAll()) {
            if (o.sampleId == sample.id && o.status != OrderStatus::REJECTED)
                pending += o.quantity;
        }

        SampleStockInfo info;
        info.sample          = sample;
        info.pendingQuantity = pending;

        if (sample.stock == 0)
            info.status = StockStatus::GOGAL;
        else if (sample.stock < pending)
            info.status = StockStatus::BUJOK;
        else
            info.status = StockStatus::YUYU;

        result.push_back(info);
    }
    return result;
}

std::vector<Order> MonitoringController::getConfirmedOrders() const {
    return getOrdersByStatus(OrderStatus::CONFIRMED);
}

void MonitoringController::shipOrder(const std::string& orderNumber) {
    auto orderOpt = m_orderStorage->readById(orderNumber);
    if (!orderOpt.has_value())
        throw std::runtime_error("Order not found: " + orderNumber);
    if (orderOpt->status != OrderStatus::CONFIRMED)
        throw std::runtime_error("Only CONFIRMED orders can be shipped");

    Order order  = *orderOpt;
    order.status = OrderStatus::RELEASE;
    m_orderStorage->update(order);
}
