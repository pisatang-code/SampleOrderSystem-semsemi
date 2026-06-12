#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "OrderController.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

OrderController::OrderController(std::shared_ptr<SampleStorage> sampleStorage,
                                 std::shared_ptr<OrderStorage>  orderStorage)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage) {}

std::string OrderController::receiveOrder(const std::string& sampleId,
                                           const std::string& customerName,
                                           int quantity) {
    if (quantity <= 0)
        throw std::invalid_argument("quantity must be > 0");
    if (!m_sampleStorage->readById(sampleId).has_value())
        throw std::runtime_error("Sample not found: " + sampleId);

    Order o;
    o.orderNumber  = generateOrderNumber();
    o.customerName = customerName;
    o.sampleId     = sampleId;
    o.quantity     = quantity;
    o.status       = OrderStatus::RESERVED;
    m_orderStorage->create(o);
    return o.orderNumber;
}

void OrderController::approveOrder(const std::string& orderNumber) {
    auto orderOpt = m_orderStorage->readById(orderNumber);
    if (!orderOpt.has_value())
        throw std::runtime_error("Order not found: " + orderNumber);
    if (orderOpt->status != OrderStatus::RESERVED)
        throw std::runtime_error("Only RESERVED orders can be approved");

    auto sampleOpt = m_sampleStorage->readById(orderOpt->sampleId);
    if (!sampleOpt.has_value())
        throw std::runtime_error("Sample not found: " + orderOpt->sampleId);

    Order order   = *orderOpt;
    Sample sample = *sampleOpt;

    if (sample.stock >= order.quantity) {
        sample.stock -= order.quantity;
        order.status  = OrderStatus::CONFIRMED;
    } else {
        order.status = OrderStatus::PRODUCING;
    }

    m_sampleStorage->update(sample);
    m_orderStorage->update(order);
}

void OrderController::rejectOrder(const std::string& orderNumber) {
    auto orderOpt = m_orderStorage->readById(orderNumber);
    if (!orderOpt.has_value())
        throw std::runtime_error("Order not found: " + orderNumber);
    if (orderOpt->status != OrderStatus::RESERVED)
        throw std::runtime_error("Only RESERVED orders can be rejected");

    Order order  = *orderOpt;
    order.status = OrderStatus::REJECTED;
    m_orderStorage->update(order);
}

std::vector<Order> OrderController::getReservedOrders() const {
    std::vector<Order> result;
    for (const auto& o : m_orderStorage->readAll())
        if (o.status == OrderStatus::RESERVED) result.push_back(o);
    return result;
}

std::string OrderController::generateOrderNumber() const {
    // 날짜 취득 (Windows SYSTEMTIME)
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream date;
    date << std::setfill('0')
         << std::setw(4) << st.wYear
         << std::setw(2) << st.wMonth
         << std::setw(2) << st.wDay;

    // 당일 최대 시퀀스 번호 계산
    std::string prefix = "ORD-" + date.str() + "-";
    int maxSeq = 0;
    for (const auto& o : m_orderStorage->readAll()) {
        if (o.orderNumber.substr(0, prefix.size()) == prefix) {
            int seq = std::stoi(o.orderNumber.substr(prefix.size()));
            maxSeq  = std::max(maxSeq, seq);
        }
    }

    std::ostringstream ss;
    ss << prefix << std::setfill('0') << std::setw(4) << (maxSeq + 1);
    return ss.str();
}
