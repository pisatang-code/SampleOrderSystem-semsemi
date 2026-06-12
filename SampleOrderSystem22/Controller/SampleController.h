#pragma once
#include "../Model/SampleStorage.h"
#include <memory>
#include <vector>
#include <string>

class SampleController {
public:
    explicit SampleController(std::shared_ptr<SampleStorage> storage);

    void registerSample(const std::string& id, const std::string& name,
                        int avgProductionTime, double yieldRate);
    std::vector<Sample> getAllSamples() const;
    std::vector<Sample> searchByName(const std::string& keyword) const;

private:
    std::shared_ptr<SampleStorage> m_storage;
};
