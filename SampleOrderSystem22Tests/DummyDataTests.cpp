#include <gtest/gtest.h>
#include "Model/SampleStorage.h"
#include "Model/OrderStorage.h"
#include "Model/DummyDataGenerator.h"
#include <cstdio>

class DummyDataGeneratorTest : public ::testing::Test {
protected:
    const std::string m_sampleFile = "test_dummy_samples_tmp.json";
    const std::string m_orderFile  = "test_dummy_orders_tmp.json";

    void SetUp()    override {
        std::remove(m_sampleFile.c_str());
        std::remove(m_orderFile.c_str());
    }
    void TearDown() override {
        std::remove(m_sampleFile.c_str());
        std::remove(m_orderFile.c_str());
    }
};

TEST_F(DummyDataGeneratorTest, PopulateSamplesCreatesExpectedCount) {
    auto sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
    auto orderStorage  = std::make_shared<OrderStorage>(m_orderFile);

    DummyDataGenerator gen(sampleStorage, orderStorage);
    gen.populate();

    auto samples = sampleStorage->readAll();
    EXPECT_GE(samples.size(), 3u);
}

TEST_F(DummyDataGeneratorTest, PopulateOrdersCreatesExpectedCount) {
    auto sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
    auto orderStorage  = std::make_shared<OrderStorage>(m_orderFile);

    DummyDataGenerator gen(sampleStorage, orderStorage);
    gen.populate();

    auto orders = orderStorage->readAll();
    EXPECT_GE(orders.size(), 3u);
}

TEST_F(DummyDataGeneratorTest, PopulatedSamplesHaveValidFields) {
    auto sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
    auto orderStorage  = std::make_shared<OrderStorage>(m_orderFile);

    DummyDataGenerator gen(sampleStorage, orderStorage);
    gen.populate();

    for (const auto& s : sampleStorage->readAll()) {
        EXPECT_FALSE(s.id.empty());
        EXPECT_FALSE(s.name.empty());
        EXPECT_GT(s.avgProductionTime, 0);
        EXPECT_GT(s.yieldRate, 0.0);
        EXPECT_LE(s.yieldRate, 1.0);
        EXPECT_GE(s.stock, 0);
    }
}

TEST_F(DummyDataGeneratorTest, PopulatedOrdersReferenceExistingSamples) {
    auto sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
    auto orderStorage  = std::make_shared<OrderStorage>(m_orderFile);

    DummyDataGenerator gen(sampleStorage, orderStorage);
    gen.populate();

    for (const auto& o : orderStorage->readAll()) {
        EXPECT_TRUE(sampleStorage->readById(o.sampleId).has_value());
        EXPECT_GT(o.quantity, 0);
        EXPECT_FALSE(o.customerName.empty());
    }
}

TEST_F(DummyDataGeneratorTest, PopulateIdempotentOnSecondCall) {
    auto sampleStorage = std::make_shared<SampleStorage>(m_sampleFile);
    auto orderStorage  = std::make_shared<OrderStorage>(m_orderFile);

    DummyDataGenerator gen(sampleStorage, orderStorage);
    gen.populate();
    size_t countAfterFirst = sampleStorage->readAll().size();
    gen.populate();
    size_t countAfterSecond = sampleStorage->readAll().size();

    EXPECT_EQ(countAfterFirst, countAfterSecond);
}
