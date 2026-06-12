#include "ProductionController.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

ProductionController::ProductionController(std::shared_ptr<SampleStorage> sampleStorage,
                                           std::shared_ptr<OrderStorage>  orderStorage)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage) {}

int ProductionController::calcActualProduction(int shortage, double yieldRate) {
    return static_cast<int>(std::ceil(shortage / (yieldRate * 0.9)));
}

std::vector<Order> ProductionController::getQueue() const {
    std::vector<Order> queue;
    for (const auto& o : m_orderStorage->readAll())
        if (o.status == OrderStatus::PRODUCING) queue.push_back(o);
    return queue;  // 삽입 순서 = FIFO
}

void ProductionController::completeProduction(const std::string& orderNumber) {
    auto orderOpt = m_orderStorage->readById(orderNumber);
    if (!orderOpt.has_value())
        throw std::runtime_error("Order not found: " + orderNumber);
    if (orderOpt->status != OrderStatus::PRODUCING)
        throw std::runtime_error("Only PRODUCING orders can be completed");

    auto sampleOpt = m_sampleStorage->readById(orderOpt->sampleId);
    if (!sampleOpt.has_value())
        throw std::runtime_error("Sample not found: " + orderOpt->sampleId);

    Order  order  = *orderOpt;
    Sample sample = *sampleOpt;

    int actualProduction = calcActualProduction(order.quantity, sample.yieldRate);
    sample.stock += actualProduction;
    sample.stock -= order.quantity;   // 주문분 즉시 차감

    order.status = OrderStatus::CONFIRMED;

    m_sampleStorage->update(sample);
    m_orderStorage->update(order);
}
