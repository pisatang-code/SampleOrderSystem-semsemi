#include "DummyDataGenerator.h"

DummyDataGenerator::DummyDataGenerator(std::shared_ptr<SampleStorage> sampleStorage,
                                        std::shared_ptr<OrderStorage>  orderStorage)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage) {}

void DummyDataGenerator::populate() {
    if (m_sampleStorage->readAll().empty()) populateSamples();
    if (m_orderStorage->readAll().empty())  populateOrders();
}

void DummyDataGenerator::populateSamples() {
    auto make = [](const std::string& id, const std::string& name,
                   int apt, double yr, int stock) {
        Sample s;
        s.id = id; s.name = name;
        s.avgProductionTime = apt; s.yieldRate = yr; s.stock = stock;
        return s;
    };

    m_sampleStorage->create(make("S001", "AlphaSample",  30, 0.90, 200));
    m_sampleStorage->create(make("S002", "BetaSample",   45, 0.85, 150));
    m_sampleStorage->create(make("S003", "GammaSample",  20, 0.95,  80));
    m_sampleStorage->create(make("S004", "DeltaSample",  60, 0.80,  30));
    m_sampleStorage->create(make("S005", "EpsilonSample",15, 0.92, 500));
}

void DummyDataGenerator::populateOrders() {
    auto make = [](const std::string& num, const std::string& customer,
                   const std::string& sid, int qty, OrderStatus st) {
        Order o;
        o.orderNumber = num; o.customerName = customer;
        o.sampleId = sid; o.quantity = qty; o.status = st;
        return o;
    };

    m_orderStorage->create(make("ORD-20260101-0001", "CompanyA",  "S001", 50,  OrderStatus::CONFIRMED));
    m_orderStorage->create(make("ORD-20260101-0002", "CompanyB",  "S002", 30,  OrderStatus::PRODUCING));
    m_orderStorage->create(make("ORD-20260101-0003", "CompanyC",  "S003", 20,  OrderStatus::RESERVED));
    m_orderStorage->create(make("ORD-20260101-0004", "CompanyD",  "S001", 10,  OrderStatus::RELEASE));
    m_orderStorage->create(make("ORD-20260101-0005", "CompanyE",  "S004",  5,  OrderStatus::REJECTED));
    m_orderStorage->create(make("ORD-20260101-0006", "CompanyF",  "S005", 100, OrderStatus::RESERVED));
}
