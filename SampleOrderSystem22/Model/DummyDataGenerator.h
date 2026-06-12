#pragma once
#include "SampleStorage.h"
#include "OrderStorage.h"
#include <memory>

class DummyDataGenerator {
public:
    DummyDataGenerator(std::shared_ptr<SampleStorage> sampleStorage,
                       std::shared_ptr<OrderStorage>  orderStorage);

    // 기존 데이터가 없을 때만 더미 데이터를 삽입한다 (멱등성)
    void populate();

private:
    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;

    void populateSamples();
    void populateOrders();
};
