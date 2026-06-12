#include "ProductionController.h"
#include <cmath>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iomanip>

ProductionController::ProductionController(std::shared_ptr<SampleStorage> sampleStorage,
                                           std::shared_ptr<OrderStorage>  orderStorage)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage) {}

// ── static 헬퍼 ──────────────────────────────────────────

int ProductionController::calcActualProduction(int shortage, double yieldRate) {
    return static_cast<int>(std::ceil(shortage / (yieldRate * 0.9)));
}

int ProductionController::calcTotalTimeMinutes(int actualProduction, int avgProductionTime) {
    return actualProduction * avgProductionTime;
}

long long ProductionController::parseEpoch(const std::string& s) {
    if (s.empty()) return 0;
    try { return std::stoll(s); }
    catch (...) { return 0; }
}

std::string ProductionController::epochToHHMM(long long epoch) {
    time_t t = static_cast<time_t>(epoch);
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << tm_buf.tm_hour
       << ":" << std::setw(2) << tm_buf.tm_min;
    return ss.str();
}

// ── 큐 조회 ─────────────────────────────────────────────

std::vector<Order> ProductionController::getQueue() const {
    std::vector<Order> queue;
    for (const auto& o : m_orderStorage->readAll())
        if (o.status == OrderStatus::PRODUCING) queue.push_back(o);
    return queue;
}

std::vector<ProductionController::ProductionInfo>
ProductionController::getQueueInfo() const {
    std::vector<ProductionInfo> result;
    long long now = static_cast<long long>(time(nullptr));

    for (const auto& o : m_orderStorage->readAll()) {
        if (o.status != OrderStatus::PRODUCING) continue;

        auto sampleOpt = m_sampleStorage->readById(o.sampleId);
        if (!sampleOpt) continue;

        int shortage = std::max(0, o.quantity - sampleOpt->stock);
        int actual   = calcActualProduction(shortage, sampleOpt->yieldRate);
        int totalMin = calcTotalTimeMinutes(actual, sampleOpt->avgProductionTime);

        ProductionInfo info;
        info.order           = o;
        info.sample          = *sampleOpt;
        info.actualProduction = actual;
        info.totalTimeMinutes = totalMin;

        long long startEpoch = parseEpoch(o.approvedAt);
        if (startEpoch > 0 && totalMin > 0) {
            info.hasTimer = true;
            long long elapsedSec  = now - startEpoch;
            long long totalSec    = static_cast<long long>(totalMin) * 60;
            info.elapsedMinutes   = static_cast<int>(elapsedSec / 60);
            info.progressPercent  = static_cast<int>(
                std::min(100LL, elapsedSec * 100LL / totalSec));
            info.isComplete       = (elapsedSec >= totalSec);
            info.etaStr           = epochToHHMM(startEpoch + totalSec);
        } else {
            info.hasTimer        = false;
            info.progressPercent = 0;
            info.etaStr          = "--:--";
        }

        result.push_back(info);
    }
    return result;
}

// ── 생산 완료 ────────────────────────────────────────────

void ProductionController::completeProduction(const std::string& orderNumber) {
    auto orderOpt = m_orderStorage->readById(orderNumber);
    if (!orderOpt.has_value())
        throw std::runtime_error("Order not found: " + orderNumber);
    if (orderOpt->status != OrderStatus::PRODUCING)
        throw std::runtime_error("Only PRODUCING orders can be completed");

    auto sampleOpt = m_sampleStorage->readById(orderOpt->sampleId);
    if (!sampleOpt.has_value())
        throw std::runtime_error("Sample not found: " + orderOpt->sampleId);

    Order  order  = *orderOpt;
    Sample sample = *sampleOpt;

    int shortage         = std::max(0, order.quantity - sample.stock);
    int actualProduction = calcActualProduction(shortage, sample.yieldRate);
    sample.stock += actualProduction;
    sample.stock -= order.quantity;

    order.status     = OrderStatus::CONFIRMED;
    order.approvedAt = "";  // 완료 후 초기화

    m_sampleStorage->update(sample);
    m_orderStorage->update(order);
}

// ── 자동 완료 ────────────────────────────────────────────

void ProductionController::autoComplete() {
    long long now = static_cast<long long>(time(nullptr));

    // approvedAt이 없는 PRODUCING 주문에 현재 시각을 기록 (마이그레이션)
    for (const auto& o : m_orderStorage->readAll()) {
        if (o.status == OrderStatus::PRODUCING && o.approvedAt.empty()) {
            Order updated     = o;
            updated.approvedAt = std::to_string(now);
            m_orderStorage->update(updated);
        }
    }

    // 총 생산 시간을 초과한 주문 자동 완료
    for (const auto& info : getQueueInfo()) {
        if (info.hasTimer && info.isComplete) {
            try { completeProduction(info.order.orderNumber); }
            catch (...) {}
        }
    }
}
