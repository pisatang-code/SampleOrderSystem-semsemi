#include "MainController.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// ── 생성자 ──────────────────────────────────────────────

MainController::MainController(std::shared_ptr<SampleStorage> sampleStorage,
                               std::shared_ptr<OrderStorage>  orderStorage,
                               std::shared_ptr<ConsoleView>   view)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage)
    , m_view(view)
    , m_sampleCtrl(std::make_shared<SampleController>(sampleStorage))
    , m_orderCtrl(std::make_shared<OrderController>(sampleStorage, orderStorage))
    , m_productionCtrl(std::make_shared<ProductionController>(sampleStorage, orderStorage))
    , m_monitoringCtrl(std::make_shared<MonitoringController>(sampleStorage, orderStorage)) {}

// ── 메인 루프 ───────────────────────────────────────────

void MainController::run() {
    while (true) {
        m_view->setSummary(buildSummary());
        m_view->render();
        int choice = m_view->getInput();
        if (choice == 0) break;
        switch (choice) {
        case 1: handleSampleManagement(); break;
        case 2: handleOrderReception();   break;
        case 3: handleOrderApproval();    break;
        case 4: handleMonitoring();       break;
        case 5: handleProductionLine();   break;
        case 6: handleShipment();         break;
        default:
            m_view->showMessage("\xEC\x9E\x98\xEB\xAB\xBB\xEB\x90\x9C \xEC\x84\xA0\xED\x83\x9D\xEC\x9E\x85\xEB\x8B\x88\xEB\x8B\xA4.");
            m_view->pressEnterToContinue();
        }
    }
    m_view->showMessage("\xED\x94\x84\xEB\xA1\x9C\xEA\xB7\xB8\xEB\x9E\xA8\xEC\x9D\x84 \xEC\xA2\x85\xEB\xA3\x8C\xED\x95\xA9\xEB\x8B\x88\xEB\x8B\xA4.");
}

ConsoleView::Summary MainController::buildSummary() const {
    ConsoleView::Summary s;
    for (const auto& sample : m_sampleStorage->readAll()) {
        ++s.sampleCount;
        s.totalStock += sample.stock;
    }
    for (const auto& order : m_orderStorage->readAll()) {
        if (order.status != OrderStatus::REJECTED) ++s.orderCount;
        if (order.status == OrderStatus::PRODUCING) ++s.queueCount;
    }
    return s;
}

// ── 메뉴 1: 시료 관리 ───────────────────────────────────

void MainController::handleSampleManagement() {
    while (true) {
        m_view->clearScreen();
        m_view->showHeader("\xEC\x8B\x9C\xEB\xA3\x8C \xEA\xB4\x80\xEB\xA6\xAC");
        std::cout << " 1. \xEC\x8B\x9C\xEB\xA3\x8C \xEB\x93\xB1\xEB\xA1\x9D\n";
        std::cout << " 2. \xEC\x8B\x9C\xEB\xA3\x8C \xEC\xA1\xB0\xED\x9A\x8C\n";
        std::cout << " 3. \xEC\x8B\x9C\xEB\xA3\x8C \xEA\xB2\x80\xEC\x83\x89\n";
        std::cout << " 0. \xEB\x8F\x8C\xEC\x95\x84\xEA\xB0\x80\xEA\xB8\xB0\n";
        m_view->showSeparator();
        std::cout << "\xEC\x84\xA0\xED\x83\x9D: ";
        int choice = m_view->getInput();
        if (choice == 0) break;

        m_view->clearScreen();

        if (choice == 1) {
            // 시료 등록
            m_view->showHeader("\xEC\x8B\x9C\xEB\xA3\x8C \xEB\x93\xB1\xEB\xA1\x9D");
            std::string id   = m_view->getString("\xEC\x8B\x9C\xEB\xA3\x8C ID: ");
            std::string name = m_view->getString("\xEC\x8B\x9C\xEB\xA3\x8C\xEB\xAA\x85: ");
            int    apt = m_view->getInt("\xED\x8F\x89\xEA\xB7\x9C \xEC\x83\x9D\xEC\x82\xB0\xEC\x8B\x9C\xEA\xB0\x84(min/ea): ");
            double yr  = m_view->getDouble("\xEC\x88\x98\xEC\x9C\xA8(0.0~1.0): ");
            try {
                m_sampleCtrl->registerSample(id, name, apt, yr);
                std::cout << "\n[\xEB\x93\xB1\xEB\xA1\x9D \xEC\x99\x84\xEB\xA3\x8C] ID: " << id << "  \xEC\x8B\x9C\xEB\xA3\x8C\xEB\xAA\x85: " << name << "\n";
            } catch (const std::exception& e) {
                std::cout << "[\xEC\x98\xA4\xEB\xA5\x98] " << e.what() << "\n";
            }
        }
        else if (choice == 2) {
            // 시료 조회
            m_view->showHeader("\xEC\x8B\x9C\xEB\xA3\x8C \xEC\xA1\xB0\xED\x9A\x8C");
            auto samples = m_sampleCtrl->getAllSamples();
            if (samples.empty()) {
                std::cout << "\xEB\x93\xB1\xEB\xA1\x9D\xEB\x90\x9C \xEC\x8B\x9C\xEB\xA3\x8C\xEA\xB0\x80 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
            } else {
                std::cout << std::left
                          << std::setw(8)  << "ID"
                          << std::setw(18) << "\xEC\x9D\xB4\xEB\xA6\x84"
                          << std::setw(10) << "\xEC\x83\x9D\xEC\x82\xB0\xEC\x8B\x9C\xEA\xB0\x84"
                          << std::setw(8)  << "\xEC\x88\x98\xEC\x9C\xA8"
                          << std::setw(6)  << "\xEC\x9E\xAC\xEA\xB3\xA0"
                          << "\n";
                m_view->showSeparator(50);
                for (const auto& s : samples) {
                    std::ostringstream apt;
                    apt << s.avgProductionTime << "min";
                    std::cout << std::left
                              << std::setw(8)  << s.id
                              << std::setw(18) << s.name
                              << std::setw(10) << apt.str()
                              << std::setw(8)  << std::fixed << std::setprecision(2) << s.yieldRate
                              << std::setw(6)  << s.stock
                              << "\n";
                }
                std::cout << "  \xEC\xB4\x9D " << samples.size() << "\xEA\xB1\xB4\n";
            }
        }
        else if (choice == 3) {
            // 시료 검색
            m_view->showHeader("\xEC\x8B\x9C\xEB\xA3\x8C \xEA\xB2\x80\xEC\x83\x89");
            std::string kw = m_view->getString("\xEA\xB2\x80\xEC\x83\x89\xEC\x96\xB4: ");
            auto results = m_sampleCtrl->searchByName(kw);
            if (results.empty()) {
                std::cout << "\xEA\xB2\x80\xEC\x83\x89 \xEA\xB2\xB0\xEA\xB3\xBC\xEA\xB0\x80 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
            } else {
                std::cout << "\xEA\xB2\x80\xEC\x83\x89 \xEA\xB2\xB0\xEA\xB3\xBC " << results.size() << "\xEA\xB1\xB4:\n";
                m_view->showSeparator(50);
                for (const auto& s : results) {
                    std::cout << "  " << s.id << "  " << s.name
                              << "  " << s.avgProductionTime << "min"
                              << "  \xEC\x88\x98\xEC\x9C\xA8: " << std::fixed << std::setprecision(2) << s.yieldRate
                              << "  \xEC\x9E\xAC\xEA\xB3\xA0: " << s.stock << "\n";
                }
            }
        }
        m_view->pressEnterToContinue();
    }
}

// ── 메뉴 2: 시료 주문 ────────────────────────────────────

void MainController::handleOrderReception() {
    m_view->clearScreen();
    m_view->showHeader("\xEC\x8B\x9C\xEB\xA3\x8C \xEC\xA3\xBC\xEB\xAC\xB8");

    auto samples = m_sampleCtrl->getAllSamples();
    if (samples.empty()) {
        std::cout << "\xEB\x93\xB1\xEB\xA1\x9D\xEB\x90\x9C \xEC\x8B\x9C\xEB\xA3\x8C\xEA\xB0\x80 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
        m_view->pressEnterToContinue();
        return;
    }

    // 등록 시료 목록 표시
    std::cout << "[\xEB\x93\xB1\xEB\xA1\x9D\xEB\x90\x9C \xEC\x8B\x9C\xEB\xA3\x8C]\n";
    for (const auto& s : samples)
        std::cout << "  " << s.id << "  " << s.name << "  \xEC\x9E\xAC\xEA\xB3\xA0: " << s.stock << "\n";
    m_view->showSeparator();

    std::string sampleId    = m_view->getString("\xEC\x8B\x9C\xEB\xA3\x8C ID: ");
    std::string customerName = m_view->getString("\xEA\xB3\xA0\xEA\xB0\x9D\xEB\xAA\x85: ");
    int quantity             = m_view->getInt("\xEC\xA3\xBC\xEB\xAC\xB8 \xEC\x88\x98\xEB\x9F\x89: ");

    try {
        std::string orderNum = m_orderCtrl->receiveOrder(sampleId, customerName, quantity);
        std::cout << "\n[\xEC\xA3\xBC\xEB\xAC\xB8 \xEC\xA0\x91\xEC\x88\x98 \xEC\x99\x84\xEB\xA3\x8C]\n";
        std::cout << "  \xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8: " << orderNum << "\n";
        std::cout << "  \xEC\x83\x81\xED\x83\x9C: RESERVED\n";
    } catch (const std::exception& e) {
        std::cout << "[\xEC\x98\xA4\xEB\xA5\x98] " << e.what() << "\n";
    }
    m_view->pressEnterToContinue();
}

// ── 메뉴 3: 주문 승인/거절 ──────────────────────────────

void MainController::handleOrderApproval() {
    while (true) {
        m_view->clearScreen();
        m_view->showHeader("\xEC\xA3\xBC\xEB\xAC\xB8 \xEC\x8A\xB9\xEC\x9D\xB8/\xEA\xB1\xB0\xEC\xA0\x88");

        auto reserved = m_orderCtrl->getReservedOrders();
        if (reserved.empty()) {
            std::cout << "\xEC\xB2\x98\xEB\xA6\xAC\xED\x95\xA0 RESERVED \xEC\xA3\xBC\xEB\xAC\xB8\xEC\x9D\xB4 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
            m_view->pressEnterToContinue();
            return;
        }

        std::cout << std::left
                  << std::setw(4)  << "No."
                  << std::setw(20) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8"
                  << std::setw(14) << "\xEA\xB3\xA0\xEA\xB0\x9D\xEB\xAA\x85"
                  << std::setw(8)  << "\xEC\x8B\x9C\xEB\xA3\x8CID"
                  << std::setw(6)  << "\xEC\x88\x98\xEB\x9F\x89"
                  << "\n";
        m_view->showSeparator(52);
        for (size_t i = 0; i < reserved.size(); ++i) {
            const auto& o = reserved[i];
            std::cout << std::left
                      << std::setw(4)  << (i + 1)
                      << std::setw(20) << o.orderNumber
                      << std::setw(14) << o.customerName
                      << std::setw(8)  << o.sampleId
                      << std::setw(6)  << o.quantity
                      << "\n";
        }
        m_view->showSeparator(52);
        int sel = m_view->getInt("\xEC\xA3\xBC\xEB\xAC\xB8 \xEB\xB2\x88\xED\x98\xB8 \xEC\x84\xA0\xED\x83\x9D (0:\xEB\x8F\x8C\xEC\x95\x84\xEA\xB0\x80\xEA\xB8\xB0): ");
        if (sel == 0) break;
        if (sel < 1 || sel > static_cast<int>(reserved.size())) {
            m_view->showMessage("\xEC\x9E\x98\xEB\xAB\xBB\xEB\x90\x9C \xEC\x84\xA0\xED\x83\x9D\xEC\x9E\x85\xEB\x8B\x88\xEB\x8B\xA4.");
            m_view->pressEnterToContinue();
            continue;
        }

        const std::string& orderNum = reserved[sel - 1].orderNumber;
        std::cout << "\n 1. \xEC\x8A\xB9\xEC\x9D\xB8\n 2. \xEA\xB1\xB0\xEC\xA0\x88\n 0. \xEB\x8F\x8C\xEC\x95\x84\xEA\xB0\x80\xEA\xB8\xB0\n";
        int action = m_view->getInt("\xEC\x84\xA0\xED\x83\x9D: ");

        if (action == 1) {
            try {
                m_orderCtrl->approveOrder(orderNum);
                auto result = m_orderStorage->readById(orderNum);
                std::string statusStr = (result && result->status == OrderStatus::CONFIRMED)
                    ? "CONFIRMED (\xEC\x9E\xAC\xEA\xB3\xA0 \xEC\xB6\x9C\xEB\x8F\x99)"
                    : "PRODUCING (\xEC\x83\x9D\xEC\x82\xB0 \xEB\x93\xB1\xEB\xA1\x9D)";
                std::cout << "\n[\xEC\x8A\xB9\xEC\x9D\xB8 \xEC\x99\x84\xEB\xA3\x8C] " << orderNum << " -> " << statusStr << "\n";
            } catch (const std::exception& e) {
                std::cout << "[\xEC\x98\xA4\xEB\xA5\x98] " << e.what() << "\n";
            }
        } else if (action == 2) {
            try {
                m_orderCtrl->rejectOrder(orderNum);
                std::cout << "\n[\xEA\xB1\xB0\xEC\xA0\x88 \xEC\x99\x84\xEB\xA3\x8C] " << orderNum << " -> REJECTED\n";
            } catch (const std::exception& e) {
                std::cout << "[\xEC\x98\xA4\xEB\xA5\x98] " << e.what() << "\n";
            }
        }
        m_view->pressEnterToContinue();
    }
}

// ── 메뉴 4: 모니터링 ────────────────────────────────────

void MainController::handleMonitoring() {
    m_view->clearScreen();
    m_view->showHeader("\xEB\xAA\xA8\xEB\x8B\x88\xED\x84\xB0\xEB\xA7\x81");

    auto counts  = m_monitoringCtrl->getOrderCountByStatus();
    auto stocks  = m_monitoringCtrl->getStockStatuses();

    // 주문 현황
    std::cout << "[\xEC\xA3\xBC\xEB\xAC\xB8 \xED\x98\x84\xED\x99\xA9]\n";
    std::cout << "  RESERVED : " << counts.reserved  << "\xEA\xB1\xB4\n";
    std::cout << "  PRODUCING: " << counts.producing << "\xEA\xB1\xB4\n";
    std::cout << "  CONFIRMED: " << counts.confirmed << "\xEA\xB1\xB4\n";
    std::cout << "  RELEASE  : " << counts.released  << "\xEA\xB1\xB4\n";
    m_view->showSeparator();

    // 재고 현황
    std::cout << "[\xEC\x9E\xAC\xEA\xB3\xA0 \xED\x98\x84\xED\x99\xA9]\n";
    std::cout << std::left
              << std::setw(8)  << "\xEC\x8B\x9C\xEB\xA3\x8CID"
              << std::setw(18) << "\xEC\x9D\xB4\xEB\xA6\x84"
              << std::setw(8)  << "\xEC\x9E\xAC\xEA\xB3\xA0"
              << std::setw(10) << "\xEB\x8C\x80\xEA\xB8\xB0\xEB\x9F\xAC\n";
    m_view->showSeparator(44);
    for (const auto& info : stocks) {
        std::string statusLabel;
        switch (info.status) {
        case MonitoringController::StockStatus::YUYU:  statusLabel = "\xEC\x97\xAC\xEC\x9C\xA0"; break;
        case MonitoringController::StockStatus::BUJOK: statusLabel = "\xEB\xB6\x80\xEC\xA1\xB1"; break;
        case MonitoringController::StockStatus::GOGAL: statusLabel = "\xEA\xB3\xA0\xEA\xB0\x88"; break;
        }
        std::cout << std::left
                  << std::setw(8)  << info.sample.id
                  << std::setw(18) << info.sample.name
                  << std::setw(8)  << info.sample.stock
                  << statusLabel   << "\n";
    }

    m_view->pressEnterToContinue();
}

// ── 메뉴 5: 생산라인 조회 ───────────────────────────────

void MainController::handleProductionLine() {
    while (true) {
        m_view->clearScreen();
        m_view->showHeader("\xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8 \xEC\xA1\xB0\xED\x9A\x8C");

        auto queue = m_productionCtrl->getQueue();
        if (queue.empty()) {
            std::cout << "\xEC\x83\x9D\xEC\x82\xB0 \xEB\x8C\x80\xEA\xB8\xB0 \xEC\xA3\xBC\xEB\xAC\xB8\xEC\x9D\xB4 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
            m_view->pressEnterToContinue();
            return;
        }

        std::cout << std::left
                  << std::setw(4)  << "No."
                  << std::setw(20) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8"
                  << std::setw(8)  << "\xEC\x8B\x9C\xEB\xA3\x8CID"
                  << std::setw(8)  << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\x9F\xAC"
                  << std::setw(10) << "\xEC\x8B\xA4\xEC\x83\x9D\xEC\x82\xB0\xEB\x9F\xAC"
                  << "\xEC\xB4\x9D\xEC\x8B\x9C\xEA\xB0\x84\n";
        m_view->showSeparator(54);

        for (size_t i = 0; i < queue.size(); ++i) {
            const auto& o = queue[i];
            auto sampleOpt = m_sampleStorage->readById(o.sampleId);
            if (!sampleOpt) continue;
            int actual = ProductionController::calcActualProduction(o.quantity, sampleOpt->yieldRate);
            int totalTime = sampleOpt->avgProductionTime * actual;
            std::cout << std::left
                      << std::setw(4)  << (i + 1)
                      << std::setw(20) << o.orderNumber
                      << std::setw(8)  << o.sampleId
                      << std::setw(8)  << o.quantity
                      << std::setw(10) << actual
                      << totalTime << "min\n";
        }
        m_view->showSeparator(54);

        std::string orderNum = m_view->getString(
            "\xEC\x83\x9D\xEC\x82\xB0 \xEC\x99\x84\xEB\xA3\x8C \xEC\xB2\x98\xEB\xA6\xAC\xED\x95\xA0 \xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8 (0:\xEB\x8F\x8C\xEC\x95\x84\xEA\xB0\x80\xEA\xB8\xB0): ");
        if (orderNum == "0") break;

        try {
            m_productionCtrl->completeProduction(orderNum);
            std::cout << "\n[\xEC\x83\x9D\xEC\x82\xB0 \xEC\x99\x84\xEB\xA3\x8C] " << orderNum << " -> CONFIRMED\n";
        } catch (const std::exception& e) {
            std::cout << "[\xEC\x98\xA4\xEB\xA5\x98] " << e.what() << "\n";
        }
        m_view->pressEnterToContinue();
    }
}

// ── 메뉴 6: 출고 처리 ───────────────────────────────────

void MainController::handleShipment() {
    while (true) {
        m_view->clearScreen();
        m_view->showHeader("\xEC\xB6\x9C\xEA\xB3\xA0 \xEC\xB2\x98\xEB\xA6\xAC");

        auto confirmed = m_monitoringCtrl->getConfirmedOrders();
        if (confirmed.empty()) {
            std::cout << "\xEC\xB6\x9C\xEA\xB3\xA0 \xEB\x8C\x80\xEA\xB8\xB0 CONFIRMED \xEC\xA3\xBC\xEB\xAC\xB8\xEC\x9D\xB4 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B\x88\xEB\x8B\xA4.\n";
            m_view->pressEnterToContinue();
            return;
        }

        std::cout << std::left
                  << std::setw(4)  << "No."
                  << std::setw(20) << "\xEC\xA3\xBC\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8"
                  << std::setw(14) << "\xEA\xB3\xA0\xEA\xB0\x9D\xEB\xAC\xB8\xEB\xB2\x88\xED\x98\xB8"
                  << std::setw(8)  << "\xEC\x8B\x9C\xEB\xA3\x8CID"
                  << std::setw(6)  << "\xEC\x88\x98\xEB\x9F\xAC\n";
        m_view->showSeparator(52);
        for (size_t i = 0; i < confirmed.size(); ++i) {
            const auto& o = confirmed[i];
            std::cout << std::left
                      << std::setw(4)  << (i + 1)
                      << std::setw(20) << o.orderNumber
                      << std::setw(14) << o.customerName
                      << std::setw(8)  << o.sampleId
                      << std::setw(6)  << o.quantity << "\n";
        }
        m_view->showSeparator(52);

        int sel = m_view->getInt("\xEC\xA3\xBC\xEB\xAC\xB8 \xEB\xB2\x88\xED\x98\xB8 \xEC\x84\xA0\xED\x83\x9D (0:\xEB\x8F\x8C\xEC\x95\x84\xEA\xB0\x80\xEA\xB8\xB0): ");
        if (sel == 0) break;
        if (sel < 1 || sel > static_cast<int>(confirmed.size())) {
            m_view->showMessage("\xEC\x9E\x98\xEB\xAB\xBB\xEB\x90\x9C \xEC\x84\xA0\xED\x83\x9D\xEC\x9E\x85\xEB\x8B\x88\xEB\x8B\xA4.");
            m_view->pressEnterToContinue();
            continue;
        }

        const std::string& orderNum = confirmed[sel - 1].orderNumber;
        try {
            m_monitoringCtrl->shipOrder(orderNum);
            std::cout << "\n[\xEC\xB6\x9C\xEA\xB3\xA0 \xEC\x99\x84\xEB\xA3\x8C] " << orderNum << " -> RELEASE\n";
        } catch (const std::exception& e) {
            std::cout << "[\xEC\x98\xA4\xEB\xA5\x98] " << e.what() << "\n";
        }
        m_view->pressEnterToContinue();
    }
}
