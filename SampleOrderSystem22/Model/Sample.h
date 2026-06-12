#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;  // min/ea
    double      yieldRate         = 0.0; // 0.0 ~ 1.0
    int         stock             = 0;

    std::string getId() const { return id; }
    std::string toJsonObject() const;
    static Sample fromJsonObject(const std::string& json);
};
