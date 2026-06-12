#include <gtest/gtest.h>
#include "Model/OrderStatus.h"
#include "Model/Sample.h"
#include "Model/Order.h"

// ===== OrderStatus =====

TEST(OrderStatusTest, AllStatusesToString) {
    EXPECT_EQ(orderStatusToString(OrderStatus::RESERVED),  "RESERVED");
    EXPECT_EQ(orderStatusToString(OrderStatus::REJECTED),  "REJECTED");
    EXPECT_EQ(orderStatusToString(OrderStatus::PRODUCING), "PRODUCING");
    EXPECT_EQ(orderStatusToString(OrderStatus::CONFIRMED), "CONFIRMED");
    EXPECT_EQ(orderStatusToString(OrderStatus::RELEASE),   "RELEASE");
}

TEST(OrderStatusTest, FromStringRoundTrip) {
    EXPECT_EQ(orderStatusFromString("RESERVED"),  OrderStatus::RESERVED);
    EXPECT_EQ(orderStatusFromString("REJECTED"),  OrderStatus::REJECTED);
    EXPECT_EQ(orderStatusFromString("PRODUCING"), OrderStatus::PRODUCING);
    EXPECT_EQ(orderStatusFromString("CONFIRMED"), OrderStatus::CONFIRMED);
    EXPECT_EQ(orderStatusFromString("RELEASE"),   OrderStatus::RELEASE);
}

// ===== Sample =====

TEST(SampleTest, CanSetAndGetFields) {
    Sample s;
    s.id = "S001";
    s.name = "SampleA";
    s.avgProductionTime = 30;
    s.yieldRate = 0.9;
    s.stock = 100;

    EXPECT_EQ(s.id, "S001");
    EXPECT_EQ(s.name, "SampleA");
    EXPECT_EQ(s.avgProductionTime, 30);
    EXPECT_DOUBLE_EQ(s.yieldRate, 0.9);
    EXPECT_EQ(s.stock, 100);
}

TEST(SampleTest, GetIdReturnsSampleId) {
    Sample s;
    s.id = "S001";
    EXPECT_EQ(s.getId(), "S001");
}

TEST(SampleTest, ToJsonContainsAllFields) {
    Sample s;
    s.id = "S001";
    s.name = "SampleA";
    s.avgProductionTime = 30;
    s.yieldRate = 0.9;
    s.stock = 100;

    std::string json = s.toJsonObject();
    EXPECT_NE(json.find("\"id\":\"S001\""),              std::string::npos);
    EXPECT_NE(json.find("\"avgProductionTime\":30"),      std::string::npos);
    EXPECT_NE(json.find("\"stock\":100"),                 std::string::npos);
}

TEST(SampleTest, FromJsonParsesAllFields) {
    std::string json =
        "{\"id\":\"S001\",\"name\":\"SampleA\","
        "\"avgProductionTime\":30,\"yieldRate\":0.9,\"stock\":100}";
    Sample s = Sample::fromJsonObject(json);

    EXPECT_EQ(s.id, "S001");
    EXPECT_EQ(s.name, "SampleA");
    EXPECT_EQ(s.avgProductionTime, 30);
    EXPECT_DOUBLE_EQ(s.yieldRate, 0.9);
    EXPECT_EQ(s.stock, 100);
}

TEST(SampleTest, JsonRoundTrip) {
    Sample original;
    original.id = "S002";
    original.name = "TestSample";
    original.avgProductionTime = 45;
    original.yieldRate = 0.85;
    original.stock = 50;

    Sample restored = Sample::fromJsonObject(original.toJsonObject());
    EXPECT_EQ(restored.id,                  original.id);
    EXPECT_EQ(restored.name,                original.name);
    EXPECT_EQ(restored.avgProductionTime,   original.avgProductionTime);
    EXPECT_DOUBLE_EQ(restored.yieldRate,    original.yieldRate);
    EXPECT_EQ(restored.stock,               original.stock);
}

// ===== Order =====

TEST(OrderTest, DefaultStatusIsReserved) {
    Order o;
    EXPECT_EQ(o.status, OrderStatus::RESERVED);
}

TEST(OrderTest, CanSetAndGetFields) {
    Order o;
    o.orderNumber  = "ORD-20260612-0001";
    o.customerName = "HongGilDong";
    o.sampleId     = "S001";
    o.quantity     = 10;
    o.status       = OrderStatus::RESERVED;

    EXPECT_EQ(o.orderNumber,  "ORD-20260612-0001");
    EXPECT_EQ(o.customerName, "HongGilDong");
    EXPECT_EQ(o.sampleId,     "S001");
    EXPECT_EQ(o.quantity,     10);
    EXPECT_EQ(o.status,       OrderStatus::RESERVED);
}

TEST(OrderTest, GetIdReturnsOrderNumber) {
    Order o;
    o.orderNumber = "ORD-20260612-0001";
    EXPECT_EQ(o.getId(), "ORD-20260612-0001");
}

TEST(OrderTest, ToJsonContainsAllFields) {
    Order o;
    o.orderNumber  = "ORD-20260612-0001";
    o.customerName = "HongGilDong";
    o.sampleId     = "S001";
    o.quantity     = 10;
    o.status       = OrderStatus::RESERVED;

    std::string json = o.toJsonObject();
    EXPECT_NE(json.find("\"orderNumber\":\"ORD-20260612-0001\""), std::string::npos);
    EXPECT_NE(json.find("\"quantity\":10"),                       std::string::npos);
    EXPECT_NE(json.find("\"status\":\"RESERVED\""),               std::string::npos);
}

TEST(OrderTest, FromJsonParsesAllFields) {
    std::string json =
        "{\"orderNumber\":\"ORD-20260612-0001\","
        "\"customerName\":\"HongGilDong\","
        "\"sampleId\":\"S001\","
        "\"quantity\":10,"
        "\"status\":\"CONFIRMED\"}";
    Order o = Order::fromJsonObject(json);

    EXPECT_EQ(o.orderNumber,  "ORD-20260612-0001");
    EXPECT_EQ(o.customerName, "HongGilDong");
    EXPECT_EQ(o.sampleId,     "S001");
    EXPECT_EQ(o.quantity,     10);
    EXPECT_EQ(o.status,       OrderStatus::CONFIRMED);
}

TEST(OrderTest, JsonRoundTrip) {
    Order original;
    original.orderNumber  = "ORD-20260612-0002";
    original.customerName = "KimCheolsu";
    original.sampleId     = "S002";
    original.quantity     = 20;
    original.status       = OrderStatus::PRODUCING;

    Order restored = Order::fromJsonObject(original.toJsonObject());
    EXPECT_EQ(restored.orderNumber,  original.orderNumber);
    EXPECT_EQ(restored.customerName, original.customerName);
    EXPECT_EQ(restored.sampleId,     original.sampleId);
    EXPECT_EQ(restored.quantity,     original.quantity);
    EXPECT_EQ(restored.status,       original.status);
}
