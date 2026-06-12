#pragma once
#include "../Model/SampleStorage.h"
#include "../Model/OrderStorage.h"
#include <memory>
#include <vector>

class ProductionController {
public:
    ProductionController(std::shared_ptr<SampleStorage> sampleStorage,
                         std::shared_ptr<OrderStorage>  orderStorage);

    // 생산 수량 계산: ceil(shortage / (yieldRate * 0.9))
    static int calcActualProduction(int shortage, double yieldRate);

    std::vector<Order> getQueue() const;
    void completeProduction(const std::string& orderNumber);

private:
    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;
};
