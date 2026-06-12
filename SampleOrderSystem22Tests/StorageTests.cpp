#include <gtest/gtest.h>
#include "Model/SampleStorage.h"
#include "Model/OrderStorage.h"
#include <cstdio>

// ===== SampleStorage =====

class SampleStorageTest : public ::testing::Test {
protected:
    const std::string m_file = "test_samples_tmp.json";

    void SetUp()    override { std::remove(m_file.c_str()); }
    void TearDown() override { std::remove(m_file.c_str()); }

    Sample make(const std::string& id, const std::string& name,
                int apt, double yr, int stock) {
        Sample s;
        s.id = id; s.name = name;
        s.avgProductionTime = apt; s.yieldRate = yr; s.stock = stock;
        return s;
    }
};

TEST_F(SampleStorageTest, EmptyFileReturnsEmptyVector) {
    SampleStorage st(m_file);
    EXPECT_TRUE(st.readAll().empty());
}

TEST_F(SampleStorageTest, CreateThenReadAll) {
    SampleStorage st(m_file);
    st.create(make("S001", "SampleA", 30, 0.9, 100));

    auto all = st.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id,   "S001");
    EXPECT_EQ(all[0].name, "SampleA");
    EXPECT_EQ(all[0].stock, 100);
}

TEST_F(SampleStorageTest, CreateMultiple) {
    SampleStorage st(m_file);
    st.create(make("S001", "SampleA", 30, 0.9, 100));
    st.create(make("S002", "SampleB", 45, 0.85, 50));

    EXPECT_EQ(st.readAll().size(), 2u);
}

TEST_F(SampleStorageTest, ReadByIdFound) {
    SampleStorage st(m_file);
    st.create(make("S001", "SampleA", 30, 0.9, 100));

    auto result = st.readById("S001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, "S001");
}

TEST_F(SampleStorageTest, ReadByIdNotFound) {
    SampleStorage st(m_file);
    EXPECT_FALSE(st.readById("NOTEXIST").has_value());
}

TEST_F(SampleStorageTest, UpdateExistingItem) {
    SampleStorage st(m_file);
    st.create(make("S001", "SampleA", 30, 0.9, 100));

    Sample updated = make("S001", "SampleA-Updated", 40, 0.95, 200);
    EXPECT_TRUE(st.update(updated));

    auto found = st.readById("S001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name,  "SampleA-Updated");
    EXPECT_EQ(found->stock, 200);
}

TEST_F(SampleStorageTest, UpdateNonExistingReturnsFalse) {
    SampleStorage st(m_file);
    EXPECT_FALSE(st.update(make("NOTEXIST", "x", 0, 0.0, 0)));
}

TEST_F(SampleStorageTest, RemoveExistingItem) {
    SampleStorage st(m_file);
    st.create(make("S001", "SampleA", 30, 0.9, 100));
    st.create(make("S002", "SampleB", 45, 0.85, 50));

    EXPECT_TRUE(st.remove("S001"));

    auto all = st.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, "S002");
}

TEST_F(SampleStorageTest, RemoveNonExistingReturnsFalse) {
    SampleStorage st(m_file);
    EXPECT_FALSE(st.remove("NOTEXIST"));
}

TEST_F(SampleStorageTest, PersistenceAcrossInstances) {
    {
        SampleStorage st(m_file);
        st.create(make("S001", "SampleA", 30, 0.9, 100));
    }
    SampleStorage st2(m_file);
    auto all = st2.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, "S001");
}

// ===== OrderStorage =====

class OrderStorageTest : public ::testing::Test {
protected:
    const std::string m_file = "test_orders_tmp.json";

    void SetUp()    override { std::remove(m_file.c_str()); }
    void TearDown() override { std::remove(m_file.c_str()); }

    Order make(const std::string& num, const std::string& customer,
               const std::string& sampleId, int qty, OrderStatus status) {
        Order o;
        o.orderNumber = num; o.customerName = customer;
        o.sampleId = sampleId; o.quantity = qty; o.status = status;
        return o;
    }
};

TEST_F(OrderStorageTest, EmptyFileReturnsEmptyVector) {
    OrderStorage st(m_file);
    EXPECT_TRUE(st.readAll().empty());
}

TEST_F(OrderStorageTest, CreateThenReadAll) {
    OrderStorage st(m_file);
    st.create(make("ORD-20260612-0001", "HongGilDong", "S001", 10, OrderStatus::RESERVED));

    auto all = st.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].orderNumber, "ORD-20260612-0001");
    EXPECT_EQ(all[0].status,      OrderStatus::RESERVED);
}

TEST_F(OrderStorageTest, ReadByIdFound) {
    OrderStorage st(m_file);
    st.create(make("ORD-20260612-0001", "HongGilDong", "S001", 10, OrderStatus::RESERVED));

    auto result = st.readById("ORD-20260612-0001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->customerName, "HongGilDong");
}

TEST_F(OrderStorageTest, ReadByIdNotFound) {
    OrderStorage st(m_file);
    EXPECT_FALSE(st.readById("NOTEXIST").has_value());
}

TEST_F(OrderStorageTest, UpdateStatusChange) {
    OrderStorage st(m_file);
    st.create(make("ORD-20260612-0001", "HongGilDong", "S001", 10, OrderStatus::RESERVED));

    Order updated = make("ORD-20260612-0001", "HongGilDong", "S001", 10, OrderStatus::CONFIRMED);
    EXPECT_TRUE(st.update(updated));

    auto found = st.readById("ORD-20260612-0001");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderStorageTest, RemoveExistingOrder) {
    OrderStorage st(m_file);
    st.create(make("ORD-20260612-0001", "HongGilDong", "S001", 10, OrderStatus::RESERVED));
    st.create(make("ORD-20260612-0002", "KimCheolsu",  "S002",  5, OrderStatus::CONFIRMED));

    EXPECT_TRUE(st.remove("ORD-20260612-0001"));

    auto all = st.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].orderNumber, "ORD-20260612-0002");
}

TEST_F(OrderStorageTest, PersistenceAcrossInstances) {
    {
        OrderStorage st(m_file);
        st.create(make("ORD-20260612-0001", "HongGilDong", "S001", 10, OrderStatus::RESERVED));
    }
    OrderStorage st2(m_file);
    auto all = st2.readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].orderNumber, "ORD-20260612-0001");
}
