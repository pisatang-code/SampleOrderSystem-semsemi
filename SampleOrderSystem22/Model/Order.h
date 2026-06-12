#pragma once
#include <string>
#include "OrderStatus.h"

struct Order {
    std::string orderNumber;
    std::string customerName;
    std::string sampleId;
    int         quantity = 0;
    OrderStatus status   = OrderStatus::RESERVED;

    std::string getId() const { return orderNumber; }
    std::string toJsonObject() const;
    static Order fromJsonObject(const std::string& json);
};
