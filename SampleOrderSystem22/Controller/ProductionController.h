#pragma once
#include "../Model/SampleStorage.h"
#include "../Model/OrderStorage.h"
#include <memory>
#include <vector>
#include <string>

class ProductionController {
public:
    struct ProductionInfo {
        Order       order;
        Sample      sample;
        int         actualProduction = 0;
        int         totalTimeMinutes = 0;
        int         elapsedMinutes   = 0;
        int         progressPercent  = 0;   // 0~100
        bool        isComplete       = false;
        std::string etaStr;                 // 완료 예정 시각 "HH:MM", 미설정 시 "--:--"
        bool        hasTimer         = false; // approvedAt이 설정된 경우
    };

    ProductionController(std::shared_ptr<SampleStorage> sampleStorage,
                         std::shared_ptr<OrderStorage>  orderStorage);

    static int calcActualProduction(int shortage, double yieldRate);
    static int calcTotalTimeMinutes(int actualProduction, int avgProductionTime);

    std::vector<Order>          getQueue() const;
    std::vector<ProductionInfo> getQueueInfo() const;
    void                        completeProduction(const std::string& orderNumber);
    void                        autoComplete();  // 경과 시간이 총 시간을 넘은 주문 자동 완료

private:
    std::shared_ptr<SampleStorage> m_sampleStorage;
    std::shared_ptr<OrderStorage>  m_orderStorage;

    static long long    parseEpoch(const std::string& s);
    static std::string  epochToHHMM(long long epoch);
};
