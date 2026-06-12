#include "SampleController.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace {
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return s;
    }
}

SampleController::SampleController(std::shared_ptr<SampleStorage> storage)
    : m_storage(storage) {}

void SampleController::registerSample(const std::string& id, const std::string& name,
                                      int avgProductionTime, double yieldRate) {
    if (avgProductionTime <= 0)
        throw std::invalid_argument("avgProductionTime must be > 0");
    if (yieldRate <= 0.0 || yieldRate > 1.0)
        throw std::invalid_argument("yieldRate must be in (0.0, 1.0]");
    if (m_storage->readById(id).has_value())
        throw std::runtime_error("Sample ID already exists: " + id);

    Sample s;
    s.id = id; s.name = name;
    s.avgProductionTime = avgProductionTime;
    s.yieldRate = yieldRate;
    s.stock = 0;
    m_storage->create(s);
}

std::vector<Sample> SampleController::getAllSamples() const {
    return m_storage->readAll();
}

std::vector<Sample> SampleController::searchByName(const std::string& keyword) const {
    std::string lowerKey = toLower(keyword);

    std::vector<Sample> results;
    for (const auto& s : m_storage->readAll()) {
        if (toLower(s.name).find(lowerKey) != std::string::npos)
            results.push_back(s);
    }
    return results;
}
