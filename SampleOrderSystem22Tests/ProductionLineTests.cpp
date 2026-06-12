#include <gtest/gtest.h>
#include "Model/SampleStorage.h"
#include "Model/OrderStorage.h"
#include "Controller/ProductionController.h"
#include <cstdio>
#include <cmath>

class ProductionControllerTest : public ::testing::Test {
protected:
    const std::string m_sampleFile = "test_prod_samples_tmp.json";
    const std::string m_orderFile  = "test_prod_orders_tmp.json";

    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;

    void SetUp() override {
        std::remove(m_sampleFile.c_str());
        std::remove(m_orderFile.c_str());
        m_sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
        m_orderStorage  = std::make_shared<OrderStorage>(m_orderFile);
    }

    void TearDown() override {
        std::remove(m_sampleFile.c_str());
        std::remove(m_orderFile.c_str());
    }

    Sample makeSample(const std::string& id, int apt, double yr, int stock = 0) {
        Sample s;
        s.id = id; s.name = id + "_name";
        s.avgProductionTime = apt; s.yieldRate = yr; s.stock = stock;
        return s;
    }

    Order makeProducingOrder(const std::string& num, const std::string& sid, int qty) {
        Order o;
        o.orderNumber = num; o.customerName = "Customer";
        o.sampleId = sid; o.quantity = qty;
        o.status = OrderStatus::PRODUCING;
        return o;
    }
};

// ===== 생산 수량 계산 =====

TEST_F(ProductionControllerTest, CalcActualProduction_ExactFormula) {
    // shortage=10, yieldRate=0.9 → ceil(10 / (0.9 * 0.9)) = ceil(12.34) = 13
    int actual = ProductionController::calcActualProduction(10, 0.9);
    EXPECT_EQ(actual, 13);
}

TEST_F(ProductionControllerTest, CalcActualProduction_ShortageOne) {
    // shortage=1, yieldRate=1.0 → ceil(1 / 0.9) = ceil(1.11) = 2
    int actual = ProductionController::calcActualProduction(1, 1.0);
    EXPECT_EQ(actual, 2);
}

TEST_F(ProductionControllerTest, CalcActualProduction_HighYield) {
    // shortage=5, yieldRate=0.95 → ceil(5 / (0.95*0.9)) = ceil(5.85) = 6
    int actual = ProductionController::calcActualProduction(5, 0.95);
    EXPECT_EQ(actual, 6);
}

// ===== 생산 큐 =====

TEST_F(ProductionControllerTest, GetQueueReturnsOnlyProducingOrders) {
    m_sampleStorage->create(makeSample("S001", 30, 0.9));
    m_orderStorage->create(makeProducingOrder("ORD-001", "S001", 10));

    Order confirmed;
    confirmed.orderNumber = "ORD-002"; confirmed.customerName = "C";
    confirmed.sampleId = "S001"; confirmed.quantity = 5;
    confirmed.status = OrderStatus::CONFIRMED;
    m_orderStorage->create(confirmed);

    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    auto queue = ctrl.getQueue();

    ASSERT_EQ(queue.size(), 1u);
    EXPECT_EQ(queue[0].orderNumber, "ORD-001");
}

TEST_F(ProductionControllerTest, GetQueueIsFIFO) {
    m_sampleStorage->create(makeSample("S001", 30, 0.9));
    m_orderStorage->create(makeProducingOrder("ORD-001", "S001", 5));
    m_orderStorage->create(makeProducingOrder("ORD-002", "S001", 8));
    m_orderStorage->create(makeProducingOrder("ORD-003", "S001", 3));

    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    auto queue = ctrl.getQueue();

    ASSERT_EQ(queue.size(), 3u);
    EXPECT_EQ(queue[0].orderNumber, "ORD-001");
    EXPECT_EQ(queue[1].orderNumber, "ORD-002");
    EXPECT_EQ(queue[2].orderNumber, "ORD-003");
}

TEST_F(ProductionControllerTest, CompleteProductionChangesStatusToConfirmed) {
    m_sampleStorage->create(makeSample("S001", 30, 0.9, 0));
    m_orderStorage->create(makeProducingOrder("ORD-001", "S001", 10));

    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    ctrl.completeProduction("ORD-001");

    auto order = m_orderStorage->readById("ORD-001");
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::CONFIRMED);
}

TEST_F(ProductionControllerTest, CompleteProductionAddsStockAndDeducts) {
    // stock=0, order=10, yieldRate=0.9
    // actualProduction = ceil(10/(0.9*0.9)) = 13
    // 생산 후 재고 = 13 - 10 = 3
    m_sampleStorage->create(makeSample("S001", 30, 0.9, 0));
    m_orderStorage->create(makeProducingOrder("ORD-001", "S001", 10));

    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    ctrl.completeProduction("ORD-001");

    auto sample = m_sampleStorage->readById("S001");
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->stock, 3);  // 13 생산 - 10 출고 = 3 남음
}

TEST_F(ProductionControllerTest, CompleteProductionWithExistingStock) {
    // stock=5, order=10, yieldRate=0.9
    // shortage = 10-5 = 5
    // actualProduction = ceil(5/(0.9*0.9)) = ceil(6.17) = 7
    // 생산 후 재고 = 5 + 7 - 10 = 2
    m_sampleStorage->create(makeSample("S001", 30, 0.9, 5));
    m_orderStorage->create(makeProducingOrder("ORD-001", "S001", 10));

    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    ctrl.completeProduction("ORD-001");

    auto sample = m_sampleStorage->readById("S001");
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->stock, 2);
}

TEST_F(ProductionControllerTest, CompleteProductionNonProducingThrows) {
    m_sampleStorage->create(makeSample("S001", 30, 0.9));
    Order o;
    o.orderNumber = "ORD-001"; o.customerName = "C";
    o.sampleId = "S001"; o.quantity = 5;
    o.status = OrderStatus::CONFIRMED;
    m_orderStorage->create(o);

    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    EXPECT_THROW(ctrl.completeProduction("ORD-001"), std::runtime_error);
}

TEST_F(ProductionControllerTest, GetQueueEmptyWhenNoProducingOrders) {
    ProductionController ctrl(m_sampleStorage, m_orderStorage);
    EXPECT_TRUE(ctrl.getQueue().empty());
}
