#include <gtest/gtest.h>
#include "Model/SampleStorage.h"
#include "Model/OrderStorage.h"
#include "Controller/OrderController.h"
#include <cstdio>
#include <string>

class OrderControllerTest : public ::testing::Test {
protected:
    const std::string m_sampleFile = "test_oc_samples_tmp.json";
    const std::string m_orderFile  = "test_oc_orders_tmp.json";

    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;

    void SetUp() override {
        std::remove(m_sampleFile.c_str());
        std::remove(m_orderFile.c_str());
        m_sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
        m_orderStorage  = std::make_shared<OrderStorage>(m_orderFile);

        // 기본 시료 등록 (재고 100)
        Sample s;
        s.id = "S001"; s.name = "SampleA";
        s.avgProductionTime = 30; s.yieldRate = 0.9; s.stock = 100;
        m_sampleStorage->create(s);
    }

    void TearDown() override {
        std::remove(m_sampleFile.c_str());
        std::remove(m_orderFile.c_str());
    }
};

// ===== 주문 접수 =====

TEST_F(OrderControllerTest, ReceiveOrderCreatesReservedStatus) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);

    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);

    auto orders = m_orderStorage->readAll();
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].status,       OrderStatus::RESERVED);
    EXPECT_EQ(orders[0].customerName, "HongGilDong");
    EXPECT_EQ(orders[0].sampleId,     "S001");
    EXPECT_EQ(orders[0].quantity,     10);
    EXPECT_FALSE(num.empty());
}

TEST_F(OrderControllerTest, OrderNumberFormatIsCorrect) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);

    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);

    // ORD-YYYYMMDD-NNNN 형식 확인
    EXPECT_EQ(num.substr(0, 4), "ORD-");
    EXPECT_EQ(num.size(), 17u);   // ORD-20260612-0001
    EXPECT_EQ(num[12], '-');
}

TEST_F(OrderControllerTest, OrderNumberSequentialForSameDay) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);

    std::string num1 = ctrl.receiveOrder("S001", "CustomerA", 5);
    std::string num2 = ctrl.receiveOrder("S001", "CustomerB", 3);

    EXPECT_NE(num1, num2);
    EXPECT_EQ(num1.substr(num1.size() - 4), "0001");
    EXPECT_EQ(num2.substr(num2.size() - 4), "0002");
}

TEST_F(OrderControllerTest, ReceiveOrderForNonExistentSampleThrows) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);

    EXPECT_THROW(ctrl.receiveOrder("NOTEXIST", "HongGilDong", 10), std::runtime_error);
}

TEST_F(OrderControllerTest, ReceiveOrderWithZeroQuantityThrows) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);

    EXPECT_THROW(ctrl.receiveOrder("S001", "HongGilDong", 0), std::invalid_argument);
}

// ===== 주문 승인: 재고 충분 =====

TEST_F(OrderControllerTest, ApproveOrderWithSufficientStockBecomesConfirmed) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);  // stock=100, need=10

    ctrl.approveOrder(num);

    auto order = m_orderStorage->readById(num);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderControllerTest, ApproveOrderDeductsStock) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);

    ctrl.approveOrder(num);

    auto sample = m_sampleStorage->readById("S001");
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->stock, 90);
}

// ===== 주문 승인: 재고 부족 =====

TEST_F(OrderControllerTest, ApproveOrderWithInsufficientStockBecomesProducing) {
    // 재고 5, 주문 10 → 부족 → PRODUCING
    Sample s2;
    s2.id = "S002"; s2.name = "SampleB";
    s2.avgProductionTime = 20; s2.yieldRate = 0.8; s2.stock = 5;
    m_sampleStorage->create(s2);

    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num = ctrl.receiveOrder("S002", "HongGilDong", 10);

    ctrl.approveOrder(num);

    auto order = m_orderStorage->readById(num);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::PRODUCING);
}

// ===== 주문 거절 =====

TEST_F(OrderControllerTest, RejectOrderBecomesRejected) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);

    ctrl.rejectOrder(num);

    auto order = m_orderStorage->readById(num);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::REJECTED);
}

TEST_F(OrderControllerTest, ApproveNonReservedOrderThrows) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);
    ctrl.rejectOrder(num);  // REJECTED 상태로 전환

    EXPECT_THROW(ctrl.approveOrder(num), std::runtime_error);
}

TEST_F(OrderControllerTest, RejectNonReservedOrderThrows) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num = ctrl.receiveOrder("S001", "HongGilDong", 10);
    ctrl.approveOrder(num);  // CONFIRMED 상태로 전환

    EXPECT_THROW(ctrl.rejectOrder(num), std::runtime_error);
}

TEST_F(OrderControllerTest, GetReservedOrdersReturnsOnlyReserved) {
    OrderController ctrl(m_sampleStorage, m_orderStorage);
    std::string num1 = ctrl.receiveOrder("S001", "CustomerA", 5);
    std::string num2 = ctrl.receiveOrder("S001", "CustomerB", 3);
    ctrl.approveOrder(num1);  // CONFIRMED

    auto reserved = ctrl.getReservedOrders();
    ASSERT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0].orderNumber, num2);
}
