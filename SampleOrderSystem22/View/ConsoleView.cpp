#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "ConsoleView.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

int ConsoleView::displayWidth(const std::string& s) {
    int w = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if      (c < 0x80) { w += 1; i += 1; }
        else if (c < 0xE0) { w += 1; i += 2; }
        else if (c < 0xF0) { w += 2; i += 3; }  // 3-byte: 한글/CJK 2칸
        else               { w += 2; i += 4; }
    }
    return w;
}

std::string ConsoleView::padRight(const std::string& s, int col) {
    int pad = col - displayWidth(s);
    return s + (pad > 0 ? std::string(pad, ' ') : "");
}

void ConsoleView::clearScreen() {
    system("cls");
}

void ConsoleView::showSeparator(int width) {
    std::cout << std::string(width, '-') << "\n";
}

void ConsoleView::showHeader(const std::string& title) {
    std::cout << std::string(42, '=') << "\n";
    std::cout << " [" << title << "]\n";
    std::cout << std::string(42, '=') << "\n";
}

void ConsoleView::setSummary(const Summary& summary) {
    m_summary = summary;
}

void ConsoleView::render() {
    clearScreen();
    std::cout << std::string(42, '=') << "\n";
    std::cout << "   \xEB\xB0\x98\xEB\x8F\x84\xEC\xB2\xB4 \xEC\x8B\x9C\xEB\xA3\x8C \xEC\x83\x9D\xEC\x82\xB0\xEC\xA3\xBC\xEB\xAC\xB8\xEA\xB4\x80\xEB\xA6\xAC \xEC\x8B\x9C\xEC\x8A\xA4\xED\x85\x9C\n";
    std::cout << std::string(42, '=') << "\n";
    std::cout << " \xEB\x93\xB1\xEB\xA1\x9D \xEC\x8B\x9C\xEB\xA3\x8C: " << m_summary.sampleCount
              << "  \xEC\xB4\x9D \xEC\x9E\xAC\xEA\xB3\xA0: "             << m_summary.totalStock
              << "  \xEC\xB4\x9D \xEC\xA3\xBC\xEB\xAC\xB8: "             << m_summary.orderCount
              << "  \xEC\x83\x9D\xEC\x82\xB0 \xEB\x8C\x80\xEA\xB8\xB0: " << m_summary.queueCount << "\n";
    std::cout << std::string(42, '-') << "\n";
    std::cout << " 1. \xEC\x8B\x9C\xEB\xA3\x8C \xEA\xB4\x80\xEB\xA6\xAC\n";
    std::cout << " 2. \xEC\x8B\x9C\xEB\xA3\x8C \xEC\xA3\xBC\xEB\xAC\xB8\n";
    std::cout << " 3. \xEC\xA3\xBC\xEB\xAC\xB8 \xEC\x8A\xB9\xEC\x9D\xB8/\xEA\xB1\xB0\xEC\xA0\x88\n";
    std::cout << " 4. \xEB\xAA\xA8\xEB\x8B\x88\xED\x84\xB0\xEB\xA7\x81\n";
    std::cout << " 5. \xEC\x83\x9D\xEC\x82\xB0\xEB\x9D\xBC\xEC\x9D\xB8 \xEC\xA1\xB0\xED\x9A\x8C\n";
    std::cout << " 6. \xEC\xB6\x9C\xEA\xB3\xA0 \xEC\xB2\x98\xEB\xA6\xAC\n";
    std::cout << " 0. \xEC\xA2\x85\xEB\xA3\x8C\n";
    std::cout << std::string(42, '=') << "\n";
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
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

std::string ConsoleView::getString(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();
    std::string value;
    std::getline(std::cin, value);
    return value;
}

int ConsoleView::getInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::cout.flush();
        int value;
        std::cin >> value;
        if (!std::cin.fail()) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [\xEC\x9E\x85\xEB\xA0\xA5 \xEC\x98\xA4\xEB\xA5\x98] \xEC\x88\xAB\xEC\x9E\x90\xEB\xA5\xBC \xEC\x9E\x85\xEB\xA0\xA5\xED\x95\x98\xEC\x84\xB8\xEC\x9A\x94.\n";
    }
}

double ConsoleView::getDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::cout.flush();
        double value;
        std::cin >> value;
        if (!std::cin.fail()) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [\xEC\x9E\x85\xEB\xA0\xA5 \xEC\x98\xA4\xEB\xA5\x98] \xEC\x88\xAB\xEC\x9E\x90\xEB\xA5\xBC \xEC\x9E\x85\xEB\xA0\xA5\xED\x95\x98\xEC\x84\xB8\xEC\x9A\x94.\n";
    }
}

void ConsoleView::pressEnterToContinue() {
    std::cout << "\n\xEA\xB3\x84\xEC\x86\x8D\xED\x95\x98\xEB\xA0\xA4\xEB\xA9\xB4 Enter\xEB\xA5\xBC \xEB\x88\x84\xEB\xA5\xB4\xEC\x84\xB8\xEC\x9A\x94...";
    std::cin.get();
}
