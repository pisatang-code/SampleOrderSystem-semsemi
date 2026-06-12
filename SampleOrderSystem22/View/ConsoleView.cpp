#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "ConsoleView.h"
#include <iostream>
#include <limits>

void ConsoleView::setSummary(const Summary& summary) {
    m_summary = summary;
}

void ConsoleView::render() {
    system("cls");
    std::cout << "========================================\n";
    std::cout << "   \xEB\xB0\x98\xEB\x8F\x84\xEC\xB2\xB4 \xEC\x8B\x9C\xEB\xA3\x8C \xEC\x83\x9D\xEC\x82\xB0\xEC\xA3\xBC\xEB\xAC\xB8\xEA\xB4\x80\xEB\xA6\xAC \xEC\x8B\x9C\xEC\x8A\xA4\xED\x85\x9C\n";
    std::cout << "========================================\n";
    std::cout << " \xEB\x93\xB1\xEB\xA1\x9D \xEC\x8B\x9C\xEB\xA3\x8C: " << m_summary.sampleCount
              << "  \xEC\xB4\x9D \xEC\x9E\xAC\xEA\xB3\xA0: "             << m_summary.totalStock
              << "  \xEC\xB4\x9D \xEC\xA3\xBC\xEB\xAC\xB8: "             << m_summary.orderCount
              << "  \xEC\x83\x9D\xEC\x82\xB0 \xEB\x8C\x80\xEA\xB8\xB0: " << m_summary.queueCount << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << " 1. \xEC\x8B\x9C\xEB\xA3\x8C \xEA\xB4\x80\xEB\xA6\xAC\n";
    std::cout << " 2. \xEC\x8B\x9C\xEB\xA3\x8C \xEC\xA3\xBC\xEB\xAC\xB8\n";
    std::cout << " 3. \xEC\xA3\xBC\xEB\xAC\xB8 \xEC\x8A\xB9\xEC\x9D\xB8/\xEA\xB1\xB0\xEC\xA0\x88\n";
    std::cout << " 4. \xEB\xAA\xA8\xEB\x8B\x88\xED\x84\xB0\xEB\xA7\x81\n";
    std::cout << " 5. \xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8 \xEC\xA1\xB0\xED\x9A\x8C\n";
    std::cout << " 6. \xEC\xB6\x9C\xEA\xB3\xA0 \xEC\xB2\x98\xEB\xA6\xAC\n";
    std::cout << " 0. \xEC\xA2\xBD\xEB\xA3\x8C\n";
    std::cout << "========================================\n";
    std::cout << "\xEC\x84\xA0\xED\x83\x9D: ";
    std::cout.flush();
}

void ConsoleView::showMessage(const std::string& msg) {
    std::cout << "[\xEC\x95\x8C\xEB\xA6\xBC] " << msg << "\n";
}

int ConsoleView::getInput() {
    int choice = -1;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = -1;
    }
    return choice;
}
