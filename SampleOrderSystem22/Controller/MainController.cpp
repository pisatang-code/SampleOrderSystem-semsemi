#include "MainController.h"
#include <iostream>
#include <limits>

MainController::MainController(std::shared_ptr<SampleStorage> sampleStorage,
                               std::shared_ptr<OrderStorage>  orderStorage,
                               std::shared_ptr<ConsoleView>   view)
    : m_sampleStorage(sampleStorage)
    , m_orderStorage(orderStorage)
    , m_view(view) {}

void MainController::run() {
    while (true) {
        m_view->setSummary(buildSummary());
        m_view->render();
        int choice = m_view->getInput();
        if (choice == 0) break;
        handleMenu(choice);
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

void MainController::handleMenu(int choice) {
    switch (choice) {
    case 1: m_view->showMessage("\xEC\x8B\x9C\xEB\xA3\x8C \xEA\xB4\x80\xEB\xA6\xAC - \xEC\xA4\x80\xEB\xB9\x84 \xEC\xA4\x91"); break;
    case 2: m_view->showMessage("\xEC\x8B\x9C\xEB\xA3\x8C \xEC\xA3\xBC\xEB\xAC\xB8 - \xEC\xA4\x80\xEB\xB9\x84 \xEC\xA4\x91"); break;
    case 3: m_view->showMessage("\xEC\xA3\xBC\xEB\xAC\xB8 \xEC\x8A\xB9\xEC\x9D\xB8/\xEA\xB1\xB0\xEC\xA0\x88 - \xEC\xA4\x80\xEB\xB9\x84 \xEC\xA4\x91"); break;
    case 4: m_view->showMessage("\xEB\xAA\xA8\xEB\x8B\x88\xED\x84\xB0\xEB\xA7\x81 - \xEC\xA4\x80\xEB\xB9\x84 \xEC\xA4\x91"); break;
    case 5: m_view->showMessage("\xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8 \xEC\xA1\xB0\xED\x9A\x8C - \xEC\xA4\x80\xEB\xB9\x84 \xEC\xA4\x91"); break;
    case 6: m_view->showMessage("\xEC\xB6\x9C\xEA\xB3\xA0 \xEC\xB2\x98\xEB\xA6\xAC - \xEC\xA4\x80\xEB\xB9\x84 \xEC\xA4\x91"); break;
    default: m_view->showMessage("\xEC\x9E\x98\xEB\xAA\xBB\xEB\x90\x9C \xEC\x84\xA0\xED\x83\x9D\xEC\x9E\x85\xEB\x8B\x88\xEB\x8B\xA4."); break;
    }
    std::cout << "\xEA\xB3\x84\xEC\x86\x8D\xED\x95\x98\xEB\xA0\xA4\xEB\xA9\xB4 Enter\xEB\xA5\xBC \xEB\x88\x84\xEB\xA5\xB4\xEC\x84\xB8\xEC\x9A\x94...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}
