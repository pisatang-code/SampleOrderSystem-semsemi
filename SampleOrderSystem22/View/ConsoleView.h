#pragma once
#include "IView.h"
#include <string>

class ConsoleView : public IView {
public:
    struct Summary {
        int sampleCount = 0;
        int totalStock  = 0;
        int orderCount  = 0;
        int queueCount  = 0;
    };

    void setSummary(const Summary& summary);
    void render()                              override;
    void showMessage(const std::string& msg)   override;
    int  getInput()                            override;

    std::string getString(const std::string& prompt);
    int         getInt(const std::string& prompt);
    double      getDouble(const std::string& prompt);
    void        pressEnterToContinue();
    void        clearScreen();
    void        showSeparator(int width = 42);
    void        showHeader(const std::string& title);

    // 한글(2칸) 포함 문자열을 col 디스플레이 너비에 맞게 우측 패딩
    static int         displayWidth(const std::string& s);
    static std::string padRight(const std::string& s, int col);

private:
    Summary m_summary;
};
