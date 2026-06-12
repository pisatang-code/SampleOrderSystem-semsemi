#pragma once
#include "IDataStorage.h"
#include "Sample.h"
#include <string>

class SampleStorage : public IDataStorage<Sample> {
public:
    explicit SampleStorage(const std::string& filePath = "db/samples.json");
    void create(const Sample& item) override;
    std::vector<Sample> readAll() const override;
    std::optional<Sample> readById(const std::string& id) const override;
    bool update(const Sample& item) override;
    bool remove(const std::string& id) override;

private:
    std::string m_filePath;
    void save(const std::vector<Sample>& items) const;
};
