#include <gtest/gtest.h>
#include "Model/SampleStorage.h"
#include "Model/OrderStorage.h"
#include "Controller/MonitoringController.h"
#include <cstdio>

class MonitoringControllerTest : public ::testing::Test {
protected:
    const std::string m_sampleFile = "test_mon_samples_tmp.json";
    const std::string m_orderFile  = "test_mon_orders_tmp.json";

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

    Sample makeSample(const std::string& id, int stock) {
        Sample s;
        s.id = id; s.name = id + "_name";
        s.avgProductionTime = 30; s.yieldRate = 0.9; s.stock = stock;
        return s;
    }

    Order makeOrder(const std::string& num, const std::string& sid,
                    int qty, OrderStatus status) {
        Order o;
        o.orderNumber = num; o.customerName = "C";
        o.sampleId = sid; o.quantity = qty; o.status = status;
        return o;
    }
};

// ===== 주문량 확인 =====

TEST_F(MonitoringControllerTest, GetOrderCountByStatus) {
    m_orderStorage->create(makeOrder("O1", "S001", 5,  OrderStatus::RESERVED));
    m_orderStorage->create(makeOrder("O2", "S001", 3,  OrderStatus::PRODUCING));
    m_orderStorage->create(makeOrder("O3", "S001", 2,  OrderStatus::CONFIRMED));
    m_orderStorage->create(makeOrder("O4", "S001", 1,  OrderStatus::RELEASE));
    m_orderStorage->create(makeOrder("O5", "S001", 10, OrderStatus::REJECTED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto counts = ctrl.getOrderCountByStatus();

    EXPECT_EQ(counts.reserved,  1);
    EXPECT_EQ(counts.producing, 1);
    EXPECT_EQ(counts.confirmed, 1);
    EXPECT_EQ(counts.released,  1);
    // REJECTED는 OrderCounts에 포함하지 않는다
}

TEST_F(MonitoringControllerTest, GetOrdersByStatusFiltersCorrectly) {
    m_orderStorage->create(makeOrder("O1", "S001", 5, OrderStatus::RESERVED));
    m_orderStorage->create(makeOrder("O2", "S001", 3, OrderStatus::CONFIRMED));
    m_orderStorage->create(makeOrder("O3", "S001", 2, OrderStatus::CONFIRMED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto confirmed = ctrl.getOrdersByStatus(OrderStatus::CONFIRMED);

    ASSERT_EQ(confirmed.size(), 2u);
}

TEST_F(MonitoringControllerTest, RejectedNotIncludedInOrdersByStatus) {
    m_orderStorage->create(makeOrder("O1", "S001", 5, OrderStatus::REJECTED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto reserved = ctrl.getOrdersByStatus(OrderStatus::RESERVED);
    EXPECT_TRUE(reserved.empty());
}

// ===== 재고 상태 =====

TEST_F(MonitoringControllerTest, StockStatusYuyu_SufficientStock) {
    // 재고 100, RESERVED 주문 10 → 여유
    m_sampleStorage->create(makeSample("S001", 100));
    m_orderStorage->create(makeOrder("O1", "S001", 10, OrderStatus::RESERVED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto statuses = ctrl.getStockStatuses();

    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0].status, MonitoringController::StockStatus::YUYU);
}

TEST_F(MonitoringControllerTest, StockStatusBujok_InsufficientStock) {
    // 재고 5, RESERVED 주문 10 → 부족
    m_sampleStorage->create(makeSample("S001", 5));
    m_orderStorage->create(makeOrder("O1", "S001", 10, OrderStatus::RESERVED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto statuses = ctrl.getStockStatuses();

    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0].status, MonitoringController::StockStatus::BUJOK);
}

TEST_F(MonitoringControllerTest, StockStatusGogal_ZeroStock) {
    // 재고 0 → 고갈
    m_sampleStorage->create(makeSample("S001", 0));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto statuses = ctrl.getStockStatuses();

    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0].status, MonitoringController::StockStatus::GOGAL);
}

TEST_F(MonitoringControllerTest, StockStatusIgnoresRejectedOrders) {
    // 재고 5, REJECTED 주문 100 → 여유 (REJECTED는 무시)
    m_sampleStorage->create(makeSample("S001", 5));
    m_orderStorage->create(makeOrder("O1", "S001", 100, OrderStatus::REJECTED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto statuses = ctrl.getStockStatuses();

    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_EQ(statuses[0].status, MonitoringController::StockStatus::YUYU);
}

// ===== 출고 처리 =====

TEST_F(MonitoringControllerTest, ShipOrderChangesConfirmedToRelease) {
    m_sampleStorage->create(makeSample("S001", 50));
    m_orderStorage->create(makeOrder("O1", "S001", 10, OrderStatus::CONFIRMED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    ctrl.shipOrder("O1");

    auto order = m_orderStorage->readById("O1");
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::RELEASE);
}

TEST_F(MonitoringControllerTest, ShipNonConfirmedOrderThrows) {
    m_sampleStorage->create(makeSample("S001", 50));
    m_orderStorage->create(makeOrder("O1", "S001", 10, OrderStatus::RESERVED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    EXPECT_THROW(ctrl.shipOrder("O1"), std::runtime_error);
}

TEST_F(MonitoringControllerTest, GetConfirmedOrdersReturnsOnlyConfirmed) {
    m_orderStorage->create(makeOrder("O1", "S001", 5,  OrderStatus::CONFIRMED));
    m_orderStorage->create(makeOrder("O2", "S001", 3,  OrderStatus::RESERVED));
    m_orderStorage->create(makeOrder("O3", "S001", 2,  OrderStatus::CONFIRMED));

    MonitoringController ctrl(m_sampleStorage, m_orderStorage);
    auto confirmed = ctrl.getConfirmedOrders();

    ASSERT_EQ(confirmed.size(), 2u);
}
