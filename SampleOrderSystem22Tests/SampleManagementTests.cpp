#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Model/SampleStorage.h"
#include "Controller/SampleController.h"
#include <cstdio>
#include <sstream>

// ===== SampleController Tests =====

class SampleControllerTest : public ::testing::Test {
protected:
    const std::string m_file = "test_sc_samples_tmp.json";

    void SetUp()    override { std::remove(m_file.c_str()); }
    void TearDown() override { std::remove(m_file.c_str()); }

    Sample make(const std::string& id, const std::string& name,
                int apt, double yr, int stock = 0) {
        Sample s;
        s.id = id; s.name = name;
        s.avgProductionTime = apt; s.yieldRate = yr; s.stock = stock;
        return s;
    }
};

TEST_F(SampleControllerTest, RegisterSamplePersists) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "SampleA", 30, 0.9);

    auto all = storage->readAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id,   "S001");
    EXPECT_EQ(all[0].name, "SampleA");
    EXPECT_EQ(all[0].avgProductionTime, 30);
    EXPECT_DOUBLE_EQ(all[0].yieldRate, 0.9);
}

TEST_F(SampleControllerTest, RegisterDuplicateIdThrows) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "SampleA", 30, 0.9);
    EXPECT_THROW(ctrl.registerSample("S001", "SampleB", 20, 0.8), std::runtime_error);
}

TEST_F(SampleControllerTest, GetAllSamplesReturnsAll) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "SampleA", 30, 0.9);
    ctrl.registerSample("S002", "SampleB", 45, 0.85);

    auto all = ctrl.getAllSamples();
    EXPECT_EQ(all.size(), 2u);
}

TEST_F(SampleControllerTest, SearchByNameFindsMatch) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "AlphaSample", 30, 0.9);
    ctrl.registerSample("S002", "BetaSample",  45, 0.85);
    ctrl.registerSample("S003", "GammaSample", 20, 0.95);

    auto results = ctrl.searchByName("Beta");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].id, "S002");
}

TEST_F(SampleControllerTest, SearchByNameCaseInsensitive) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "AlphaSample", 30, 0.9);

    auto results = ctrl.searchByName("alpha");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(SampleControllerTest, SearchByNameNoMatchReturnsEmpty) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "SampleA", 30, 0.9);

    auto results = ctrl.searchByName("XYZ");
    EXPECT_TRUE(results.empty());
}

TEST_F(SampleControllerTest, SearchByNameMultipleMatches) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    ctrl.registerSample("S001", "SampleAlpha", 30, 0.9);
    ctrl.registerSample("S002", "SampleBeta",  45, 0.85);
    ctrl.registerSample("S003", "GammaSample", 20, 0.95);

    auto results = ctrl.searchByName("Sample");
    EXPECT_EQ(results.size(), 3u);
}

TEST_F(SampleControllerTest, InvalidYieldRateThrows) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    EXPECT_THROW(ctrl.registerSample("S001", "SampleA", 30, 1.5), std::invalid_argument);
    EXPECT_THROW(ctrl.registerSample("S002", "SampleB", 30, -0.1), std::invalid_argument);
}

TEST_F(SampleControllerTest, InvalidProductionTimeThrows) {
    auto storage = std::make_shared<SampleStorage>(m_file);
    SampleController ctrl(storage);

    EXPECT_THROW(ctrl.registerSample("S001", "SampleA", 0, 0.9), std::invalid_argument);
    EXPECT_THROW(ctrl.registerSample("S002", "SampleB", -1, 0.9), std::invalid_argument);
}
