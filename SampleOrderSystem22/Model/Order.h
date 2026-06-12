#pragma once
#include <string>
#include "OrderStatus.h"

struct Order {
    std::string orderNumber;
    std::string customerName;
    std::string sampleId;
    int         quantity   = 0;
    OrderStatus status     = OrderStatus::RESERVED;
    std::string approvedAt;  // PRODUCING 전환 시 epoch(초) 문자열, 미설정 시 ""

    std::string getId() const { return orderNumber; }
    std::string toJsonObject() const;
    static Order fromJsonObject(const std::string& json);
};
