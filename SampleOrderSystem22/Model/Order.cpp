#include "Order.h"
#include "JsonUtil.h"
#include <sstream>

std::string Order::toJsonObject() const {
    std::ostringstream ss;
    ss << "{"
       << "\"orderNumber\":\""  << JsonUtil::escapeJson(orderNumber)  << "\","
       << "\"customerName\":\"" << JsonUtil::escapeJson(customerName) << "\","
       << "\"sampleId\":\""     << JsonUtil::escapeJson(sampleId)     << "\","
       << "\"quantity\":"       << quantity                            << ","
       << "\"status\":\""       << orderStatusToString(status)        << "\""
       << "}";
    return ss.str();
}

Order Order::fromJsonObject(const std::string& json) {
    Order o;
    o.orderNumber  = JsonUtil::getValue(json, "orderNumber");
    o.customerName = JsonUtil::getValue(json, "customerName");
    o.sampleId     = JsonUtil::getValue(json, "sampleId");
    auto qty = JsonUtil::getValue(json, "quantity");
    if (!qty.empty()) o.quantity = std::stoi(qty);
    auto st = JsonUtil::getValue(json, "status");
    if (!st.empty())  o.status = orderStatusFromString(st);
    return o;
}
